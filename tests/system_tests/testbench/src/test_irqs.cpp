#include "test_irqs.h"
#include "systick.h"
#include <assert.h>
#include <atomic>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define EXPECTED_SYSTICKS 93
#define EXPECTED_SYSTICK_STRING "SysTick_Handler called " TOSTRING(EXPECTED_SYSTICKS) " times"

#define EXPECTED_SVC 10
#define EXPECTED_SVC_STRING "SVC_Handler called " TOSTRING(EXPECTED_SVC) " times"
#undef MEASURE_TIME

// SVC_Handler
// ----------------------------
extern "C" void SVC_Handler() __attribute__((interrupt("IRQ")));

char svc_buffer_0[256U];
char svc_buffer_1[256U];

std::atomic<uint32_t> svc_next_buffer{0U};
std::atomic<uint32_t> svc_call_count{0U};

void SVC_Handler() {
  svc_call_count++;

  // this looks weird; but it is a simple way to stress the emulator.
  // sprintf does a lot of nasty things and is a good test for the emulator
  if (svc_next_buffer == 1U) {
    sprintf(svc_buffer_1, "SVC_Handler called %i times", svc_call_count.load());
    svc_next_buffer = 0U;
  } else {
    sprintf(svc_buffer_0, "SVC_Handler called %i times", svc_call_count.load());
    svc_next_buffer = 1U;
  }

  // Burn cycles

  // During this loop another SysTick_Handler call is expected
  // The emulator should be able to handle this
  for (int i = 0; i < 1000U; i++) {
    __asm__("NOP");
  }
};

// SysTick_Handler
// ----------------------------
extern "C" void SysTick_Handler() __attribute__((interrupt("IRQ")));
char systick_buffer_0[256U];
char systick_buffer_1[256U];

std::atomic<uint32_t> systick_next_buffer{0U};
std::atomic<uint32_t> systick_call_count{0U};

void SysTick_Handler() {

  // Call SVC_Handler every 10th time
  if (systick_call_count % 10 == 0U) {
    __asm__("SVC #0x02           \n" : : :);
  }

  systick_call_count++;

  // this looks weird; but it is a simple way to stress the emulator.
  // sprintf does a lot of nasty things and is a good test for the emulator
  if (systick_next_buffer == 1U) {
    sprintf(systick_buffer_1, "SysTick_Handler called %i times", systick_call_count.load());
    systick_next_buffer = 0U;
  } else {
    sprintf(systick_buffer_0, "SysTick_Handler called %i times", systick_call_count.load());
    systick_next_buffer = 1U;
  }
};

void StartIrqTest() {
  // Initialize SysTick
  SysTickInit(2000); // assume this processor runs at 100MHz and the Systick at 500Hz
}

void TestIrqs() {
  printf("# Test: IRQs\n");
  SysTickDisable(); // deactivate SysTick and wait for the test to finish

  if (systick_call_count != EXPECTED_SYSTICKS) {
    printf("SysTick_Handler called %i times but expected were %i\n", systick_call_count.load(),
           EXPECTED_SYSTICKS);

    assert(systick_call_count != EXPECTED_SYSTICKS);
  };

  // take the actual buffer
  if (systick_next_buffer == 1U) {
    assert(strcmp(EXPECTED_SYSTICK_STRING, systick_buffer_0) == 0U);
  } else {
    assert(strcmp(EXPECTED_SYSTICK_STRING, systick_buffer_1) == 0U);
  }

  if (svc_call_count != EXPECTED_SVC) {
    printf("SVC_Handler called %i times but expected were %i\n", svc_call_count.load(),
           EXPECTED_SVC);

    assert(svc_call_count != EXPECTED_SVC);
  };

  // take the actual buffer
  if (svc_next_buffer == 1U) {
    assert(strcmp(EXPECTED_SVC_STRING, svc_buffer_0) == 0U);
  } else {
    assert(strcmp(EXPECTED_SVC_STRING, svc_buffer_1) == 0U);
  }
}