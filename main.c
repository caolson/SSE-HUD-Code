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
#include "main.h"
#include "screen.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PLAN_C 1                        //Activate Plan C Mode

#define THROTTLE_MAX 50 //Throttle is limited to this value
#define THROTTLE_MIN 3  //Throttle goes to 0 below this value

__IO uint32_t CaptureNumber = 0, PeriodValue = 0;
uint32_t GetLSIFrequency(void);
uint8_t speedMPH = 0;

uint8_t didItWork = 0;

char* itos(uint8_t in);

void init_tach(void) {
    /* Set variables used */
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    /* Enable clock for GPIOB */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    /* Enable clock for SYSCFG */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    
    /* Set pin as input */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
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
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
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
    
    
    
    //TODO: Initialize timer
    //TODO: Create timer interupt handler
    
}

void init_watchdog() {
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  uint32_t lsiFreq = 32000; //GetLSIFrequency();
  IWDG_SetPrescaler(IWDG_Prescaler_32);
  IWDG_SetReload(lsiFreq/32); //Magic Constant for 1 sec
  IWDG_ReloadCounter();
  IWDG_Enable();
}

void init_InputGPIO(void){
  /* Configure pin to be Input 
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_x;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOx, &GPIO_InitStruct);
  */
  
  //Enabling Clocks for GPIOs
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
  
  //Initialze GPIO Structure
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  //GPIOA pin PA0 - Motor Enable
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  //GPIOB pin PB1 - Tachometer Reading
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  //GPIOA pin PA3 - Brake Light Signal
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void init_OutputGPIO(void){
  //Initialze GPIO Structure
  //GPIO_InitTypeDef  GPIO_InitStruct;
  
  /* Configure pin to be Output
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_xx
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOx, &GPIO_InitStruct);
  */
}

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
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;			// enable pull up resistors
    GPIO_Init(GPIOB, &GPIO_InitStruct);			        // init GPIOB
  
     
 //   GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
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
    TIM_OCStruct.TIM_Pulse = 2099;
    TIM_OC1Init(TIM4, &TIM_OCStruct);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

}


int main(void){
  TIM_OCInitTypeDef TIM_OCStruct;
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  
  //SystemInit();
  
  if (!PLAN_C) {
    init_I2C1();                //Initialize I2C

    TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
  }
  else {
   TM_TIMER_Init();                 //Initialize PWM output for brushed motor on PB6
   TM_PWM_Init();
  }
  
  init_InputGPIO();             //Initialize Inputs: Motor Enable, Tach, Brake Lights
  init_Throttle();              //Initialize Throttle
  init_screen();
  init_tach();
  init_watchdog();
  
  //EXTI->SWIER = EXTI_Line1;
 
  uint8_t MotorEnabled = 0x00;  //Is motor enabled pressed?
  uint16_t threshold = 10;      //Debounce threshold
  uint32_t count = 0;           //Debounce Counter
  
  
  uint32_t throttle = 0;         //Normalized throttle value
  uint8_t enable = 0;           //Debounce motor enable
  //Currently 7 bits of 0 + motor enable
  uint8_t status = 0;           //Status byte. LSb is enable
  
  
  uint8_t mcspeed = 0;          //Motor controller speed in rev/s
  uint8_t mcstatus = 0;         //Motor controller status
  //Bit 0 = echo enable
  //Bit 1 = fault pin
  //Bits 2-7 = 0
  
  uint16_t updatecount = 0;
  uint8_t test = 0;
  
  
  stringtoscreen("SSE 2016 Heads Up Display System This is junk text to make it longer ~",UPPERSCREEN);
  stringtoscreen(itos(88), LOWERSCREEN);
  
  Delay(500);
  
  while (1){
   MotorEnabled = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0);
   if (MotorEnabled){
     count++;
     if (count > threshold){
       if (count > 1000000000){
           count = threshold + 1;
       }
       enable = 1; //Motor is hot
     } else{
       enable = 0; 
     }
   } else{      
     count = 0;
     enable = 0;
   }
   //Read Throttle value
   throttle = POTGetValue(); //Normalize to 0-100
   if (!enable) { throttle = 0; } //Do not send a value if motor is disabled
   if (throttle > 4000) { throttle = 4000; }
   if (throttle < THROTTLE_MIN) { throttle = 0; }
   throttle = throttle/40;
   if (throttle > THROTTLE_MAX) { throttle = THROTTLE_MAX; }
  
   if (updatecount == (PLAN_C ? 1000 : 100)) {
     test++;
     char str[10];
     sprintf(str, "%d %d", throttle, test);
     stringtoscreen(str, LOWERSCREEN);
     
   Delay(0xFFFFF);
     //stringtoscreen(enable ? " Enabled" : " Disabled", UPPERSCREEN);
     stringtoscreen(didItWork ? " Yes" : " No ", UPPERSCREEN);
     updatecount = 0;
   }
   updatecount++;
   
   //Read Break press Boolean
   
   //Tach
   
   //Calculation for throttle output using tach
   
   screenupdate();
   
   //Watchdog feeding
   IWDG_ReloadCounter();
   
   //Update status byte
   status = enable<<0;
   
   if(!PLAN_C) {
    //Send data to motor controller via I2C
    I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode
    
    I2C_write(I2C1, 0x01);
    //Send the speed
    I2C_write(I2C1, throttle);
    I2C_stop(I2C1);
    
    I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode
    I2C_write(I2C1, 0x02);
    //Send the status byte
    I2C_write(I2C1, status);
    I2C_stop(I2C1); // stop the transmission
   
    
    I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode
    I2C_write(I2C1, 0x01);
    I2C_stop(I2C1);
    
    //Read data from motor controller via I2C
    I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Receiver); // start a transmission in Master receiver mode
    //Read Speed Byte
    mcspeed = I2C_read_ack(I2C1);
    //Read Status Byte
    mcstatus = I2C_read_nack(I2C1); // read one byte and don't request another byte, stop transmission
    //Deal with this information
   }
   
   else { //PLAN C - just modify the PWM duty cycle
    TIM4->CCR1 = ((8399 + 1) * throttle) / 100; //Pulse Length
    }
   Delay(0x5F);
   
  }
}

void Delay(__IO uint32_t nCount){
  while(nCount--)
  {
  }
}

char* itos(uint8_t value) {
  char* strs[101] = { "0",
   "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
   "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
   "21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
   "31", "32", "33", "34", "35", "36", "37", "38", "39", "40",
   "41", "42", "43", "44", "45", "46", "47", "48", "49", "50",
   "51", "52", "53", "54", "55", "56", "57", "58", "59", "60",
   "61", "62", "63", "64", "65", "66", "67", "68", "69", "70",
   "71", "72", "73", "74", "75", "76", "77", "78", "79", "80",
   "81", "82", "83", "84", "85", "86", "87", "88", "89", "90",
   "91", "92", "93", "94", "95", "96", "97", "98", "99", "100" };
  
   return strs[value%101];
}
	
