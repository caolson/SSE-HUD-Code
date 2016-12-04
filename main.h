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

#include "screen.h"
#include "tach.h"
#include "throttle.h"  
#include "watchdog.h" 
#include "i2c.h"
#include "elapsed.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*------MODE TYPE------*/
#define IC              1
#define BLDC            2
#define PLAN_C          3

/*--------------------I2C------------------------*/
#define I2CDELAY                200  //Dont set below 100
#define I2C_UPDATE_MAX_SPEED    1 //Each tick is approximately 0.6 ms, so 200 is approximately 100 ms

/*--------------Car Constants--------------------*/
#define TIRE_DIAMETER           19
#define TACH_TIMER_FREQUENCY    375000

/*----------Global Variable Definitions----------*/
extern volatile float speedMPH;         //Throttle
extern volatile uint32_t count;         //TODO: don't know
extern volatile uint32_t seconds;       //Elapsed


/*--------------Useful Functions-----------------*/
void Delay(__IO uint32_t nCount);
char* itos(char str[11], uint32_t value);
char* insertString(char* buffer, char* str, uint8_t pos); 

#endif