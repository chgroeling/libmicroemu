#pragma once

#include "libmicroemu/internal/logic/reg_access.h"
#include "libmicroemu/internal/logic/spec_reg_access.h"
#include "libmicroemu/internal/processor/step_flags.h"
#include "libmicroemu/internal/trace/intstr_to_mnemonic.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/microemu.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"
#include <assert.h>
#include <cstddef>
#include <cstdint>

namespace microemu {
namespace internal {

// todo: rename these to ops and separate the ops from the processor
template <typename TRegOps, typename TSpecRegOps, typename TItOps, typename TPcOps,
          typename TExcOps, typename TExcTrig>
class ProcessorOps {
public:
  using Reg = TRegOps;
  using SReg = TSpecRegOps;
  using It = TItOps;
  using Pc = TPcOps;
  using Exc = TExcOps;
  using ExcTrig = TExcTrig;
};

template <typename TProcessorStates, typename TBus, typename TProcessorOps, typename TFetcher,
          typename TDecoder, typename TExecutor, typename TLogger = NullLogger>
class Processor {
public:
  using Reg = typename TProcessorOps::Reg;
  using SReg = typename TProcessorOps::SReg;
  using It = typename TProcessorOps::It;
  using Exc = typename TProcessorOps::Exc;
  using Pc = typename TProcessorOps::Pc;
  using ExcTrig = typename TProcessorOps::ExcTrig;

  // Check if execution mode is thumb .. if not, raise usage fault and return true
  static bool IsThumbModeOrRaise(TProcessorStates &pstates) {
    const auto epsr = SReg::template ReadRegister<SpecialRegisterId::kEpsr>(pstates);
    const bool is_thumb = (epsr & static_cast<u32>(EpsrRegister::kTMsk)) != 0u;

    // if EPSR.T == 0, a UsageFault(‘Invalid State’) is taken
    if (!is_thumb) {
      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      cfsr |= CfsrUsageFault::kInvStateMsk;
      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);
      ExcTrig::SetPending(pstates, ExceptionType::kUsageFault);
      return false;
    }
    return true;
  }

