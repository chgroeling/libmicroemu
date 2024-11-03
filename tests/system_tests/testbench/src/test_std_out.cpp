#include "test_std_out.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void TestStdOut() {
  printf("# Test: Standard output\n");

  // char tests
  fputc(' ', stdout);
  fputc(' ', stdout);
  fputc('T', stdout);
  fputc('E', stdout);
  fputc('S', stdout);
  fputc('T', stdout);
  fputc('!', stdout);
  fputc('\n', stdout);

  printf("  Hello World\n");

  printf("  0123456789012345678901234567890123456789\n");

  fprintf(stdout, "  fprintf output\n");
  iprintf("  iprintf output\n");
}