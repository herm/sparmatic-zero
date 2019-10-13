# Button handling
Button press raises an external interrupt (`PCINT1_vect`) which (re-)enables the LCD interrupt(`LCD_vect`). 
The LCD interrupt calls the key handler (`keyPeriodicScan`) which sets flags for all pressed keys. These flags are queried from the main loop.

# Power loss
A interrupt is raised (`PCINT0_vect`) when power is lost. All system functions are disabled and the current date & time are written to the EEPROM (`sysShutdown`).

# ADC channels
* 1: NTC
* 2: Motor
* 30: Bandgap 1.1V

# Timers
* LCD frame interrupt (64Hz): Used for button and motor handling
* Timer 0 (1kHz): Overflow (256ms): Timeout for button presses, OCR0: Used for enableTimeout()/setTimeout() for motor control
* Timer 1: unused
* Timer 2 (32Hz): Overflow(8s): RTC, OCR2A: Unknown function in system sleep
