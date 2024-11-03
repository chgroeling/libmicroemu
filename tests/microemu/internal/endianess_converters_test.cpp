#include "libmicroemu/internal/bus/endianess_converters.h"

#include <gtest/gtest.h>

#include <cstdint>

using namespace microemu;

/// \test BigToLittleEndianConverterTest
/// \test_verifies
/// \test_item Convert
/// \test_scenario convert big endian u8 data to little endian
/// \test_expected_behaviour Output data is the same
TEST(BigToLittleEndianConverterTest, Convert_uint8_NoChange) {
  u8 data = 0xABu;

  auto result = internal::BigToLittleEndianConverter::Convert<u8>(data);
  ASSERT_EQ(result, 0xABu);
}

/// \test BigToLittleEndianConverterTest
/// \test_verifies
/// \test_item Convert
/// \test_scenario convert big endian u16 data to little endian
/// \test_expected_behaviour Output data is the same with flipped bytes
TEST(BigToLittleEndianConverterTest, Convert_uint16_NoChange_BytesFlipped) {
  u16 data = 0xABCDu;
  auto result = internal::BigToLittleEndianConverter::Convert<u16>(data);
  ASSERT_EQ(result, 0xCDABu);
}

/// \test BigToLittleEndianConverterTest
/// \test_verifies
/// \test_item Convert
/// \test_scenario convert big endian u32 data to little endian
/// \test_expected_behaviour Output data is the same with flipped bytes
TEST(BigToLittleEndianConverterTest, Convert_uint32_NoChange_BytesFlipped) {
  u32 data = 0x1234ABCDu;
  auto result = internal::BigToLittleEndianConverter::Convert<u32>(data);
  ASSERT_EQ(result, 0xCDAB3412u);
}

/// \test BigToLittleEndianConverterTest
/// \test_verifies
/// \test_item Convert
/// \test_scenario convert big endian u64 data to little endian
/// \test_expected_behaviour Output data is the same with flipped bytes
TEST(BigToLittleEndianConverterTest, Convert_uint64_NoChange_BytesFlipped) {
  u64 data = 0x1234567890ABCDEFu;
  auto result = internal::BigToLittleEndianConverter::Convert<u64>(data);
  ASSERT_EQ(result, 0xEFCDAB9078563412u);
}
