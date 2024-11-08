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
  static std::string_view GetRegisterName(const SpecialRegisterId &reg_id) {
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

  template <SpecialRegisterId SId> static inline u32 ReadRegister(const TProcessorStates &pstates) {
    constexpr auto RegId = static_cast<u8>(SId);
    static_assert(RegId < CountSpecialRegisters(), "Invalid special register id");

    switch (SId) {
    case SpecialRegisterId::kEpsr:
      return ReadEpsr(pstates);
    case SpecialRegisterId::kXpsr:
      return ReadXpsr(pstates);
    case SpecialRegisterId::kControl:
      return ReadControl(pstates);
    default:
      // Persistent special register access
      return pstates.GetSpecialRegisters()[RegId];
    }
    // Not reachable
    return 0u;
  }

  static inline u32 ReadRegister(const TProcessorStates &pstates, const SpecialRegisterId &reg_id) {
    assert(static_cast<u8>(reg_id) < CountSpecialRegisters() && "Invalid special register id");
    switch (reg_id) {
    case SpecialRegisterId::kEpsr:
      return ReadEpsr(pstates);
    case SpecialRegisterId::kXpsr:
      return ReadXpsr(pstates);
    case SpecialRegisterId::kControl:
      return ReadControl(pstates);
    default:
      // Persistent special register access
      return pstates.GetSpecialRegisters()[static_cast<u8>(reg_id)];
    }
    // Not reachable
    return 0u;
  }

  template <SpecialRegisterId SId>
  static inline void WriteRegister(TProcessorStates &pstates, u32 value) {
    constexpr auto RegId = static_cast<u8>(SId);
    static_assert(RegId < CountSpecialRegisters(), "Invalid special register id");

    switch (SId) {
    case SpecialRegisterId::kEpsr:
      return WriteEpsr(pstates, value);
    case SpecialRegisterId::kXpsr:
      return WriteXpsr(pstates, value);
    case SpecialRegisterId::kControl:
      return WriteControl(pstates, value);
    default:
      // Persistent special register access
      pstates.GetSpecialRegisters()[RegId] = value;
    }

    // not reachable
  }

  static inline void WriteRegister(TProcessorStates &pstates, const SpecialRegisterId &reg_id,
                                   u32 value) {
    assert(static_cast<u8>(reg_id) < CountSpecialRegisters() && "Invalid special register id");
    switch (reg_id) {
    case SpecialRegisterId::kEpsr:
      return WriteEpsr(pstates, value);
    case SpecialRegisterId::kXpsr:
      return WriteXpsr(pstates, value);
    case SpecialRegisterId::kControl:
      return WriteControl(pstates, value);
    default:
      const auto rid = static_cast<u8>(reg_id);
      // Persistent special register access
      pstates.GetSpecialRegisters()[rid] = value;
    }
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