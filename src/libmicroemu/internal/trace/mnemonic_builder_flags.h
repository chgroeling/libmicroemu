#pragma once

#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

using MnemonicBuilderFlagsSet = u8;
enum class MnemonicBuilderFlags : MnemonicBuilderFlagsSet {
  kNone = 0u,
  kReduceRd = 1u << 0u,
  kSupressShiftType = 1u << 1u,
};
} // namespace internal
} // namespace microemu