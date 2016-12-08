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

/*--------------Car Constants--------------------*/
#define TIRE_DIAMETER           19

/*----------Global Variable Definitions----------*/
extern volatile float speedMPH;         //Throttle
extern volatile uint32_t count;         //TODO: don't know
extern volatile uint32_t seconds;       //Elapsed
extern volatile uint32_t revolutions;

/*--------------Useful Functions-----------------*/
void Delay(__IO uint32_t nCount);
char* itos(char str[11], uint32_t value);
char* insertString(char* buffer, char* str, uint8_t pos); 

#endif