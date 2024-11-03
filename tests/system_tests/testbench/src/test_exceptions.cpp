#include "test_exceptions.h"
#include "scb.h"
#include "systick.h"
#include <assert.h>
#include <atomic>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

std::atomic<uint32_t> return_adr{0};

extern "C" void BusFault_Handler() __attribute__((naked));
extern "C" void BusFault_Handler_ABI(uint32_t *stack_frame) __attribute__((interrupt("IRQ")));
std::atomic<uint32_t> bus_faults{0u};

/// BusFault_Handler_ABI
/// This function is called from the assembler BusFault_Handler function which is called by the
/// Cortex-M processor when a bus fault occurs. The function oversteps the instruction that caused
/// the fault and increments the PC in the stack frame.
void BusFault_Handler_ABI(uint32_t *stack_frame) {

  uint32_t faulty_pc = stack_frame[6]; // Read PC from stack frame
                                       // printf("BusFault_Handler 0x%08x\n", faulty_pc);

  if (return_adr == 0U) {
    uint16_t *instr = (uint16_t *)(faulty_pc & ~0x1U); // read the instruction that caused the fault
    uint16_t instr_val = *instr;

    if (((instr_val & 0xF800U) == 0xF800U) || ((instr_val & 0xF800U) == 0xE800U) ||
        ((instr_val & 0xF800U) == 0xF000U)) { // Check if the instruction is a 32-Bit instruction
      stack_frame[6U] = faulty_pc + 4U;       // overstep 32-Bit instruction
    } else {
      stack_frame[6U] = faulty_pc + 2U; // overstep 16-Bit instruction
    }
  } else {
    stack_frame[6U] = return_adr;
    return_adr = 0;
  }

  auto *scb = GetSCBBase();
  auto cfsr = scb->CFSR;
  if (cfsr & kCfsrIBusErrFlag) {
    // abort();
  }
  bus_faults++;
}

void BusFault_Handler(void) {
  __asm volatile("MOV    r0, sp                  \n" // Load the current value of the Main Stack
                                                     // Pointer (MSP) into r0
                 "B BusFault_Handler_ABI        \n"  // Call the C function BusFault_Handler_ABI
  );
}

extern "C" void UsageFault_Handler() __attribute__((naked));
extern "C" void UsageFault_Handler_ABI(uint32_t *stack_frame) __attribute__((interrupt("IRQ")));

std::atomic<uint32_t> usage_faults{0u};

/// UsageFault_Handler_ABI
/// This function is called from the assembler UsageFault_Handler function which is called by the
/// Cortex-M processor when a usage fault occurs. The function oversteps the instruction that caused
/// the fault and increments the PC in the stack frame.
void UsageFault_Handler_ABI(uint32_t *stack_frame) {

  uint32_t faulty_pc = stack_frame[6]; // Read PC from stack frame

  if (return_adr == 0U) {
    uint16_t *instr = (uint16_t *)(faulty_pc & ~0x1U); // read the instruction that caused the fault
    uint16_t instr_val = *instr;

    if (((instr_val & 0xF800U) == 0xF800U) || ((instr_val & 0xF800U) == 0xE800U) ||
        ((instr_val & 0xF800U) == 0xF000U)) { // Check if the instruction is a 32-Bit instruction
      stack_frame[6U] = faulty_pc + 4U;       // overstep 32-Bit instruction
    } else {
      stack_frame[6U] = faulty_pc + 2U; // overstep 16-Bit instruction
    }
  } else {

    stack_frame[6U] = return_adr;
    return_adr = 0;
  }

  // some tests set the t-bit intentionally to 0. This is not allowed in the ARMv7-M architecture
  // Reverse the T-bit to 1 in case such an instruction is executed
  auto xpsr = stack_frame[7U];
  xpsr = xpsr | (1 << 24); // Force the T-bit to 1
  stack_frame[7U] = xpsr;

  usage_faults++;
}

void UsageFault_Handler(void) {
  __asm volatile("MOV    r0, sp                  \n"  // Load the current value of the Main Stack
                                                      // Pointer (MSP) into r0
                 "B UsageFault_Handler_ABI        \n" // Call the C function BusFault_Handler_ABI
  );
}

