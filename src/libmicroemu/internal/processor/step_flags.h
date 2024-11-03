#pragma once
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {
using StepFlagsSet = u8;

// TODAY: Ok is the absmce of any error
enum class StepFlags : StepFlagsSet {
  kStepOk = 1u << 0u,
  kStepTerminationRequest = 1u << 1u,
};

} // namespace internal
} // namespace microemu