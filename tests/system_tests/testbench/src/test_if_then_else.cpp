#include "test_if_then_else.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

uint32_t BranchTest(volatile int32_t arg) {
  if (arg > 0) {
    return 1u;
  } else if (arg >= 5) {
    return 2u;
  } else if (arg >= 10) {
    return 3u;
  } else if (arg < -10) {
    return 4u;
  } else if (arg < 0) {
    return 5u;
  }
  return 0u;
}

void TestIfThenElse() {
  printf("# Test: If then else\n");
  volatile int32_t arg = -5;
  assert(BranchTest(arg) == 5u);
  arg = -11;
  assert(BranchTest(arg) == 4u);
  arg = 3;
  assert(BranchTest(arg) == 1u);
}