#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/i_breakpoint.h"
#include "libmicroemu/internal/logic/predicates.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

template <typename TInstrContext, typename TLogger = NullLogger> class SpecialInstr {
public:
  using TCpuAccessor = decltype(TInstrContext::cpua);
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  using ExcTrig = typename TInstrContext::ExcTrig;
  /**
   * @brief It instruction
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 236
   */
  static Result<InstrExecResult> ItInstr(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                         const u32 &firstcond, const u32 &mask) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    auto istate = ictx.cpua.template ReadRegister<SpecialRegisterId::kIstate>();
    istate = firstcond << 4U | mask;

    ictx.cpua.template WriteRegister<SpecialRegisterId::kIstate>(istate);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

  /**
   * @brief Svc instruction
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 213
   */
  template <typename TDelegates>
  static Result<InstrExecResult> Svc(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                     const u32 &imm32, TDelegates &delegates) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    InstrExecFlagsSet eflags{kNoInstrExecFlags};
    if (delegates.IsSvcSet()) {
      TRY_ASSIGN(svc_flags, InstrExecResult, delegates.Svc(imm32));

      if ((svc_flags & static_cast<SvcFlagsSet>(SvcFlags::kRequestExit)) != 0U) {
        eflags |= static_cast<InstrExecFlagsSet>(InstrExecFlags::kSvcReqExit);
      } else if ((svc_flags & static_cast<SvcFlagsSet>(SvcFlags::kRequestErrorExit)) != 0U) {
        eflags |= static_cast<InstrExecFlagsSet>(InstrExecFlags::kSvcReqErrorExit);
      }

      if ((svc_flags & static_cast<SvcFlagsSet>(SvcFlags::kOmitException)) == 0U) {
        ExcTrig::SetPending(ictx.cpua, ExceptionType::kSVCall);
      }
    } else {
      ExcTrig::SetPending(ictx.cpua, ExceptionType::kSVCall);
    }
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(InstrExecResult{eflags});
  }

  /**
   * @brief Bkpt instruction
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 212
   */
  template <typename TDelegates>
  static Result<InstrExecResult> Bkpt(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const u32 &imm32, TDelegates &delegates) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    InstrExecFlagsSet eflags{kNoInstrExecFlags};
    if (delegates.IsBkptSet()) {
      TRY_ASSIGN(bkpt_flags, InstrExecResult, delegates.Bkpt(imm32));

      if ((bkpt_flags & static_cast<BkptFlagsSet>(BkptFlags::kRequestExit)) != 0U) {
        eflags |= static_cast<InstrExecFlagsSet>(InstrExecFlags::kBkptReqExit);
      } else if ((bkpt_flags & static_cast<BkptFlagsSet>(BkptFlags::kRequestErrorExit)) != 0U) {
        eflags |= static_cast<InstrExecFlagsSet>(InstrExecFlags::kBkptReqErrorExit);
      }
      if ((bkpt_flags & static_cast<BkptFlagsSet>(BkptFlags::kOmitException)) == 0U) {
        ExcTrig::SetPending(ictx.cpua, ExceptionType::kHardFault);
      }
    } else {
      ExcTrig::SetPending(ictx.cpua, ExceptionType::kHardFault);
    }
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(InstrExecResult{eflags});
  }

  /**
   * @brief Branch condition
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 205
   */
  static Result<InstrExecResult> BCond(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                       const u32 &imm32, const u8 &cond) {
    const auto condition_passed = It::ConditionPassed(ictx.cpua, cond);
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    Pc::BranchWritePC(ictx.cpua, pc + imm32);
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

  /**
   * @brief Table branch
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 416
   */
  template <typename TArg0, typename TArg1>
  static Result<InstrExecResult> Tbhh(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_m, TArg1 arg_n) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    const bool is_tbh = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kTbh)) != 0U;
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    me_adr_t halfwords{0U};
    if (is_tbh) {
      me_adr_t adr = (rn + Alu32::LSL(rm, 1));
      TRY_ASSIGN(out, InstrExecResult,
                 ictx.bus.template ReadOrRaise<u16>(ictx.cpua, adr,
                                                    BusExceptionType::kRaisePreciseDataBusError));
      halfwords = out;
    } else {
      me_adr_t adr = rn + rm;
      TRY_ASSIGN(out, InstrExecResult,
                 ictx.bus.template ReadOrRaise<u8>(ictx.cpua, adr,
                                                   BusExceptionType::kRaisePreciseDataBusError));
      halfwords = out;
    }

    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    Pc::BranchWritePC(ictx.cpua, pc + (halfwords << 1U));
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

  /**
   * @brief Compare branch
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 216
   */
  template <typename TArg0>
  static Result<InstrExecResult> CbNZ(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_n, const u32 &imm32) {
    const bool is_non_zero = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kNonZero)) != 0U;
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    const u32 new_pc = pc + imm32;

    if ((rn == 0U) && (is_non_zero == false)) {
      Pc::BranchWritePC(ictx.cpua, new_pc);
    } else if ((rn != 0U) && (is_non_zero == true)) {
      Pc::BranchWritePC(ictx.cpua, new_pc);
    } else {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
    };

    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

  /**
   * @brief Bfi
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 208
   */
  template <typename TArg0, typename TArg1>
  static Result<InstrExecResult> Bfi(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                     const TArg0 &arg_d, const TArg1 &arg_n, const u8 &lsbit,
                                     const u8 &msbit) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    if (msbit >= lsbit) {
      const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
      const auto rd = ictx.cpua.ReadRegister(arg_d.Get());

      const auto src_bitmask = static_cast<u32>(static_cast<u32>(1U) << (msbit - lsbit + 1U)) - 1U;
      const auto dest_bitmask = static_cast<u32>(
          ((static_cast<u32>(1U) << (msbit - lsbit + 1U)) - static_cast<u32>(1U)) << lsbit);

      const auto rn_slice = (rn & src_bitmask) << lsbit;
      const auto rd_result = (rd & ~dest_bitmask) | rn_slice;

      ictx.cpua.WriteRegister(arg_d.Get(), rd_result);
    } else {
      return Err<InstrExecResult>(StatusCode::kExecutorUnpredictable);
    }

    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

  /**
   * @brief Ubfx
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 424
   */
  template <typename TArg0, typename TArg1>
  static Result<InstrExecResult> Ubfx(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_d, const TArg1 &arg_n, const u8 &lsbit,
                                      const u8 &widthminus1) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    const u8 msbit = lsbit + widthminus1;
    if (msbit > 31U) {
      return Err<InstrExecResult>(StatusCode::kExecutorUnpredictable);
    }

    u32 msk = ((static_cast<u32>(1U) << (msbit - lsbit + 1U)) - static_cast<u32>(1U)) << lsbit;
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto result = (rn & msk) >> lsbit;

    PostExecWriteRegPcExcluded::Call(ictx, arg_d, result);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

  /**
   * @brief Ldrd
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 257
   */
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecResult> Ldrd(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_t, const TArg1 &arg_t2, const TArg2 &arg_n,
                                      const u32 &imm32) {
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const me_adr_t offset_addr = is_add == true ? rn + imm32 : rn - imm32;
    const me_adr_t address = is_index == true ? offset_addr : rn;

    // Read from address
    TRY_ASSIGN(data, InstrExecResult,
               ictx.bus.template ReadOrRaise<u32>(ictx.cpua, address,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    TRY_ASSIGN(data2, InstrExecResult,
               ictx.bus.template ReadOrRaise<u32>(ictx.cpua, address + 0x4U,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    if (is_wback) {
      PostExecWriteRegPcExcluded::Call(ictx, arg_n, offset_addr);
    }
    PostExecWriteRegPcExcluded::Call(ictx, arg_t, data);
    PostExecWriteRegPcExcluded::Call(ictx, arg_t2, data2);
    PostExecAdvancePcAndIt::Call(ictx, iflags);

    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

  /**
   * @brief Msr
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 677
   */
  template <typename TArg0>
  static Result<InstrExecResult> Msr(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                     const TArg0 &arg_n, const uint8_t mask, const uint8_t SYSm) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto SYSm_7_3 = Bm32::ExtractBits1R<7U, 3U>(SYSm);
    switch (SYSm_7_3) {
    case 0b00000U: {
      if (mask & 0x1) {
        // APSR_g Application Program Status Register
        assert(false && "not implemented");
      } else {
        // APSR_nzcvq Application Program Status Register
        assert(false && "not implemented");
      }
      break;
    }
    case 0b00001U: {
      const auto SYSm_2_0 = Bm32::ExtractBits1R<2U, 0U>(SYSm);
      switch (SYSm_2_0) {
      case 0b000U: {
        // MSP - Main Stack Pointer
        ictx.cpua.template WriteRegister<SpecialRegisterId::kSpMain>(rn);
        LOG_TRACE(TLogger, "MSR Call - Write main stack pointer: 0x%08X", rn);
        break;
      }
      case 0b001U: {
        // PSP - Process Stack Pointer
        ictx.cpua.template WriteRegister<SpecialRegisterId::kSpProcess>(rn);
        LOG_TRACE(TLogger, "MSR Call - Write process stack pointer: 0x%08X", rn);
        break;
      }
      default:
        // undefined
        assert(false && "undefined");
        break;
      }
      break;
    }
    case 0b00010U: {
      const auto SYSm_2_0 = Bm32::ExtractBits1R<2U, 0U>(SYSm);
      switch (SYSm_2_0) {
      case 0b000U:
        // PRIMASK - Priority Mask
        assert(false && "not implemented");
        break;
      case 0b001U:
        // BASEPRI - Base Priority
        assert(false && "not implemented");
        break;
      case 0b010U:
        // BASEPRI_MAX - Base Priority Max
        assert(false && "not implemented");
        break;
      case 0b011U:
        // FAULTMASK- Fault Mask
        assert(false && "not implemented");
        break;
      case 0b100U: {
        // CONTROL- Control
        const auto is_privileged = Predicates::IsCurrentModePrivileged<TCpuAccessor>(ictx.cpua);

        if (is_privileged) {
          auto control = ictx.cpua.template ReadRegister<SpecialRegisterId::kControl>();
          //    CONTROL.nPRIV = R[n]<0>;
          control &= ~ControlRegister::kNPrivMsk;
          control |= ((rn & 0x1) >> 0U) << ControlRegister::kNPrivPos;
          if (Predicates::IsThreadMode(ictx.cpua)) {
            // CONTROL.SPSEL = R[n]<1>;
            control &= ~ControlRegister::kSpselMsk;
            control |= ((rn & 0x2U) >> 1U) << ControlRegister::kSpselPos;
          }
          LOG_TRACE(TLogger, "MSR Call - Write CONTROL: 0x%08X", control);

          ictx.cpua.template WriteRegister<SpecialRegisterId::kControl>(control);
        }

        // if HaveFPExt() then CONTROL.FPCA = R[n]<2>;
        break;
      }
      default:
        // undefined
        assert(false && "undefined");
        break;
      }
    } break;
    default:
      // undefined
      assert(false && "undefined");
      break;
    }

    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

  /**
   * @brief Mrs
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 675
   */
  template <typename TArg0>
  static Result<InstrExecResult> Mrs(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                     const TArg0 &arg_d, const uint8_t mask, const uint8_t SYSm) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    const auto SYSm_7_3 = Bm32::ExtractBits1R<7U, 3U>(SYSm);
    u32 rd_val = 0U;
    switch (SYSm_7_3) {
    case 0b00000U: {
      if (mask & 0x1) {
        // APSR_g Application Program Status Register
        assert(false && "not implemented");
      } else {
        // APSR_nzcvq Application Program Status Register
        assert(false && "not implemented");
      }
      break;
    }
    case 0b00001U: {
      const auto SYSm_2_0 = Bm32::ExtractBits1R<2U, 0U>(SYSm);
      switch (SYSm_2_0) {
      case 0b000U: {
        // MSP - Main Stack Pointer
        rd_val = ictx.cpua.template ReadRegister<SpecialRegisterId::kSpMain>();
        LOG_TRACE(TLogger, "MRS Call - Read MSP: 0x%08X", rd_val);
        break;
      }
      case 0b001U: {
        // PSP - Process Stack Pointer
        rd_val = ictx.cpua.template ReadRegister<SpecialRegisterId::kSpProcess>();
        LOG_TRACE(TLogger, "MRS Call - Read PSP: 0x%08X", rd_val);
        break;
      }
      default:
        // undefined
        assert(false && "undefined");
        break;
      }
      break;
    }
    case 0b00010U: {
      const auto SYSm_2_0 = Bm32::ExtractBits1R<2U, 0U>(SYSm);
      switch (SYSm_2_0) {
      case 0b000U:
        // PRIMASK - Priority Mask
        assert(false && "not implemented");
        break;
      case 0b001U:
        // BASEPRI - Base Priority
        assert(false && "not implemented");
        break;
      case 0b010U:
        // BASEPRI_MAX - Base Priority Max
        assert(false && "not implemented");
        break;
      case 0b011U:
        // FAULTMASK- Fault Mask
        assert(false && "not implemented");
        break;
      case 0b100U: {
        // CONTROL- Control

        const auto control = ictx.cpua.template ReadRegister<SpecialRegisterId::kControl>();
        // if HaveFPExt() then
        if (false) {
          // R[d]<2:0> = CONTROL<2:0>;
          rd_val |= control & ControlRegister::kControlBit2toBit0Msk;
        } else {

          // R[d]<1:0> = CONTROL<1:0>;
          rd_val |= control & ControlRegister::kControlBit1toBit0Msk;
        }

        LOG_TRACE(TLogger, "MRS Call - Read CONTROL: 0x%08X", rd_val);
        break;
      }
      default:
        // undefined
        assert(false && "undefined");
        break;
      }
    } break;
    default:
      // undefined
      assert(false && "undefined");
      break;
    }

    PostExecWriteRegPcExcluded::Call(ictx, arg_d, rd_val);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

  /**
   * @brief Strd
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 393
   */
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecResult> Strd(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_t, const TArg1 &arg_t2, const TArg2 &arg_n,
                                      const u32 &imm32) {

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());

    const u32 offset_addr = is_add == true ? rn + imm32 : rn - imm32;
    const u32 address = is_index == true ? offset_addr : rn;

    const auto rt = ictx.cpua.ReadRegister(arg_t.Get());
    TRY(InstrExecResult, ictx.bus.template WriteOrRaise<u32>(
                             ictx.cpua, address, rt, BusExceptionType::kRaisePreciseDataBusError));

    //---
    const auto rt2 = ictx.cpua.ReadRegister(arg_t2.Get());
    TRY(InstrExecResult,
        ictx.bus.template WriteOrRaise<u32>(ictx.cpua, address + 0x4, rt2,
                                            BusExceptionType::kRaisePreciseDataBusError));

    if (is_wback) {
      PostExecWriteRegPcExcluded::Call(ictx, arg_n, offset_addr);
    }
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

public:
private:
  /**
   * @brief Constructor
   */
  SpecialInstr() = delete;

  /**
   * @brief Destructor
   */
  ~SpecialInstr() = delete;

  /**
   * @brief Copy constructor for SpecialInstr.
   * @param r_src the object to be copied
   */
  SpecialInstr(const SpecialInstr &r_src) = delete;

  /**
   * @brief Copy assignment operator for SpecialInstr.
   * @param r_src the object to be copied
   */
  SpecialInstr &operator=(const SpecialInstr &r_src) = delete;

  /**
   * @brief Move constructor for SpecialInstr.
   * @param r_src the object to be moved
   */
  SpecialInstr(SpecialInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  SpecialInstr.
   * @param r_src the object to be moved
   */
  SpecialInstr &operator=(SpecialInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal