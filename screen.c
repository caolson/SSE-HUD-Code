#include "stm32f4xx.h"
#include "stm32f4_discovery.h" 
#include "screen.h"

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

Peripherials
None :)
--------------------------*/
  
/*----GLOBAL VARIALBES----*/
uint8_t upperscreen [80];
uint8_t lowerscreen [80];

uint8_t upperupdate = 0;
uint8_t lowerupdate = 0;
  
uint8_t upperloc = 0;
uint8_t lowerloc = 0;
uint8_t upperend = 0;
uint8_t lowerend = 0;

//DON'T CHANGE THIS
//The Char map bits are flipped on the datasheet
//Ascii table with correct corresponding characters

uint8_t charmap[256] = { 
  0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F,
  0x80, 0x88, 0x84, 0x8C, 0x82, 0x8A, 0x86, 0x8E, 0x81, 0x89, 0x85, 0x8D, 0x83, 0x8B, 0x87, 0x8F,
  0x40, 0x48, 0x44, 0x4C, 0x42, 0x4A, 0x46, 0x4E, 0x41, 0x49, 0x45, 0x4D, 0x43, 0x4B, 0x47, 0x4F,
  0xC0, 0xC8, 0xC4, 0xCC, 0xC2, 0xCA, 0xC6, 0xCE, 0xC1, 0xC9, 0xC5, 0xCD, 0xC3, 0xCB, 0xC7, 0xCF,
  0x20, 0x28, 0x24, 0x2C, 0x22, 0x2A, 0x26, 0x2E, 0x21, 0x29, 0x25, 0x2D, 0x23, 0x2B, 0x27, 0x2F,
  0xA0, 0xA8, 0xA4, 0xAC, 0xA2, 0xAA, 0xA6, 0xAE, 0xA1, 0xA9, 0xA5, 0xAD, 0xA3, 0xAB, 0xA7, 0xAF,
  0x60, 0x68, 0x64, 0x6C, 0x62, 0x6A, 0x66, 0x6E, 0x61, 0x69, 0x65, 0x6D, 0x63, 0x6B, 0x67, 0x6F,
  0xE0, 0xE8, 0xE4, 0xEC, 0xE2, 0xEA, 0xE6, 0xEE, 0xE1, 0xE9, 0xE5, 0xED, 0xE3, 0xEB, 0xE7, 0xEF,
  0x10, 0x18, 0x14, 0x1C, 0x12, 0x1A, 0x16, 0x1E, 0x11, 0x19, 0x15, 0x1D, 0x13, 0x1B, 0x17, 0x1F,
  0x90, 0x98, 0x94, 0x9C, 0x92, 0x9A, 0x96, 0x9E, 0x91, 0x99, 0x95, 0x9D, 0x93, 0x9B, 0x97, 0x9F,
  0x50, 0x58, 0x54, 0x5C, 0x52, 0x5A, 0x56, 0x5E, 0x51, 0x59, 0x55, 0x5D, 0x53, 0x5B, 0x57, 0x5F,
  0xD0, 0xD8, 0xD4, 0xDC, 0xD2, 0xDA, 0xD6, 0xDE, 0xD1, 0xD9, 0xD5, 0xDD, 0xD3, 0xDB, 0xD7, 0xDF,
  0x30, 0x38, 0x34, 0x3C, 0x32, 0x3A, 0x36, 0x3E, 0x31, 0x39, 0x35, 0x3D, 0x33, 0x3B, 0x37, 0x3F,
  0xB0, 0xB8, 0xB4, 0xBC, 0xB2, 0xBA, 0xB6, 0xBE, 0xB1, 0xB9, 0xB5, 0xBD, 0xB3, 0xBB, 0xB7, 0xBF,
  0x70, 0x78, 0x74, 0x7C, 0x72, 0x7A, 0x76, 0x7E, 0x71, 0x79, 0x75, 0x7D, 0x73, 0x7B, 0x77, 0x7F,
  0xF0, 0xF8, 0xF4, 0xFC, 0xF2, 0xFA, 0xF6, 0xFE, 0xF1, 0xF9, 0xF5, 0xFD, 0xF3, 0xFB, 0xF7, 0xFF,
};

