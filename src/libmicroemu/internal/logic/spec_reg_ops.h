#pragma once

#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace microemu {
namespace internal {

//----

template <typename TProcessorStates, typename TLogger = NullLogger> class SpecRegOps {
public:
  static const char *GetRegisterName(const SpecialRegisterId &reg_id) {
    switch (reg_id) {
    case SpecialRegisterId::kSysCtrl:
      return "SYSCTRL";
    case SpecialRegisterId::kApsr:
      return "APSR";
    case SpecialRegisterId::kIstate:
      return "ISTATE";
    case SpecialRegisterId::kSysTickCsr:
      return "SYSTICK_CSR";
    case SpecialRegisterId::kSysTickRvr:
      return "SYSTICK_RVR";
    case SpecialRegisterId::kSysTickCvr:
      return "SYSTICK_CVR";
    case SpecialRegisterId::kSysTickCalib:
      return "SYSTICK_CALIB";
    case SpecialRegisterId::kXpsr:
      return "XPSR";
    default:
      return "UNDEFINED";
    }
  }

  inline static u32 ReadRegister(const TProcessorStates &pstates, const u8 reg_id) {
    // Persistent special register access
    if (reg_id < CountPersistentSpecialRegisters()) {
      return pstates.GetSpecialRegisters()[reg_id];
    }

    // Runtime special register access
    if (reg_id == static_cast<u8>(SpecialRegisterId::kEpsr)) {
      return ReadEpsr(pstates);
    }
    if (reg_id == static_cast<u8>(SpecialRegisterId::kXpsr)) {
      return ReadXpsr(pstates);
    }
    if (reg_id == static_cast<u8>(SpecialRegisterId::kControl)) {
      return ReadControl(pstates);
    }

    assert(false && "Runtime special register access out of range");
    // Default case: out of range
    return 0u;
  }

  template <u8 RegId, int U = 0U> static inline u32 ReadRegister(const TProcessorStates &pstates) {
    static_assert(RegId < CountSpecialRegisters(), "Invalid special register id");

    // Persistent special register access
    if constexpr (RegId < CountPersistentSpecialRegisters()) {
      return pstates.GetSpecialRegisters()[RegId];
    }

    // Runtime special register access
    if constexpr (RegId == static_cast<u8>(SpecialRegisterId::kEpsr)) {
      return ReadEpsr(pstates);
    }
    if constexpr (RegId == static_cast<u8>(SpecialRegisterId::kXpsr)) {
      return ReadXpsr(pstates);
    }
    if constexpr (RegId == static_cast<u8>(SpecialRegisterId::kControl)) {
      return ReadControl(pstates);
    }

    assert(false && "Runtime special register access out of range");

    // Default case: out of range
    return 0u;
  }

  template <SpecialRegisterId SId> static inline u32 ReadRegister(const TProcessorStates &pstates) {
    return ReadRegister<static_cast<u32>(SId)>(pstates);
  }

  static inline u32 ReadRegister(const TProcessorStates &pstates, const SpecialRegisterId &reg_id) {
    return ReadRegister(pstates, static_cast<u8>(reg_id));
  }

  template <u8 RegId, int U = 0U>
  static inline void WriteRegister(TProcessorStates &pstates, u32 value) {
    static_assert(RegId < CountSpecialRegisters(), "Invalid special register id");

    // Persistent special register access
    if constexpr (RegId < CountPersistentSpecialRegisters()) {
      pstates.GetSpecialRegisters()[RegId] = value;
      return;
    }
    // Runtime special register access
    if constexpr (RegId == static_cast<u8>(SpecialRegisterId::kEpsr)) {
      WriteEpsr(pstates, value);
      return;
    }
    if constexpr (RegId == static_cast<u8>(SpecialRegisterId::kXpsr)) {
      WriteXpsr(pstates, value);
      return;
    }
    if constexpr (RegId == static_cast<u8>(SpecialRegisterId::kControl)) {
      WriteControl(pstates, value);
      return;
    }
    assert(false && "Runtime special register access out of range");
    // No need for else, out-of-range will do nothing
  }

  static inline void WriteRegister(TProcessorStates &pstates, u8 reg_id, u32 value) {
    // Persistent special register access
    if (reg_id < CountPersistentSpecialRegisters()) {
      pstates.GetSpecialRegisters()[reg_id] = value;
      return;
    }

    // Runtime special register access
    if (reg_id == static_cast<u8>(SpecialRegisterId::kEpsr)) {
      WriteEpsr(pstates, value);
      return;
    }
    if (reg_id == static_cast<u8>(SpecialRegisterId::kXpsr)) {
      WriteXpsr(pstates, value);
      return;
    }

    if (reg_id == static_cast<u8>(SpecialRegisterId::kControl)) {
      WriteControl(pstates, value);
      return;
    }
    assert(false && "Runtime special register access out of range");
    // If reg_id is greater than CountSpecialRegisters(), do nothing (out-of-range)
  }

  template <SpecialRegisterId SId>
  static inline void WriteRegister(TProcessorStates &pstates, u32 value) {
    WriteRegister<static_cast<u32>(SId)>(pstates, value);
  }

  static inline void WriteRegister(TProcessorStates &pstates, const SpecialRegisterId &reg_id,
                                   u32 value) {
    WriteRegister(pstates, static_cast<u8>(reg_id), value);
  }

  static inline u32 ReadEpsr(const TProcessorStates &pstates) {
    u32 epsr = 0u;
    auto sys_ctrl = ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);
    auto BitT = (sys_ctrl & SysCtrlRegister::kTMsk) >> SysCtrlRegister::kTPos; // Thumb mode bit
    epsr |= BitT << EpsrRegister::kTPos;

    // extract it bits from Istate and write them to epsr
    auto istate = ReadRegister<SpecialRegisterId::kIstate>(pstates);
    auto it_1_0 = (istate & IstateRegister::kItBit1to0Msk) >> IstateRegister::kItBit0Pos;
    auto it_7_2 = (istate & IstateRegister::kItBit7to2Msk) >> IstateRegister::kItBit2Pos;
    epsr |= (it_1_0 << EpsrRegister::kItBit0Pos) | (it_7_2 << EpsrRegister::kItBit2Pos);
    return epsr;
  }

