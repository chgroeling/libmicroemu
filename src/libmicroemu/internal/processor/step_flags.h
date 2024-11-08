#pragma once
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {
using StepFlagsSet = u8;

// TODAY: Ok is the absmce of any error
enum class StepFlags : StepFlagsSet {
  kStepOk = 1U << 0U,
  kStepTerminationRequest = 1U << 1U,
};

} // namespace internal
} // namespace microemu