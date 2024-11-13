#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/i_breakpoint.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from register adress to register
template <typename TInstrContext, typename TLogger = NullLogger> class SpecialInstr {
public:
  using TProcessorStates = decltype(TInstrContext::pstates);
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  using ExcTrig = typename TInstrContext::ExcTrig;
  /// It instruction
  /// see Armv7-M Architecture Reference Manual Issue E.e p.236
  static Result<ExecResult> ItInstr(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                    const u32 &firstcond, const u32 &mask) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    auto istate = SReg::template ReadRegister<SpecialRegisterId::kIstate>(ictx.pstates);

    istate = firstcond << 4U | mask;

    SReg::template WriteRegister<SpecialRegisterId::kIstate>(ictx.pstates, istate);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Svc instruction
  /// see Armv7-M Architecture Reference Manual Issue E.e p.213
  template <typename TDelegates>
  static Result<ExecResult> Svc(TInstrContext &ictx, const InstrFlagsSet &iflags, const u32 &imm32,
                                TDelegates &delegates) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};

    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }
    if (delegates.IsSvcSet()) {
      TRY_ASSIGN(svc_flags, ExecResult, delegates.Svc(imm32));

      if ((svc_flags & static_cast<SvcFlagsSet>(SvcFlags::kRequestExit)) != 0U) {
        eflags |= static_cast<ExecFlagsSet>(ExecFlags::kSvcReqExit);
      } else if ((svc_flags & static_cast<SvcFlagsSet>(SvcFlags::kRequestErrorExit)) != 0U) {
        eflags |= static_cast<ExecFlagsSet>(ExecFlags::kSvcReqErrorExit);
      }

      if ((svc_flags & static_cast<SvcFlagsSet>(SvcFlags::kOmitException)) == 0U) {
        ExcTrig::SetPending(ictx.pstates, ExceptionType::kSVCall);
      }
    } else {
      ExcTrig::SetPending(ictx.pstates, ExceptionType::kSVCall);
    }

    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Bkpt instruction
  /// see Armv7-M Architecture Reference Manual Issue E.e p.212
  template <typename TDelegates>
  static Result<ExecResult> Bkpt(TInstrContext &ictx, const InstrFlagsSet &iflags, const u32 &imm32,
                                 TDelegates &delegates) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    if (delegates.IsBkptSet()) {
      TRY_ASSIGN(bkpt_flags, ExecResult, delegates.Bkpt(imm32));

      if ((bkpt_flags & static_cast<BkptFlagsSet>(BkptFlags::kRequestExit)) != 0U) {
        eflags |= static_cast<ExecFlagsSet>(ExecFlags::kBkptReqExit);
      } else if ((bkpt_flags & static_cast<BkptFlagsSet>(BkptFlags::kRequestErrorExit)) != 0U) {
        eflags |= static_cast<ExecFlagsSet>(ExecFlags::kBkptReqErrorExit);
      }
      if ((bkpt_flags & static_cast<BkptFlagsSet>(BkptFlags::kOmitException)) == 0U) {
        ExcTrig::SetPending(ictx.pstates, ExceptionType::kHardFault);
      }
    } else {
      ExcTrig::SetPending(ictx.pstates, ExceptionType::kHardFault);
    }
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Branch condition
  /// see Armv7-M Architecture Reference Manual Issue E.e p.205
  static Result<ExecResult> BCond(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                  const u32 &imm32, const u8 &cond) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    const auto condition_passed = It::ConditionPassed(ictx.pstates, cond);
    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const me_adr_t pc = static_cast<me_adr_t>(Reg::ReadPC(ictx.pstates));
    Pc::BranchWritePC(ictx.pstates, pc + imm32);
    return Ok(ExecResult{eflags});
  }

  /// Table branch
  /// see Armv7-M Architecture Reference Manual Issue E.e p.416
  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Tbhh(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_m, TArg1 arg_n) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const bool is_tbh = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kTbh)) != 0U;

    const auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());
    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    me_adr_t halfwords = 0U;
    if (is_tbh) {
      me_adr_t adr = (rn + Alu32::LSL(rm, 1));
      TRY_ASSIGN(out, ExecResult,
                 ictx.bus.template ReadOrRaise<u16>(ictx.pstates, adr,
                                                    BusExceptionType::kRaisePreciseDataBusError));
      halfwords = out;
    } else {
      me_adr_t adr = rn + rm;
      TRY_ASSIGN(out, ExecResult,
                 ictx.bus.template ReadOrRaise<u8>(ictx.pstates, adr,
                                                   BusExceptionType::kRaisePreciseDataBusError));
      halfwords = out;
    }

    const me_adr_t pc = static_cast<me_adr_t>(Reg::ReadPC(ictx.pstates));
    Pc::BranchWritePC(ictx.pstates, pc + (halfwords << 1U));

    return Ok(ExecResult{eflags});
  }

  /// Compare branch
  /// see Armv7-M Architecture Reference Manual Issue E.e p.216
  template <typename TArg0>
  static Result<ExecResult> CbNZ(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const u32 &imm32) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    const bool is_non_zero = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kNonZero)) != 0U;

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    const me_adr_t pc = static_cast<me_adr_t>(Reg::ReadPC(ictx.pstates));

    ExecFlagsSet eflags{0x0U};

    const u32 new_pc = pc + imm32;
    if ((rn == 0U) && (is_non_zero == false)) {
      Pc::BranchWritePC(ictx.pstates, new_pc);
    } else if ((rn != 0U) && (is_non_zero == true)) {
      Pc::BranchWritePC(ictx.pstates, new_pc);
    } else {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
    };

    return Ok(ExecResult{eflags});
  }

  /// Bfi
  /// see Armv7-M Architecture Reference Manual Issue E.e p.208
  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Bfi(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                const TArg0 &arg_d, const TArg1 &arg_n, const u8 &lsbit,
                                const u8 &msbit) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    if (msbit >= lsbit) {
      const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
      const auto rd = Reg::ReadRegister(ictx.pstates, arg_d.Get());

      const auto src_bitmask = static_cast<u32>(static_cast<u32>(1U) << (msbit - lsbit + 1U)) - 1U;
      const auto dest_bitmask = static_cast<u32>(
          ((static_cast<u32>(1U) << (msbit - lsbit + 1U)) - static_cast<u32>(1U)) << lsbit);

      const auto rn_slice = (rn & src_bitmask) << lsbit;
      const auto rd_result = (rd & ~dest_bitmask) | rn_slice;

      Reg::WriteRegister(ictx.pstates, arg_d.Get(), rd_result);
    } else {
      return Err<ExecResult>(StatusCode::kScExecutorUnpredictable);
    }

    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Ubfx
  /// see Armv7-M Architecture Reference Manual Issue E.e p.424
  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Ubfx(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_d, const TArg1 &arg_n, const u8 &lsbit,
                                 const u8 &widthminus1) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const u8 msbit = lsbit + widthminus1;

    if (msbit <= 31U) {
      u32 msk = ((static_cast<u32>(1U) << (msbit - lsbit + 1U)) - static_cast<u32>(1U)) << lsbit;

      const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
      const auto result = (rn & msk) >> lsbit;
      Reg::WriteRegister(ictx.pstates, arg_d.Get(), result);
    } else {
      return Err<ExecResult>(StatusCode::kScExecutorUnpredictable);
    }

    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Ldrd
  /// see Armv7-M Architecture Reference Manual Issue E.e p.257
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Ldrd(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_t, const TArg1 &arg_t2, const TArg2 &arg_n,
                                 const u32 &imm32) {

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }
    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());

    const me_adr_t offset_addr = is_add == true ? rn + imm32 : rn - imm32;
    const me_adr_t address = is_index == true ? offset_addr : rn;

    // Read from address
    TRY_ASSIGN(data, ExecResult,
               ictx.bus.template ReadOrRaise<u32>(ictx.pstates, address,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    TRY_ASSIGN(data2, ExecResult,
               ictx.bus.template ReadOrRaise<u32>(ictx.pstates, address + 0x4U,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    if (is_wback == true) {
      Reg::WriteRegister(ictx.pstates, arg_n.Get(), offset_addr);
    }

    Reg::WriteRegister(ictx.pstates, arg_t.Get(), data);
    Reg::WriteRegister(ictx.pstates, arg_t2.Get(), data2);

    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Umull
  /// see Armv7-M Architecture Reference Manual Issue E.e p.257
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Umull(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                  const TArg0 &arg_d_lo, const TArg0 &arg_d_hi, const TArg1 &arg_n,
                                  const TArg2 &arg_m) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    const auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());

    const u64 result = (static_cast<u64>(rn) * static_cast<u64>(rm));
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);
    Reg::WriteRegister(ictx.pstates, arg_d_lo.Get(), result_lo);
    Reg::WriteRegister(ictx.pstates, arg_d_hi.Get(), result_hi);
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }
  /// Umlal
  /// see Armv7-M Architecture Reference Manual Issue E.e p.434
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Umlal(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                  const TArg0 &arg_d_lo, const TArg0 &arg_d_hi, const TArg1 &arg_n,
                                  const TArg2 &arg_m) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const u32 rdhi = Reg::ReadRegister(ictx.pstates, arg_d_hi.Get());
    const u32 rdlo = Reg::ReadRegister(ictx.pstates, arg_d_lo.Get());
    const u32 rd = static_cast<u64>(rdhi) << 32U | static_cast<u64>(rdlo);

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    const auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());

    const u64 result = (static_cast<u64>(rn) * static_cast<u64>(rm)) + rd;
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);
    Reg::WriteRegister(ictx.pstates, arg_d_lo.Get(), result_lo);
    Reg::WriteRegister(ictx.pstates, arg_d_hi.Get(), result_hi);
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Msr
  /// see Armv7-M Architecture Reference Manual Issue E.e p.677
  template <typename TArg0>
  static Result<ExecResult> Msr(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                const TArg0 &arg_n, const uint8_t mask, const uint8_t SYSm) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }
    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
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
        SReg::template WriteRegister<SpecialRegisterId::kSpMain>(ictx.pstates, rn);
        LOG_TRACE(TLogger, "MSR Call - Write main stack pointer: 0x%08X", rn);
        break;
      }
      case 0b001U: {
        // PSP - Process Stack Pointer
        SReg::template WriteRegister<SpecialRegisterId::kSpProcess>(ictx.pstates, rn);
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
        if (CurrentModeIsPrivileged(ictx.pstates)) {
          auto control = SReg::template ReadRegister<SpecialRegisterId::kControl>(ictx.pstates);
          //    CONTROL.nPRIV = R[n]<0>;
          control &= ~ControlRegister::kNPrivMsk;
          control |= ((rn & 0x1) >> 0U) << ControlRegister::kNPrivPos;

          auto sys_ctrl = SReg::template ReadRegister<SpecialRegisterId::kSysCtrl>(ictx.pstates);
          const auto exec_mode = sys_ctrl & SysCtrlRegister::kExecModeMsk;
          const auto is_thread_mode = exec_mode == SysCtrlRegister::kExecModeThread;
          if (is_thread_mode) {
            // CONTROL.SPSEL = R[n]<1>;
            control &= ~ControlRegister::kSpselMsk;
            control |= ((rn & 0x2U) >> 1U) << ControlRegister::kSpselPos;
          }
          LOG_TRACE(TLogger, "MSR Call - Write CONTROL: 0x%08X", control);

          SReg::template WriteRegister<SpecialRegisterId::kControl>(ictx.pstates, control);
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

    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Mrs
  /// see Armv7-M Architecture Reference Manual Issue E.e p.675
  template <typename TArg0>
  static Result<ExecResult> Mrs(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                const TArg0 &arg_d, const uint8_t mask, const uint8_t SYSm) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
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
        rd_val = SReg::template ReadRegister<SpecialRegisterId::kSpMain>(ictx.pstates);
        LOG_TRACE(TLogger, "MRS Call - Read MSP: 0x%08X", rd_val);
        break;
      }
      case 0b001U: {
        // PSP - Process Stack Pointer
        rd_val = SReg::template ReadRegister<SpecialRegisterId::kSpProcess>(ictx.pstates);
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

        const auto control = SReg::template ReadRegister<SpecialRegisterId::kControl>(ictx.pstates);
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
    Reg::WriteRegister(ictx.pstates, arg_d.Get(), rd_val);

    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Smull
  /// see Armv7-M Architecture Reference Manual Issue E.e p.372
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Smull(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                  const TArg0 &arg_d_lo, const TArg0 &arg_d_hi, const TArg1 &arg_n,
                                  const TArg2 &arg_m) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = static_cast<i32>(Reg::ReadRegister(ictx.pstates, arg_n.Get()));
    const auto rm = static_cast<i32>(Reg::ReadRegister(ictx.pstates, arg_m.Get()));
    const u64 result = static_cast<u64>(static_cast<i64>(rn) * static_cast<i64>(rm));
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);
    Reg::WriteRegister(ictx.pstates, arg_d_lo.Get(), result_lo);
    Reg::WriteRegister(ictx.pstates, arg_d_hi.Get(), result_hi);
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /// Strd
  /// see Armv7-M Architecture Reference Manual Issue E.e p.393
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Strd(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_t, const TArg1 &arg_t2, const TArg2 &arg_n,
                                 const u32 &imm32) {

    ExecFlagsSet eflags{0x0U};
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }
    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());

    const u32 off_adr = is_add == true ? rn + imm32 : rn - imm32;
    const u32 address = is_index == true ? off_adr : rn;

    const auto rt = Reg::ReadRegister(ictx.pstates, arg_t.Get());
    TRY(ExecResult, ictx.bus.template WriteOrRaise<u32>(
                        ictx.pstates, address, rt, BusExceptionType::kRaisePreciseDataBusError));

    //---
    const auto rt2 = Reg::ReadRegister(ictx.pstates, arg_t2.Get());
    TRY(ExecResult,
        ictx.bus.template WriteOrRaise<u32>(ictx.pstates, address + 0x4, rt2,
                                            BusExceptionType::kRaisePreciseDataBusError));

    if (is_wback == true) {
      Reg::WriteRegister(ictx.pstates, arg_n.Get(), off_adr);
    }
    //---

    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);
    return Ok(ExecResult{eflags});
  }

