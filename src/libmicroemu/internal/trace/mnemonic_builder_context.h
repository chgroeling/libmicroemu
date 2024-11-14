#pragma once
#include "libmicroemu/internal/utils/const_string_builder.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {
template <typename TCpuAccessor, typename TItOps, typename TRegOps, typename TSpecRegOps>
struct MnemonicBuilderContext {

  using It = TItOps;
  using Reg = TRegOps;
  using SReg = TSpecRegOps;

  const TCpuAccessor &cpua;
  ConstStringBuilder &builder;
};

} // namespace internal
} // namespace libmicroemu