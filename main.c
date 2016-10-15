#include "main.h"

#define PLAN_C 1                       //Activate Plan C Mode
#define I2CDELAY 200  //Dont set below 100

#define I2C_UPDATE_MAX_SPEED 1 //Each tick is approximately 0.6 ms, so 200 is approximately 100 ms
volatile  float speedMPH = 0;
volatile uint32_t count = 0;
float smallAverage = 0;
float averageSpeed = 0;
int nSmall = 0;
int nAverage = 0;

uint16_t i2cCounter = 0;
uint8_t oldThrottle = 101; //Some value the throttle can never be. 

extern volatile uint8_t I2CTimedOut = 0;
extern volatile uint32_t revolutions;

char* itos(char str[11], uint32_t value);
char* insertString(char* buffer, char* str, uint8_t pos);

void init_I2C_Timeout() {

  NVIC_InitTypeDef      NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  /* TIM5 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  /* Compute the prescaler value */
  uint16_t PrescalerValue = (uint16_t) 71; //Standard Clock Divider of 71 Results in 1 MHz clock
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 1500; //CHANGE ME TO CHANGE I2C TIMEOUT. 750 IS OK, BUT KINDA SLOW
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  
  TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
  
  /* TIM5 enable counter */
  TIM_Cmd(TIM5, DISABLE);
  TIM5->CNT = 0;
} 

extern volatile uint32_t elapsed = 0;

void init_ElapsedTime() {

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

void TIM3_IRQHandler(void) {
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  elapsed++;
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
  
  //GPIOA pin PA3 - Engine RPM
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  //GPIOC pin PC15 - BLDC Mode Switch
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  //GPIOC pin PC13 - PLAN_C Mode Switch
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  //GPIOE pin PE5 - IC Mode Switch
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  //GPIOB pin PB10 - Extra 1
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  //GPIOB pin PB11 - Extra 2
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  //GPIOC pin PC3 - Extra 4
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC, &GPIO_InitStruct); 

}

