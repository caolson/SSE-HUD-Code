#ifndef I2C_INCLUDE
#define I2C_INCLUDE

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_rcc.h"

#define SLAVE_ADDRESS 0x0F              //The slave address
#define I2C_ADDRESS 0x42                //My address

void init_I2C1(void);
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
void I2C_stop(I2C_TypeDef* I2Cx);
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);

#endif