#pragma once

#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

struct OpResult {
  u32 value;
  bool carry_out;
  bool overflow;
};

struct BranchOpResult {
  me_adr_t new_pc;
};

} // namespace internal
} // namespace libmicroemu