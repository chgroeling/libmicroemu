#include "test_uint_calc.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void TestUIntCalc() {
  printf("# Test: Simple calculations with unsigned integers\n");
  // -----------------------------
  // 8-bit
  // -----------------------------
  volatile uint8_t accu8 = 123U;

  accu8 += 123U;
  assert(accu8 == 246U);

  accu8 = accu8 - 238U;
  assert(accu8 == 8U);

  accu8 *= 7U;
  assert(accu8 == 56U);

  accu8 -= 57U;
  assert(accu8 == 255U);

  accu8 /= 25U;
  assert(accu8 == 10U);

  accu8 = 1U + 2U + (3U * 3U - 9U) * 50U;
  assert(accu8 == 3U);

  // overflow test
  accu8 = 255U;
  accu8 += 1U;
  assert(accu8 == 0U);
  accu8 -= 1U;
  assert(accu8 == 255U);

  // accumulate tests
  accu8 = 1U;
  accu8 = accu8 * accu8 + accu8; // 2
  accu8 = accu8 * accu8 + accu8; // 6
  accu8 = accu8 * accu8 + accu8; // 42
  assert(accu8 == 42U);

  // -----------------------------
  // 16-bit
  // -----------------------------
  volatile uint16_t accu16 = 123U;

  accu16 += 123U;
  assert(accu16 == 246U);

  accu16 *= 3U;
  assert(accu16 == 738U);

  accu16 = accu16 - 238U;
  assert(accu16 == 500U);

  accu16 /= 25U;
  assert(accu16 == 20U);

  accu16 = 1U + 2U + (3U * 3U - 9U) * 50U;
  assert(accu16 == 3U);

  // overflow test
  accu16 = 65535U;
  accu16 += 1U;
  assert(accu16 == 0U);
  accu16 -= 1U;
  assert(accu16 == 65535U);

  // accumulate tests
  accu16 = 1U;
  accu16 = accu16 * accu16 + accu16; // 2
  accu16 = accu16 * accu16 + accu16; // 6
  accu16 = accu16 * accu16 + accu16; // 42
  assert(accu16 == 42U);

  // -----------------------------
  // 32-bit
  // -----------------------------
  volatile uint32_t accu32 = 123U;

  accu32 += 123U;
  assert(accu32 == 246U);

  accu32 *= 3U;
  assert(accu32 == 738U);

  accu32 = accu32 - 238U;
  assert(accu32 == 500U);

  accu32 /= 25U;
  assert(accu32 == 20U);

  accu32 = 1U + 2U + (3U * 3U - 9U) * 50U;
  assert(accu32 == 3U);

  // overflow test
  accu32 = 4294967295U;
  accu32 += 1U;
  assert(accu32 == 0U);
  accu32 -= 1U;
  assert(accu32 == 4294967295U);

  // accumulate tests
  accu32 = 1U;
  accu32 = accu32 * accu32 + accu32; // 2
  accu32 = accu32 * accu32 + accu32; // 6
  accu32 = accu32 * accu32 + accu32; // 42
  assert(accu32 == 42U);

  // -----------------------------
  // 32-bit Modulo
  // -----------------------------

  volatile uint32_t a = 0x80000000U;
  volatile uint32_t b = 0x80000000U;

  assert((a % b) == 0U);

  a += 1U;
  assert((a % b) == 1U);
}