#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "libmicroemu/internal/utils/bit_manip.h"

using ::testing::Return;

using namespace microemu;
using namespace microemu::internal;

TEST(BitManipTests, GenerateBitMaskLowNibble) {
  u32 mask = BitManip<u32>::GenerateBitMask<3u, 0u>();
  ASSERT_EQ(mask, 0xF);
}

TEST(BitManipTests, GenerateBitMaskHighNibble) {
  u32 mask = BitManip<u32>::GenerateBitMask<31u, 28u>();
  ASSERT_EQ(mask, 0xF0000000);
}

TEST(BitManipTests, IsolateBits12And11ReturnsCorrectValue) {
  u32 val = 0b0001100000000000;
  u32 result = BitManip<u32>::Slice1R<12u, 11u>(val);
  ASSERT_EQ(result, 0x3);
}

TEST(BitManipTests, IsolateBits2_R15And15_R12And11Test1ReturnsCorrectValue) {
  u32 val = 0b0001100000000000;
  u32 result = BitManip<u32>::Slice2R<15u, 15u, 12u, 11u>(val);
  ASSERT_EQ(result, 0x3);
}

TEST(BitManipTests, IsolateBits2_R15And15_R12And11Test2ReturnsCorrectValue) {
  u32 val = 0b1001100000000000;
  u32 result = BitManip<u32>::Slice2R<15u, 15u, 12u, 11u>(val);
  ASSERT_EQ(result, 0b111);
}

TEST(BitManipTests, SignExtend7BitTo32BitTopBit0Val0x20) {
  u8 val = 0b0100000;
  i32 result = static_cast<i32>(BitManip<u8>::SignExtend<u32, 6>(val));
  ASSERT_EQ(result, 32);
}

TEST(BitManipTests, SignExtend7BitTo32BitTopBit1Val1) {
  u8 val = 0b1100000;
  i32 result = static_cast<i32>(BitManip<u8>::SignExtend<u32, 6>(val));
  ASSERT_EQ(result, -32);
}
TEST(BitManipTests, SignExtend16BitTo32BitTopBit1Valm128) {
  i16 val = -128;
  u32 val_u32 = static_cast<u32>(val);
  i32 result = static_cast<i32>(BitManip<u16>::SignExtend<u32, 15>(val_u32));
  ASSERT_EQ(result, -128);
}

TEST(BitManipTests, BitCountValue01010101Return4) {
  u8 val = 0b1010101;
  u8 result = static_cast<i32>(BitManip<u8>::BitCount(val));
  ASSERT_EQ(result, 4);
}