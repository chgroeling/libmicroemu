#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

enum class BkptFlags : u8 {
  kRequestExit = 0x1u << 0u,      // Application exits via exit keyword
  kRequestErrorExit = 0x1u << 1u, // Application exit was by unusual way.
  kOmitException = 0x1u << 2u     // Omit Exception ..bkpt does not execute Exception
};

using BkptFlagsSet = uint8_t;

} // namespace internal
} // namespace microemu