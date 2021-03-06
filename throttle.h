#ifndef THROTTLE_INCL
#define THROTTLE_INCL

/*------THROTTLE CONSTANTS------*/
#define THROTTLE_MAX 90 //Throttle is limited to this value
#define THROTTLE_MIN 3  //Throttle goes to 0 below this value

/*---------GLOBAL VARIABLES--------*/
extern volatile uint8_t currentThrottle;

/*-------FUNCTION PROTOTYPES------*/
void init_Throttle(void);
int POTGetValue(void);
void TM_TIMER_Init(void);
void TM_PWM_Init(void);
void updateThrottle(uint8_t enable);

#endif