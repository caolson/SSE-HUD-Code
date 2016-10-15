#ifndef SCREEN_INCL
#define SCREEN_INCL

#define UPPERSCREEN 1
#define LOWERSCREEN 0
#define CLEARSCREEN 0x08

void screenwrite (uint8_t rs,uint8_t rw,uint8_t data, uint8_t screen);

void screenmodewrite ();

void screenmoderead();

void init_screen();

void stringtoscreen (char * str, uint8_t screen);

void screenwrite (uint8_t rs,uint8_t rw,uint8_t data, uint8_t screen);

uint8_t screenread (uint8_t rs, uint8_t rw, uint8_t screen);

void screenupdate();

#endif