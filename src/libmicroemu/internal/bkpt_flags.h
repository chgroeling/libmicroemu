#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

enum class BkptFlags : u8 {
  kRequestExit = 0x1U << 0U,      // Application exits via exit keyword
  kRequestErrorExit = 0x1U << 1U, // Application exit was by unusual way.
  kOmitException = 0x1U << 2U     // Omit Exception ..bkpt does not execute Exception
};

using BkptFlagsSet = uint8_t;

} // namespace internal
} // namespace microemu