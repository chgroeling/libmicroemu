#pragma once

#include "libmicroemu/internal/bkpt_flags.h"
#include "libmicroemu/internal/i_breakpoint.h"
#include "libmicroemu/internal/svc_flags.h"
#include "libmicroemu/microemu.h"
#include "libmicroemu/result.h"

namespace libmicroemu {
namespace internal {
class Delegates {
public:
  using PreExecDelegate = FPreExecStepCallback;
  using PostExecDelegate = FPostExecStepCallback;
  using BkptDelegate = std::function<Result<BkptFlagsSet>(const u32 &)>;
  using SvcDelegate = std::function<Result<SvcFlagsSet>(const u32 &)>;

  explicit Delegates(PreExecDelegate pre_exec_delegate, PostExecDelegate post_exec_delegate,
                     BkptDelegate bkpt_delegate, SvcDelegate svc_delegate)
      : pre_exec_delegate_(pre_exec_delegate), post_exec_delegate_(post_exec_delegate),
        bkpt_delegate_(bkpt_delegate), svc_delegate_(svc_delegate) {}

  Delegates() = delete;
  ~Delegates() = default;
  Delegates(const Delegates &) = default;
  Delegates &operator=(const Delegates &) = default;
  Delegates(Delegates &&) = default;
  Delegates &operator=(Delegates &&) = default;

  bool IsPreExecSet() { return pre_exec_delegate_ != nullptr; }
  bool IsPostExecSet() { return post_exec_delegate_ != nullptr; }
  bool IsBkptSet() { return bkpt_delegate_ != nullptr; }
  bool IsSvcSet() { return svc_delegate_ != nullptr; }

  void PreExec(EmuContext &emu_ctx) { pre_exec_delegate_(emu_ctx); }

  void PostExec(EmuContext &emu_ctx) { post_exec_delegate_(emu_ctx); }

  Result<BkptFlagsSet> Bkpt(const u32 &imm32) { return bkpt_delegate_(imm32); }
  Result<SvcFlagsSet> Svc(const u32 &imm32) { return svc_delegate_(imm32); }

private:
  PreExecDelegate pre_exec_delegate_{nullptr};
  PostExecDelegate post_exec_delegate_{nullptr};
  BkptDelegate bkpt_delegate_{nullptr};
  SvcDelegate svc_delegate_{nullptr};
};
} // namespace internal
} // namespace libmicroemu