#include "test_stack_alignment.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static constexpr uint32_t kStackAlign = 8; // Stack is always 8 byte aligned

static uint32_t ReadMSP() {
  uint32_t msp_val;
  __asm volatile("MRS %0, MSP\n" : "=r"(msp_val));
  return msp_val;
}

uint32_t StackAlignmentTest() {
  // check alignment of stack pointer
  assert(ReadMSP() % kStackAlign == 0);

  volatile uint32_t ret32 = 0xCAFEU;
  volatile uint16_t ret16 = static_cast<uint16_t>(ret32);
  volatile uint8_t ret8 = static_cast<uint8_t>(ret16);

  // check alignment of stack pointer
  assert(ReadMSP() % kStackAlign == 0);

  return ret8;
};

uint32_t CallFct3(uint32_t a) {
  // check alignment of stack pointer
  assert(ReadMSP() % kStackAlign == 0);

  volatile uint32_t ret = a;
  return ret;
}

uint16_t CallFct2(uint16_t a) {
  // check alignment of stack pointer
  assert(ReadMSP() % kStackAlign == 0);

  volatile uint16_t ret = static_cast<uint16_t>(CallFct3(static_cast<uint32_t>(a)));
  return ret;
}

uint8_t CallFct1(uint8_t a) {
  // check alignment of stack pointer
  assert(ReadMSP() % kStackAlign == 0);

  volatile uint8_t ret = static_cast<uint8_t>(CallFct2(static_cast<uint16_t>(a)));
  return ret;
}

void TestStackAlignment() {
  printf("# Test: Stack alignment\n");
  auto msp_before = ReadMSP();

  assert(StackAlignmentTest() == 0xFE);

  // check alignment of stack pointer
  assert(ReadMSP() % kStackAlign == 0);

  assert(CallFct1(0xDE) == 0xDE);

  // check alignment of stack pointer
  assert(ReadMSP() % kStackAlign == 0);

  auto msp_after = ReadMSP();
  assert(msp_before == msp_after);
}