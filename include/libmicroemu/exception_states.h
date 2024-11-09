#pragma once

#include "libmicroemu/exception_type.h"
#include "libmicroemu/types.h"
#include <array>

namespace libmicroemu {

using ExceptionFlagsSet = u8;

enum class ExceptionFlags : ExceptionFlagsSet {
  kPending = 1U << 0U,
  kActive = 1U << 1U,
};

static constexpr i16 kLowestExceptionPriority = 255U;

/** @brief Represents the state of a single exception.
 */
struct SingleExceptionState {
  explicit constexpr SingleExceptionState(u8 number) : number_{number}, priority_{0}, flags_{0U} {}

  /** @brief Clears all flags of the exception.
   */
  inline void ClearFlags() noexcept { flags_ = 0U; }

  /** @brief Checks if the exception is pending.
   *
   * @return True if the exception is pending, false otherwise.
   */
  inline bool IsPending() const noexcept {
    return (flags_ & static_cast<ExceptionFlagsSet>(ExceptionFlags::kPending)) != 0U;
  }

  /** @brief Clears the pending flag of the exception.
   */
  inline void ClearPending() noexcept {
    flags_ &= ~static_cast<ExceptionFlagsSet>(ExceptionFlags::kPending);
  }

  /** @brief Sets the pending flag of the exception.
   */
  inline void SetPending() noexcept {
    flags_ |= static_cast<ExceptionFlagsSet>(ExceptionFlags::kPending);
  }

  /** @brief Checks if the exception is active.
   *
   * @return True if the exception is active, false otherwise.
   */
  inline bool IsActive() const noexcept {
    return (flags_ & static_cast<ExceptionFlagsSet>(ExceptionFlags::kActive)) != 0U;
  }

  /** @brief Clears the active flag of the exception.
   */
  inline void ClearActive() noexcept {
    flags_ &= ~static_cast<ExceptionFlagsSet>(ExceptionFlags::kActive);
  }

  /** @brief Sets the active flag of the exception.
   */
  inline void SetActive() noexcept {
    flags_ |= static_cast<ExceptionFlagsSet>(ExceptionFlags::kActive);
  }

  /** @brief Gets the priority of the exception.
   *
   * @return The priority of the exception.
   */
  inline i16 GetPriority() const noexcept { return priority_; }

  /** @brief Sets the priority of the exception.
   *
   * @param priority The priority of the exception.
   */
  inline void SetPriority(i16 priority) noexcept { priority_ = priority; }

  /** @brief Gets the number of the exception.
   *
   * @return The number of the exception.
   */
  inline u8 GetNumber() const noexcept { return number_; }

private:
  const u8 number_;
  i16 priority_;
  ExceptionFlagsSet flags_;
};

template <std::size_t... Indices>
constexpr auto MakeExceptionArray(std::index_sequence<Indices...>) {
  return std::array<SingleExceptionState, sizeof...(Indices)>{
      SingleExceptionState(static_cast<u8>(Indices))...};
}

struct ExceptionStates {
  ExceptionStates()
      : exception{MakeExceptionArray(std::make_index_sequence<CountExceptions()>())} {}

  u32 pending_exceptions{0U};
  std::array<SingleExceptionState, CountExceptions()> exception;
};

} // namespace libmicroemu
