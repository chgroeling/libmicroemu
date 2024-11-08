#include "test_switch_case.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

uint8_t TableFct0() {
  volatile uint32_t temp = 0;
  static_cast<void>(temp);
  return 0;
}
uint8_t TableFct1() {
  volatile uint64_t temp = 0;
  static_cast<void>(temp);
  return 1;
}
uint8_t TableFct2() {
  volatile uint16_t temp = 0;
  static_cast<void>(temp);
  return 2;
}

uint8_t TableFct3() {
  volatile uint8_t temp = 0;
  static_cast<void>(temp);
  return 3;
}

uint8_t TableFct4() {
  volatile int8_t temp = 0;
  static_cast<void>(temp);
  return 4;
}
uint8_t TableFct5() {
  volatile int32_t temp = 0;
  static_cast<void>(temp);
  return 5;
}
uint8_t TableFct6() {
  volatile int16_t temp = 0;
  static_cast<void>(temp);
  return 6;
}

uint8_t TableFct7() {
  volatile int64_t temp = 0;
  static_cast<void>(temp);
  return 7;
}

uint32_t TestTableFct1(char buf) {
  switch (buf) {
  case 0:
    return 1U;
  case 100:
    return 2U;
  case 232:
    return 3U;
  case 115:
    return 4U;
  case 121:
    return 5U;
  case 125:
    return 6U;
  case 12:
    return 7U;
  case 255:
    return 8U;
  default:
    return 1U;
  }
  return 0U;
}
uint32_t TestTableFct2() {
  uint32_t accu = 0;
  for (uint8_t i = 0; i < 8; ++i) {
    switch (i) {
    case 0U:
      accu += TableFct0(); // 0
      break;
    case 1U:
      accu += TableFct1(); // 1
      break;
    case 2U:
      accu += TableFct2(); // 3
      break;
    case 3U:
      accu += TableFct3(); // 6
      break;
    case 4U:
      accu += TableFct4(); // 10
      break;
    case 5U:
      accu += TableFct5(); // 15
      break;
    case 6U:
      accu += TableFct6(); // 21
      break;
    case 7U:
      accu += TableFct7(); // 28
      break;
    default: // clearly an error
      assert(false);
      break;
    }
  }

  return accu;
}

void TestSwitchCase() {
  printf("# Test: Switch/case\n");
  assert(TestTableFct1('s') == 4U);
  assert(TestTableFct1('h') == 1U);
  assert(TestTableFct2() == 28U);
}