void TestVariousFaults() {
  uint32_t faulty_address = 0x40000001;

  usage_faults = 0U;
  bus_faults = 0U;
  auto *scb = GetSCBBase();
  assert(scb->CFSR == 0U);
  assert(usage_faults == 0U);
  assert(bus_faults == 0U);
  return_adr = reinterpret_cast<uint32_t>(&&label_wrong_mode);
  asm volatile goto("" :: ::label_wrong_mode, label_fetch_fault);
  // provoke an Usage Fault .. wrong execution state
  __asm__ volatile("mov r0, 0x10\n"
                   "BX r0"
                   :      // no output
                   :      // no input
                   : "r0" // clobber
  );

label_wrong_mode:
  __asm__ volatile("nop"
                   : // no output
                   : // no input
                   : // no clobber
  );

  assert(usage_faults == 1U);
  assert(scb->CFSR == kCfsrInvStateFlag);
  scb->CFSR = kCfsrInvStateFlag; // reset fault
  assert(scb->CFSR == 0U);

  // provoke an Bus Fault .. instruction bus fault
  return_adr = reinterpret_cast<uint32_t>(&&label_fetch_fault);

  __asm__ volatile("mov r0, %0\n"
                   "BX r0"
                   :                     // no output
                   : "r"(faulty_address) // no input
                   : "r0"                // no clobber
  );

label_fetch_fault:
  __asm__ volatile("nop"
                   : // no output
                   : // no input
                   : // no clobber
  );
  assert(scb->CFSR == kCfsrIBusErrFlag);
  scb->CFSR = kCfsrIBusErrFlag; // reset fault
  assert(scb->CFSR == 0U);
}

void TestBusfault() {
  bus_faults = 0U;
  auto *scb = GetSCBBase();
  assert(scb->CFSR == 0U);
  assert(bus_faults == 0u);
  assert(scb->CFSR == 0x0U);

  // Provoke a bus fault (write)
  volatile uint32_t *p = reinterpret_cast<uint32_t *>(0xF0000000);
  *p = 0xDEADBEEFU;

  assert(bus_faults == 1U);
  assert(scb->CFSR == (kCfsrBfarValidFlag | kCfsrPreciseErrFlag));
  assert(scb->BFAR == reinterpret_cast<uint32_t>(p));

  // reset bus fault
  scb->CFSR = kCfsrBfarValidFlag;
  assert(scb->CFSR == kCfsrPreciseErrFlag);

  scb->CFSR = kCfsrPreciseErrFlag;

  assert(scb->CFSR == 0x0U);

  // Provoke a bus fault (read)
  p = reinterpret_cast<uint32_t *>(0x0A000000);
  volatile uint32_t x = *p;

  assert(bus_faults == 2U);
  assert(scb->CFSR == (kCfsrBfarValidFlag | kCfsrPreciseErrFlag));
  assert(scb->BFAR == reinterpret_cast<uint32_t>(p));

  // reset bus fault
  scb->CFSR = kCfsrBfarValidFlag | kCfsrPreciseErrFlag;
  assert(scb->CFSR == 0x0U);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiv-by-zero"

void TestDivideByZero() {
  volatile int32_t i_divisor = 0U;
  volatile uint32_t u_divisor = 0U;
  usage_faults = 0U;
  auto *scb = GetSCBBase();
  assert(scb->CFSR == 0U);
  assert((scb->CCR & kCcrDivByZeroTrapEnableFlag) == 0U);
  assert(usage_faults == 0U);

  volatile uint32_t x1 = 123U / u_divisor; // provoke a divide by zero without exception
  assert(usage_faults == 0U);
  assert(x1 == 0);

  volatile int32_t x2 = 123 / i_divisor; // provoke a divide by zero without exception
  assert(usage_faults == 0U);
  assert(x2 == 0);

  // Enable divide by zero trap
  scb->CCR = scb->CCR | kCcrDivByZeroTrapEnableFlag;

  volatile uint32_t x3 = 123U / u_divisor; // provoke a divide by zero exception
  assert(usage_faults == 1U);
  assert(scb->CFSR == kCfsrDivByZeroFlag);
  scb->CFSR = kCfsrDivByZeroFlag; // reset fault
  assert(scb->CFSR == 0U);

  volatile int32_t x4 = 123 / i_divisor; // provoke a divide by zero exception
  assert(usage_faults == 2U);
  assert(scb->CFSR == kCfsrDivByZeroFlag);
  scb->CFSR = kCfsrDivByZeroFlag; // reset fault
  assert(scb->CFSR == 0U);

  // Disable divide by zero trap
  scb->CCR = scb->CCR & ~kCcrDivByZeroTrapEnableFlag;
}
#pragma GCC diagnostic pop

void TestExceptions() {
  printf("# Test: Exceptions\n");

  // SysTickDisable(); // deactivate SysTick and wait for the test to finish

  TestVariousFaults();
  TestBusfault();
  TestDivideByZero();
}