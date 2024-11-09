#include "libmicroemu/types.h"

#pragma once

namespace libmicroemu {

using RawInstrFlagsSet = u8;

enum class RawInstrFlagsPos {
  k32Bit = 0U,
};

enum class RawInstrFlagsMsk : RawInstrFlagsSet {
  k32Bit = 1U << static_cast<u8>(RawInstrFlagsPos::k32Bit)
};

} // namespace libmicroemu