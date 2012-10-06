Firmware for mechatronic radiator thermostats based on Atmel AVR microcontroller, written in C.
Made in Germany by Eurotronic Technology GmbH, several product models (Sparmatic Comet / Zero / Basic, Aldi Thermy) are based on similar hardware.

The aim is to create firmware supporting all hardware features, allowing further development of control algorithms and additional wireless communications, improving and surpassing the original features.

Documentation in German http://www.mikrocontroller.net/articles/Sparmatic_Heizungsthermostate (translation: http://translate.google.com/translate?sl=de&tl=en&js=n&prev=_t&hl=en&ie=UTF-8&layout=2&eotf=1&u=http%3A%2F%2Fwww.mikrocontroller.net%2Farticles%2FSparmatic_Heizungsthermostate&act=url )
Discussion in German http://www.mikrocontroller.net/topic/237375

WORK IN PROGRESS: it may not be working. Please see log.
USE AT YOUR OWN RISK.

Microcontroller: Atmel AVR ATmega169PV @ 1 MHz (internal RC oscillator), 32 Khz crystal
Supply:          2 AA alkaline batteries
Valve actuator:  brushed DC motor controlled by discrete H-bridge, optical tacho
Display:         Segment LCD (4 chars/digits and symbols) driven by ATmega169 internal LCD controller
Input:           5 keys or 3 keys and "scroll" wheel
Temp.-sens.:     NTC internal






Original notes below (https://github.com/NerdyProjects/sparmatic-zero)

sparmatic-zero
==============

my development sources of sparmatic zero. As of 10/2012 I do not plan any more development. There is just basic functionality implemented.


See git log for information.

License:
Everything done by myself is published under GPLv2. Be careful! There may be some code not done by me. (Bootloader snippets from mikrocontroller.net, nRF24L01 Code from MIRF Project and r0cket, eventually some more)
