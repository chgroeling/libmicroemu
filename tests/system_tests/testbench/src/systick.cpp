
#include "systick.h"

volatile uint32_t systick_uptime_millis;

void SysTickInit(uint32_t reload_val) {
  GetSysTickBase()->RVR = reload_val;
  SysTickEnable();
}

void SysTickDisable() { GetSysTickBase()->CSR = kSystickCsrClkSourceCore; }

void SysTickEnable() {
  /* re-enables init registers without changing reload val */
  GetSysTickBase()->CSR = (kSystickCsrClkSourceCore | kSystickCsrEnable | kSystickCsrTickIntPend);
}
