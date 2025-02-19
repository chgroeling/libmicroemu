#include "test_type_casting.h"
#include <assert.h>
#include <cstring>
#include <stdint.h>
#include <stdio.h>

void TestTypeCasting() {
  printf("# Test: Type casting\n");
  volatile uint64_t u64 = 12345U;
  volatile uint32_t u32 = static_cast<uint32_t>(u64);
  assert(u32 == 12345U);

  uint64_t u64_cpy = u64;
  volatile uint32_t u32_cpy = static_cast<uint32_t>(u64_cpy);
  assert(u32_cpy == 12345U);

  uint64_t u64_mcpy = 0x0U;
  memcpy(&u64_mcpy, &u64_cpy, 4U);
  uint32_t u32_mcpy = static_cast<uint32_t>(u64_mcpy);
  assert(u32_mcpy == 12345U);
}