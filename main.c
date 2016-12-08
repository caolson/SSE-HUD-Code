#include "main.h"

// TODO: change all of the brake code to read the engine hall this will affect brake.c and .h as 
// well as throttle.c and then there will need to be a function and a screen output for the engine rpm 
// figure out what kind of signal is coming from the the engine that will be giving us the RPM
// this will be on PA3

// TODO: change all of the places that say global variables in all the .c s to call out the global and domestic variables

/*--------------------------
GPIO Pinout:
BLDC                    PC15
PLAN_C                  PC13
IC                      PE5
I2C Data (throttle)     PB7
I2C Clock (throttle)    PB6
Tach                    PB1
Engine Hall             PA3
Throttle                PB0
Motor Enable            PC2
Confirmation Button     PA13
Extra_4                 PC3
Extra_1 (I2C)           PB10
Extra_2 (I2C)           PB11
--------------------------*/

/*--------------------------
Screen Pinout:
ENABLE UPPER            PA1 
ENABLE LOWER            PA2 
R/W SIGNAL              PA6 
R/S SIGNAL              PA7
DL0                     PE10
DL1                     PE11 
DL2                     PE12
DL3                     PE13
DH0                     PB12
DH1                     PB13
DH2                     PB14
DH3                     PB15
--------------------------*/

/*----GLOBAL VARIALBES----*/
volatile  float speedMPH = 0;
volatile uint32_t count = 0;  //Debounce counter
volatile uint8_t I2CTimedOut = 0;
uint16_t i2cCounter = 0;
uint8_t oldThrottle = 101; //Some value the throttle can never be. 


//TODO: Move this into a Pins File
void init_InputGPIO(void){ //Clean up what this is called and what pins are doing what To avoid double initialization of pins
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
  
 //GPIOA pin PC2 - Motor Enable
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  
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
  
  //GPIOC pin PC7 - Confirmation Button
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  
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
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC, &GPIO_InitStruct); 
  
  //GPIOC pin PA13 - Motor Confirmation
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &GPIO_InitStruct); 


}

