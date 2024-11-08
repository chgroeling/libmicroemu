#include "test_quicksort.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// Quicksort algorithmn
template <typename T> void Swap(T *a, T *b) {
  T temp = *a;
  *a = *b;
  *b = temp;
}

template <typename T> int Partition(T arr[], int low, int high) {
  T pivot = arr[high];
  int i = (low - 1);

  for (int j = low; j <= high - 1; j++) {
    if (arr[j] < pivot) {
      i++;
      Swap(&arr[i], &arr[j]);
    }
  }
  Swap(&arr[i + 1], &arr[high]);
  return (i + 1);
}

template <typename T> void QuickSort(T arr[], int low, int high) {
  if (low < high) {
    int pi = Partition(arr, low, high);

    QuickSort(arr, low, pi - 1);
    QuickSort(arr, pi + 1, high);
  }
}

void TestQuicksort() {
  printf("# Test: Quicksort\n");

  // 32-bit
  volatile uint32_t arr32[] = {9, 3, 7, 1, 5, 4, 8, 6, 2, 0};
  int n32 = sizeof(arr32) / sizeof(arr32[0]);
  QuickSort(arr32, 0, n32 - 1);
  assert(arr32[0] == 0U);
  assert(arr32[1] == 1U);
  assert(arr32[2] == 2U);
  assert(arr32[3] == 3U);
  assert(arr32[4] == 4U);
  assert(arr32[5] == 5U);
  assert(arr32[6] == 6U);
  assert(arr32[7] == 7U);
  assert(arr32[8] == 8U);
  assert(arr32[9] == 9U);

  // 16-bit
  volatile uint16_t arr16[] = {9, 3, 7, 1, 5, 4, 8, 6, 2, 0};
  int n16 = sizeof(arr16) / sizeof(arr16[0]);
  QuickSort(arr16, 0, n16 - 1);
  assert(arr16[0] == 0U);
  assert(arr16[1] == 1U);
  assert(arr16[2] == 2U);
  assert(arr16[3] == 3U);
  assert(arr16[4] == 4U);
  assert(arr16[5] == 5U);
  assert(arr16[6] == 6U);
  assert(arr16[7] == 7U);
  assert(arr16[8] == 8U);
  assert(arr16[9] == 9U);

  // 8-bit
  volatile uint8_t arr8[] = {9, 3, 7, 1, 5, 4, 8, 6, 2, 0};
  int n8 = sizeof(arr8) / sizeof(arr8[0]);
  QuickSort(arr8, 0, n8 - 1);
  assert(arr8[0] == 0U);
  assert(arr8[1] == 1U);
  assert(arr8[2] == 2U);
  assert(arr8[3] == 3U);
  assert(arr8[4] == 4U);
  assert(arr8[5] == 5U);
  assert(arr8[6] == 6U);
  assert(arr8[7] == 7U);
  assert(arr8[8] == 8U);
  assert(arr8[9] == 9U);
}