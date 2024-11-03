#include "test_float.h"
#include <assert.h>
#include <cmath>
#include <stdint.h>
#include <stdio.h>

static constexpr float kEps = 1.0e-4f;

bool IsFloatEqual(float a, float b) { return ((a >= b - kEps) && (a <= b + kEps)); }

void TestFloat() {
  printf("# Test: Simple calculations with floats\n");

  volatile float f = 123.5f;

  assert(f == 123.5f);

  volatile int32_t f_i = static_cast<int32_t>(f);
  assert(f_i == 123);

  volatile float frac = f - f_i;
  assert((frac >= (0.5f - kEps)) && (frac <= (0.5f + kEps)));

  for (int i = 0; i < 3; ++i) { // 6 Nachkommastellen
    frac *= 10;
    int digit = static_cast<int>(frac);
    assert(digit >= 0 && digit <= 9);
    assert(i != 0 || digit == 5);
    assert(i != 1 || digit == 0);
    assert(i != 2 || digit == 0);
    frac -= digit;
  }

  // float equation
  volatile float a = 2.0f;

  a = a * 2.0f; // 4.0
  a = a + 1.0f; // 5.0
  a = a - 0.5f; // 4.5
  a = a / 0.5f; // 9.0
  assert(IsFloatEqual(a, 9.0f));

  // rounding
  a = 1.499f;

  volatile auto a_ceil = std::ceil(a);
  assert(IsFloatEqual(a_ceil, 2.0f));

  volatile auto a_floor = std::floor(a);
  assert(IsFloatEqual(a_floor, 1.0f));

  volatile auto a_round = std::round(a);
  assert(IsFloatEqual(a_round, 1.0f));

  assert(std::isnan(NAN) == true);
  assert(std::isnan(INFINITY) == false);
  assert(std::isnan(0.0) == false);
  assert(std::isinf(INFINITY) == true);
}