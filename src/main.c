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

/* \brief Occurs at each new LCD frame or every second LCD frame in low power mode => 64Hz.
 * This is used as a generic time base without having to waste power for a timer. */
ISR(LCD_vect)
{
    static uint8_t cnt = 0;
    uint8_t keep_running = 0;

    keep_running += motorTimer();

    keyPeriodicScan(); /* TODO: Use return code for keep_running. */
    if (key_state & KEY_ALL)
        cnt = 0;
    else
        cnt++;

    if (cnt > 4 && !keep_running) {
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
        motorIrq();
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
    encoderInit();
    ntcInit();
#ifdef RADIO
	funkInit();
#endif
    sei();
    debugString("Init done\r\n");
    while (!motorIsAdapted()) {
        motorAdapt();
    }
    while (1) {
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