  template <typename TDelegates>
  static Result<StepFlagsSet> Step(TProcessorStates &pstates, TBus &bus, TDelegates &delegates) {
    static constexpr u8 kRaw32BitMsk = static_cast<RawInstrFlagsSet>(RawInstrFlagsMsk::k32Bit);

    StepFlagsSet step_flags{0u};

    // *** FETCH ***
    // The pc points to the current instruction +4 . Therefore decrement 4 to
    // get the current address.

    // Get the current program counter
    me_adr_t pc_this_instr = static_cast<me_adr_t>(Reg::ReadPC(pstates) - 4u);

    // Check for exceptions raised by peripherals before fetching the instruction
    const auto exc_ctx_pre_fetch = ExceptionContext{pc_this_instr};
    TRY_ASSIGN(if_pre_fetch_exception, StepFlagsSet,
               Exc::template CheckExceptions<ExceptionPreFetch>(pstates, bus, exc_ctx_pre_fetch));

    // If an asynchronous exception should be executed, the pc must be updated
    // to the new instruction address.
    if (if_pre_fetch_exception) {
      pc_this_instr = static_cast<me_adr_t>(Reg::ReadPC(pstates) - 4u);
    }

    RawInstr raw_instr;

    // Check if execution mode is thumb .. if not, raise usage fault and continue
    if (IsThumbModeOrRaise(pstates)) {
      auto r_raw_instr = TFetcher::Fetch(pstates, bus, pc_this_instr);
      if (r_raw_instr.IsErr()) {
        ErrorHandler(pstates, r_raw_instr, bus);
        return Err<RawInstr, StepFlagsSet>(r_raw_instr);
      }
      raw_instr = r_raw_instr.content;
    }

    const auto exc_ctx_post_fetch = ExceptionContext{pc_this_instr};

    // If an invalid state exception was raised, this will processed here
    // Check for exceptions after fetching the instruction
    TRY_ASSIGN(is_fetch_exception, StepFlagsSet,
               Exc::template CheckExceptions<ExceptionPostFetch>(pstates, bus, exc_ctx_post_fetch));

    if (is_fetch_exception) {
      // special case: if the instruction fetch raises an exception, this cycle is
      // considered as a NOP cycle.

      // The pc was updated in the exception handler
      step_flags |= static_cast<StepFlagsSet>(StepFlags::kStepOk);
      return Ok<StepFlagsSet>(step_flags);
    }

    // *** DECODE ***
    auto r_instr = TDecoder::Decode(pstates, raw_instr);
    if (r_instr.IsErr()) {
      ErrorHandler(pstates, r_instr, bus);
      return Err<Instr, StepFlagsSet>(r_instr);
    }
    const auto &instr = r_instr.content;

    // Decoder can not raise any exceptions
#ifndef NDEBUG
    static constexpr u8 k32BitMsk = static_cast<InstrFlagsSet>(InstrFlags::k32Bit);
    // check if the raw instruction and the decoded instruction have the same width
    assert(((((raw_instr.flags & kRaw32BitMsk) != 0u) && ((instr.nop.flags & k32BitMsk) != 0u)) ||
            (((raw_instr.flags & kRaw32BitMsk) == 0u) && ((instr.nop.flags & k32BitMsk) == 0u))));
#endif

    // *** CALLBACK ***
    if (delegates.IsPreExecSet()) {
      const auto is_32bit = (raw_instr.flags & kRaw32BitMsk) == kRaw32BitMsk;
      auto op_code = OpCode{raw_instr.low, raw_instr.high, is_32bit};
      auto instr_to_mnemonic = InstrToMnemonic<TProcessorStates, It, Reg, SReg>(pstates, instr);
      auto reg_access = RegAccess<TProcessorStates, Reg, SReg>(pstates);
      auto spec_reg_access = SpecialRegAccess<TProcessorStates, SReg>(pstates);

      auto emu_ctx =
          EmuContext(pc_this_instr, op_code, instr_to_mnemonic, reg_access, spec_reg_access);
      delegates.PreExec(emu_ctx);
    }

    // *** EXECUTE ***
    const auto r_execute = TExecutor::Execute(pstates, bus, instr, delegates);

    if (r_execute.IsErr()) {
      ErrorHandler(pstates, r_execute, bus);
      return Err<ExecResult, StepFlagsSet>(r_execute);
    }

    if (delegates.IsPostExecSet()) {
      const auto is_32bit = (raw_instr.flags & kRaw32BitMsk) == kRaw32BitMsk;
      auto op_code = OpCode{raw_instr.low, raw_instr.high, is_32bit};
      auto instr_to_mnemonic = InstrToMnemonic<TProcessorStates, It, Reg, SReg>(pstates, instr);
      auto reg_access = RegAccess<TProcessorStates, Reg, SReg>(pstates);
      auto spec_reg_access = SpecialRegAccess<TProcessorStates, SReg>(pstates);
      auto emu_ctx =
          EmuContext(pc_this_instr, op_code, instr_to_mnemonic, reg_access, spec_reg_access);
      delegates.PostExec(emu_ctx);
    }

    // *** Exit Conditions ***
    const auto &execute_flags = r_execute.content.flags;

    // all exit flags are mutually exclusive
    if ((execute_flags & kExitFlagsMask) != 0u) {
      if ((execute_flags & static_cast<ExecFlagsSet>(ExecFlags::kBkptReqExit)) != 0u) {
        step_flags |= static_cast<StepFlagsSet>(StepFlags::kStepTerminationRequest);
        return Ok<StepFlagsSet>(step_flags);
      }
      if ((execute_flags & static_cast<ExecFlagsSet>(ExecFlags::kSvcReqExit)) != 0u) {
        step_flags |= static_cast<StepFlagsSet>(StepFlags::kStepTerminationRequest);
        return Ok<StepFlagsSet>(step_flags);
      }
      if ((execute_flags & static_cast<ExecFlagsSet>(ExecFlags::kBkptReqErrorExit)) != 0u) {
        return Err<StepFlagsSet>(StatusCode::kScExecutorExitWithError);
      }
      if ((execute_flags & static_cast<ExecFlagsSet>(ExecFlags::kSvcReqErrorExit)) != 0u) {
        return Err<StepFlagsSet>(StatusCode::kScExecutorExitWithError);
      }
    }

    // get the next instruction address
    const auto exc_ctc_post_exec = ExceptionContext{pc_this_instr};

    TRY(StepFlagsSet,
        Exc::template CheckExceptions<ExceptionPostExecution>(pstates, bus, exc_ctc_post_exec));

    step_flags |= static_cast<StepFlagsSet>(StepFlags::kStepOk);
    return Ok<StepFlagsSet>(step_flags);
  }

