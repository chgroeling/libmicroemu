#include "test_stack_alignment.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

uint32_t StackAlignmentTest() {
  volatile uint32_t ret32 = 0xCAFEu;
  volatile uint16_t ret16 = static_cast<uint16_t>(ret32);
  volatile uint8_t ret8 = static_cast<uint8_t>(ret16);
  return ret8;
};

uint32_t CallFct3(uint32_t a) {
  volatile uint32_t ret = a;
  return ret;
}

uint16_t CallFct2(uint16_t a) {
  volatile uint16_t ret = static_cast<uint16_t>(CallFct3(static_cast<uint32_t>(a)));
  return ret;
}

uint8_t CallFct1(uint8_t a) {
  volatile uint8_t ret = static_cast<uint8_t>(CallFct2(static_cast<uint16_t>(a)));
  return ret;
}

// TODO: Read out the stack pointer and check if it is aligned
void TestStackAlignment() {
  printf("# Test: Stack alignment\n");
  assert(StackAlignmentTest() == 0xFE);
  assert(CallFct1(0xDE) == 0xDE);
}