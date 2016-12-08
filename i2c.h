#ifndef I2C_INCLUDE
#define I2C_INCLUDE

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_rcc.h"

/*--------------------I2C------------------------*/
#define I2CDELAY                200     //Dont set below 100
#define I2C_UPDATE_MAX_SPEED    1       //Each tick is approximately 0.6 ms, so 200 is approximately 100 ms
#define SLAVE_ADDRESS 0x0F              //The slave address
#define I2C_ADDRESS 0x42                //My address

/*----------------GLOBAL VARIABLES---------------*/
extern volatile uint8_t I2CTimedOut;

/*--------------PROTOTYPE FUNCTIONS--------------*/
void init_I2C1(void);
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
void I2C_stop(I2C_TypeDef* I2Cx);
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);
void init_I2C_Timeout();

#endif