//Write to screen initialization
//Configure pins PA 1,2,6,7 , PE 10,11,12,13 , and PB 12,13,14,15 for output
void screenmodewrite () {

   //Initialze GPIO Structure
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  // Configure pin to be Output
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
}

//Read from screen initialization
//Configure pins PA 1,2,6,7 , PE 10,11,12,13 , and PB 12,13,14,15 for input
void screenmoderead() {

   //Initialze GPIO Structure
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  // Configure pin to be Output
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
}

//Turns on and clears screen
void init_screen(){
  
  //See Header for screen command definitions
  screenwrite(0,0,FUNCTIONSET,UPPERSCREEN);  
  Delay(10);
  screenwrite(0,0,DISPLAYON,UPPERSCREEN);    
  Delay(10);
  screenwrite(0,0,CLEARSCREEN,UPPERSCREEN);  
  Delay(10);
  
  screenwrite(0,0,FUNCTIONSET,LOWERSCREEN);
  Delay(10);
  screenwrite(0,0,DISPLAYON,LOWERSCREEN);
  Delay(10);
  screenwrite(0,0,CLEARSCREEN,LOWERSCREEN);
  Delay(10);
  
}

//Converts message to be written to screen
void stringtoscreen (char * str, uint8_t screen){ // string
  uint8_t i = 0;
  // Creates strings for upper and lower screens
  while(str[i] > 0 && i < 80) {
    if (screen == UPPERSCREEN) {
      upperscreen[i] = charmap[str[i]];
    }
    else {
      lowerscreen[i] = charmap[str[i]];
    }
    i++;
  }
  //Passes where to write and to update the screen
  if (screen == UPPERSCREEN) {
    upperloc = 0;
    upperend = i;
    upperupdate = 1;
  }
  else {
    lowerloc = 0;
    lowerend = i;
    lowerupdate = 1;
  }
  //Clears screen
  screenwrite(0,0,RETURNHOME,screen);
}

//Write command to screen
void screenwrite (uint8_t rs,uint8_t rw,uint8_t data, uint8_t screen){
 
  screenmodewrite();
  
  //Set rs to 1 and rw to 0 to write text to the screen
  //Set rs to 0 and rw to 0 to change modes and command the screen
  //screenwrite(1,0,"Text",xxxxxSCREEN);
  if (rs){
    GPIO_SetBits(GPIOA,GPIO_Pin_7);
  }
  else{
    GPIO_ResetBits(GPIOA,GPIO_Pin_7);
  }
  if (rw){
    GPIO_SetBits(GPIOA,GPIO_Pin_6);
  }
  else{
    GPIO_ResetBits(GPIOA,GPIO_Pin_6);
  }
  
  Delay(1000);
  
  //Upper Lower Write Select 
  //Screen 1 = UPPERSCREEN 
  //Screen 0 = LOWERSCREEN
  if (screen){
    GPIO_SetBits(GPIOA,GPIO_Pin_1);
  }
  else{
    GPIO_SetBits(GPIOA,GPIO_Pin_2);
  }
  
  Delay(1000);
  
  //Setting data to correct pin
  if (data & 1){
    GPIO_SetBits(GPIOE,GPIO_Pin_10);
  }
  else{
    GPIO_ResetBits(GPIOE,GPIO_Pin_10);
  }

  if (data & 2){
    GPIO_SetBits(GPIOE,GPIO_Pin_11);
  }
  else{
    GPIO_ResetBits(GPIOE,GPIO_Pin_11);
  }
  
  if (data & 4){
    GPIO_SetBits(GPIOE,GPIO_Pin_12);
  }
  else{
    GPIO_ResetBits(GPIOE,GPIO_Pin_12);
  }
  
  if (data & 8){
    GPIO_SetBits(GPIOE,GPIO_Pin_13);
  }
  else{
    GPIO_ResetBits(GPIOE,GPIO_Pin_13);
  }
  
  if (data & 16){
    GPIO_SetBits(GPIOB,GPIO_Pin_12);
  }
  else{
    GPIO_ResetBits(GPIOB,GPIO_Pin_12);
  }
  
  if (data & 32){
    GPIO_SetBits(GPIOB,GPIO_Pin_13);
  }
  else{
    GPIO_ResetBits(GPIOB,GPIO_Pin_13);
  }
  
  if (data & 64){
    GPIO_SetBits(GPIOB,GPIO_Pin_14);
  }
  else{
    GPIO_ResetBits(GPIOB,GPIO_Pin_14);
  }
  
  if (data & 128){
    GPIO_SetBits(GPIOB,GPIO_Pin_15);
  }
  else{
    GPIO_ResetBits(GPIOB,GPIO_Pin_15);
  }

  
  Delay(2000);
  //resets the upper lower write select
  GPIO_ResetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2);
  
  Delay(2000);
  //resets the enable write
  GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7);
  
  
  Delay(1000);
  //Resets data pins
  GPIO_ResetBits(GPIOE,GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13);
  GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
  
  Delay(4000);
  
  
}

