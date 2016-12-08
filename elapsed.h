#ifndef ELAPSED_INCL
#define ELAPSED_INCL

/*------DEFINE CONSTANTS------*/


/*------GLOBAL VARIABLES-------*/
extern volatile uint32_t seconds;

/*-------FUNCTION PROTOTYPES------*/
void init_ElapsedTime(void);
void TIM3_IRQHandler(void);

#endif