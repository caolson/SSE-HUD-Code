#include "i2c.h"
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_rcc.h"


/*--------------------------
GPIO Pinout:
I2C Data (throttle)     PB7
I2C Clock (throttle)    PB6

Peripherals:
I2C1 for I2C
TIM5 for I2C Timeout 
--------------------------*/

void init_I2C1(void){                   //I2C1 with SCL @ PB6 and SDA @ PB7
  //Initialze GPIO Structure
  GPIO_InitTypeDef  GPIO_InitStruct;
  //I2C initializing structure
  I2C_InitTypeDef I2C_InitStruct;
  
  //GPIO Periph clock enable (SET GPIOx/I2Cx TO CORRECT DESTINATION)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
          
  //Configure Pins for I2C
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;           // I2C will use PB6 and PB7 for I2C1
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
  GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
  GPIO_Init(GPIOB, &GPIO_InitStruct);			        // init GPIOB
  
  // Connect I2C1 pins to AF  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);	// SCL
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);       // SDA
  
  //Configure I2C in Master Mode
  I2C_InitStruct.I2C_ClockSpeed = 100000; 		                 // 100kHz
  I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;			         // I2C mode
  I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;	                 // 50% duty cycle --> standard
  I2C_InitStruct.I2C_OwnAddress1 = I2C_ADDRESS;			         // own addres
  I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;		                 // disable acknowledge when reading (can be changed later on)
  I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
  I2C_Init(I2C1, &I2C_InitStruct);
  
  //Enable I2C
  I2C_Cmd(I2C1, ENABLE);
  
  //Enable Event interupt for I2C
  I2C_ITConfig(I2C1,I2C_IT_EVT,ENABLE);
}

void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction){
  // wait until I2C1 is not busy anymore
  while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
  
  // Send I2C1 START condition 
  I2C_GenerateSTART(I2Cx, ENABLE);
	  
  // wait for I2C1 EV5 --> Slave has acknowledged start condition
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
		
  // Send slave Address for write 
  I2C_Send7bitAddress(I2Cx, address, direction);
	  
  /* wait for I2C1 EV6, check if 
    * either Slave has acknowledged Master transmitter or
    * Master receiver mode, depending on the transmission
    * direction
  */ 
  if(direction == I2C_Direction_Transmitter){
          while(!(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) || I2CTimedOut));
  }
  else if(direction == I2C_Direction_Receiver){
	while(!(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)  || I2CTimedOut));
  }
}

void I2C_write(I2C_TypeDef* I2Cx, uint8_t data){
	I2C_SendData(I2Cx, data);
	// wait for I2C1 EV8_2 --> byte has been transmitted
	while(!(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)  || I2CTimedOut));
}

void I2C_stop(I2C_TypeDef* I2Cx){
	// Send I2C1 STOP Condition 
	I2C_GenerateSTOP(I2Cx, ENABLE);
}

uint8_t I2C_read_nack(I2C_TypeDef* I2Cx){
	// disabe acknowledge of received data
	// nack also generates stop condition after last byte received
	// see reference manual for more info
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)  || I2CTimedOut));
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

uint8_t I2C_read_ack(I2C_TypeDef* I2Cx){
	// enable acknowledge of recieved data
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)  || I2CTimedOut) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

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