#pragma once

#include "libmicroemu/internal/bus/mem_access_results.h"
#include "libmicroemu/types.h"
#include <cassert>
#include <type_traits>

namespace microemu::internal {

/// Optional assignable memory class.
/// This class represent an memory area which can be read and written to. The memory area is
/// represented by a buffer and a size. If the buffer is nullptr or the size is 0, the memory
/// behaves as if it is not present.
/// \tparam Id the id of the memory
/// \tparam TEndianessC the endianess converter

template <unsigned Id, typename TProcessorStates, typename TEndianessC> class MemRwOptional {
public:
  static constexpr bool kReadOnly = false;

  /// \brief Constructor
  explicit MemRwOptional(u8 *const buf, const me_size_t buf_size, const me_adr_t vadr_offset)
      : buf_(buf), vadr_offset_(vadr_offset), buf_size_(buf_size) {}

  /// \brief Destructor
  virtual ~MemRwOptional() = default;

  /// \brief Copy constructor for MemOpt.
  /// \param r_src the object to be copied
  MemRwOptional(const MemRwOptional &r_src) = default;

  /// \brief Copy assignment operator for MemOpt.
  /// \param r_src the object to be copied
  MemRwOptional &operator=(const MemRwOptional &r_src) = default;

  /// \brief Move constructor for MemOpt.
  /// \param r_src the object to be copied
  MemRwOptional(MemRwOptional &&r_src) = default;

  /// \brief Move assignment operator for MemOpt.
  /// \param r_src the object to be copied
  MemRwOptional &operator=(MemRwOptional &&r_src) = default;

  template <typename T> ReadResult<T> Read(TProcessorStates &pstates, me_adr_t vadr) const {
    // clang-format off
    static_assert(
        std::is_same<T, u32>::value || 
        std::is_same<T, u16>::value ||
        std::is_same<T, u8>::value,  
        "Read only allows u32, u16 and u8");
    // clang-format on
    static_cast<void>(pstates);

    assert(buf_ != nullptr && buf_size_ != 0U);

    const me_adr_t padr = ConvertToPhysicalAdr(vadr);
    assert(IsPAdrInRange(padr) == true);
    T val = *reinterpret_cast<const T *>(&buf_[padr]);
    T cval = TEndianessC::template Convert<T>(val);

    return ReadResult<T>{cval, ReadStatusCode::kOk};
  }

  template <typename T>
  WriteResult<T> Write(TProcessorStates &pstates, me_adr_t vadr, T value) const {
    // clang-format off
    static_assert(
        std::is_same<T, u32>::value || 
        std::is_same<T, u16>::value ||
        std::is_same<T, u8>::value,  
         "Write only allows u32, u16 and u8 types");
    // clang-format on
    static_cast<void>(pstates);

    assert(buf_ != nullptr && buf_size_ != 0U);
    const me_adr_t padr = ConvertToPhysicalAdr(vadr);
    assert(IsPAdrInRange(padr) == true);

    *reinterpret_cast<T *const>(&buf_[padr]) = value;

    return WriteResult<T>{WriteStatusCode::kOk};
  }

  bool IsVAdrInRange(me_adr_t vadr) const {
    // if no buffer was assigned always return that this memory has to valid access range
    if (buf_ == nullptr || buf_size_ == 0U) {
      return false;
    }

    const me_adr_t padr = ConvertToPhysicalAdr(vadr);
    if (IsPAdrInRange(padr) == false) {
      return false;
    }
    return true;
  }

private:
  u8 *const buf_{nullptr};
  const me_adr_t vadr_offset_{0U};
  const me_size_t buf_size_{0U};

  me_adr_t ConvertToPhysicalAdr(me_adr_t vadr) const { return vadr - vadr_offset_; }
  bool IsPAdrInRange(me_adr_t padr) const { return padr < buf_size_; }
};

} // namespace microemu::internal
