#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/i_breakpoint.h"
#include "libmicroemu/internal/logic/predicates.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from register adress to register
 */
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
  static Result<ExecResult> ItInstr(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                    const u32 &firstcond, const u32 &mask) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    auto istate = ictx.cpua.template ReadRegister<SpecialRegisterId::kIstate>();

    istate = firstcond << 4U | mask;

    ictx.cpua.template WriteRegister<SpecialRegisterId::kIstate>(istate);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Svc instruction
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 213
   */
  template <typename TDelegates>
  static Result<ExecResult> Svc(TInstrContext &ictx, const InstrFlagsSet &iflags, const u32 &imm32,
                                TDelegates &delegates) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};

    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
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
        ExcTrig::SetPending(ictx.cpua, ExceptionType::kSVCall);
      }
    } else {
      ExcTrig::SetPending(ictx.cpua, ExceptionType::kSVCall);
    }

    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Bkpt instruction
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 212
   */
  template <typename TDelegates>
  static Result<ExecResult> Bkpt(TInstrContext &ictx, const InstrFlagsSet &iflags, const u32 &imm32,
                                 TDelegates &delegates) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
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
        ExcTrig::SetPending(ictx.cpua, ExceptionType::kHardFault);
      }
    } else {
      ExcTrig::SetPending(ictx.cpua, ExceptionType::kHardFault);
    }
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Branch condition
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 205
   */
  static Result<ExecResult> BCond(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                  const u32 &imm32, const u8 &cond) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    const auto condition_passed = It::ConditionPassed(ictx.cpua, cond);
    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    Pc::BranchWritePC(ictx.cpua, pc + imm32);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Table branch
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 416
   */
  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Tbhh(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_m, TArg1 arg_n) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const bool is_tbh = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kTbh)) != 0U;

    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    me_adr_t halfwords = 0U;
    if (is_tbh) {
      me_adr_t adr = (rn + Alu32::LSL(rm, 1));
      TRY_ASSIGN(out, ExecResult,
                 ictx.bus.template ReadOrRaise<u16>(ictx.cpua, adr,
                                                    BusExceptionType::kRaisePreciseDataBusError));
      halfwords = out;
    } else {
      me_adr_t adr = rn + rm;
      TRY_ASSIGN(out, ExecResult,
                 ictx.bus.template ReadOrRaise<u8>(ictx.cpua, adr,
                                                   BusExceptionType::kRaisePreciseDataBusError));
      halfwords = out;
    }

    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    Pc::BranchWritePC(ictx.cpua, pc + (halfwords << 1U));

    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Compare branch
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 216
   */
  template <typename TArg0>
  static Result<ExecResult> CbNZ(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const u32 &imm32) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    const bool is_non_zero = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kNonZero)) != 0U;

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());

    ExecFlagsSet eflags{0x0U};

    const u32 new_pc = pc + imm32;
    if ((rn == 0U) && (is_non_zero == false)) {
      Pc::BranchWritePC(ictx.cpua, new_pc);
    } else if ((rn != 0U) && (is_non_zero == true)) {
      Pc::BranchWritePC(ictx.cpua, new_pc);
    } else {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
    };

    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Bfi
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 208
   */
  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Bfi(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                const TArg0 &arg_d, const TArg1 &arg_n, const u8 &lsbit,
                                const u8 &msbit) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
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
      return Err<ExecResult>(StatusCode::kScExecutorUnpredictable);
    }

    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Ubfx
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 424
   */
  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Ubfx(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_d, const TArg1 &arg_n, const u8 &lsbit,
                                 const u8 &widthminus1) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const u8 msbit = lsbit + widthminus1;

    if (msbit <= 31U) {
      u32 msk = ((static_cast<u32>(1U) << (msbit - lsbit + 1U)) - static_cast<u32>(1U)) << lsbit;

      const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
      const auto result = (rn & msk) >> lsbit;
      ictx.cpua.WriteRegister(arg_d.Get(), result);
    } else {
      return Err<ExecResult>(StatusCode::kScExecutorUnpredictable);
    }

    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Ldrd
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 257
   */
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Ldrd(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_t, const TArg1 &arg_t2, const TArg2 &arg_n,
                                 const u32 &imm32) {

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());

    const me_adr_t offset_addr = is_add == true ? rn + imm32 : rn - imm32;
    const me_adr_t address = is_index == true ? offset_addr : rn;

    // Read from address
    TRY_ASSIGN(data, ExecResult,
               ictx.bus.template ReadOrRaise<u32>(ictx.cpua, address,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    TRY_ASSIGN(data2, ExecResult,
               ictx.bus.template ReadOrRaise<u32>(ictx.cpua, address + 0x4U,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    if (is_wback == true) {
      ictx.cpua.WriteRegister(arg_n.Get(), offset_addr);
    }

    ictx.cpua.WriteRegister(arg_t.Get(), data);
    ictx.cpua.WriteRegister(arg_t2.Get(), data2);

    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Umull
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 257
   */
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Umull(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                  const TArg0 &arg_d_lo, const TArg0 &arg_d_hi, const TArg1 &arg_n,
                                  const TArg2 &arg_m) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());

    const u64 result = (static_cast<u64>(rn) * static_cast<u64>(rm));
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);
    ictx.cpua.WriteRegister(arg_d_lo.Get(), result_lo);
    ictx.cpua.WriteRegister(arg_d_hi.Get(), result_hi);
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }
  /**
   * @brief Umlal
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 434
   */
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Umlal(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                  const TArg0 &arg_d_lo, const TArg0 &arg_d_hi, const TArg1 &arg_n,
                                  const TArg2 &arg_m) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const u32 rdhi = ictx.cpua.ReadRegister(arg_d_hi.Get());
    const u32 rdlo = ictx.cpua.ReadRegister(arg_d_lo.Get());
    const u32 rd = static_cast<u64>(rdhi) << 32U | static_cast<u64>(rdlo);

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());

    const u64 result = (static_cast<u64>(rn) * static_cast<u64>(rm)) + rd;
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);
    ictx.cpua.WriteRegister(arg_d_lo.Get(), result_lo);
    ictx.cpua.WriteRegister(arg_d_hi.Get(), result_hi);
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Msr
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 677
   */
  template <typename TArg0>
  static Result<ExecResult> Msr(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                const TArg0 &arg_n, const uint8_t mask, const uint8_t SYSm) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
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

    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Mrs
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 675
   */
  template <typename TArg0>
  static Result<ExecResult> Mrs(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                const TArg0 &arg_d, const uint8_t mask, const uint8_t SYSm) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
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
    ictx.cpua.WriteRegister(arg_d.Get(), rd_val);

    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Smull
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 372
   */
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Smull(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                  const TArg0 &arg_d_lo, const TArg0 &arg_d_hi, const TArg1 &arg_n,
                                  const TArg2 &arg_m) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = static_cast<i32>(ictx.cpua.ReadRegister(arg_n.Get()));
    const auto rm = static_cast<i32>(ictx.cpua.ReadRegister(arg_m.Get()));
    const u64 result = static_cast<u64>(static_cast<i64>(rn) * static_cast<i64>(rm));
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);
    ictx.cpua.WriteRegister(arg_d_lo.Get(), result_lo);
    ictx.cpua.WriteRegister(arg_d_hi.Get(), result_hi);
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
  }

  /**
   * @brief Strd
   *
   * see Armv7-M Architecture Reference Manual Issue E.e p. 393
   */
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Strd(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_t, const TArg1 &arg_t2, const TArg2 &arg_n,
                                 const u32 &imm32) {

    ExecFlagsSet eflags{0x0U};
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());

    const u32 off_adr = is_add == true ? rn + imm32 : rn - imm32;
    const u32 address = is_index == true ? off_adr : rn;

    const auto rt = ictx.cpua.ReadRegister(arg_t.Get());
    TRY(ExecResult, ictx.bus.template WriteOrRaise<u32>(
                        ictx.cpua, address, rt, BusExceptionType::kRaisePreciseDataBusError));

    //---
    const auto rt2 = ictx.cpua.ReadRegister(arg_t2.Get());
    TRY(ExecResult,
        ictx.bus.template WriteOrRaise<u32>(ictx.cpua, address + 0x4, rt2,
                                            BusExceptionType::kRaisePreciseDataBusError));

    if (is_wback == true) {
      ictx.cpua.WriteRegister(arg_n.Get(), off_adr);
    }
    //---

    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
    return Ok(ExecResult{eflags});
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
  SpecialInstr(const SpecialInstr &r_src) = default;

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

} // namespace internal
} // namespace libmicroemu