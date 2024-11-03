#pragma once
#include "libmicroemu/types.h"

namespace microemu {

using EmuFlagsSet = u8;

// TODAY: Ok is the absmce of any error
enum class EmuFlags : EmuFlagsSet {
  kEmuOk = 1u << 0u,
  kEmuMaxInstructionsReached = 1u << 1u,
  kEmuTerminated = 1u << 2u,
};

struct EmuResult {
  EmuFlagsSet flags;
  u32 status_code;
};

} // namespace microemu