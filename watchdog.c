#include "stm32f4xx.h"
#include "stm32f4_discovery.h" 
#include "watchdog.h"

/*--------------------------
Peripherals:
Watchdog
--------------------------*/

void init_watchdog() {
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  uint32_t lsiFreq = 32000; //GetLSIFrequency();
  IWDG_SetPrescaler(IWDG_Prescaler_32);
  IWDG_SetReload(lsiFreq/32); //Magic Constant for 1 sec
  IWDG_ReloadCounter();
  IWDG_Enable();
}