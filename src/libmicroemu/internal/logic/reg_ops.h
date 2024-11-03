#pragma once

#include "libmicroemu/logger.h"
#include "libmicroemu/register_id.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace microemu {
namespace internal {

template <typename TProcessorStates, typename TSpecRegOps, typename TLogger = NullLogger>
class RegOps {
public:
  using SReg = TSpecRegOps;

  static const char *GetRegisterName(const u8 &reg_id) {
    switch (reg_id) {
    case 0x0u:
      return "R0";
    case 0x1u:
      return "R1";
    case 0x2u:
      return "R2";
    case 0x3u:
      return "R3";
    case 0x4u:
      return "R4";
    case 0x5u:
      return "R5";
    case 0x6u:
      return "R6";
    case 0x7u:
      return "R7";
    case 0x8u:
      return "R8";
    case 0x9u:
      return "R9";
    case 0xAu: // stack limit or scratch register (r10)
      return "SL";
    case 0xBu: // frame pointer (r11)
      return "FP";
    case 0xCu: // intra-procedure call scratch register (r12)
      return "IP";
    case static_cast<u8>(microemu::RegisterId::kSp): // stack-pointer (r13)
      return "SP";
    case 0xEu: // link-register (r14)
      return "LR";
    case static_cast<u8>(microemu::RegisterId::kPc): // r15
      return "PC";
    default:
      return "UNDEFINED";
    }
  };

  static inline SpecialRegisterId LookUpSP(const TProcessorStates &pstates) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.521
    auto sys_ctrl = SReg::ReadRegister(pstates, SpecialRegisterId::kSysCtrl);

    // if CONTROL.SPSEL == '1' then
    if ((sys_ctrl & SysCtrlRegister::kControlSpSelMsk) == SysCtrlRegister::kControlSpSelMsk) {

      //    if CurrentMode==Mode_Thread then
      //        sp = RNameSP_process;
      return SpecialRegisterId::kSpProcess;
      //    else
      //        TODO: UNPREDICTABLE;
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
    return pc + 0x4u;
  }

  template <RegisterId Id> static inline u32 ReadRegister(const TProcessorStates &pstates) {
    static_assert(static_cast<u8>(Id) < kNoOfRegisters, "Invalid register id");
    return ReadRegister<static_cast<u8>(Id), 0U>(pstates);
  }

  // U is used to make the signature unique
  template <u8 Id, int U = 0U> static inline u32 ReadRegister(const TProcessorStates &pstates) {
    static_assert(Id < 16u, "Invalid register id");

    // Retrieve the register tuple from the processor state
    const auto &registers = pstates.GetRegisters();

    // Registers R0 to R12
    if constexpr (Id <= 12u) {
      return std::get<Id>(registers);
    }

    // Stack Pointer (SP)
    if constexpr (Id == static_cast<u8>(RegisterId::kSp)) {
      return ReadSP(pstates);
    }

    // Link Register (LR)
    if constexpr (Id == 14u) {
      return std::get<14u>(registers);
    }

    // Program Counter (PC)
    if constexpr (Id == static_cast<u8>(RegisterId::kPc)) {
      return ReadPC(pstates);
    }

    assert(false && "Invalid register id");
    // Should not happen, but returns 0u if somehow out of range
    return 0u;
  }

  static inline u32 ReadRegister(const TProcessorStates &pstates, u8 id) {
    // Retrieve the register array from the processor state
    const auto &registers = pstates.GetRegisters();

    // Registers R0 to R12
    if (id <= 12u) {
      return registers[id];
    }

    // Stack Pointer (SP)
    if (id == static_cast<u8>(RegisterId::kSp)) {
      return ReadSP(pstates);
    }

    // Link Register (LR)
    if (id == 14u) {
      return registers[id];
    }

    // Program Counter (PC)
    if (id == static_cast<u8>(RegisterId::kPc)) {
      return ReadPC(pstates);
    }

    assert(false && "Invalid register id");
    // Default: out of range, return 0
    return 0u;
  }

  template <u8 Id, int U = 0U>
  static inline void WriteRegister(TProcessorStates &pstates, u32 value) {
    static_assert(Id < 16u, "Invalid register id");
    static_assert(Id != static_cast<u8>(RegisterId::kPc), "PC is not assignable by this function");
    auto &registers = pstates.GetRegisters();

    // Registers R0 to R12
    if constexpr (Id <= 12u) {
      registers[Id] = value;
      return;
    }

    // Stack Pointer (SP)
    if constexpr (Id == static_cast<u8>(RegisterId::kSp)) {
      WriteSP(pstates, value);
      return;
    }

    // Link Register (LR)
    if constexpr (Id == 14u) {
      registers[14u] = value;
      return;
    }

    assert(false && "Invalid register id");
    // Do nothing for invalid cases (though this should never happen due to static_assert)
  }

  template <RegisterId Id>
  static inline void WriteRegister(TProcessorStates &pstates, const u32 &value) {
    static_assert(static_cast<u8>(Id) < kNoOfRegisters, "Invalid register id");
    WriteRegister<static_cast<u8>(Id)>(pstates, value);
  }

  static inline void WriteRegister(TProcessorStates &pstates, u8 id, u32 value) {
    auto &registers = pstates.GetRegisters();

    // Registers R0 to R12
    if (id <= 12u) {
      registers[id] = value;
      return;
    }

    // Stack Pointer (SP)
    if (id == static_cast<u8>(RegisterId::kSp)) {
      WriteSP(pstates, value);
      return;
    }

    // Link Register (LR)
    if (id == 14u) {
      registers[id] = value;
      return;
    }

    // Program Counter (PC) - Not assignable

    assert(false && "Invalid register id");
    // No action needed for out-of-range or unhandled IDs
  }

  // ---

private:
  RegOps() = delete;
  ~RegOps() = delete;
  RegOps &operator=(const RegOps &r_src) = delete;
  RegOps(RegOps &&r_src) = delete;
  RegOps &operator=(RegOps &&r_src) = delete;
  RegOps(const RegOps &r_src) = delete;
};

} // namespace internal
} // namespace microemu