//Read command to screen
uint8_t screenread (uint8_t rs, uint8_t rw, uint8_t screen){
  //Configures pins to inputs
  screenmoderead();
  
  //Set rs to 0 and rw to 1 to read from the screen
  if (rs){
    GPIO_SetBits(GPIOA,GPIO_Pin_7);
  }
  else{
    GPIO_ResetBits(GPIOA,GPIO_Pin_7);
  }
  
  if (rw){
    GPIO_SetBits(GPIOA,GPIO_Pin_6);
  }
  else{
    GPIO_ResetBits(GPIOA,GPIO_Pin_6);
  }
  
  Delay(500);
  
  //Upper Lower Select 
  //Screen 1 = Upper 
  //Screen 0 = Lower
  if (screen == UPPERSCREEN){
    GPIO_SetBits(GPIOA,GPIO_Pin_1);
  }
  else{
    GPIO_SetBits(GPIOA,GPIO_Pin_2);
  }
  
  Delay(1000);
  //Reads data from screen and stores to data
  uint8_t data = 0;
  data += (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_10)<<0);
  data += (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)<<1);
  data += (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_12)<<2);
  data += (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13)<<3);
  data += (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)<<4);
  data += (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13)<<5);
  data += (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)<<6);
  data += (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15)<<7);
  
  //resets upper lower select, rs, and rw
  GPIO_ResetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_6|GPIO_Pin_7);
  
  return data;
}

void screenupdate (){
  
   while (upperupdate){
     //Read busy flag
     uint8_t stat = screenread(0,1,UPPERSCREEN);
     //if (stat & 0x80){
       screenwrite(1,0,upperscreen[upperloc],UPPERSCREEN);
       upperloc++;
       if (upperloc >= upperend){
        upperupdate = 0;
        upperloc = 0;
       }
     //}
   }
   while (lowerupdate){
   //Read busy flag
     uint8_t stat = screenread(0,1,LOWERSCREEN);
     //if (!(stat & 0x80)){
       screenwrite(1,0,lowerscreen[lowerloc],LOWERSCREEN);
       lowerloc++;
       if (lowerloc >= lowerend){
        lowerupdate = 0;
        lowerloc = 0;
       }
     //}
   }
}


/* Special concat written for 40 character screen. buffer must be 80 characters long */
char* concat(char* buffer, char* str1, char* str2){
  uint8_t i;
  for (i = 0; i < 80; i++){
   if(i < 40){
      if(str1[i] == 0){
        buffer[i] = ' ';
      }
      else{
        buffer[i] = str1[i];
      } 
    }
    else{
      buffer[i] = str2[i-40];
    }
  }
  
  buffer[79] = 0;
  return buffer;
}


