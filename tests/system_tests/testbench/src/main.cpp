#include "scb.h"
#include "systick.h"
#include "test_crc.h"
#include "test_exceptions.h"
#include "test_fibonacci.h"
#include "test_float.h"
#include "test_heap.h"
#include "test_if_then_else.h"
#include "test_int_calc.h"
#include "test_irqs.h"
#include "test_polymorphismn.h"
#include "test_process_stack.h"
#include "test_quicksort.h"
#include "test_scb.h"
#include "test_sprintf.h"
#include "test_stack_alignment.h"
#include "test_std_algos.h"
#include "test_std_out.h"
#include "test_switch_case.h"
#include "test_type_casting.h"
#include "test_uint_calc.h"
#include <assert.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Main
// ----------------------------
int main() {

#ifdef MEASURE_TIME
  clock_t start_t, end_t;
  start_t = clock();
#endif
  printf("-------------------------------\n");
  printf("Emulator test bench\n");
  printf("-------------------------------\n");

  TestStdOut();

  // Irq test runs in parallel
  StartIrqTest();

  TestStackAlignment();
  TestHeap();
  TestIfThenElse();
  TestSwitchCase();
  TestIntCalc();
  TestUIntCalc();
  TestFloat();
  TestTypeCasting();
  TestPolymorphismn();
  TestStdAlgos();
  TestSprintf();
  TestScb();
  TestFibonacci();
  TestQuicksort();
  TestCrc();
  TestExceptions();
  TestProcessStack();
  TestIrqs(); // must be at the end
  printf("-------------------------------\n");
#ifdef MEASURE_TIME
  end_t = clock();
  auto diff_t = static_cast<unsigned long>(end_t) - static_cast<unsigned long>(start_t);
  printf("# Test: Time measurement start_t:%u / end_t:%u / diff_t: %u / CLOCKS_PER_SEC:%u \n",
         start_t, end_t, diff_t, CLOCKS_PER_SEC);
#endif
  // ----------------------------
  return EXIT_SUCCESS;
}