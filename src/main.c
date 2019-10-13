#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "main.h"

#include "debug.h"
#include "lcd.h"
#include "keys.h"
#include "ntc.h"
#include "motor.h"
#include "timer.h"
#include "adc.h"
#include "control.h"
#include "programming.h"
#include "menu.h"
#include "encoder.h"
#include "power.h"

#ifdef RADIO
#include "nRF24L01.h"
#include "nRF24L01_ll.h"
#include "radio.h"
#endif

/// \brief Occurs at each new LCD frame or every second LCD frame in low power mode => 64Hz.
//TODO: Why are these things which are unrelated to the LCD handled in the LCD interrupt?
ISR(LCD_vect)
{
    static uint8_t cnt = 0;

    uint8_t motorRunning = motorIsRunning();
    if (motorRunning) {
        motorTimer();
    }

    keyPeriodicScan();
    if (key_state & KEY_ALL)
        cnt = 0;
    else
        cnt++;

    if (cnt > 4 && !motorRunning) { /* TODO this simple check may lead to missed keys when they are pressed in the moment the motor is stopped */
        cnt = 0;
        LCDCRA &= ~(1 << LCDIE); /* disable LCD Interrupt when keys are handled */
    }
}

/* External interrupt handler.
 * Functions:
 * - Emergency wakeup on power loss
 * - Motor step counter
 * - Radio IRQ
 * Triggers on both edges, so state change needs to be tracked in software.
 */
#define PCINT0_PORTIN PINE
ISR(PCINT0_vect)
{
#ifdef RADIO
	static unsigned char lastState = (1 << IRQ_PIN);	// init to defaults
#else
    static unsigned char lastState = 0; // init to defaults
#endif

    unsigned char newState = PCINT0_PORTIN;
    unsigned char changed = newState ^ lastState;
    lastState = newState;

    // save data when battery removed
    if (newState & (1 << POWERLOSS_PIN)) {
        sysShutdown();
    }

    // motor step
    if (changed & (1 << MOTOR_SENSE_PIN)) {
        motorStep();
    }

#ifdef RADIO
	// radio IRQ
	if(~newState & (1 << IRQ_PIN))
	{
		nRF24L01_IRQ();
	}
#endif
}


void ioInit(void)
{
    DIDR0 = 0xFF; /* Disable digital inputs on Port F */
}

/* Valve initialization UI.
 * returns 1 on error
 */
static uint8_t valveInit(void)
{
    displayString("OPEN");
    // open valve (retract actuator)
    if (motorFullOpen() != 0) {
        displayString("EI1 ");
        return 1;
    }

    // wait for user input
    displayString("INST");
    while (!get_key_press(1 << KEY_OK)) {
    }

    // close valve (protract actuator)
    displayString("ADAP");
    if (motorAdapt()) {
        displayString("EI2 ");
        return 1;
    }

    return 0;
}

int main(void)
{
    _delay_ms(50);
    debugInit();
    debugString("Start\r\n");
    timerInit();
    pwrInit();
    ioInit();
    motorInit();
    lcdInit();
    keyInit();
#ifdef ENCODER
    encoderInit();
#endif
    ntcInit();
#ifdef RADIO
	funkInit();
#endif
    sei();
    debugString("Init done\r\n");
#if 0
    if (valveInit()) {
        while (1) // we do not want to operate with an incorrect setup
            sysSleep();
    }
#endif

    while (1) {
#ifdef ENCODERTEST
		#define DISPLAYDEBUG
		static int8_t delta = 0;
		// delta += encoderRead();	// this will show the value ("---1" or "   1") and a blank screen (0="    ") alternating
		delta += get_key_increment();	// this will show the value ("---1" or "   1") and a blank screen (0="    ") alternating
		// int8_t delta = ((PINB & (1<<PB7)) >> 7) * 10 + (PINB & (1<<PB0));	// show the state
		displayNumber(delta, 4);
		// _delay_ms(100);
		#endif

#ifndef DISPLAYDEBUG
        updateNtcTemperature();
        updateBattery();
        menu();

#ifdef RADIO
		if(lastStatusMessageSent + RF_STATUS_MESSAGES < SystemTime)
		{
			radioSend();
			lastStatusMessageSent = SystemTime;
		}
		#endif

#endif
        sysSleep();
    }

    return -1;
}
