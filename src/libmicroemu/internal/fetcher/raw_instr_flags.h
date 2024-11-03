#include "libmicroemu/types.h"

#pragma once

namespace microemu {

using RawInstrFlagsSet = u8;

enum class RawInstrFlagsPos {
  k32Bit = 0u,
};

enum class RawInstrFlagsMsk : RawInstrFlagsSet {
  k32Bit = 1u << static_cast<u8>(RawInstrFlagsPos::k32Bit)
};

} // namespace microemu