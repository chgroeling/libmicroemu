#pragma once

#include "libmicroemu/types.h"
namespace libmicroemu {

struct ThumbImmediateResult {
  u32 out;
  bool carry_out;

  // ThumbImmediateResult() : out(0), carry_out(false) {}
  // ThumbImmediateResult(u32 out, bool carry_out) : out(out), carry_out(carry_out) {}
};

} // namespace libmicroemu
