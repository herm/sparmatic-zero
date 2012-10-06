/// \file menu.h
/// 
/// 
/// 

#ifndef MENU_H_
#define MENU_H_

void menu(void);
static int16_t inputNumber(int16_t min, int16_t max, int16_t init, int16_t step, uint8_t displayLength);
static uint8_t inputSelector(const char text[][5], uint8_t min, uint8_t max, uint8_t init, int8_t offset); 

#endif