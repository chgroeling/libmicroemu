#include "test_if_then_else.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

uint32_t BranchTest(volatile int32_t arg) {
  if (arg > 0) {
    return 1U;
  } else if (arg >= 5) {
    return 2U;
  } else if (arg >= 10) {
    return 3U;
  } else if (arg < -10) {
    return 4U;
  } else if (arg < 0) {
    return 5U;
  }
  return 0U;
}

void TestIfThenElse() {
  printf("# Test: If then else\n");
  volatile int32_t arg = -5;
  assert(BranchTest(arg) == 5U);
  arg = -11;
  assert(BranchTest(arg) == 4U);
  arg = 3;
  assert(BranchTest(arg) == 1U);
}