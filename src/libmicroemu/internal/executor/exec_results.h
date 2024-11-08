#pragma once
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

using ExecFlagsSet = u8;

enum class ExecFlags : ExecFlagsSet {
  kBkptReqExit = 1U << 3U,
  kSvcReqExit = 1U << 4U,
  kBkptReqErrorExit = 1U << 5U,
  kSvcReqErrorExit = 1U << 6U
};

static constexpr ExecFlagsSet kExitFlagsMask =
    static_cast<ExecFlagsSet>(ExecFlags::kSvcReqExit) |
    static_cast<ExecFlagsSet>(ExecFlags::kBkptReqExit) |
    static_cast<ExecFlagsSet>(ExecFlags::kBkptReqErrorExit) |
    static_cast<ExecFlagsSet>(ExecFlags::kSvcReqErrorExit);

struct ExecResult {
  ExecFlagsSet flags;
};

} // namespace internal
} // namespace microemu