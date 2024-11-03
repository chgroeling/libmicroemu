#pragma once

#include "libmicroemu/types.h"

// TODO: Rework like special_register_id.h
namespace microemu {
enum class RegisterId : u8 {
  kR0 = 0u,
  kR1 = 1u,
  kR2 = 2u,
  kR3 = 3u,
  kR4 = 4u,
  kR5 = 5u,
  kR6 = 6u,
  kR7 = 7u,
  kR8 = 8u,
  kR9 = 9u,
  kR10 = 10u,
  kR11 = 11u,
  kR12 = 12u,
  //
  kSp = 13u,
  kR13 = 13u,
  // ---
  kLr = 14u,
  kR14 = 14u,
  // ---
  kPc = 15u,
  kR15 = 15u
};

static constexpr u32 kNoOfRegisters = 16u;
} // namespace microemu