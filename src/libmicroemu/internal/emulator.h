#pragma once

#include "libmicroemu/emu_flags.h"
#include "libmicroemu/internal/bus/bus.h"
#include "libmicroemu/internal/bus/endianess_converters.h"
#include "libmicroemu/internal/bus/mem/mem_map_rw.h"
#include "libmicroemu/internal/bus/mem/mem_ro.h"
#include "libmicroemu/internal/bus/mem/mem_rw.h"
#include "libmicroemu/internal/bus/mem/mem_rw_optional.h"
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/delegates.h"
#include "libmicroemu/internal/executor/executor.h"
#include "libmicroemu/internal/fetcher/fetcher.h"
#include "libmicroemu/internal/logic/exceptions_ops.h"
#include "libmicroemu/internal/logic/if_then_ops.h"
#include "libmicroemu/internal/logic/pc_ops.h"
#include "libmicroemu/internal/logic/reg_ops.h"
#include "libmicroemu/internal/logic/spec_reg_ops.h"
#include "libmicroemu/internal/peripherals/sys_ctrl_block.h"
#include "libmicroemu/internal/peripherals/sys_tick.h"
#include "libmicroemu/internal/processor/processor.h"
#include "libmicroemu/internal/processor/step_flags.h"
#include "libmicroemu/internal/semihosting/semihosting.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu::internal {
template <typename TProcessorStates> class Emulator {
  // Forward declarations
  class ExceptionTrigger;
  class ExceptionReturn;

public:
  using ProcessorStates = TProcessorStates;

  // Aliases for register operations
  using SpecRegOps = SpecRegOps<TProcessorStates, StaticLogger>;
  using RegOps = RegOps<TProcessorStates, SpecRegOps, StaticLogger>;

  // Aliases for peripherals
  using SysCtrlBlock = SysCtrlBlock<TProcessorStates, SpecRegOps, StaticLogger>;
  using SysTick = SysTick<TProcessorStates, SpecRegOps, ExceptionTrigger, StaticLogger>;

  // Aliases for bus clients
  using EndConv = LittleToLittleEndianConverter;
  using Flash = MemRo<0U, TProcessorStates, EndConv>;
  using Ram0 = MemRw<1U, TProcessorStates, EndConv>;
  using Ram1 = MemRwOptional<2U, TProcessorStates, EndConv>;

  // clang-format off
  using Peripherals = MemMapRw<3U, 0xE0000000U, 0xFFFFU, 
      TProcessorStates, SpecRegOps, ExceptionTrigger, StaticLogger,

      // Peripherals
      SysCtrlBlock, 
      SysTick
  >;
  // clang-format on

  // clang-format off
  using Bus = Bus<
      TProcessorStates, SpecRegOps, ExceptionTrigger, StaticLogger, 
      
      // Bus clients
      Flash,
      Ram0, 
      Ram1, 
      Peripherals
   >;

  // clang-format on

  // Aliases for advance processor operations
  using PcOps = PcOps<TProcessorStates, Bus, RegOps, SpecRegOps, ExceptionReturn, StaticLogger>;
  using ExcOps = ExceptionsOps<TProcessorStates, RegOps, SpecRegOps, PcOps, StaticLogger>;
  using ItOps = IfThenOps<TProcessorStates, SpecRegOps>;

  using Semihosting = Semihosting<TProcessorStates, Bus, RegOps, SpecRegOps, StaticLogger>;

  // aliases for uC steps
  using Fetcher = Fetcher<TProcessorStates, Bus>;
  using Decoder = Decoder<TProcessorStates, SpecRegOps, ItOps>;
  using Executor = Executor<TProcessorStates, Bus, RegOps, SpecRegOps, PcOps, ItOps,
                            ExceptionTrigger, StaticLogger>;

  // aliases for processor
  using ProcessorOps = ProcessorOps<RegOps, SpecRegOps, ItOps, PcOps, ExcOps, ExceptionTrigger>;
  using Processor =
      Processor<TProcessorStates, Bus, ProcessorOps, Fetcher, Decoder, Executor, StaticLogger>;

  Emulator(TProcessorStates &pstates) : pstates_(pstates) {}

  void SetCodeSection(const u8 *section_ptr, me_size_t section_size) {
    code_ = section_ptr;
    code_size_ = section_size;
  }

  void SetRam1Section(u8 *section_ptr, me_size_t section_size, me_adr_t vadr) {
    ram1_ = section_ptr;
    ram1_size_ = section_size;
    ram1_vadr_ = vadr;
  }

  void SetRam2Section(u8 *section_ptr, me_size_t section_size, me_adr_t vadr) {
    ram2_ = section_ptr;
    ram2_size_ = section_size;
    ram2_vadr_ = vadr;
  }

  Bus BuildBus() {
    Flash code_access(const_cast<u8 *>(code_), code_size_, 0x0U);
    Ram0 rw_mem_access(ram1_, ram1_size_, ram1_vadr_);
    Ram1 rw_stack_access(ram2_, ram2_size_, ram2_vadr_);
    Peripherals peripheral_access;

    Bus bus(code_access, rw_mem_access, rw_stack_access, peripheral_access);
    return bus;
  }

  Result<void> Reset() {
    auto bus = BuildBus();
    auto res = Processor::TakeReset(pstates_, bus);
    TRY(void, res);
    return Ok();
  }

  void SetEntryPoint(u32 entry_point) {
    const auto aligned_entry_point = entry_point & (~0x1U);

    LOG_INFO(StaticLogger, "Overwrite entry point to 0x%X", aligned_entry_point);
    PcOps::BranchTo(pstates_, aligned_entry_point);
  }

  Result<EmuResult> Exec(i32 instr_limit, FPreExecStepCallback cb_pre_exec,
                         FPostExecStepCallback cb_post_exec) {
    // TODO: Semihosting should be part of application??
    auto bus = BuildBus();
    auto semihosting = Semihosting(pstates_, bus);

    u32 iter{0U};
    bool is_instr_limit = instr_limit > 0;
    u32 u_instr_limit = static_cast<u32>(instr_limit);

    // TODO: Introduce Exception delegate and Error delegate
    Delegates delegates(
        cb_pre_exec, cb_post_exec,
        [&semihosting](const u32 &imm32) -> Result<BkptFlagsSet> {
          return semihosting.Call(imm32);
        },
        [](const u32 &imm32) -> Result<SvcFlagsSet> {
          SvcFlagsSet svc_flags{0U};

          // TODO: Make this configurable
          if (imm32 == 0x1U) { // RESET
            svc_flags |= static_cast<SvcFlagsSet>(SvcFlags::kOmitException);
            svc_flags |= static_cast<SvcFlagsSet>(SvcFlags::kRequestExit);
            return Ok<SvcFlagsSet>(svc_flags);
          }

          return Ok<SvcFlagsSet>(svc_flags);
        });

    while (true) {
      EmuFlagsSet emu_flags{0U};

      auto ret = Processor::Step(pstates_, bus, delegates);
      TRY_ASSIGN(step_flags, EmuResult, ret);

      if (step_flags & static_cast<StepFlagsSet>(StepFlags::kStepTerminationRequest)) {
        emu_flags |= static_cast<EmuFlagsSet>(EmuFlags::kEmuTerminated);
        return Ok(EmuResult{emu_flags, semihosting.GetExitStatusCode()});
      }
      TRY(EmuResult, SysTick::Step(pstates_));

      ++iter;

      if (is_instr_limit && iter >= u_instr_limit) {
        emu_flags |= static_cast<EmuFlagsSet>(EmuFlags::kEmuMaxInstructionsReached);
        return Ok(EmuResult{emu_flags, 0U});
      }
    }
    return Err<EmuResult>(StatusCode::kScUnexpected);
  }

private:
  // -------------------------------------------------
  class ExceptionTrigger {
  public:
    template <typename... Args> static void SetPending(Args &&...args) {
      ExcOps::SetExceptionPending(std::forward<Args>(args)...);
    }

    ExceptionTrigger() = delete;
    ~ExceptionTrigger() = delete;
    ExceptionTrigger(const ExceptionTrigger &) = delete;
    ExceptionTrigger(ExceptionTrigger &&) = delete;

    auto &operator=(const ExceptionTrigger &) = delete;
    auto &operator=(ExceptionTrigger &&) = delete;
  };

  // -------------------------------------------------
  class ExceptionReturn {
  public:
    template <typename... Args> static Result<void> Return(Args &&...args) {
      return ExcOps::ExceptionReturn(std::forward<Args>(args)...);
    }

    ExceptionReturn() = delete;
    ~ExceptionReturn() = delete;
    ExceptionReturn(const ExceptionReturn &) = delete;
    ExceptionReturn(ExceptionReturn &&) = delete;

    auto &operator=(const ExceptionReturn &) = delete;
    auto &operator=(ExceptionReturn &&) = delete;
  };

  const u8 *code_{nullptr};
  me_size_t code_size_{0U};

  u8 *ram1_{nullptr};
  me_size_t ram1_size_{0U};
  me_adr_t ram1_vadr_{0U};

  u8 *ram2_{nullptr};
  me_size_t ram2_size_{0U};
  me_adr_t ram2_vadr_{0U};

  TProcessorStates &pstates_;
};

} // namespace microemu::internal
