#pragma once
#include "libmicroemu/types.h"

namespace libmicroemu {

using EmuFlagsSet = u8;

// TODAY: Ok is the absmce of any error
enum class EmuFlags : EmuFlagsSet {
  kEmuOk = 1U << 0U,
  kEmuMaxInstructionsReached = 1U << 1U,
  kEmuTerminated = 1U << 2U,
};

struct EmuResult {
  EmuFlagsSet flags;
  u32 status_code;
};

} // namespace libmicroemu