public:
private:
  static bool CurrentModeIsPrivileged(TProcessorStates &pstates) {
    /// see Armv7-M Architecture Reference Manual Issue E.e p.512
    auto sys_ctrl = SReg::template ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);
    const auto exec_mode = sys_ctrl & SysCtrlRegister::kExecModeMsk;
    const auto is_handler_mode = exec_mode == SysCtrlRegister::kExecModeHandler;
    const auto is_privileged = (sys_ctrl & SysCtrlRegister::kControlNPrivMsk) == 0U;

    return (is_handler_mode || is_privileged);
  }

  /// \brief Constructor
  SpecialInstr() = delete;

  /// \brief Destructor
  ~SpecialInstr() = delete;

  /// \brief Copy constructor for SpecialInstr.
  /// \param r_src the object to be copied
  SpecialInstr(const SpecialInstr &r_src) = default;

  /// \brief Copy assignment operator for SpecialInstr.
  /// \param r_src the object to be copied
  SpecialInstr &operator=(const SpecialInstr &r_src) = delete;

  /// \brief Move constructor for SpecialInstr.
  /// \param r_src the object to be copied
  SpecialInstr(SpecialInstr &&r_src) = delete;

  /// \brief Move assignment operator for SpecialInstr.
  /// \param r_src the object to be copied
  SpecialInstr &operator=(SpecialInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu