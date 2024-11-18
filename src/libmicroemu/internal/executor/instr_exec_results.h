#pragma once
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

using InstrExecFlagsSet = u8;

enum class InstrExecFlags : InstrExecFlagsSet {
  kBkptReqExit = 1U << 0U,
  kSvcReqExit = 1U << 1U,
  kBkptReqErrorExit = 1U << 2U,
  kSvcReqErrorExit = 1U << 3U
};

static constexpr InstrExecFlagsSet kExitFlagsMask =
    static_cast<InstrExecFlagsSet>(InstrExecFlags::kSvcReqExit) |
    static_cast<InstrExecFlagsSet>(InstrExecFlags::kBkptReqExit) |
    static_cast<InstrExecFlagsSet>(InstrExecFlags::kBkptReqErrorExit) |
    static_cast<InstrExecFlagsSet>(InstrExecFlags::kSvcReqErrorExit);

struct InstrExecResult {
  InstrExecFlagsSet flags;
};

} // namespace internal
} // namespace libmicroemu