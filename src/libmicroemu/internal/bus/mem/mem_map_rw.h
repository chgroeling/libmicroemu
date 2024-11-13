#pragma once

#include "libmicroemu/internal/bus/mem/mem_traits.h"
#include "libmicroemu/internal/bus/mem_access_results.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"
#include <tuple>

namespace libmicroemu::internal {

template <typename TProcessorStates, typename TSpecRegOps, typename TPeripheral,
          typename TLogger = NullLogger>
class MemMapAccessPoint {
public:
  static_assert(has_kRegisters_v<TPeripheral>, "TPeripheral must have kRegisters");
  static_assert(std::is_const_v<decltype(TPeripheral::kRegisters)>,
                "kRegisters must be a constant");

  using SReg = TSpecRegOps;
  using MapEnum = typename TPeripheral::MapEnum;

  static constexpr u32 GetBeginPhysicalAddress() { return TPeripheral::GetBeginPhysicalAddress(); }
  static constexpr u32 GetEndPhysicalAddress() { return TPeripheral::GetEndPhysicalAddress(); }

  static constexpr auto kRegisters = TPeripheral::kRegisters;

  template <typename T>
  static ReadResult<T> ReadRegister(TProcessorStates &pstates, me_adr_t padr) {
    return ReadRegisterRecursive<T, decltype(kRegisters)>(padr, pstates);
  }

  template <typename T>
  static WriteResult<T> WriteRegister(TProcessorStates &pstates, me_adr_t padr, T value) {
    return WriteRegisterRecursive<T, decltype(kRegisters)>(padr, pstates, value);
  }

private:
  template <typename T, typename Registers, std::size_t Index = 0U>
  static ReadResult<T> ReadRegisterRecursive(const u32 &padr, TProcessorStates &pstates) {
    if constexpr (Index < std::tuple_size_v<Registers>) {
      using TypeRegAccess = std::tuple_element_t<Index, Registers>;

      auto constexpr pstart_adr = static_cast<me_adr_t>(TypeRegAccess::kAdr);
      auto constexpr pend_adr = static_cast<me_adr_t>(pstart_adr + sizeof(u32) - 1U);
      if (padr >= pstart_adr && padr <= pend_adr) {

        const auto read_32 = TypeRegAccess::ReadRegister(pstates);
        const me_adr_t start_byte = padr - pstart_adr;
        T read_t = Bm32::ExtractType<T>(read_32, start_byte);
        LOG_TRACE(TLogger, "READ: padr = 0x%X, width = %u, value = 0x%X", padr, sizeof(T), read_t);

        return ReadResult<T>{read_t, ReadStatusCode::kOk};
      } else {
        return ReadRegisterRecursive<T, Registers, Index + 1U>(padr, pstates);
      }
    }
    return ReadResult<T>{0x0U, ReadStatusCode::kReadNotAllowed};
  }

  template <
      typename TRegAccess, typename T,
      typename std::enable_if_t<!TRegAccess::kReadOnly && !TRegAccess::kUseReadModifyWrite, T> = 0>
  static WriteResult<T> PerformWrite(const u32 &padr, TProcessorStates &pstates, T value) {
    u32 write_value{0x0U};
    if constexpr (std::is_same_v<T, u32>) {
      write_value = value;
    } else {
      me_adr_t start_byte = padr - static_cast<me_adr_t>(TRegAccess::kAdr);
      write_value = Bm32::InsertType(0x0U, start_byte, value);
    }

    LOG_TRACE(TLogger, "WRITE (No read): padr = 0x%X, width = %u, value = 0x%X", padr, sizeof(T),
              write_value);

    TRegAccess::WriteRegister(pstates, write_value);

    return WriteResult<T>{WriteStatusCode::kOk};
  }

  template <
      typename TRegAccess, typename T,
      typename std::enable_if_t<!TRegAccess::kReadOnly && TRegAccess::kUseReadModifyWrite, T> = 0>
  static WriteResult<T> PerformWrite(const u32 &padr, TProcessorStates &pstates, T value) {
    u32 write_value{0x0U};

    if constexpr (std::is_same_v<T, u32>) {
      write_value = value;
    } else {
      me_adr_t start_byte = padr - static_cast<me_adr_t>(TRegAccess::kAdr);

      // read modify write
      const auto read_32 = TRegAccess::ReadRegister(pstates);
      write_value = Bm32::InsertType(read_32, start_byte, value);
    }

    LOG_TRACE(TLogger, "WRITE: padr = 0x%X, width = %u, value = 0x%X", padr, sizeof(T),
              write_value);

    TRegAccess::WriteRegister(pstates, write_value);

    return WriteResult<T>{WriteStatusCode::kOk};
  }

  template <
      typename TRegAccess, typename T,
      typename std::enable_if_t<TRegAccess::kReadOnly && !TRegAccess::kUseReadModifyWrite, T> = 0>
  static WriteResult<T> PerformWrite(const u32 &padr, TProcessorStates &pstates, T value) {
    static_cast<void>(padr);
    static_cast<void>(pstates);
    static_cast<void>(value);
    return WriteResult<T>{WriteStatusCode::kWriteNotAllowed};
  }

