#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/wdt.h>

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

#ifdef ENCODER
#include "encoder.h"
// #warning "encoder being included"
#endif

#ifdef BOOTLOADER
#include "bootloader/bootloader.h"
#endif

#ifdef RADIO
#include "nRF24L01.h"
#include "nRF24L01_ll.h"
#include "radio.h"
#endif

uint16_t BatteryMV;

static void updateBattery(void)
{
    uint16_t adc = getAdc(ADC_CH_REF);
    /* Uin = scale/fullscale * Uref
     * -> here: Uref = ??; Uin = const
     * Uref = Uin/scale*fullscale
     * calculate at 32 bit
     */
    BatteryMV = (ADC_REF_MV * 1024UL / adc);
}

/// \brief Disable hardware and save data to non-volatile memory on battery removal.
static void sysShutdown(void)
{
    // Lcd_Symbol(BAT, 1 );	// TESTING (barely visible)

    // ADC
    ADCSRA = 0;

    // A, C, D, G: LCD
    DDRB = 0;
    PORTB = 0;

    DDRE = 0;
    PORTE = 0;

    DDRF = 0;
    PORTF = 0;

    //lcdOff();

    // Disable LCD
    // Wait until a new frame is started.
    while (!(LCDCRA & (1 << LCDIF)))
        ;
    // Set LCD Blanking and clear interrupt flag
    // by writing a logical one to the flag.
    LCDCRA = (1 << LCDEN) | (1 << LCDIF) | (1 << LCDBL);
    // Wait until LCD Blanking is effective.
    while (!(LCDCRA & (1 << LCDIF)))
        ;
    // Disable LCD
    LCDCRA = (0 << LCDEN);

    // shut down everything else
    PRR = (1 << PRLCD) | (1 << PRTIM1) | (1 << PRSPI) | (1 << PRUSART0) | (1 << PRADC);

    DDRA = 0;
    PORTA = 0;

    DDRC = 0;
    PORTC = 0;

    DDRD = 0;
    PORTD = 0;

    DDRG = 0;
    PORTG = 0;

    // write data to EEPROM
    // time
    // temperature set-point
    // other settings should be saved when edited
}

/// \brief Occurs at each new LCD frame , ~128 ms.
/// 
/// 
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

/// \brief Emergency wakeup on power loss, motor step counter, SPI IRQ.
/// 
/// Triggers on both edges, so state change needs to be tracked in software.
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


static void sysSleep(void)
{
    OCR2A = 0;
    ADCSRA &= ~(1 << ADEN); // Disable ADC
    displaySymbols(LCD_BATTERY, LCD_BATTERY);
    while (ASSR & (1 << OCR2UB))
        /* wait at least one asynchronous clock cycle for interrupt logic to reset */
        ;
    sleep_mode();
    displaySymbols(0, LCD_BATTERY);
}

void pwrInit(void)
{
#if DEBUG_ENABLED
    PRR = (1 << PRTIM1); // disable some hardware
#else
    PRR = (1 << PRTIM1) | (1 << PRUSART0); // disable some hardware
#endif
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    PCMSK0 |= (1 << PCINT0); /* emergency power loss IRQ */
    DDRE &= ~(1 << PE0);
    EIMSK |= (1 << PCIE0);
}

void ioInit(void)
{
    DIDR0 = 0xFF; /* Disable digital inputs on Port F */
}

/// \brief Valve initialisation UI.
/// 
/// returns 1 on error
static uint8_t valveInit(void)
{
    // open valve (retract actuator)
    if (motorFullOpen()) {
        displayString("EI1 ");
        return 1;
    }

    // wait for user input
    displayString("INST");
    while (!get_key_press(1 << KEY_OK))
    {
//        debugBinary(PINB);
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
#if 1
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
