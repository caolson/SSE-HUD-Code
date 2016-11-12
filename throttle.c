#include "stm32f4xx.h"
#include "stm32f4_discovery.h" 
#include "throttle.h"


  volatile uint8_t currentThrottle = 0;

  uint8_t i;
  uint8_t throtave[50] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // *Make every element 0* 
  uint8_t ringbufferpos = 0;
  float buffave = 0;
  uint8_t currentthrottle = 0;
  uint32_t throttle = 0;        //Normalized throttle value


void init_Throttle(void){
  // Enable ADC clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  //Initialze GPIO and ADC Structure
  GPIO_InitTypeDef  GPIO_InitStruct;
  ADC_InitTypeDef ADC_InitStructure;
  
  //GPIOB pin PB0 - Throttle Signal as Analog Input
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  //ADC1 Regular Channel 12 Configuration
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
  //Enable ADC1
  ADC_Cmd(ADC1, ENABLE);
  
  ADC_RegularChannelConfig(ADC1,ADC_Channel_8,1,ADC_SampleTime_144Cycles);
}

int POTGetValue(void){                  //Used to get int value of Throttle
  
  ADC_SoftwareStartConv(ADC1);
  while(ADC_GetSoftwareStartConvStatus(ADC1));
  return ADC_GetConversionValue(ADC1);
}
 
void TM_TIMER_Init(void) {
  TIM_TimeBaseInitTypeDef TIM_BaseStruct;
    
    /* Enable clock for TIM4 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
  
  GPIO_InitTypeDef  GPIO_InitStruct;
    
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;           // I2C will use PB6 for Plan C PWM
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;		// set GPIO speed
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			// set output to open drain --> the line has to be only pulled low, not driven high
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;		// enable pull up resistors
  GPIO_Init(GPIOB, &GPIO_InitStruct);			        // init GPIOB
  
     
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
  //  GPIOB->AFR[GPIO_PinSource6 >> 0x03] = 0x02;
    
/*    
    TIM4 is connected to APB1 bus, which has on F407 device 42MHz clock                 
    But, timer has internal PLL, which double this frequency for timer, up to 84MHz     
    Remember: Not each timer is connected to APB1, there are also timers connected     
    on APB2, which works at 84MHz by default, and internal PLL increase                 
    this to up to 168MHz                                                             
    
    Set timer prescaller 
    Timer count frequency is set with 
    
    timer_tick_frequency = Timer_default_frequency / (prescaller_set + 1)        
    
    In our case, we want a max frequency for timer, so we set prescaller to 0         
    And our timer will have tick frequency        
    
    timer_tick_frequency = 84000000 / (0 + 1) = 84000000 
*/    
    TIM_BaseStruct.TIM_Prescaler = 0;
    /* Count up */
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
/*
    Set timer period when it have reset
    First you have to know max value for timer
    In our case it is 16bit = 65535
    To get your frequency for PWM, equation is simple
    
    PWM_frequency = timer_tick_frequency / (TIM_Period + 1)
    
    If you know your PWM frequency you want to have timer period set correct
    
    TIM_Period = timer_tick_frequency / PWM_frequency - 1
    
    In our case, for 10Khz PWM_frequency, set Period to
    
    TIM_Period = 84000000 / 10000 - 1 = 8399
    
    If you get TIM_Period larger than max timer value (in our case 65535),
    you have to choose larger prescaler and slow down timer tick frequency
*/
    TIM_BaseStruct.TIM_Period = 8399; /* 10kHz PWM */
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_BaseStruct.TIM_RepetitionCounter = 0;
    /* Initialize TIM4 */
    TIM_TimeBaseInit(TIM4, &TIM_BaseStruct);
    /* Start count on TIM4 */
    
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
          
    TIM_Cmd(TIM4, ENABLE);
}

void TM_PWM_Init(void) {
    TIM_OCInitTypeDef TIM_OCStruct;
    
    /* Common settings */
    
    /* PWM mode 2 = Clear on compare match */
    /* PWM mode 1 = Set on compare match */
    TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    
/*
    To get proper duty cycle, you have simple equation
    
    pulse_length = ((TIM_Period + 1) * DutyCycle) / 100 - 1
    
    where DutyCycle is in percent, between 0 and 100%
    
    25% duty cycle:     pulse_length = ((8399 + 1) * 25) / 100 - 1 = 2099
    50% duty cycle:     pulse_length = ((8399 + 1) * 50) / 100 - 1 = 4199
    75% duty cycle:     pulse_length = ((8399 + 1) * 75) / 100 - 1 = 6299
    100% duty cycle:    pulse_length = ((8399 + 1) * 100) / 100 - 1 = 8399
    
    Remember: if pulse_length is larger than TIM_Period, you will have output HIGH all the time
*/
    TIM_OCStruct.TIM_Pulse = 0;
    TIM_OC1Init(TIM4, &TIM_OCStruct);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

}

void updateThrottle(uint8_t enable) {            //TODO: Brake input why?
  //Read Throttle value // ADD LOW PASS FILTER AND RAMPING
  //Sets throttle to zero if the brake is pressed.
  
   throttle = POTGetValue();                                    //Normalize to 0-100
   if (!enable) { 
      throttle = 0;                                             //Do not send a value if motor is disabled 
   } else{
    if (throttle > 4000) { throttle = 4000; }
    throttle = throttle/40;                                     //Normalized to 0-100
    if (throttle > THROTTLE_MAX) { throttle = THROTTLE_MAX; }   //Throttle min and max defined in header
    if (throttle < THROTTLE_MIN) { throttle = 0; }
   }
   
   buffave -= throtave[ringbufferpos]/50.0;
   throtave[ringbufferpos] = throttle;
   buffave += throttle/50.0;
   ringbufferpos++; 
   if (ringbufferpos >= 50) { ringbufferpos = 0; }
   currentThrottle = buffave;
}