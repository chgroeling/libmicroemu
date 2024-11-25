#pragma once

#include "libmicroemu/types.h"

namespace libmicroemu::internal {

using MnemonicBuilderFlagsSet = u8;
enum class MnemonicBuilderFlags : MnemonicBuilderFlagsSet {
  kNone = 0U,
  kReduceRd = 1U << 0U,
  kSupressShiftType = 1U << 1U,
};

} // namespace libmicroemu::internal