#include "test_std_algos.h"
#include <assert.h>
#include <cstring>
#include <stdint.h>
#include <stdio.h>

void TestStrlen() {
  const char *test_str1 = "";
  assert(strlen(test_str1) == 0);

  const char *test_str2 = "0";
  assert(strlen(test_str2) == 1);

  const char *test_str3 = "012345678";
  assert(strlen(test_str3) == 9);

  const char *test_str4 = "main.cpp";
  assert(strlen(test_str4) == 8);
}

void TestStrcmp() {
  assert(strcmp("B", "B") == 0);
  assert(strcmp("B", "A") > 0);
  assert(strcmp("B", "C") < 0);

  assert(strcmp("AB", "AB") == 0);
  assert(strcmp("AB", "AA") > 0);
  assert(strcmp("AB", "AC") < 0);

  assert(strcmp("ABC", "ABC") == 0);
  assert(strcmp("ABC", "AAC") > 0);
  assert(strcmp("ABC", "ACC") < 0);

  assert(strcmp("ABCD", "ABCD") == 0);
  assert(strcmp("ABCD", "ABBC") > 0);
  assert(strcmp("ABCD", "ABDD") < 0);

  assert(strcmp("ABCD", "ABCD") == 0);
  assert(strcmp("ABCD", "ABBC") > 0);
  assert(strcmp("ABCD", "ABdD") < 0);

  const char *test_str_cmp = "ABCDEFG";
  assert(strcmp(test_str_cmp, "ABCDEFG") == 0);
  assert(strcmp(test_str_cmp, "ABCdEFG") < 0);
  assert(strcmp(test_str_cmp, "ABCCEFG") > 0);
}

void TestMemCpy() {
  char str1[] = "1234567890";
  char str2[] = "abcdefghij";
  memcpy(str1, str2, 3);
  assert(strcmp(str1, "abc4567890") == 0);
}

void TestMemMove() {
  char str[] = "1234567890";
  memmove(str + 4, str + 3, 3); // copies from [4, 5, 6] to [5, 6, 7]
  assert(strcmp(str, "1234456890") == 0);
}

void TestStdAlgos() {
  printf("# Test: Standard algorithmns\n");
  TestStrlen();
  TestStrcmp();
  TestMemMove();
  TestMemCpy();
}