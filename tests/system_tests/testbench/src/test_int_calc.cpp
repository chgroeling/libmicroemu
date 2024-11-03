#include "test_int_calc.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void TestIntCalc() {
  printf("# Test: Simple calculations with signed integers\n");
  // -----------------------------
  // 8-bit
  // -----------------------------
  volatile int8_t accu8 = 50;

  accu8 += 52;
  assert(accu8 == 102);

  accu8 = accu8 - 138;
  assert(accu8 == -36);

  accu8 *= 3;
  assert(accu8 == -108);

  accu8 -= 12;
  assert(accu8 == -120);

  accu8 /= -12;
  assert(accu8 == 10);

  // overflow tests
  accu8 = -128;
  accu8 = accu8 - 10;
  assert(accu8 == 118);

  // accumulate tests
  accu8 = 1u;
  accu8 = accu8 * accu8 + accu8; // 2
  accu8 = accu8 * accu8 + accu8; // 6
  accu8 = accu8 * accu8 + accu8; // 42
  assert(accu8 == 42u);

  // -----------------------------
  // 16-bit
  // -----------------------------
  volatile int16_t accu16 = 50;

  accu16 += 52;
  assert(accu16 == 102);

  accu16 = accu16 - 138;
  assert(accu16 == -36);

  accu16 *= 3;
  assert(accu16 == -108);

  accu16 -= 12;
  assert(accu16 == -120);

  accu16 /= -12;
  assert(accu16 == 10);

  // overlfow tests
  accu16 = -32768;
  accu16 = accu16 - 10;
  assert(accu16 == 32758);

  // accumulate tests
  accu16 = 1u;
  accu16 = accu16 * accu16 + accu16; // 2
  accu16 = accu16 * accu16 + accu16; // 6
  accu16 = accu16 * accu16 + accu16; // 42
  assert(accu16 == 42u);

  // -----------------------------
  // 32-bit
  // -----------------------------
  volatile int32_t accu32 = 50;

  accu32 += 52;
  assert(accu32 == 102);

  accu32 = accu32 - 138;
  assert(accu32 == -36);

  accu32 *= 3;
  assert(accu32 == -108);

  accu32 -= 12;
  assert(accu32 == -120);

  accu32 /= -12;
  assert(accu32 == 10);

  // overflow tests
  accu32 = -2147483648;
  accu32 = accu32 - 10;
  assert(accu32 == 2147483638);

  // accumulate tests
  accu32 = 1u;

  accu32 = accu32 * accu32 + accu32; // 2
  accu32 = accu32 * accu32 + accu32; // 6
  accu32 = accu32 * accu32 + accu32; // 42
  assert(accu32 == 42u);

  // -----------------------------
  // 32-bit Modulo Tests
  // -----------------------------

  volatile int32_t a = 0x80000000;
  volatile int32_t b = 0x80000000;

  assert((a % b) == 0);

  a += 1u;
  assert((a % b) == 0x80000001); // signed divison result
}