#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "lcd.h"
#include "ntc.h"

void menu(void)
{
    displayNumber((getNtcTemperature()+5)/10, 3);
    displayAsciiDigit(LCD_DEGREE, 3);
    displaySymbols(LCD_DOT, LCD_DOT);
}
