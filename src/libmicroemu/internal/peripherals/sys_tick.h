#pragma once

#include "libmicroemu/internal/bus/mem_access_results.h"
#include "libmicroemu/internal/logic/exceptions_ops.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/types.h"

namespace libmicroemu ::internal {

enum class SysTickAddressMap : me_adr_t {
  kCsr = 0xE010U,
  kRvr = 0xE014U,
  kCvr = 0xE018U,
  kCalib = 0xE01CU
};

template <typename TCpuAccessor, typename TExceptionTrigger, typename TLogger = NullLogger>
class SysTick {
public:
  using ExcTrig = TExceptionTrigger;
  using MapEnum = SysTickAddressMap;

  static constexpr u32 GetBeginPhysicalAddress() { return 0xE010; }
  static constexpr u32 GetEndPhysicalAddress() { return 0xE0FF; }

  class RegisterAccessCsr {
  public:
    static constexpr auto kAdr = SysTickAddressMap::kCsr;
    static constexpr bool kUseReadModifyWrite = false; // Perform read before write
    static constexpr bool kReadOnly = false;           // Disable write operation

    static u32 ReadRegister(TCpuAccessor &cpua) {
      auto ret = cpua.template ReadRegister<SpecialRegisterId::kSysTickCsr>();
      LOG_TRACE(TLogger, "READ SYST_CSR: 0x%X", ret);
      // clear the count flag on each read
      auto csr_wb = ret & (~SysTickRegister::kCsrCountFlagMsk);
      cpua.template WriteRegister<SpecialRegisterId::kSysTickCsr>(csr_wb);

      return ret;
    }

    static void WriteRegister(TCpuAccessor &cpua, u32 value) {
      auto csr_old = cpua.template ReadRegister<SpecialRegisterId::kSysTickCsr>();
      LOG_TRACE(TLogger, "WRITE SYST_CSR: 0x%X", value);
      if (((csr_old & SysTickRegister::kCsrEnableMsk) == 0U) &&
          ((value & SysTickRegister::kCsrEnableMsk) != 0U)) {

        auto rvr = cpua.template ReadRegister<SpecialRegisterId::kSysTickRvr>();
        cpua.template WriteRegister<SpecialRegisterId::kSysTickCvr>(rvr);
        LOG_DEBUG(TLogger, "Enable SysTick");
      } else if (((csr_old & SysTickRegister::kCsrEnableMsk) != 0U) &&
                 ((value & SysTickRegister::kCsrEnableMsk) == 0U)) {
        LOG_DEBUG(TLogger, "Disable SysTick");
      }
      // each write to the register clears the count flag
      value = value & (~SysTickRegister::kCsrCountFlagMsk);
      cpua.template WriteRegister<SpecialRegisterId::kSysTickCsr>(value);
    }
  };

  class RegisterAccessRvr {
  public:
    static constexpr auto kAdr = SysTickAddressMap::kRvr;
    static constexpr bool kUseReadModifyWrite = true; // Perform read before write
    static constexpr bool kReadOnly = false;          // Disable write operation

    static u32 ReadRegister(TCpuAccessor &cpua) {
      auto ret = cpua.template ReadRegister<SpecialRegisterId::kSysTickRvr>();
      LOG_TRACE(TLogger, "READ SYST_RVR: 0x%X", ret);
      return ret;
    }

    static void WriteRegister(TCpuAccessor &cpua, u32 value) {
      LOG_TRACE(TLogger, "WRITE SYST_RVR: 0x%X", value);
      cpua.template WriteRegister<SpecialRegisterId::kSysTickRvr>(value);
    }
  };

  class RegisterAccessCvr {
  public:
    static constexpr auto kAdr = SysTickAddressMap::kCvr;
    static constexpr bool kUseReadModifyWrite = false; // Perform read before write
    static constexpr bool kReadOnly = false;           // Disable write operation

    static u32 ReadRegister(TCpuAccessor &cpua) {
      auto ret = cpua.template ReadRegister<SpecialRegisterId::kSysTickCvr>();
      LOG_TRACE(TLogger, "READ SYST_CVR: 0x%X", ret);
      return ret;
    }

    static void WriteRegister(TCpuAccessor &cpua, u32 value) {
      value = 0x0; // write to this register clears the count
      LOG_TRACE(TLogger, "WRITE SYST_CVR: 0x%X", value);
      cpua.template WriteRegister<SpecialRegisterId::kSysTickCvr>(value);
    }
  };

  class RegisterAccessCalib {
  public:
    static constexpr auto kAdr = SysTickAddressMap::kCalib;
    static constexpr bool kUseReadModifyWrite = false; // Perform read before write
    static constexpr bool kReadOnly = true;            // Disable write operation

    static u32 ReadRegister(TCpuAccessor &cpua) {
      auto ret = cpua.template ReadRegister<SpecialRegisterId::kSysTickCalib>();
      LOG_TRACE(TLogger, "READ SYST_CALIB: 0x%X", ret);
      return ret;
    }
  };

  static constexpr auto kRegisters =
      // clang-format off
      std::tuple<
        RegisterAccessCsr,
        RegisterAccessRvr,
        RegisterAccessCvr,
        RegisterAccessCalib
      >();
  // clang-format on

  static Result<void> Step(TCpuAccessor &cpua) {
    auto csr = cpua.template ReadRegister<SpecialRegisterId::kSysTickCsr>();

    if ((csr & SysTickRegister::kCsrEnableMsk) == SysTickRegister::kCsrEnableMsk) {
      auto cvr = cpua.template ReadRegister<SpecialRegisterId::kSysTickCvr>();

      if (cvr <= 1U) {
        LOG_DEBUG(TLogger, "SysTick counted to zero");
        auto rvr = cpua.template ReadRegister<SpecialRegisterId::kSysTickRvr>();
        cpua.template WriteRegister<SpecialRegisterId::kSysTickCvr>(rvr);

        if ((csr & SysTickRegister::kCsrTickIntMsk) == SysTickRegister::kCsrTickIntMsk) {
          ExcTrig::SetPending(cpua, ExceptionType::kSysTick);
        }
        // Set the count flag ... indication that the counter has reached 0
        csr |= SysTickRegister::kCsrCountFlagMsk;
        cpua.template WriteRegister<SpecialRegisterId::kSysTickCsr>(csr);
      } else {
        cpua.template WriteRegister<SpecialRegisterId::kSysTickCvr>(cvr - 1U);
      }
    }

    return Ok();
  }

private:
  /**
   * @brief Constructor
   */
  constexpr SysTick() = delete;

  /**
   * @brief Destructor
   */
  virtual ~SysTick() = delete;

  /**
   * @brief Copy constructor for SysTick.
   * @param r_src the object to be copied
   */
  SysTick(const SysTick &r_src) = delete;

  /**
   * @brief Copy assignment operator for SysTick.
   * @param r_src the object to be copied
   */
  SysTick &operator=(const SysTick &r_src) = delete;

  /**
   * @brief Move constructor for SysTick.
   * @param r_src the object to be moved
   */
  SysTick(SysTick &&r_src) = delete;

  /**
   * @brief Move assignment operator for  SysTick.
   * @param r_src the object to be moved
   */
  SysTick &operator=(SysTick &&r_src) = delete;
};

} // namespace libmicroemu::internal