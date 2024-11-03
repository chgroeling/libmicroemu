#include "test_fibonacci.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// Recursive fibonacci algorithm
int Fibonacci(int n) {
  if (n <= 1) {
    return n;
  } else {
    return Fibonacci(n - 1) + Fibonacci(n - 2);
  }
}

void TestFibonacci() {
  printf("# Test: Fibonacci (recursive)\n");
  assert(Fibonacci(5) == 5u);
  assert(Fibonacci(6) == 8u);
  assert(Fibonacci(7) == 13u);
  assert(Fibonacci(8) == 21u);
  assert(Fibonacci(7) == 13u);
  assert(Fibonacci(6) == 8u);
  assert(Fibonacci(5) == 5u);
}