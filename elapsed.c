#include "stm32f4xx.h"
#include "stm32f4_discovery.h" 
#include "Elapsed.h"

extern volatile uint32_t elapsed = 0;

void init_ElapsedTime(void) { // used by the screen //TODO: make this actually one second

  NVIC_InitTypeDef      NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  
  /* Compute the prescaler value */
  uint16_t PrescalerValue = (uint16_t) 2666; //Standard Clock Divider of 71 Results in 1 MHz clock
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 10000;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
  
  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
} 


void TIM3_IRQHandler(void) { // used by the screen
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  elapsed++;
}

