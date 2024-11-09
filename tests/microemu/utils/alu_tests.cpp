#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "libmicroemu/internal/logic/alu.h"

using ::testing::Return;

using namespace libmicroemu;
using namespace libmicroemu::internal;

TEST(BitManipTests, AddWithCarryCarryFalseResultHasCarryFalseOverflowFalse) {
  u32 x = 123;
  u32 y = 177;
  auto result = Alu<u32>::AddWithCarry(x, y, false);

  ASSERT_EQ(result.value, 300);
  ASSERT_EQ(result.carry_out, false); // carry_out
  ASSERT_EQ(result.overflow, false);  // overflow
}

TEST(BitManipTests, AddWithCarryCarryTrueResultHasCarryFalseOverflowFalse) {
  u32 x = 123;
  u32 y = 177;
  auto result = Alu<u32>::AddWithCarry(x, y, true);

  ASSERT_EQ(result.value, 301);
  ASSERT_EQ(result.carry_out, false); // carry_out
  ASSERT_EQ(result.overflow, false);  // overflow
}

TEST(BitManipTests, AddWithCarryCarryTrueResultHasCarryTrueOverflowFalse) {
  u32 x = 0x7FFFFFFF;  //  2147483647
  u32 y = ~0x7FFFFFFF; // -2147483648
  auto result = Alu<u32>::AddWithCarry(x, y, true);

  ASSERT_EQ(result.value, 0);
  ASSERT_EQ(result.carry_out, true); // carry_out
  ASSERT_EQ(result.overflow, false); // overflow
}

TEST(BitManipTests, AddWithCarryCarryTrueResultHasCarryFalseOverflowTrue) {
  u32 x = 0x7FFFFFFF; //  2147483647
  u32 y = 0x7FFFFFFF; //  2147483647
  auto result = Alu<u32>::AddWithCarry(x, y, true);

  ASSERT_EQ(result.value, 4294967295);
  ASSERT_EQ(result.carry_out, false); // carry_out
  ASSERT_EQ(result.overflow, true);   // overflow
}

TEST(BitManipTests, AddWithCarryCarryTrueResultHasCarryTrueOverflowTrue) {
  u32 x = 0xFFFFFFFF; //  -1
  u32 y = 0x7FFFFFFF; //  2147483647
  auto result = Alu<u32>::AddWithCarry(x, y, true);

  ASSERT_EQ(result.value, 2147483647);
  ASSERT_EQ(result.carry_out, true); // carry_out
  ASSERT_EQ(result.overflow, false); // overflow
}

TEST(BitManipTests, ROR_C_Rotate1Left) {
  u32 x = 0x80000000; //  1
  auto result = Alu<u32>::ROR_C(x, -1);

  ASSERT_EQ(result.result, 0x1);
  ASSERT_EQ(result.carry_out, false); // carry_out
}

TEST(BitManipTests, ROR_C_Rotate1LRight) {
  u32 x = 0x00000001; //  1
  auto result = Alu<u32>::ROR_C(x, 1);

  ASSERT_EQ(result.result, 0x80000000);
  ASSERT_EQ(result.carry_out, true); // carry_out
}

TEST(BitManipTests, LSL_C_ShiftLeft1_Get0CarryOutTrue) {
  u32 x = 0x00000001; //  1
  auto result = Alu<u32>::LSL_C(x, 1);

  ASSERT_EQ(result.result, 0x00000002);
  ASSERT_EQ(result.carry_out, false); // carry_out
}

TEST(BitManipTests, LSL_C_ShiftLeft3_Get1CarryOutTrue) {
  u32 x = 0x00000003; //  3
  auto result = Alu<u32>::LSL_C(x, 1);

  ASSERT_EQ(result.result, 0x00000006);
  ASSERT_EQ(result.carry_out, false); // carry_out
}

TEST(BitManipTests, LSL_C_ShiftLeftF_GetFCarryOutFalse) {
  u32 x = 0xF0000000; //  3
  auto result = Alu<u32>::LSL_C(x, 4);

  ASSERT_EQ(result.result, 0x00000000);
  ASSERT_EQ(result.carry_out, true); // carry_out
}

TEST(BitManipTests, LSR_C_ShiftRight1_Get0CarryOutTrue) {
  u32 x = 0x00000001; //  1
  auto result = Alu<u32>::LSR_C(x, 1);

  ASSERT_EQ(result.result, 0x00000000);
  ASSERT_EQ(result.carry_out, true); // carry_out
}

TEST(BitManipTests, LSR_C_ShiftRight3_Get1CarryOutTrue) {
  u32 x = 0x00000003; //  3
  auto result = Alu<u32>::LSR_C(x, 1);

  ASSERT_EQ(result.result, 0x00000001);
  ASSERT_EQ(result.carry_out, true); // carry_out
}

TEST(BitManipTests, LSR_C_ShiftRightF_GetFCarryOutFalse) {
  u32 x = 0xF0000003; //  3
  auto result = Alu<u32>::LSR_C(x, 4);

  ASSERT_EQ(result.result, 0x0F000000);
  ASSERT_EQ(result.carry_out, false); // carry_out
}

TEST(BitManipTests, ASR_C_ShiftRightFSimple_GetFCarryOutFalse) {
  u32 x = 0xF0000003; //  3
  auto result = Alu<u32>::ASR_C(x, 4);

  ASSERT_EQ(result.result, 0xFF000000);
  ASSERT_EQ(result.carry_out, false); // carry_out
}

TEST(BitManipTests, ASR_C_ShiftRightFSimple_GetFCarryOutTrue) {
  u32 x = 0xF0000008; //  3
  auto result = Alu<u32>::ASR_C(x, 4);

  ASSERT_EQ(result.result, 0xFF000000);
  ASSERT_EQ(result.carry_out, true); // carry_out
}

TEST(BitManipTests, ASR_C_ShiftRightFComplex_GetFCarryOutTrue) {
  u32 x = 0xF0000018; //  3
  auto result = Alu<u32>::ASR_C(x, 4);

  ASSERT_EQ(result.result, 0xFF000001);
  ASSERT_EQ(result.carry_out, true); // carry_out
}