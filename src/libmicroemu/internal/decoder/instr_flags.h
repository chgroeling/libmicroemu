#include "libmicroemu/types.h"

#pragma once

namespace microemu {
namespace internal {

using InstrFlagsSet = u8;

enum class InstrFlagsShift {
  k32Bit = 0u,
  kAddShift = 1u,
  kIndexShift = 2u,
  kWBackShift = 3u,
  kTbhShift = 4u,
  kSetFlagsShift = 5u,
  kUnalignedAllowShift = 6u,
  kNonZeroShift = 7u
};

// TODO: Enum class
enum InstrFlags : InstrFlagsSet {
  k32Bit = 1u << static_cast<InstrFlagsSet>(InstrFlagsShift::k32Bit),
  kAdd = 1u << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift),
  kIndex = 1u << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift),
  kWBack = 1u << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift),
  kTbh = 1u << static_cast<InstrFlagsSet>(InstrFlagsShift::kTbhShift),
  kSetFlags = 1u << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift),
  kUnalignedAllow = 1u << static_cast<InstrFlagsSet>(InstrFlagsShift::kUnalignedAllowShift),
  kNonZero = 1u << static_cast<InstrFlagsSet>(InstrFlagsShift::kNonZeroShift)
};

} // namespace internal
} // namespace microemu