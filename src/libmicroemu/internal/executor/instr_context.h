#pragma once
#include "libmicroemu/internal/cpu_accessor.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

template <typename TCpuAccessor, typename TBus, typename TPcOps, typename TItOps,
          typename TExceptionTrigger>
struct InstrContext {
  using Pc = TPcOps;
  using It = TItOps;
  using ExcTrig = TExceptionTrigger;

  TCpuAccessor &cpua;
  TBus &bus;
};

} // namespace internal
} // namespace libmicroemu