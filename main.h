#ifndef __MAIN
#define __MAIN

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_iwdg.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "system_stm32f4xx.h"
#include "misc.h"
#include "screen.h"
#include "i2c.h"
#include "tach.h"
#include "throttle.h"
#include "brake.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern volatile uint8_t didItWork;
extern volatile float speedMPH;
extern volatile uint32_t count;

#define TIRE_DIAMETER 19
#define TACH_TIMER_FREQUENCY 375000

void Delay(__IO uint32_t nCount);


#endif