int main(void){
  TIM4->CCR1 = 0;
  TIM4->CNT = 0x0;
  GPIOB->ODR = GPIOB->ODR&!(0x01<<6);
  TIM_OCInitTypeDef TIM_OCStruct;
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDOn(LED4);
  STM_EVAL_LEDInit(LED5);
  
  
  
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
  
  
  
  Delay(0xFFFFF); //Gives the screen time to wake up before we initialize it
  STM_EVAL_LEDOn(LED5);
  STM_EVAL_LEDOff(LED4);
  
  //SystemInit();
  
  init_InputGPIO();             //Initialize Inputs: Motor Enable, Tach, Brake Lights
  init_Throttle();              //Initialize Throttle
  init_screen();                //Initialize Screen
  init_tach();                  //Initialize Tachometer
  init_brakes();                //Initialize Brake detection
  init_watchdog();              //Initialize Watchdog
  //init_I2C_Timeout();
  init_ElapsedTime();
  
  uint8_t MotorEnabled = 0x00;  //Is motor enabled pressed?
  uint16_t threshold = 10;      //Debounce threshold
  uint32_t count = 0;           //Debounce Counter
  
  uint8_t enable = 0;           //Debounce motor enable
  //Currently 7 bits of 0 + motor enable
  uint8_t status = 0;           //Status byte. LSb is enable
  
  uint8_t brake = 0;
  
  
  uint8_t mcspeed = 0;          //Motor controller speed in rev/s
  uint8_t mcstatus = 0;         //Motor controller status
  //Bit 0 = echo enable
  //Bit 1 = fault pin
  //Bits 2-7 = 0
  
  uint16_t updatecount = 0;
  
  uint8_t LEDCount = 0;
  
  TIM4->CCR1 = 0;
  TIM4->CNT = 0xff;
  
  //stringtoscreen("SSE 2016 Heads Up Display System This is junk text to make it longer ~",UPPERSCREEN);
  //stringtoscreen(itos(88), LOWERSCREEN);
  
  
  while (1){
    
    LEDCount++;
    if(LEDCount > 250) {
      STM_EVAL_LEDToggle(LED4);
      STM_EVAL_LEDToggle(LED5);
      LEDCount=0;
    }
    
    MotorEnabled = (GPIOA->IDR>>0x00)&0x01;
    //Motor enable/disabled and debouncing
    if (MotorEnabled){           
      count++;
      if (count > threshold){            //Safeguard from rollover on counter
        if (count > 1000000000){
          count = threshold + 1;
        }
        enable = 1;                      //Motor is hot
      } else{
        enable = 0;                      //Motor is cold
      }
    } else{      
      count = 0;
      enable = 0;
    }
    
    updateThrottle(enable, brake);
    
    
    if (updatecount == (PLAN_C ? 50 : 200)) {                  //Update times for Plan_C or I2C
      char stru[80];                                               //Defining string lengths
      char strl[80];                                               //Defining string lengths
      char str1[40] = "                                       ";
      char str2[40] = "                                       ";
      char str3[40] = "                                       ";
      char str4[40] = "                                       ";
      
      char buf[11];
      
      insertString(str1, "Speed", 0);                      //Unconcatenated strings
      insertString(str1, "Ave", 6);   
      insertString(str1, "Throttle",12);     
      insertString(str1, "Elapsed, s", 25);
      
      insertString(str2, itos(buf, speedMPH), 0);
      insertString(str2, itos(buf, averageSpeed), 6);
      insertString(str2, itos(buf, currentThrottle), 12);
      insertString(str2, itos(buf, elapsed), 25);
      concat(strl, str1, str2);        
      
      insertString(str3, enable ? "ENABLED" : "DISABLED",0);
      insertString(str3, "Distance, ft", 25);
      
      insertString(str4, itos(buf, revolutions*(TIRE_DIAMETER*3.1415)/12), 25);
      concat(stru, str3, str4);
      
      //sprintf(str2, "%3.1f   %3.1f   %2d     %2d", speedMPH, averageSpeed, currentThrottle, mcspeed);
                               
      stringtoscreen(strl, LOWERSCREEN);                           //Printing to screen
      stringtoscreen(stru, UPPERSCREEN);
      updatecount = 0;                                            //Reset update time
    }
    updatecount++;
    
    
    //Tach
    //Perform a two layer average
    if(nSmall<100){
      nSmall++;
      smallAverage = (speedMPH + nSmall*smallAverage)/(nSmall+1);
    }else{
      nAverage++;
      averageSpeed = (smallAverage + nAverage*averageSpeed)/(nAverage+1);
      nSmall =0;
      smallAverage = 0;
    }
    
    //Calculation for throttle output using tach
    
    screenupdate();
    
    //Watchdog feeding
    IWDG_ReloadCounter();
    
    //Update status byte
    status = enable<<0;
    
    
    
    if(!PLAN_C) {
      
      //TIM_Cmd(TIM5, ENABLE); //Start timeout counter
      //I2CTimedOut = 0;
      
      //Send data to motor controller via I2C
      I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode
      
      I2C_write(I2C1, 0x01);
      //Send the speed
      I2C_write(I2C1, currentThrottle);
      I2C_stop(I2C1);
      
      //    I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode
      //    I2C_write(I2C1, 0x02);
      //    //Send the status byte
      //    I2C_write(I2C1, status);
      //    I2C_stop(I2C1); // stop the transmission
      //   
      //    
      I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode
      I2C_write(I2C1, 0x01);
      I2C_stop(I2C1);
      //    
      //    //Read data from motor controller via I2C
      I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Receiver);    // start a transmission in Master receiver mode
      //    //Read Speed Byte
      mcspeed = I2C_read_nack(I2C1);
      I2C_stop(I2C1);
      //    //Read Status Byte
      //    mcstatus = I2C_read_nack(I2C1); // read one byte and don't request another byte, stop transmission
      //    //Deal with this information
      //}
      
      //TIM_Cmd(TIM5, DISABLE);
      //TIM5->CNT = 0; //Stop and Reset timeout counter
      
    }else { //PLAN C - just modify the PWM duty cycle
      TIM4->CCR1 = ((8399 + 1) * currentThrottle) / 100; //Pulse Length
    }
    Delay(0x15F);
    
  }
}

void Delay(__IO uint32_t nCount){
  while(nCount--)
  {
  }
}

char* itos(char str[11], uint32_t value) {

  sprintf(str, "%d", value);
  str[10]=0;
  
  return str;
}



/* A special purpose method for assembling output for a 40 character display
	Buffer must be a string of length 40
	Str is the string to include at position pos
	Returns a null terminated buffer
*/
char* insertString(char* buffer, char* str, uint8_t pos) {
	
	uint8_t len = 0, i = 0;
	while(str[len] != 0) { len++; } //Find the length of the string

	for (i = 0; i < (len) && (pos+i) < 39; i++) { //Copy everything but the null terminator, or until we hit the end of the buffer
		buffer[pos+i] = str[i];
	}

	buffer[39] = 0; //Ensure the final string is null terminated
	return buffer;

}
