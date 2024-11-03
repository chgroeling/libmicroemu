#pragma once
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

using ExecFlagsSet = u8;

enum class ExecFlags : ExecFlagsSet {
  kBkptReqExit = 1u << 3u,
  kSvcReqExit = 1u << 4u,
  kBkptReqErrorExit = 1u << 5u,
  kSvcReqErrorExit = 1u << 6u
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