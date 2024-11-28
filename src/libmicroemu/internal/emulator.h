#pragma once

#include "libmicroemu/internal/bus/bus.h"
#include "libmicroemu/internal/bus/endianess_converters.h"
#include "libmicroemu/internal/bus/mem/mem_map_rw.h"
#include "libmicroemu/internal/bus/mem/mem_ro.h"
#include "libmicroemu/internal/bus/mem/mem_rw.h"
#include "libmicroemu/internal/bus/mem/mem_rw_optional.h"
#include "libmicroemu/internal/cpu_accessor.h"
#include "libmicroemu/internal/cpu_ops.h"
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/delegates.h"
#include "libmicroemu/internal/executor/executor.h"
#include "libmicroemu/internal/fetcher/fetcher.h"
#include "libmicroemu/internal/logic/exceptions_ops.h"
#include "libmicroemu/internal/logic/if_then_ops.h"
#include "libmicroemu/internal/logic/pc_ops.h"
#include "libmicroemu/internal/logic/reg_ops.h"
#include "libmicroemu/internal/logic/reset_logic.h"
#include "libmicroemu/internal/logic/spec_reg_ops.h"
#include "libmicroemu/internal/peripherals/sys_ctrl_block.h"
#include "libmicroemu/internal/peripherals/sys_tick.h"
#include "libmicroemu/internal/processor/processor.h"
#include "libmicroemu/internal/processor/step_flags.h"
#include "libmicroemu/internal/semihosting/semihosting.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {
template <typename TCpuStates> class Emulator {
  // Forward declarations
  class ExceptionTrigger;
  class ExceptionReturn;

public:
  // Aliases for register operations
  using SpecRegOps = SpecRegOps<TCpuStates, StaticLogger>;
  using RegOps = RegOps<TCpuStates, SpecRegOps, StaticLogger>;

  using CpuAccessor = CpuAccessor<TCpuStates, RegOps, SpecRegOps>;

  // Aliases for peripherals
  using SysCtrlBlock = SysCtrlBlock<CpuAccessor, StaticLogger>;
  using SysTick = SysTick<CpuAccessor, ExceptionTrigger, StaticLogger>;

  // Aliases for bus clients
  using EndConv = LittleToLittleEndianConverter;
  using Flash = MemRo<0U, CpuAccessor, EndConv>;
  using Ram0 = MemRw<1U, CpuAccessor, EndConv>;
  using Ram1 = MemRwOptional<2U, CpuAccessor, EndConv>;

  // clang-format off
  using Peripherals = MemMapRw<3U, 0xE0000000U, 0xFFFFU, 
      CpuAccessor, ExceptionTrigger, StaticLogger,

      // Peripherals
      SysCtrlBlock, 
      SysTick
  >;
  // clang-format on

  // clang-format off
  using Bus = Bus<
      CpuAccessor, ExceptionTrigger, StaticLogger, 
      
      // Bus clients
      Flash,
      Ram0, 
      Ram1, 
      Peripherals
   >;
  // clang-format on

  // Semihosting modules
  using Semihosting = Semihosting<CpuAccessor, Bus, StaticLogger>;

  // Aliases for advance processor operations
  using PcOps = PcOps<CpuAccessor, Bus, ExceptionReturn, StaticLogger>;
  using ExcOps = ExceptionsOps<CpuAccessor, PcOps, StaticLogger>;
  using ItOps = IfThenOps<CpuAccessor>;
  using CpuOps = CpuOps<ItOps, PcOps, ExcOps, ExceptionTrigger>;

  // aliases for uC steps
  using Fetcher = Fetcher<CpuAccessor, Bus>;
  using Decoder = Decoder<CpuAccessor, ItOps>;
  using Executor = Executor<CpuAccessor, Bus, CpuOps, StaticLogger>;

  // Processor specific classes
  using ResetLogic = ResetLogic<CpuAccessor, Bus, CpuOps, StaticLogger>;
  using Processor = Processor<CpuAccessor, Bus, CpuOps, Fetcher, Decoder, Executor, StaticLogger>;

  Emulator(TCpuStates &cpu_states) : cpu_states_(cpu_states) {}

  void SetFlashSegment(const u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) {
    flash_ = seg_ptr;
    flash_size_ = seg_size;
    flash_vadr_ = seg_vadr;
  }

  void SetRam1Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) {
    ram1_ = seg_ptr;
    ram1_size_ = seg_size;
    ram1_vadr_ = seg_vadr;
  }

  void SetRam2Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) {
    ram2_ = seg_ptr;
    ram2_size_ = seg_size;
    ram2_vadr_ = seg_vadr;
  }

  Bus BuildBus() {
    Flash code_access(const_cast<u8 *>(flash_), flash_size_, flash_vadr_);
    Ram0 rw_mem_access(ram1_, ram1_size_, ram1_vadr_);
    Ram1 rw_stack_access(ram2_, ram2_size_, ram2_vadr_);
    Peripherals peripheral_access;

    Bus bus(code_access, rw_mem_access, rw_stack_access, peripheral_access);
    return bus;
  }

  Result<void> Reset() {
    auto bus = BuildBus();
    auto &cpua = static_cast<CpuAccessor &>(cpu_states_);

    auto res = ResetLogic::TakeReset(cpua, bus);
    TRY(void, res);
    return Ok();
  }

  void SetEntryPoint(u32 entry_point) {
    const auto aligned_entry_point = entry_point & (~0x1U);

    LOG_INFO(StaticLogger, "Overwrite entry point to 0x%X", aligned_entry_point);
    auto &cpua = static_cast<CpuAccessor &>(cpu_states_);
    PcOps::BranchTo(cpua, aligned_entry_point);
  }

  ExecResult Exec(i64 instr_limit, FPreExecStepCallback cb_pre_exec,
                  FPostExecStepCallback cb_post_exec) {
    auto bus = BuildBus();
    auto &cpua = static_cast<CpuAccessor &>(cpu_states_);
    auto semihosting = Semihosting(cpua, bus);

    u64 instr_count{0U};
    bool is_instr_limit = instr_limit > 0;
    u64 u_instr_limit = static_cast<u64>(instr_limit);

    Delegates delegates(
        cb_pre_exec, cb_post_exec,
        [&semihosting](const u32 &imm32) -> Result<BkptFlagsSet> {
          return semihosting.Call(imm32);
        },
        [](const u32 &imm32) -> Result<SvcFlagsSet> {
          SvcFlagsSet svc_flags{0U};

          if (imm32 == 0x1U) { // A supervisor call to exit the emulator
            svc_flags |= static_cast<SvcFlagsSet>(SvcFlags::kOmitException);
            svc_flags |= static_cast<SvcFlagsSet>(SvcFlags::kRequestExit);
            return Ok<SvcFlagsSet>(svc_flags);
          }

          return Ok<SvcFlagsSet>(svc_flags);
        });

    while (true) {
      const auto step_ret = Processor::Step(cpua, bus, delegates);
      if (step_ret.IsErr()) {
        return ExecResult(step_ret.status_code, EXIT_FAILURE);
      }

      const auto step_flags = step_ret.content;
      if (step_flags & static_cast<StepFlagsSet>(StepFlags::kStepTerminationRequest)) {
        return ExecResult(StatusCode::kSuccess, semihosting.GetExitStatusCode());
      }

      const auto systick_ret = SysTick::Step(cpua);
      if (systick_ret.IsErr()) {
        return ExecResult(systick_ret.status_code, EXIT_FAILURE);
      }

      ++instr_count;
      if (is_instr_limit && instr_count >= u_instr_limit) {
        return ExecResult(StatusCode::kMaxInstructionsReached, EXIT_SUCCESS);
      }
    }

    // Should never reach here
    return ExecResult(StatusCode::kUnexpected, EXIT_FAILURE);
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

  const u8 *flash_{nullptr};
  me_size_t flash_size_{0U};
  me_adr_t flash_vadr_{0U};

  u8 *ram1_{nullptr};
  me_size_t ram1_size_{0U};
  me_adr_t ram1_vadr_{0U};

  u8 *ram2_{nullptr};
  me_size_t ram2_size_{0U};
  me_adr_t ram2_vadr_{0U};

  TCpuStates &cpu_states_;
};

} // namespace libmicroemu::internal
