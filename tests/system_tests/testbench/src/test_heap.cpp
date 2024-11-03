#include "test_heap.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static constexpr uint32_t kAllocCycles = 33U;
void TestHeap() {
  printf("# Test: Heap\n");

  // get pointer to the actual heap start
  void *heap = malloc(1U);
  void *heap_start = heap;
  free(heap);

  // Test data
  for (uint32_t i = 0u; i < kAllocCycles; i++) {
    uint32_t *p1 = new uint32_t;
    uint16_t *p2 = new uint16_t;
    uint8_t *p3 = new uint8_t;
    void *p4 = malloc(97U); // 97 is a prime number

    assert(p1 != nullptr);
    assert(p2 != nullptr);
    assert(p3 != nullptr);
    assert(p4 != nullptr);
    *p1 = i;
    *p2 = i;
    *p3 = i;

    delete p1;
    delete p2;
    delete p3;
    free(p4);
  }

  heap = malloc(1U);
  assert(heap == heap_start);
  free(heap);
}