  static inline void WriteEpsr(TProcessorStates &pstates, u32 value) {
    auto sys_ctrl = ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);
    auto BitT = (value & EpsrRegister::kTMsk) >> EpsrRegister::kTPos; // Thumb mode bit
    sys_ctrl &= ~SysCtrlRegister::kTMsk;
    sys_ctrl |= BitT << SysCtrlRegister::kTPos;
    WriteRegister<SpecialRegisterId::kSysCtrl>(pstates, sys_ctrl);

    // extract it bits from "value"  and write them to Istate register
    u32 istate = 0u;
    auto it_1_0 = (value & EpsrRegister::kItBit1to0Msk) >> EpsrRegister::kItBit0Pos;
    auto it_7_2 = (value & EpsrRegister::kItBit7to2Msk) >> EpsrRegister::kItBit2Pos;
    istate |= (it_1_0 << IstateRegister::kItBit0Pos) | (it_7_2 << IstateRegister::kItBit2Pos);
    WriteRegister<SpecialRegisterId::kIstate>(pstates, istate);
  }

  static inline u32 ReadXpsr(const TProcessorStates &pstates) {
    auto xpsr = 0u;
    auto apsr = ReadRegister<SpecialRegisterId::kApsr>(pstates);
    xpsr |= apsr;

    auto epsr = ReadEpsr(pstates);
    xpsr |= epsr;

    auto ipsr = ReadRegister<SpecialRegisterId::kIpsr>(pstates);
    xpsr |= ipsr;

    return xpsr;
  }

  static inline void WriteXpsr(TProcessorStates &pstates, u32 value) {
    static_cast<void>(pstates);
    static_cast<void>(value);
    assert(false && "Not implemented");
  }

  static inline u32 ReadControl(const TProcessorStates &pstates) {
    const auto sys_ctrl = ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);
    const auto npriv = (sys_ctrl & static_cast<u32>(SysCtrlRegister::kControlNPrivMsk)) >>
                       SysCtrlRegister::kControlNPrivPos;
    const auto spsel = (sys_ctrl & static_cast<u32>(SysCtrlRegister::kControlSpSelMsk)) >>
                       SysCtrlRegister::kControlSpSelPos;
    const auto fpca = (sys_ctrl & static_cast<u32>(SysCtrlRegister::kControlFpcaMsk)) >>
                      SysCtrlRegister::kControlFpcaPos;

    auto control{0u};
    control |= npriv << ControlRegister::kNPrivPos;
    control |= spsel << ControlRegister::kSpselPos;
    control |= fpca << ControlRegister::kFpcaPos;
    static_cast<void>(pstates);
    return control;
  }

  static inline void WriteControl(TProcessorStates &pstates, u32 value) {
    const auto npriv =
        (value & static_cast<u32>(ControlRegister::kNPrivMsk)) >> ControlRegister::kNPrivPos;
    const auto spsel =
        (value & static_cast<u32>(ControlRegister::kSpselMsk)) >> ControlRegister::kSpselPos;

    const auto fpca =
        (value & static_cast<u32>(ControlRegister::kFpcaMsk)) >> ControlRegister::kFpcaPos;

    auto sys_ctrl = ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);
    sys_ctrl &= ~SysCtrlRegister::kControlNPrivMsk;
    sys_ctrl |= npriv << SysCtrlRegister::kControlNPrivPos;

    sys_ctrl &= ~SysCtrlRegister::kControlSpSelMsk;
    sys_ctrl |= spsel << SysCtrlRegister::kControlSpSelPos;

    sys_ctrl &= ~SysCtrlRegister::kControlFpcaMsk;
    sys_ctrl |= fpca << SysCtrlRegister::kControlFpcaPos;

    WriteRegister<SpecialRegisterId::kSysCtrl>(pstates, sys_ctrl);
  }

private:
  SpecRegOps() = delete;
  ~SpecRegOps() = delete;
  SpecRegOps &operator=(const SpecRegOps &r_src) = delete;
  SpecRegOps(SpecRegOps &&r_src) = delete;
  SpecRegOps &operator=(SpecRegOps &&r_src) = delete;
  SpecRegOps(const SpecRegOps &r_src) = delete;
};

} // namespace internal
} // namespace microemu