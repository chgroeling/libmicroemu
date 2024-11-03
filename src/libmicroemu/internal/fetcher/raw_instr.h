#include "libmicroemu/internal/fetcher/raw_instr_flags.h"
#include "libmicroemu/types.h"
#pragma once

namespace microemu {

struct RawInstr {
  u16 low;
  u16 high;
  RawInstrFlagsSet flags;
};

} // namespace microemu