MCU = atmega169pa
MCU_PROGRAMMER = m169
F_CPU = 1000000UL

Q = @
OPT = s

SRC = 
CPPSRC = main.cpp encoder.cpp keys.cpp lcd.cpp menu.cpp motor.cpp ntc.cpp power.cpp
ASRC =

PROGRAMMER = usbasp-clone
PROGRAMMER_FLAGS = -B2
#Enable debugging (Uses UART @ 38400 baud)
DEBUG = True

#UID=LED1

CUSTOM_LDFLAGS =

###################################################################
SYSTEMDIR = .
NO_DEFAULT_FILES = True
#include 
include Makefile.avr
