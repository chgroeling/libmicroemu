#include "libmicroemu/types.h"

#pragma once

namespace microemu {
namespace internal {

using InstrFlagsSet = u8;

enum class InstrFlagsShift {
  k32Bit = 0U,
  kAddShift = 1U,
  kIndexShift = 2U,
  kWBackShift = 3U,
  kTbhShift = 4U,
  kSetFlagsShift = 5U,
  kUnalignedAllowShift = 6U,
  kNonZeroShift = 7U
};

// TODO: Enum class
enum InstrFlags : InstrFlagsSet {
  k32Bit = 1U << static_cast<InstrFlagsSet>(InstrFlagsShift::k32Bit),
  kAdd = 1U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift),
  kIndex = 1U << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift),
  kWBack = 1U << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift),
  kTbh = 1U << static_cast<InstrFlagsSet>(InstrFlagsShift::kTbhShift),
  kSetFlags = 1U << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift),
  kUnalignedAllow = 1U << static_cast<InstrFlagsSet>(InstrFlagsShift::kUnalignedAllowShift),
  kNonZero = 1U << static_cast<InstrFlagsSet>(InstrFlagsShift::kNonZeroShift)
};

} // namespace internal
} // namespace microemu