int main(void){
  
  //TODO: make some comments for all of the sections
  float smallAverage = 0;
  float averageSpeed = 0;
  uint32_t nSmall = 0;                 //Change size
  uint32_t nAverage = 0;
  
  uint8_t MotorEnabled = 0x00;  //Is motor enabled pressed?
  uint16_t threshold = 10;      //Debounce threshold
  uint32_t count = 0;           //Debounce Counter
  uint8_t enable = 0;           //LSb is if the motor is enabled or not
  
  uint16_t updatecount = 0;
  
  uint8_t LEDCount = 0;
  
  uint8_t MotorType = 0;
  uint8_t MotorConfirm = 0;
  uint32_t BlinkCounter = 0;
  
  uint32_t hours = 0;
  uint32_t minutes = 0;
 
  // TODO: move all of the unused things to a more managable place
  // TODO: this is I2C and we are choosing to wait to deal with this until BLDC is ready
  //Bit 0 = echo enable
  //Bit 1 = fault pin
  //Bits 2-7 = 0
  uint8_t mcspeed = 0;          //Motor controller speed in rev/s
  //uint8_t mcstatus = 0;         //Motor controller status
   
  //TIM_OCInitTypeDef TIM_OCStruct;

  //Busy LEDs
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDOn(LED5);
  STM_EVAL_LEDOff(LED4);
  
  Delay(0xFFFFF);               //Gives the screen time to wake up before we initialize it
   
  init_InputGPIO();             //Initialize Inputs: Motor Enable, Tach, Brake Lights
  init_Throttle();              //Initialize Throttle
  init_screen();                //Initialize Screen
  init_tach();                  //Initialize Tachometer
  init_watchdog();              //Initialize Watchdog
  init_ElapsedTime();
 
  //init_I2C_Timeout();         //TODO: come back
  
  //TODO: make this readable
  // PWM init to 0
  TIM4->CCR1 = 0;
  TIM4->CNT = 0x0;
  GPIOB->ODR = GPIOB->ODR&!(0x01<<6);
   
  //Read mode switches
  if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5)){
    MotorType = IC;
  }
  else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15)){
    MotorType = BLDC;
  }
  else if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)){
    MotorType = PLAN_C;
  }
  else{
    MotorType = 0;
    stringtoscreen("STOP A BAD THING HAS HAPPENED!!!", UPPERSCREEN); 
    while(1){
    IWDG_ReloadCounter();
    }
  }
 
  //Screen Confirmation 
  while(MotorConfirm != 1){
    MotorConfirm = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13);
    
    if(BlinkCounter == 0){
      stringtoscreen("Confirm MODE", UPPERSCREEN);
      if(MotorType == BLDC){
        stringtoscreen("BLDC MODE", LOWERSCREEN);  
      }
      else if(MotorType == PLAN_C){
        stringtoscreen("PLAN_C MODE", LOWERSCREEN);
      }
      else if(MotorType == IC){
        stringtoscreen("IC ENGINE MODE", LOWERSCREEN);
      }
    }
    else if(BlinkCounter == 100000){
      stringtoscreen(BLANK, UPPERSCREEN);
      stringtoscreen(BLANK, LOWERSCREEN);
    }
    
    BlinkCounter++;
    IWDG_ReloadCounter();
    screenupdate();
    
    if(BlinkCounter == 200000){
      BlinkCounter=0;
    }
  }
  
  if (MotorType == BLDC) {
    init_I2C1();                //Initialize I2C
    
    //TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;                  //TODO: wtf
    //TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    //TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
  }
  else {
    TM_TIMER_Init();                 //Initialize PWM output for brushed motor on PB6
    TM_PWM_Init();
  }
    
  TIM4->CCR1 = 0;                    //TODO: make me readable
  TIM4->CNT = 0xff;
  
  while (1){
    
    //This is for a visual indication that the STM is running properly
    //LED4 and 5 should blick back and forth at a consistant interval
    LEDCount++;
    if(LEDCount > 250) {
      STM_EVAL_LEDToggle(LED4);
      STM_EVAL_LEDToggle(LED5);
      LEDCount=0;
    }
    
    MotorEnabled = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2);             //TODO: change me
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
    
    //Getting the value from the throttle trigger to send to the screen and then to powertrain
    updateThrottle(enable);
    
    //seconds, minutes, hours
    if (seconds>=60){
      minutes++;
      seconds=seconds-60;
    }
    if (minutes>=60){
      hours++;
      minutes=minutes-60;
    }
    
    
    if (updatecount == (PLAN_C ? 50 : 200)) {                  //Update times for Plan_C or I2C
      char stru[80];                                               //Defining string lengths
      char strl[80];                                               //Defining string lengths
      char str1[40] = "                                       ";
      char str2[40] = "                                       ";
      char str3[40] = "                                       ";
      char str4[40] = "                                       ";
      
      char buf[11];
      
      //TODO: Update the Character Map
      
                  //SCREEN OUTPUT
      /*-------------------------------------\ 
      |xxNABLED                 Distance     |
      |MCSPEED                         xxx.xx|
      |Speed Ave   Throttle     Elapsed Time |
      |xx    xx    xx           x:xx:xx      |
      \-------------------------------------*/
      
      //Line one
      insertString(str1, enable ? "ENABLED" : "DISABLED",0);
      insertString(str1, "Distance, ft", 25);
      
      //Line two
      insertString(str2, "MCSPEED", 0);
      insertString(str2, itos(buf, mcspeed), 12);
      insertString(str2, itos(buf, (uint32_t) (revolutions*(TIRE_DIAMETER*3.1415)/12)), 25); //TODO: make the tach speed work
      
      //Line three
      insertString(str3, "Speed", 0);
      insertString(str3, "Ave", 6);   
      insertString(str3, "Throttle",12);     
      insertString(str3, "Elapsed Time", 25);
      
      //Line four
      insertString(str4, itos(buf, (uint32_t) speedMPH), 0);
      insertString(str4, itos(buf, (uint32_t) averageSpeed), 6);
      insertString(str4, itos(buf, currentThrottle), 12);
      insertString(str4, itos(buf, hours), 25); //
      insertString(str4, ":", 26);
      insertString(str4, itos(buf, minutes), 27);
      insertString(str4, ":", 29);
      insertString(str4, itos(buf, seconds), 30);
      
      //Combining lines one and two as well as three and four before pushing to the screen
      concat(stru, str1, str2);
      concat(strl, str3, str4);
      
      //Outputs to screen
      stringtoscreen(strl, LOWERSCREEN);                           //Printing to screen
      stringtoscreen(stru, UPPERSCREEN);
      
      updatecount = 0;                                             //Reset update time
    }
    updatecount++;
        
    //Tach
    //Perform a two layer average
    //TODO: this is dumb
    if(nSmall<100){
      nSmall++;
      smallAverage = (speedMPH + nSmall*smallAverage)/(nSmall+1);
    }else{
      nAverage++;
      averageSpeed = (smallAverage + nAverage*averageSpeed)/(nAverage+1);
      nSmall =0;
      smallAverage = 0;
    }
    
    screenupdate();
    
    //Watchdog feeding
    IWDG_ReloadCounter();
    
    //Update status byte        //TODO: what is that
    //status = enable<<0;
    
    
    //Engine Kill Functionality
    if(MotorType == IC){
      if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == 1){
        GPIO_SetBits(GPIOC,GPIO_Pin_3);
      }
      else{
        GPIO_ResetBits(GPIOC,GPIO_Pin_3);
      }
    }
    
    if(MotorType == BLDC) { //TODO: clean me... this is so bad
      
      //TODO: I2C timeout actually working?
      //TODO: describe what is going on in the I2C interation and have better comments
      //TODO: take out all of the shit 
      
      //TIM_Cmd(TIM5, ENABLE); //Start timeout counter
      //I2CTimedOut = 0;
      
      //Send data to motor controller via I2C
      I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode
      
      I2C_write(I2C1, 0x01);
      
      //Send the speed
      I2C_write(I2C1, currentThrottle);
      I2C_stop(I2C1);
      
      I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode
      I2C_write(I2C1, 0x01);
      I2C_stop(I2C1);
       
      //Read data from motor controller via I2C
      I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Receiver);    // start a transmission in Master receiver mode
      //Read Speed Byte
      mcspeed = I2C_read_nack(I2C1);
      I2C_stop(I2C1);
      
      //Read Status Byte
      //    mcstatus = I2C_read_nack(I2C1); // read one byte and don't request another byte, stop transmission
      //Deal with this information
      //}
      
      //TIM_Cmd(TIM5, DISABLE);
      //TIM5->CNT = 0; //Stop and Reset timeout counter
      
    }
    else 
    { //PLAN C - just modify the PWM duty cycle  //TODO: comment this.
      TIM4->CCR1 = ((8399 + 1) * currentThrottle) / 100; //Pulse Length
    }
    Delay(0x15F);
    
  }
}
//TODO: change to a timer interrupt instead of this
//Should be a seperate interrupt that decrements the nCount--
//Figure out if this is worth the investment cost to make everything easier to work with in the future
//84MHz
void Delay(__IO uint32_t nCount){ //TODO: maybe make this a real delay loop?
  while(nCount--)
  {
  }
}


//TODO: move this into screen.c and .h

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

