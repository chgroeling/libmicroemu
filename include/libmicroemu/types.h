///

#pragma once

#include <cstdint>

namespace libmicroemu {

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;

using me_adr_t = u32;
using me_size_t = u32;
using me_offset_t = u32;

union U32_parts {
  u32 u32;
  u16 u16[2];
  u8 u8[4];
};

} // namespace libmicroemu