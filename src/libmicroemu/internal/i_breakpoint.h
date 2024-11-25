#pragma once

#include "libmicroemu/internal/bkpt_flags.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

class IBreakpoint {
public:
  virtual ~IBreakpoint() = default; // Virtual Destructor

  virtual Result<BkptFlagsSet> Call(const uint32_t &imm32) = 0;
  virtual uint32_t GetExitStatusCode() const = 0;
};

} // namespace libmicroemu::internal