  template <typename TResult>
  static void ErrorHandler(TProcessorStates &pstates, const TResult &res, const TBus &bus) {
    printf("ERROR: Emulator panic - StatusCode: %s(%i)\n", res.ToString(),
           static_cast<u32>(res.status_code));

    // error return
    me_adr_t pc = static_cast<me_adr_t>(Reg::ReadPC(pstates)) - 0x4u;
    printf(" # System state:\n");
    printf("   Actual PC: 0x%x\n\n", pc);
    printf(" # Memory dump from PC:\n");
    MemoryViewer<TProcessorStates, TBus> mem_view(bus);
    mem_view.print(pstates, pc, 32u, 3u);
  }

  // TODO: move outside of the class
  static Result<void> TakeReset(TProcessorStates &pstates, TBus &bus_) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.531
    LOG_INFO(TLogger, "Resetting processor");

    auto sys_ctrl = SReg::template ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);

    // CurrentMode = Mode_Thread;
    sys_ctrl &= ~static_cast<u32>(SysCtrlRegister::kExecModeMsk);
    sys_ctrl |= static_cast<u32>(SysCtrlRegister::kExecModeThread);

    // PRIMASK<0> = '0';  /* priority mask cleared at reset */
    // FAULTMASK<0> = '0'; /* fault mask cleared at reset */
    // BASEPRI<7:0> = Zeros(8);  /* base priority disabled at reset */

    // if HaveFPExt() then /* initialize the Floating Point Extn */
    if (false) {
      // CONTROL<2:0> = '000';  /* FP inactive, stack is Main, thread is privileged */
      // CPACR.cp10 = '00';
      // CPACR.cp11 = '00';
      // FPDSCR.AHP = '0';
      // FPDSCR.DN = '0';
      // FPDSCR.FZ = '0';
      // FPDSCR.RMode = '00';
      // FPCCR.ASPEN = '1';
      // FPCCR.LSPEN = '1';
      // FPCCR.LSPACT = '0';
      // FPCAR = bits(32) UNKNOWN;
      // FPFSR = bits(32) UNKNOWN;
      // for i = 0 to 31
      //   S[i] = bits(32) UNKNOWN;
    } else {
      // CONTROL<1:0> = '00';    /* current stack is Main, thread is privileged */
      sys_ctrl &= ~static_cast<u32>(SysCtrlRegister::kControlSpSelMsk);
      sys_ctrl &= ~static_cast<u32>(SysCtrlRegister::kControlNPrivMsk);
    }

    SReg::template WriteRegister<SpecialRegisterId::kSysCtrl>(pstates, sys_ctrl);

    // for i = 0 to 511 /* all exceptions Inactive */
    //   ExceptionActive[i] = '0';
    Exc::InitDefaultExceptionStates(pstates);

    // ResetSCSRegs(); /* catch-all function for System Control Space reset */
    // ClearExclusiveLocal(ProcessorID()); /* Synchronization (LDREX* / STREX*) monitor support */
    // ClearEventRegister(); /* see WFE instruction for more details */

    // All registers are UNKNOWN
    // for i = 0 to 12
    //   R[i] = bits(32) UNKNOWN;

    // bits(32) vectortable = VTOR<31:7>:'0000000';
    const auto vtor = SReg::template ReadRegister<SpecialRegisterId::kVtor>(pstates);
    me_adr_t vectortable = vtor << 7 | 0x0u;

    //   SP_main = MemA_with_priv[vectortable, 4, AccType_VECTABLE] AND 0xFFFFFFFC<31:0>;
    TRY_ASSIGN(sp_main, void,
               bus_.template ReadOrRaise<u32>(pstates, vectortable,
                                              BusExceptionType::kRaisePreciseDataBusError));
    SReg::template WriteRegister<SpecialRegisterId::kSpMain>(pstates, sp_main);

    //   SP_process = ((bits(30) UNKNOWN):'00');
    auto sp_process = SReg::template ReadRegister<SpecialRegisterId::kSpProcess>(pstates);
    sp_process &= ~0x3u; // clear the two least significant bits
    SReg::template WriteRegister<SpecialRegisterId::kSpProcess>(pstates, sp_process);

    //   LR = 0xFFFFFFFF<31:0>; /* preset to an illegal exception return value */
    Reg::template WriteRegister<RegisterId::kLr>(pstates, 0xFFFFFFFFU);

    //   tmp = MemA_with_priv[vectortable+4, 4, AccType_VECTABLE];
    TRY_ASSIGN(tmp, void,
               bus_.template ReadOrRaise<u32>(pstates, vectortable + 0x4U,
                                              BusExceptionType::kRaisePreciseDataBusError));

    // tbit = tmp<0>;
    auto tbit = tmp & 0x1u;

    //   APSR = bits(32) UNKNOWN; /* flags UNPREDICTABLE from reset */

    //   IPSR<8:0> = Zeros(9); /* Exception Number cleared */
    auto ipsr = SReg::template ReadRegister<SpecialRegisterId::kIpsr>(pstates);
    ipsr &= ~static_cast<u32>(IpsrRegister::kExceptionNumberMsk);
    SReg::template WriteRegister<SpecialRegisterId::kIpsr>(pstates, ipsr);

    auto epsr = SReg::template ReadRegister<SpecialRegisterId::kEpsr>(pstates);
    //   EPSR.T = tbit; /* T bit set from vector */
    //   EPSR.IT<7:0> = Zeros(8); /* IT/ICI bits cleared */
    epsr &= ~static_cast<u32>(EpsrRegister::kItMsk) & // clear it bits
            ~static_cast<u32>(EpsrRegister::kTMsk);   // clear t bit
    epsr |= tbit << EpsrRegister::kTPos;
    SReg::template WriteRegister<SpecialRegisterId::kEpsr>(pstates, epsr);

    //   BranchTo(tmp AND 0xFFFFFFFE<31:0>); /* address of reset service routine */
    auto entry_point = tmp & 0xFFFFFFFEu;
    Pc::BranchTo(pstates, entry_point);
    LOG_DEBUG(TLogger, "Set entry Point to 0x%08X / tbit:%i", entry_point, tbit);

    // CSR.STKALIGN = '1'; <-- added as default
    /* stack alignment is 8-byte aligned */
    auto ccr = SReg::template ReadRegister<SpecialRegisterId::kCcr>(pstates);
    ccr |= static_cast<u32>(CcrRegister::kStkAlignMsk);
    SReg::template WriteRegister<SpecialRegisterId::kCcr>(pstates, ccr);
    LOG_TRACE(TLogger, "CSR: 0x%08X", ccr);

#if IS_LOGLEVEL_TRACE_ENABLED == true // LOG TRACE apsr, ipsr, epsr, xpsr
    {
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
      auto ipsr = SReg::template ReadRegister<SpecialRegisterId::kIpsr>(pstates);
      auto epsr = SReg::template ReadRegister<SpecialRegisterId::kEpsr>(pstates);
      auto xpsr = SReg::template ReadRegister<SpecialRegisterId::kXpsr>(pstates);
      LOG_DEBUG(TLogger, "APSR: 0x%08X, IPSR: 0x%08X, EPSR: 0x%08X, XPSR: 0x%08X", apsr, ipsr, epsr,
                xpsr);
    }
#endif
    return Ok();
  }

private:
  Processor() = delete;
  ~Processor() = delete;
  Processor(Processor &r_src) = delete;
  Processor &operator=(const Processor &r_src) = delete;
  Processor(Processor &&r_src) = delete;
  Processor &operator=(Processor &&r_src) = delete;
};

} // namespace internal
} // namespace microemu