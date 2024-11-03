#pragma once
#include "libmicroemu/internal/utils/const_string_builder.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {
template <typename TProcessorStates, typename TItOps, typename TRegOps, typename TSpecRegOps>
struct MnemonicBuilderContext {

  using It = TItOps;
  using Reg = TRegOps;
  using SReg = TSpecRegOps;

  const TProcessorStates &pstates;
  ConstStringBuilder &builder;
};

} // namespace internal
} // namespace microemu