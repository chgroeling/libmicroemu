#pragma once

#include "libmicroemu/internal/bus/mem_access_results.h"
#include "libmicroemu/internal/logic/exceptions_ops.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/types.h"

namespace libmicroemu ::internal {

enum class SysCtrlBlockAddressMap : me_adr_t {
  kCpuId = 0xED00, // CPUID Base Register (RO)
  kIcsr = 0xED04,  // Interrupt Control and State Register (RW)
  kVtor = 0xED08,  // Vector Table Offset Register (RW)
  kAircr = 0xED0C, // Application Interrupt and Reset Control Register (RW)
  kr = 0xED10,     // System Control Register (RW)
  kCcr = 0xED14,   // Configuration and Control Register (RW)
  kShpr1 = 0xED18, // System Handler Priority Register 1 (RW)
  kShpr2 = 0xED1C, // System Handler Priority Register 2 (RW)
  kShpr3 = 0xED20, // System Handler Priority Register 3 (RW)
  kShcsr = 0xED24, // System Handler Control and State Register (RW)
  kCfsr = 0xED28,  // Configurable Fault Status Register (RW)
  kHfsr = 0xED2C,  // HardFault Status Register (RW)
  kDfsr = 0xED30,  // Debug Fault Status Register (RW)
  kMmfar = 0xED34, // MemManage Fault Address Register (RW)
  kBfar = 0xED38,  // BusFault Address Register (RW)
  kAfsr = 0xED3C,  // Auxiliary Fault Status Register (RW)
  kCpacr = 0xED88  // Coprocessor Access Control Register (RW)
};

template <typename TCpuAccessor, typename TLogger = NullLogger> class SysCtrlBlock {
public:
  using MapEnum = SysCtrlBlockAddressMap;

  static constexpr u32 GetBeginPhysicalAddress() { return 0xED00; }
  static constexpr u32 GetEndPhysicalAddress() { return 0xEDFF; }

  class RegisterAccessCcr {
  public:
    static constexpr auto kAdr = SysCtrlBlockAddressMap::kCcr;
    static constexpr bool kUseReadModifyWrite = true; // Perform read before write
    static constexpr bool kReadOnly = false;          // Disable write operation

    static u32 ReadRegister(TCpuAccessor &cpua) {
      auto read_val = cpua.template ReadRegister<SpecialRegisterId::kCcr>();
      LOG_TRACE(TLogger, "READ CCR: 0x%X", read_val);
      return read_val;
    }

    static void WriteRegister(TCpuAccessor &cpua, u32 value) {
      LOG_TRACE(TLogger, "WRITE CCR: 0x%X", value);
      cpua.template WriteRegister<SpecialRegisterId::kCcr>(value);
    }
  };

  class RegisterAccessCfsr {
  public:
    static constexpr auto kAdr = SysCtrlBlockAddressMap::kCfsr;
    static constexpr bool kUseReadModifyWrite = false; // Perform read before write
    static constexpr bool kReadOnly = false;           // Disable write operation

    static u32 ReadRegister(TCpuAccessor &cpua) {
      auto read_val = cpua.template ReadRegister<SpecialRegisterId::kCfsr>();
      LOG_TRACE(TLogger, "READ CFSR: 0x%X", read_val);
      return read_val;
    }

    static void WriteRegister(TCpuAccessor &cpua, u32 value) {
      auto read_val = cpua.template ReadRegister<SpecialRegisterId::kCfsr>();
      value = read_val & (~value);
      LOG_TRACE(TLogger, "WRITE CFSR: 0x%X", value);
      cpua.template WriteRegister<SpecialRegisterId::kCfsr>(value);
    }
  };

  class RegisterAccessBfar {
  public:
    static constexpr auto kAdr = SysCtrlBlockAddressMap::kBfar;
    static constexpr bool kUseReadModifyWrite = true; // Perform read before write
    static constexpr bool kReadOnly = false;          // Disable write operation

    static u32 ReadRegister(TCpuAccessor &cpua) {
      auto read_val = cpua.template ReadRegister<SpecialRegisterId::kBfar>();
      LOG_TRACE(TLogger, "READ BFAR: 0x%X", read_val);
      return read_val;
    }

    static void WriteRegister(TCpuAccessor &cpua, u32 value) {
      LOG_TRACE(TLogger, "WRITE BFAR: 0x%X", value);
      cpua.template WriteRegister<SpecialRegisterId::kCfsr>(value);
    }
  };

  static constexpr auto kRegisters =
      // clang-format off
      std::tuple<
        RegisterAccessCcr,
        RegisterAccessCfsr,
        RegisterAccessBfar
      >();
  // clang-format on

private:
  SysCtrlBlock() = delete;
  ~SysCtrlBlock() = delete;
  SysCtrlBlock(const SysCtrlBlock &r_src) = delete;
  SysCtrlBlock &operator=(const SysCtrlBlock &r_src) = delete;
  SysCtrlBlock(SysCtrlBlock &&r_src) = delete;
  SysCtrlBlock &operator=(SysCtrlBlock &&r_src) = delete;
};

} // namespace libmicroemu::internal
