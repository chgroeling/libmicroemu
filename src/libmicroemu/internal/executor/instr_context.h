#pragma once
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {
template <typename TProcessorStates, typename TBus, typename TRegOps, typename TSpecRegOps,
          typename TPcOps, typename TItOps, typename TExceptionTrigger>
struct InstrContext {
  TBus &bus;
  TProcessorStates &pstates;

  using Pc = TPcOps;
  using It = TItOps;
  using Reg = TRegOps;
  using SReg = TSpecRegOps;
  using ExcTrig = TExceptionTrigger;
};

} // namespace internal
} // namespace microemu