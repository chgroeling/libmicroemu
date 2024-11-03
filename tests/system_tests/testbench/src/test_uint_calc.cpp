#include "test_uint_calc.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void TestUIntCalc() {
  printf("# Test: Simple calculations with unsigned integers\n");
  // -----------------------------
  // 8-bit
  // -----------------------------
  volatile uint8_t accu8 = 123u;

  accu8 += 123u;
  assert(accu8 == 246u);

  accu8 = accu8 - 238u;
  assert(accu8 == 8u);

  accu8 *= 7u;
  assert(accu8 == 56u);

  accu8 -= 57u;
  assert(accu8 == 255u);

  accu8 /= 25u;
  assert(accu8 == 10u);

  accu8 = 1u + 2u + (3u * 3u - 9u) * 50u;
  assert(accu8 == 3u);

  // overflow test
  accu8 = 255u;
  accu8 += 1u;
  assert(accu8 == 0u);
  accu8 -= 1u;
  assert(accu8 == 255u);

  // accumulate tests
  accu8 = 1u;
  accu8 = accu8 * accu8 + accu8; // 2
  accu8 = accu8 * accu8 + accu8; // 6
  accu8 = accu8 * accu8 + accu8; // 42
  assert(accu8 == 42u);

  // -----------------------------
  // 16-bit
  // -----------------------------
  volatile uint16_t accu16 = 123u;

  accu16 += 123u;
  assert(accu16 == 246u);

  accu16 *= 3u;
  assert(accu16 == 738u);

  accu16 = accu16 - 238u;
  assert(accu16 == 500u);

  accu16 /= 25u;
  assert(accu16 == 20u);

  accu16 = 1u + 2u + (3u * 3u - 9u) * 50u;
  assert(accu16 == 3u);

  // overflow test
  accu16 = 65535u;
  accu16 += 1u;
  assert(accu16 == 0u);
  accu16 -= 1u;
  assert(accu16 == 65535u);

  // accumulate tests
  accu16 = 1u;
  accu16 = accu16 * accu16 + accu16; // 2
  accu16 = accu16 * accu16 + accu16; // 6
  accu16 = accu16 * accu16 + accu16; // 42
  assert(accu16 == 42u);

  // -----------------------------
  // 32-bit
  // -----------------------------
  volatile uint32_t accu32 = 123u;

  accu32 += 123u;
  assert(accu32 == 246u);

  accu32 *= 3u;
  assert(accu32 == 738u);

  accu32 = accu32 - 238u;
  assert(accu32 == 500u);

  accu32 /= 25u;
  assert(accu32 == 20u);

  accu32 = 1u + 2u + (3u * 3u - 9u) * 50u;
  assert(accu32 == 3u);

  // overflow test
  accu32 = 4294967295u;
  accu32 += 1u;
  assert(accu32 == 0u);
  accu32 -= 1u;
  assert(accu32 == 4294967295u);

  // accumulate tests
  accu32 = 1u;
  accu32 = accu32 * accu32 + accu32; // 2
  accu32 = accu32 * accu32 + accu32; // 6
  accu32 = accu32 * accu32 + accu32; // 42
  assert(accu32 == 42u);

  // -----------------------------
  // 32-bit Modulo
  // -----------------------------

  volatile uint32_t a = 0x80000000u;
  volatile uint32_t b = 0x80000000u;

  assert((a % b) == 0u);

  a += 1u;
  assert((a % b) == 1u);
}