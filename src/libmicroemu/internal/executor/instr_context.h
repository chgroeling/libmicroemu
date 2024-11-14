#pragma once
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {
template <typename TProcessorStates, typename TBus, typename TRegOps, typename TSpecRegOps,
          typename TPcOps, typename TItOps, typename TExceptionTrigger>
struct InstrContext {
  using ProcessorStates = TProcessorStates;
  using Pc = TPcOps;
  using It = TItOps;
  using Reg = TRegOps;
  using SReg = TSpecRegOps;
  using ExcTrig = TExceptionTrigger;

  TBus &bus;
  TProcessorStates &pstates;
};

} // namespace internal
} // namespace libmicroemu