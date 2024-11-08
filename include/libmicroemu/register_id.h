#pragma once

#include "libmicroemu/types.h"

// TODO: Rework like special_register_id.h
namespace microemu {
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
  kSp = 13U,
  kR13 = 13U,
  // ---
  kLr = 14U,
  kR14 = 14U,
  // ---
  kPc = 15U,
  kR15 = 15U
};

static constexpr u32 kNoOfRegisters = 16U;
} // namespace microemu