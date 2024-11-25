#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

enum class SvcFlags : u8 {
  kRequestExit = 0x1U << 0U,      // Application exits via exit keyword
  kRequestErrorExit = 0x1U << 1U, // Application exit was by unusual way.
  kOmitException = 0x1U << 2U     // Omit Exception ..svc does not execute Exception
};

using SvcFlagsSet = uint8_t;

} // namespace libmicroemu::internal