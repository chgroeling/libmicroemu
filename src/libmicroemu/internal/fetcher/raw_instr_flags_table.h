#pragma once

#include "libmicroemu/internal/fetcher/raw_instr_flags.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

#include <cstdint>

namespace libmicroemu::internal {

static constexpr uint32_t kFlagsOpCodeLast = 15U;
static constexpr uint32_t kFlagsOpCodeFirst = 11U;
static const RawInstrFlagsSet kRawInstrFlagsTable[] = {
    /* 00000 */ 0U,
    /* 00001 */ 0U,
    /* 00010 */ 0U,
    /* 00011 */ 0U,
    /* 00100 */ 0U,
    /* 00101 */ 0U,
    /* 00110 */ 0U,
    /* 00111 */ 0U,
    /* 01000 */ 0U,
    /* 01001 */ 0U,
    /* 01010 */ 0U,
    /* 01011 */ 0U,
    /* 01100 */ 0U,
    /* 01101 */ 0U,
    /* 01110 */ 0U,
    /* 01111 */ 0U,
    /* 10000 */ 0U,
    /* 10001 */ 0U,
    /* 10010 */ 0U,
    /* 10011 */ 0U,
    /* 10100 */ 0U,
    /* 10101 */ 0U,
    /* 10110 */ 0U,
    /* 10111 */ 0U,
    /* 11000 */ 0U,
    /* 11001 */ 0U,
    /* 11010 */ 0U,
    /* 11011 */ 0U,
    /* 11100 */ 0U,
    /* 11101 */ static_cast<RawInstrFlagsSet>(RawInstrFlagsMsk::k32Bit),
    /* 11110 */ static_cast<RawInstrFlagsSet>(RawInstrFlagsMsk::k32Bit),
    /* 11111 */ static_cast<RawInstrFlagsSet>(RawInstrFlagsMsk::k32Bit)};
} // namespace libmicroemu::internal