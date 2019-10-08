# Button handling
Button press raises an external interrupt (`PCINT1_vect`) which (re-)enables the LCD interrupt(`LCD_vect`). 
The LCD interrupt calls the key handler (`keyPeriodicScan`) which sets flags for all pressed keys. These flags are queried from the main loop.

# Power loss
A interrupt is raised (`PCINT0_vect`) when power is lost. All system functions are disabled and the current date & time are written to the EEPROM (`sysShutdown`).