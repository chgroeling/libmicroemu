#include "test_scb.h"
#include "scb.h"
#include <assert.h>
#include <atomic>
#include <stdint.h>
#include <stdio.h>

void TestScb() {
  printf("# Test: System control bus\n");
  auto *scb = GetSCBBase();

  assert(scb->CCR == (kCcrStkAlignFlag));
}