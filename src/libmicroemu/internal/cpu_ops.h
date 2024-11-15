#pragma once

#include "libmicroemu/internal/logic/predicates.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/register_id.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace libmicroemu::internal {

template <typename TItOps, typename TPcOps, typename TExcOps, typename TExcTrig> class CpuOps {
public:
  using It = TItOps;
  using Pc = TPcOps;
  using Exc = TExcOps;
  using ExcTrig = TExcTrig;
};

} // namespace libmicroemu::internal