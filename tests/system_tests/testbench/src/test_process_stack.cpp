#include <assert.h>
#include <cassert>
#include <cstdint>
#include <stdint.h>
#include <stdio.h>

// Inline assembly helper to set the PSP
__attribute__((always_inline)) inline void SetPSP(uint32_t pspValue) {
  __asm volatile("MSR PSP, %0\n" ::"r"(pspValue) : "memory"); // Input value for r0
}

uint32_t ReadPSP() {
  uint32_t psp_val;
  __asm volatile("MRS %0, PSP\n" : "=r"(psp_val));
  return psp_val;
}

uint32_t ReadMSP() {
  uint32_t msp_val;
  __asm volatile("MRS %0, MSP\n" : "=r"(msp_val));
  return msp_val;
}

// Function to switch to use PSP
__attribute__((always_inline)) inline void SwitchToPSP() {
  // Read CONTROL register, set the least significant bit (bit 1) to 1 to use PSP
  __asm volatile("MRS r0, CONTROL\n"   // Move CONTROL register to r0
                 "ORR r0, r0, #0x02\n" // Set bit 1 to 1 (use PSP)
                 "MSR CONTROL, r0\n"   // Move the updated value back to CONTROL
                 ::
                     : "r0");
}

__attribute__((always_inline)) inline void SwitchToMSP() {
  // Read CONTROL register, set the least significant bit (bit 1) to 0 to use MSP
  __asm volatile("MRS r0, CONTROL\n"   // Move CONTROL register to r0
                 "BIC r0, r0, #0x02\n" // Set bit 1 to 0 (use MSP)
                 "MSR CONTROL, r0\n"   // Move the updated value back to CONTROL
                 ::
                     : "r0");
}

// Function to read the CONTROL register
uint32_t ReadControl() {
  uint32_t controlValue;
  __asm volatile("MRS %0, CONTROL\n" : "=r"(controlValue));
  return controlValue;
}

void __attribute__((noinline)) ProcessStackFct() {
  auto control{0U};
  control = ReadControl(); // Read the CONTROL register
  assert(control == 0x2U); // Check if the CONTROL register is set to use PSP
}

static uint32_t process_stack[1024]; // Define a stack for the process mode
void TestProcessStack() {
  printf("# Test: Process Stack Pointer\n");
  auto control = ReadControl(); // Read the CONTROL register
  assert(control == 0x0U);      // Check if the CONTROL register is set to use MSP

  uint32_t psp_val =
      reinterpret_cast<uint32_t>(&process_stack[1024]); // Initialize PSP (top of stack)

  SetPSP(psp_val);              // Set the PSP
  assert(ReadPSP() == psp_val); // Check if the PSP is set correctly
  SwitchToPSP();                // Switch to use PSP for stack operations

  ProcessStackFct(); // Call the function to test the process stack

  SwitchToMSP();           // Switch to use MSP for stack operations
  control = ReadControl(); // Read the CONTROL register
  assert(control == 0x0U); // Check if the CONTROL register is set to use MSP
}
