#pragma once

#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

using MnemonicBuilderFlagsSet = u8;
enum class MnemonicBuilderFlags : MnemonicBuilderFlagsSet {
  kNone = 0U,
  kReduceRd = 1U << 0U,
  kSupressShiftType = 1U << 1U,
};
} // namespace internal
} // namespace microemu