#pragma once
#include "libmicroemu/internal/cpu_accessor.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

template <typename TCpuAccessor, typename TBus, typename TCpuOps> struct InstrContext {
  using Pc = typename TCpuOps::Pc;
  using It = typename TCpuOps::It;
  using ExcTrig = typename TCpuOps::ExcTrig;

  TCpuAccessor &cpua;
  TBus &bus;
};

} // namespace internal
} // namespace libmicroemu