#pragma once

#include "libmicroemu/logger.h"
#include "libmicroemu/register_id.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace libmicroemu {
namespace internal {

template <typename TProcessorStates, typename TSpecRegOps, typename TLogger = NullLogger>
class RegOps {
public:
  using SReg = TSpecRegOps;

  static std::string_view GetRegisterName(const RegisterId &id) {
    using enum_type = std::underlying_type<RegisterId>::type;
    const enum_type raw_id = static_cast<enum_type>(id);

    switch (raw_id) {
    case 0x0U:
      return "R0";
    case 0x1U:
      return "R1";
    case 0x2U:
      return "R2";
    case 0x3U:
      return "R3";
    case 0x4U:
      return "R4";
    case 0x5U:
      return "R5";
    case 0x6U:
      return "R6";
    case 0x7U:
      return "R7";
    case 0x8U:
      return "R8";
    case 0x9U:
      return "R9";
    case 0xAU: // stack limit or scratch register (r10)
      return "SL";
    case 0xBU: // frame pointer (r11)
      return "FP";
    case 0xCU: // intra-procedure call scratch register (r12)
      return "IP";
    case static_cast<u8>(libmicroemu::RegisterId::kSp): // stack-pointer (r13)
      return "SP";
    case 0xEU: // link-register (r14)
      return "LR";
    case static_cast<u8>(libmicroemu::RegisterId::kPc): // r15
      return "PC";
    default:
      return "UNDEFINED";
    }
  }

  static inline SpecialRegisterId LookUpSP(const TProcessorStates &pstates) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.521
    auto sys_ctrl = SReg::ReadRegister(pstates, SpecialRegisterId::kSysCtrl);

    // if CONTROL.SPSEL == '1' then
    if ((sys_ctrl & SysCtrlRegister::kControlSpSelMsk) == SysCtrlRegister::kControlSpSelMsk) {

      //    if CurrentMode==Mode_Thread then
      //        sp = RNameSP_process;
      return SpecialRegisterId::kSpProcess;
      //    else
      //        UNPREDICTABLE;
    }

    return SpecialRegisterId::kSpMain;
  }
  static inline u32 ReadSP(const TProcessorStates &pstates) {
    // The actual value of R13 is determined by the current stack
    // pointer selection bit in the CONTROL. The value in the
    // register array is not used.
    const auto sp_reg = LookUpSP(pstates);
    const auto sp = SReg::ReadRegister(pstates, sp_reg);
    return sp;
  }

  static inline void WriteSP(TProcessorStates &pstates, const u32 &value) {
    const auto sp_reg = LookUpSP(pstates);
    SReg::WriteRegister(pstates, sp_reg, value);
  }

  static inline void ForceWritePC(TProcessorStates &pstates, const u32 &pc) {
    auto &registers = pstates.GetRegisters();
    registers[static_cast<u8>(RegisterId::kPc)] = pc;
  }

  static inline u32 ReadPC(const TProcessorStates &pstates) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.521
    const auto &registers = pstates.GetRegisters();
    const auto &pc = std::get<static_cast<u8>(RegisterId::kPc)>(registers);
    return pc + 0x4U;
  }

  template <RegisterId Id> static inline u32 ReadRegister(const TProcessorStates &pstates) {
    static_assert(static_cast<u8>(Id) < CountRegisters(), "Invalid register id");

    using enum_type = std::underlying_type<RegisterId>::type;

    switch (Id) {
    case RegisterId::kSp:
      return ReadSP(pstates);
    case RegisterId::kPc:
      return ReadPC(pstates);
    default:
      // Retrieve the register array from the processor state
      const auto &registers = pstates.GetRegisters();
      return registers[static_cast<enum_type>(Id)];
    }
    // Should not happen, but returns 0U if somehow out of range
    return 0;
  }

  static inline u32 ReadRegister(const TProcessorStates &pstates, RegisterId id) {
    using enum_type = std::underlying_type<RegisterId>::type;
    assert(static_cast<enum_type>(id) < CountRegisters() && "Invalid register id");
    switch (id) {
    case RegisterId::kSp:
      return ReadSP(pstates);
    case RegisterId::kPc:
      return ReadPC(pstates);
    default:
      // Retrieve the register array from the processor state
      const auto &registers = pstates.GetRegisters();
      return registers[static_cast<enum_type>(id)];
    }

    // Not reachable
    return 0U;
  }

  template <RegisterId Id>
  static inline void WriteRegister(TProcessorStates &pstates, const u32 &value) {
    static_assert(static_cast<u8>(Id) < CountRegisters(), "Invalid register id");
    static_assert(Id != RegisterId::kPc, "PC is not assignable by this function");

    using enum_type = std::underlying_type<RegisterId>::type;

    switch (Id) {
    case RegisterId::kSp:
      return WriteSP(pstates, value);
    default:
      // Retrieve the register array from the processor state
      auto &registers = pstates.GetRegisters();
      registers[static_cast<enum_type>(Id)] = value;
    }
    // No action needed for out-of-range or unhandled IDs
  }

  static inline void WriteRegister(TProcessorStates &pstates, RegisterId id, u32 value) {
    using enum_type = std::underlying_type<RegisterId>::type;

    assert(static_cast<enum_type>(id) < CountRegisters() && "Invalid register id");

    switch (id) {
    case RegisterId::kSp:
      return WriteSP(pstates, value);
    case RegisterId::kPc:
      assert(false && "PC is not assignable by this function");
      break;
    default:
      // Retrieve the register array from the processor state
      auto &registers = pstates.GetRegisters();
      registers[static_cast<enum_type>(id)] = value;
    }
    // No action needed for out-of-range or unhandled IDs
  }

private:
  RegOps() = delete;
  ~RegOps() = delete;
  RegOps &operator=(const RegOps &r_src) = delete;
  RegOps(RegOps &&r_src) = delete;
  RegOps &operator=(RegOps &&r_src) = delete;
  RegOps(const RegOps &r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu