#pragma once

#include "libmicroemu/internal/logic/predicates.h"
#include "libmicroemu/internal/logic/reg_access.h"
#include "libmicroemu/internal/logic/spec_reg_access.h"
#include "libmicroemu/internal/processor/step_flags.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/trace/intstr_to_mnemonic.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/machine.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"
#include <assert.h>
#include <cstddef>
#include <cstdint>

namespace libmicroemu::internal {

template <typename TCpuAccessor, typename TBus, typename TProcessorOps, typename TFetcher,
          typename TDecoder, typename TExecutor, typename TLogger = NullLogger>
class Processor {
public:
  using It = typename TProcessorOps::It;
  using Exc = typename TProcessorOps::Exc;
  using Pc = typename TProcessorOps::Pc;
  using ExcTrig = typename TProcessorOps::ExcTrig;

  // Check if execution mode is thumb .. if not, raise usage fault and return true
  static bool IsThumbModeOrRaise(TCpuAccessor &cpua) {
    const bool is_thumb = Predicates::IsThumbMode(cpua);

    // if EPSR.T == 0, a UsageFault(‘Invalid State’) is taken
    if (!is_thumb) {
      auto cfsr = cpua.template ReadRegister<SpecialRegisterId::kCfsr>();
      cfsr |= CfsrUsageFault::kInvStateMsk;
      cpua.template WriteRegister<SpecialRegisterId::kCfsr>(cfsr);
      ExcTrig::SetPending(cpua, ExceptionType::kUsageFault);
      return false;
    }
    return true;
  }

  template <typename TDelegates>
  static Result<StepFlagsSet> Step(TCpuAccessor &cpua, TBus &bus, TDelegates &delegates) {
    static constexpr u8 kRaw32BitMsk = static_cast<RawInstrFlagsSet>(RawInstrFlagsMsk::k32Bit);

    StepFlagsSet step_flags{0U};

    // *** FETCH ***
    // The pc points to the current instruction +4 . Therefore decrement 4 to
    // get the current address.

    // Get the current program counter
    const me_adr_t pc = static_cast<me_adr_t>(cpua.template ReadRegister<RegisterId::kPc>());
    me_adr_t pc_this_instr = static_cast<me_adr_t>(pc - 4U);

    // Check for exceptions raised by peripherals before fetching the instruction
    const auto exc_ctx_pre_fetch = ExceptionContext{pc_this_instr};
    TRY_ASSIGN(if_pre_fetch_exception, StepFlagsSet,
               Exc::template CheckExceptions<ExceptionPreFetch>(cpua, bus, exc_ctx_pre_fetch));

    // If an asynchronous exception should be executed, the pc must be updated
    // to the new instruction address.
    if (if_pre_fetch_exception) {
      const me_adr_t pc = static_cast<me_adr_t>(cpua.template ReadRegister<RegisterId::kPc>());
      pc_this_instr = static_cast<me_adr_t>(pc - 4U);
    }

    RawInstr raw_instr;

    // Check if execution mode is thumb .. if not, raise usage fault and continue
    if (IsThumbModeOrRaise(cpua)) {
      auto r_raw_instr = TFetcher::Fetch(cpua, bus, pc_this_instr);
      if (r_raw_instr.IsErr()) {
        ErrorHandler(cpua, r_raw_instr, bus);
        return Err<RawInstr, StepFlagsSet>(r_raw_instr);
      }
      raw_instr = r_raw_instr.content;
    }

    const auto exc_ctx_post_fetch = ExceptionContext{pc_this_instr};

    // If an invalid state exception was raised, this will processed here
    // Check for exceptions after fetching the instruction
    TRY_ASSIGN(is_fetch_exception, StepFlagsSet,
               Exc::template CheckExceptions<ExceptionPostFetch>(cpua, bus, exc_ctx_post_fetch));

    if (is_fetch_exception) {
      // special case: if the instruction fetch raises an exception, this cycle is
      // considered as a NOP cycle.

      // The pc was updated in the exception handler
      step_flags |= static_cast<StepFlagsSet>(StepFlags::kStepOk);
      return Ok<StepFlagsSet>(step_flags);
    }

    // *** DECODE ***
    auto r_instr = TDecoder::Decode(cpua, raw_instr);
    if (r_instr.IsErr()) {
      ErrorHandler(cpua, r_instr, bus);
      return Err<Instr, StepFlagsSet>(r_instr);
    }
    const auto &instr = r_instr.content;

    // Decoder can not raise any exceptions
#ifndef NDEBUG
    static constexpr u8 k32BitMsk = static_cast<InstrFlagsSet>(InstrFlags::k32Bit);
    // check if the raw instruction and the decoded instruction have the same width
    assert(((((raw_instr.flags & kRaw32BitMsk) != 0U) && ((instr.nop.flags & k32BitMsk) != 0U)) ||
            (((raw_instr.flags & kRaw32BitMsk) == 0U) && ((instr.nop.flags & k32BitMsk) == 0U))));
#endif

    // *** CALLBACK ***
    if (delegates.IsPreExecSet()) {
      const auto is_32bit = (raw_instr.flags & kRaw32BitMsk) == kRaw32BitMsk;
      auto op_code = OpCode{raw_instr.low, raw_instr.high, is_32bit};
      auto instr_to_mnemonic = InstrToMnemonic<TCpuAccessor, It>(cpua, instr);
      auto reg_access = RegAccessor(cpua);
      auto spec_reg_access = SpecialRegAccessor(cpua);

      auto emu_ctx =
          EmuContext(pc_this_instr, op_code, instr_to_mnemonic, reg_access, spec_reg_access);
      delegates.PreExec(emu_ctx);
    }

    // *** EXECUTE ***
    const auto r_execute = TExecutor::Execute(cpua, bus, instr, delegates);
    if (r_execute.IsErr()) {
      ErrorHandler(cpua, r_execute, bus);
      return Err<InstrExecResult, StepFlagsSet>(r_execute);
    }

    if (delegates.IsPostExecSet()) {
      const auto is_32bit = (raw_instr.flags & kRaw32BitMsk) == kRaw32BitMsk;
      auto op_code = OpCode{raw_instr.low, raw_instr.high, is_32bit};
      auto instr_to_mnemonic = InstrToMnemonic<TCpuAccessor, It>(cpua, instr);
      auto reg_access = RegAccessor(cpua);
      auto spec_reg_access = SpecialRegAccessor(cpua);
      auto emu_ctx =
          EmuContext(pc_this_instr, op_code, instr_to_mnemonic, reg_access, spec_reg_access);
      delegates.PostExec(emu_ctx);
    }

    // *** Exit Conditions ***
    const auto &eflags = r_execute.content.flags;

    // all exit flags are mutually exclusive
    if ((eflags & kExitFlagsMask) != 0U) {
      if ((eflags & static_cast<InstrExecFlagsSet>(InstrExecFlags::kBkptReqExit)) != 0U) {
        step_flags |= static_cast<StepFlagsSet>(StepFlags::kStepTerminationRequest);
        return Ok<StepFlagsSet>(step_flags);
      }
      if ((eflags & static_cast<InstrExecFlagsSet>(InstrExecFlags::kSvcReqExit)) != 0U) {
        step_flags |= static_cast<StepFlagsSet>(StepFlags::kStepTerminationRequest);
        return Ok<StepFlagsSet>(step_flags);
      }
      if ((eflags & static_cast<InstrExecFlagsSet>(InstrExecFlags::kBkptReqErrorExit)) != 0U) {
        return Err<StepFlagsSet>(StatusCode::kExecutorExitWithError);
      }
      if ((eflags & static_cast<InstrExecFlagsSet>(InstrExecFlags::kSvcReqErrorExit)) != 0U) {
        return Err<StepFlagsSet>(StatusCode::kExecutorExitWithError);
      }
    }

    // get the next instruction address
    const auto exc_ctc_post_exec = ExceptionContext{pc_this_instr};

    TRY(StepFlagsSet,
        Exc::template CheckExceptions<ExceptionPostExecution>(cpua, bus, exc_ctc_post_exec));

    step_flags |= static_cast<StepFlagsSet>(StepFlags::kStepOk);
    return Ok<StepFlagsSet>(step_flags);
  }

  template <typename TResult>
  static void ErrorHandler(TCpuAccessor &cpua, const TResult &res, const TBus &bus) {
    printf("ERROR: Emulator panic - StatusCode: %s(%i)\n", res.ToString().data(),
           static_cast<u32>(res.status_code));

    // error return
    const me_adr_t pc = static_cast<me_adr_t>(cpua.template ReadRegister<RegisterId::kPc>());
    me_adr_t pc_this_instr = static_cast<me_adr_t>(pc - 0x4U);
    printf(" # System state:\n");
    printf("   Actual PC: 0x%x\n\n", pc_this_instr);
    printf(" # Memory dump from PC:\n");

    MemoryViewer<TCpuAccessor, TBus>::Print(cpua, bus, pc_this_instr, 32U, 3U);
  }

private:
  Processor() = delete;
  ~Processor() = delete;
  Processor(Processor &r_src) = delete;
  Processor &operator=(const Processor &r_src) = delete;
  Processor(Processor &&r_src) = delete;
  Processor &operator=(Processor &&r_src) = delete;
};

} // namespace libmicroemu::internal