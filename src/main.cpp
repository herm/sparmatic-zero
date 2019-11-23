#include "main.h"

#include "config.h"

#include "debug.h"
#include "lcd.h"
#include "keys.h"
#include "ntc.h"
#include "motor.h"
#include "adc.h"
#include "control.h"
#include "menu.h"
#include "encoder.h"
#include "power.h"
#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* \brief Occurs at each new LCD frame or every second LCD frame in low power mode => 64Hz.
 * This is used as a generic time base without having to waste power for a timer. */
ISR(LCD_vect)
{
    uint8_t keep_running = 0; /* If any handler returns non-zero this interrupt is kept enabled. */
    keep_running |= motorTimer();
    keep_running |= keyPeriodicScan();
    if (!keep_running) {
        LCDCRA &= ~(1 << LCDIE); /* disable LCD Interrupt when it is no longer required */
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
    static unsigned char lastState = 0; // init to defaults

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

}

void ioInit(void)
{
    DIDR0 = 0x0F; /* Disable digital inputs on Port F 0-3*/
    DDRF = (1 << NRF24L01_PIN_CE) | (1 << NRF24L01_PIN_CSN);
    SPI_DDR |= (1 << SPI_PIN_MOSI) | (1 << SPI_PIN_SCK);
}

int main(void)
{
    _delay_ms(50);
    debugInit();
    pwrInit();
    ioInit();
    motorInit();
    lcdInit();
    keyInit();
    encoderInit();
    ntcInit();
    spiInit();
    sei();
    debugString("Init done\r\n");
    while (!motorIsAdapted()) {
        motorAdapt();
    }
    while (1) {
        updateNtcTemperature();
        updateBattery();
        menu();

        sysSleep();
    }
}
