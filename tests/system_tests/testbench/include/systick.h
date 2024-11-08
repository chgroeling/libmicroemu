
#pragma once

#include <stdint.h>

// Systick register map
struct SysTickRegMap {
  volatile uint32_t CSR; // Control and status register
  volatile uint32_t RVR; // Reload value register
  volatile uint32_t CNT; // Current value register {"count"}
  volatile uint32_t CVR; // Calibration value register
};

// SysTick register map base pointer
static inline SysTickRegMap *GetSysTickBase() {
  return reinterpret_cast<SysTickRegMap *>(reinterpret_cast<unsigned *>(0xE000E010U));
};

// Control and status registers
static constexpr uint32_t kSystickCsrCountFlag{16U};
static constexpr uint32_t kSystickCsrClkSource{2U};
static constexpr uint32_t kSystickCsrClkSourceExternal{0U};
static constexpr uint32_t kSystickCsrClkSourceCore{2U};
static constexpr uint32_t kSystickCsrTickInt{1U};
static constexpr uint32_t kSystickCsrTickIntPend{1U};
static constexpr uint32_t kSystickCsrTickIntNoPend{0U};
static constexpr uint32_t kSystickCsrEnable{0U};
static constexpr uint32_t kSystickCsrEnableMultishot{0U};
static constexpr uint32_t kSystickCsrEnableDisabled{0U};

// Calibration value register
static constexpr uint32_t kSystickCvrNoRef{31U};
static constexpr uint32_t kSystickCvrSkew{30U};
static constexpr uint32_t kSystickCvrTenms{0xFFFFFFu};

extern volatile uint32_t systick_uptime_millis;

static inline uint32_t SysTickUpTime(void) { return systick_uptime_millis; }

void SysTickInit(uint32_t reload_val);
void SysTickDisable();
void SysTickEnable();

static inline uint32_t SystickGetCount(void) { return GetSysTickBase()->CNT; }

static inline uint32_t SystickCheckUnderflow(void) {
  return GetSysTickBase()->CSR & kSystickCsrCountFlag;
}
