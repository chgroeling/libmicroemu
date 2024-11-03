#pragma once

#include "libmicroemu/internal/fetcher/raw_instr_flags.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

#include <cstdint>

namespace microemu {
namespace internal {

static constexpr uint32_t kFlagsOpCodeLast = 15u;
static constexpr uint32_t kFlagsOpCodeFirst = 11u;
static const RawInstrFlagsSet kRawInstrFlagsTable[] = {
    /* 00000 */ 0u,
    /* 00001 */ 0u,
    /* 00010 */ 0u,
    /* 00011 */ 0u,
    /* 00100 */ 0u,
    /* 00101 */ 0u,
    /* 00110 */ 0u,
    /* 00111 */ 0u,
    /* 01000 */ 0u,
    /* 01001 */ 0u,
    /* 01010 */ 0u,
    /* 01011 */ 0u,
    /* 01100 */ 0u,
    /* 01101 */ 0u,
    /* 01110 */ 0u,
    /* 01111 */ 0u,
    /* 10000 */ 0u,
    /* 10001 */ 0u,
    /* 10010 */ 0u,
    /* 10011 */ 0u,
    /* 10100 */ 0u,
    /* 10101 */ 0u,
    /* 10110 */ 0u,
    /* 10111 */ 0u,
    /* 11000 */ 0u,
    /* 11001 */ 0u,
    /* 11010 */ 0u,
    /* 11011 */ 0u,
    /* 11100 */ 0u,
    /* 11101 */ static_cast<RawInstrFlagsSet>(RawInstrFlagsMsk::k32Bit),
    /* 11110 */ static_cast<RawInstrFlagsSet>(RawInstrFlagsMsk::k32Bit),
    /* 11111 */ static_cast<RawInstrFlagsSet>(RawInstrFlagsMsk::k32Bit)};
} // namespace internal
} // namespace microemu