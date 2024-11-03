#include "test_crc.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

uint16_t Crc16(const uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;              // Initial value
  const uint16_t polynomial = 0x1021; // CRC-16-CCITT polynomial

  for (size_t i = 0; i < length; ++i) {
    crc ^= (data[i] << 8); // XOR byte into the top byte of the 16-bit CRC

    for (int bit = 0; bit < 8; ++bit) {
      if (crc & 0x8000) { // If the top bit is set
        crc = (crc << 1) ^ polynomial;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc;
}

void TestCrc() {
  printf("# Test: CRC\n");

  // Test data
  char data[] = "123456789";
  size_t length = sizeof(data) / sizeof(data[0]) - 1U; // -1 to exclude null terminator

  // Calculate CRC16
  uint16_t result = Crc16(reinterpret_cast<uint8_t *>(data), length);

  // printf("CRC16: 0x%X\n", result);
  assert(result == 0x29B1);
}