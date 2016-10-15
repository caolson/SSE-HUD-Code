#ifndef SCREEN_INCL
#define SCREEN_INCL

#define UPPERSCREEN 1
#define LOWERSCREEN 0
#define CLEARSCREEN 0x08 //Returns home and     clears screen 
#define RETURNHOME  0x04 //Returns home without clearing screen
#define ENTRYMODE   0x0E //Sets screen to increment and shift 1 character
#define DISPLAYON   0x03 //Display on cursor off blink off
#define BLINKYON    0x0F //Display on cursor on  blink on
#define FUNCTIONSET 0xC3 //Sets to 8bit, and two line display
#define BLANK       "                                                                              "

void screenwrite (uint8_t rs,uint8_t rw,uint8_t data, uint8_t screen);

void screenmodewrite ();

void screenmoderead();

void init_screen();

void stringtoscreen (char * str, uint8_t screen);

void screenwrite (uint8_t rs,uint8_t rw,uint8_t data, uint8_t screen);

uint8_t screenread (uint8_t rs, uint8_t rw, uint8_t screen);

void screenupdate();

char* concat(char* buffer, char* str1, char* str2);

#endif