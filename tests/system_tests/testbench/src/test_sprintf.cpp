#include "test_sprintf.h"
#include <assert.h>
#include <cmath>
#include <cstring>
#include <stdint.h>
#include <stdio.h>

void TestSprintf() {
  printf("# Test: Sprintf\n");

  char buf[50];
  sprintf(buf, "%s, %i, %u", "ABC", -123, 345);
  assert(strcmp(buf, "ABC, -123, 345") == 0);

  uint32_t test0{1234u};
  sprintf(buf, "  1234 == %u\n", test0);
  assert(strcmp(buf, "  1234 == 1234\n") == 0);

  uint32_t test1{0xCAFEBABEu};
  sprintf(buf, "  0xCAFEBABE == 0x%X\n", test1);
  assert(strcmp(buf, "  0xCAFEBABE == 0xCAFEBABE\n") == 0);

  sprintf(buf, "  Float: %f\n", 123.45678f);
  assert(strcmp(buf, "  Float: 123.456779\n") == 0);

  sprintf(buf, "  --> %s \n", "inserted str");
  assert(strcmp(buf, "  --> inserted str \n") == 0);
}