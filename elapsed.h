#ifndef ELAPSED_INCL
#define ELAPSED_INCL

extern volatile uint32_t seconds;

void init_ElapsedTime(void);

void TIM3_IRQHandler(void);

#endif