  template <typename T, typename Registers, std::size_t Index = 0U>
  static WriteResult<T> WriteRegisterRecursive(const u32 &padr, TProcessorStates &pstates,
                                               T value) {
    if constexpr (Index < std::tuple_size_v<Registers>) {
      using TypeRegAccess = std::tuple_element_t<Index, Registers>;

      auto constexpr pstart_adr = static_cast<me_adr_t>(TypeRegAccess::kAdr);
      auto constexpr pend_adr = static_cast<me_adr_t>(pstart_adr + sizeof(u32) - 1U);

      if (padr >= pstart_adr && padr <= pend_adr) {
        return PerformWrite<TypeRegAccess, T>(padr, pstates, value);
      } else {
        return WriteRegisterRecursive<T, Registers, Index + 1U>(padr, pstates, value);
      }
    }
    return WriteResult<T>{WriteStatusCode::kWriteNotAllowed};
  }
};

template <unsigned Id, unsigned VadrOffset, unsigned VadrRange, typename TProcessorStates,
          typename TSpecRegOps, typename TExceptionTrigger, typename TLogger,
          typename... TPeripherals>
class MemMapRw {
public:
  using SReg = TSpecRegOps;
  using ExcTrig = TExceptionTrigger;

  /**
   * @brief Constructor
   */
  MemMapRw() = default;

  /**
   * @brief Destructor
   */
  virtual ~MemMapRw() = default;

  /**
   * @brief Copy constructor for MemMapRw.
   * @param r_src the object to be copied
   */
  MemMapRw(const MemMapRw &r_src) = default;

  /**
   * @brief Copy assignment operator for MemMapRw.
   * @param r_src the object to be copied
   */
  MemMapRw &operator=(const MemMapRw &r_src) = default;

  /**
   * @brief Move constructor for MemMapRw.
   * @param r_src the object to be moved
   */
  MemMapRw(MemMapRw &&r_src) = default;

  /**
   * @brief Move assignment operator for  MemMapRw.
   * @param r_src the object to be moved
   */
  MemMapRw &operator=(MemMapRw &&r_src) = default;

  static constexpr bool kReadOnly = false;

  // -------------------------------------------------
  // Read from Peripheral List
  // -------------------------------------------------

  template <typename T, typename First, typename... Rest>
  static ReadResult<T> ReadRegisters(TProcessorStates &pstates, const u32 &padr) {
    using Type = MemMapAccessPoint<TProcessorStates, TSpecRegOps, First, TLogger>;

    if (padr >= Type::GetBeginPhysicalAddress() && padr <= Type::GetEndPhysicalAddress()) {
      const auto read_32 = Type::template ReadRegister<T>(pstates, padr);
      return read_32;
    } else {
      return ReadRegisters<T, Rest...>(pstates, padr);
    }
  }

  template <typename T>
  static ReadResult<T> ReadRegisters(TProcessorStates &pstates, const u32 &padr) {
    static_cast<void>(pstates);
    static_cast<void>(padr);
    return ReadResult<T>{0x0U, ReadStatusCode::kReadNotAllowed};
  }

  // -------------------------------------------------
  // Write from Peripheral List
  // -------------------------------------------------
  template <typename T, typename First, typename... Rest>
  static WriteResult<T> WriteRegisters(TProcessorStates &pstates, const u32 &padr, const T &value) {
    using Type = MemMapAccessPoint<TProcessorStates, TSpecRegOps, First, TLogger>;

    if (padr >= Type::GetBeginPhysicalAddress() && padr <= Type::GetEndPhysicalAddress()) {
      const auto write_32 = Type::template WriteRegister<T>(pstates, padr, value);
      return write_32;
    } else {
      return WriteRegisters<T, Rest...>(pstates, padr, value);
    }
  }

  template <typename T>
  static WriteResult<T> WriteRegisters(TProcessorStates &pstates, const u32 &padr, const T &value) {
    static_cast<void>(pstates);
    static_cast<void>(padr);
    static_cast<void>(value);
    return WriteResult<T>{WriteStatusCode::kWriteNotAllowed};
  }

  // -------------------------------------------------
  // API Methods
  // -------------------------------------------------
  template <typename T> ReadResult<T> Read(TProcessorStates &pstates, me_adr_t vadr) const {
    // clang-format off
    static_assert(
        std::is_same<T, u32>::value || 
        std::is_same<T, u16>::value ||
        std::is_same<T, u8>::value,  
        "Read only allows u32, u16 and u8");
    // clang-format on

    const me_adr_t padr = ConvertToPhysicalAdr(vadr);
    assert(IsPAdrInRange(padr) == true);
    auto read_res = ReadRegisters<T, TPeripherals...>(pstates, padr);
    return ReadResult<T>{static_cast<T>(read_res.content), read_res.status_code};
  }

  template <typename T>
  WriteResult<T> Write(TProcessorStates &pstates, me_adr_t vadr, const T &value) const {
    // clang-format off
    static_assert(
        std::is_same<T, u32>::value || 
        std::is_same<T, u16>::value ||
        std::is_same<T, u8>::value,  
         "Write only allows u32, u16 and u8 types");
    // clang-format on

    const me_adr_t padr = ConvertToPhysicalAdr(vadr);
    assert(IsPAdrInRange(padr) == true);

    auto write_res = WriteRegisters<T, TPeripherals...>(pstates, padr, value);

    return WriteResult<T>{write_res.status_code};
  }

  bool IsVAdrInRange(me_adr_t vadr) const {
    const me_adr_t padr = ConvertToPhysicalAdr(vadr);
    if (IsPAdrInRange(padr) == false) {
      return false;
    }
    return true;
  }

private:
  me_adr_t ConvertToPhysicalAdr(me_adr_t vadr) const { return vadr - VadrOffset; }
  bool IsPAdrInRange(me_adr_t padr) const { return padr < VadrRange; }
};

} // namespace libmicroemu::internal
