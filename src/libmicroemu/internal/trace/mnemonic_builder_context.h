#pragma once
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/const_string_builder.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {
template <typename TCpuAccessor, typename TItOps, typename TRegOps, typename TSpecRegOps>
struct MnemonicBuilderContext {

  using It = TItOps;
  using Reg = TRegOps;
  using SReg = TSpecRegOps;

  const TCpuAccessor &cpua;
  ConstStringBuilder &builder;
};

} // namespace libmicroemu::internal