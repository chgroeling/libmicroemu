#pragma once

#include "libmicroemu/exception_type.h"
#include "libmicroemu/types.h"
#include <array>

namespace microemu {

using ExceptionFlagsSet = u8;

enum class ExceptionFlags : ExceptionFlagsSet {
  kPending = 1u << 0u,
  kActive = 1u << 1u,
};

static constexpr i16 kLowestExceptionPriority = 255u;

struct SingleExceptionState {
  // TODO: Make private
  i16 priority;
  u8 number;
  ExceptionFlagsSet flags;

  SingleExceptionState() : priority{0}, number{0}, flags{0} {}

  inline bool IsPending() const noexcept {
    return (flags & static_cast<ExceptionFlagsSet>(ExceptionFlags::kPending)) != 0u;
  }

  inline void ClearPending() noexcept {
    flags &= ~static_cast<ExceptionFlagsSet>(ExceptionFlags::kPending);
  }

  inline void SetPending() noexcept {
    flags |= static_cast<ExceptionFlagsSet>(ExceptionFlags::kPending);
  }

  inline bool IsActive() const noexcept {
    return (flags & static_cast<ExceptionFlagsSet>(ExceptionFlags::kActive)) != 0u;
  }

  inline void ClearActive() noexcept {
    flags &= ~static_cast<ExceptionFlagsSet>(ExceptionFlags::kActive);
  }

  inline void SetActive() noexcept {
    flags |= static_cast<ExceptionFlagsSet>(ExceptionFlags::kActive);
  }
};

struct ExceptionStates {
  ExceptionStates() { exception.fill(SingleExceptionState()); }

  u32 pending_exceptions{0u};
  std::array<SingleExceptionState, CountExceptions()> exception;
};

} // namespace microemu
