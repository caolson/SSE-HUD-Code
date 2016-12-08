#include "stm32f4xx.h"
#include "stm32f4_discovery.h" 
#include "tach.h"

/*--------------------------
GPIO Pinout:
Tach                    PB1

Peripherals:
TIM2 for duration reading 
--------------------------*/

/*------GLOBAL VARIABLES-------*/
uint16_t PrescalerValue = 0;

void init_tach(void) {
    /* Set variables used */
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    
    /* Enable clock for GPIOB */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    /* Enable clock for SYSCFG */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    
    /* Set pin as input */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN; //TODO: This should probably be a no pull or pull up
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* Tell system that you will use PB1 for EXTI_Line1 */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);
    
    /* PB1 is connected to EXTI_Line1 */
    EXTI_InitStruct.EXTI_Line = EXTI_Line1;
    /* Enable interrupt */
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    /* Interrupt mode */
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    /* Triggers on rising and falling edge */
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; //TODO: This should probably be falling edge
    /* Add to EXTI */
    EXTI_Init(&EXTI_InitStruct);
    
    /* Add IRQ vector to NVIC */
    NVIC_EnableIRQ(EXTI1_IRQn);
    /* PB1 is connected to EXTI_Line1, which has EXTI1_IRQn vector */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
    /* Set priority */
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x0F;
    /* Set sub priority */
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x0F;
    /* Enable interrupt */
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    /* Add to NVIC */
    NVIC_Init(&NVIC_InitStruct);
    
      /* TIM2 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  
    /* Compute the prescaler value */
    uint16_t PrescalerValue = (uint16_t) 71; //Standard Clock Divider of 71 Results in 1 MHz clock
  
    /* Time base configuration */
    TIM_TimeBaseStruct.TIM_Period = 1500000;
    TIM_TimeBaseStruct.TIM_Prescaler = PrescalerValue;
    TIM_TimeBaseStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
  
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);
  
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
   
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn ;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct); 
  
    /* TIM2 enable counter */
    TIM_Cmd(TIM2, ENABLE);
}