#pragma once

#include "libmicroemu/types.h"

namespace microemu {

/**
 * @brief Enumeration of register IDs.
 *
 * This enumeration represents the IDs of general-purpose registers.
 * General-purpose registers are used for storing data and intermediate results.
 */
enum class RegisterId : u8 {
  kR0 = 0U,
  kR1 = 1U,
  kR2 = 2U,
  kR3 = 3U,
  kR4 = 4U,
  kR5 = 5U,
  kR6 = 6U,
  kR7 = 7U,
  kR8 = 8U,
  kR9 = 9U,
  kR10 = 10U,
  kR11 = 11U,
  kR12 = 12U,
  //
  kSp = 13U, ///< Stack-pointer register
  kR13 = 13U,
  // ---
  kLr = 14U, ///< Link-register
  kR14 = 14U,
  // ---
  kPc = 15U, ///< Program-counter register
  kR15 = 15U
};

/// @brief The last register ID.
constexpr auto kLastRegister = RegisterId::kR15;

/// @brief Returns the number of general-purpose registers.
/// @return The number of general-purpose registers.
static constexpr u32 CountRegisters() noexcept { return static_cast<u32>(kLastRegister) + 1U; };
} // namespace microemu