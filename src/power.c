#include "power.h"
#include "config.h"
#include "lcd.h"
#include "adc.h"

#include <avr/io.h>
#include <avr/sleep.h>

#define ADC_CH_REF 30
#define ADC_REF_MV 1100
uint16_t BatteryMV; // battery volage in mV

void pwrInit(void)
{
#if DEBUG_ENABLED
    PRR = (1 << PRTIM1); // disable some hardware
#else
    PRR = (1 << PRTIM1) | (1 << PRUSART0); // disable some hardware
#endif
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    PCMSK0 |= (1 << POWERLOSS_PIN); /* emergency power loss IRQ */
    POWERLOSS_DDR &= ~(1 << POWERLOSS_PIN);
    EIMSK |= (1 << PCIE0);
}

/* Put system into low power mode. */
void sysSleep(void)
{
    OCR2A = 0; //TODO
    ADCSRA &= ~(1 << ADEN); // Disable ADC
    displaySymbols(LCD_BATTERY, LCD_BATTERY); //TODO: For debugging only
    while (ASSR & (1 << OCR2UB))
        /* wait at least one asynchronous clock cycle for interrupt logic to reset */
        ;
    sleep_mode();
    displaySymbols(0, LCD_BATTERY);
}

/// \brief Disable hardware and save data to non-volatile memory on battery removal.
//TODO: Do not disable pull ups!
void sysShutdown(void)
{
    // Lcd_Symbol(BAT, 1 ); // TESTING (barely visible)

    // ADC
    ADCSRA = 0;

    // A, C, D, G: LCD
    DDRB = 0;
    PORTB = 0;

    DDRE = 0;
    PORTE = 0;

    DDRF = 0;
    PORTF = 0;

    lcdOff();

    // shut down everything else
    PRR = (1 << PRLCD) | (1 << PRTIM1) | (1 << PRSPI) | (1 << PRUSART0) | (1 << PRADC);

    //TODO: Make sure no pins are floating
    DDRA = 0;
    PORTA = 0;

    DDRC = 0;
    PORTC = 0;

    DDRD = 0;
    PORTD = 0;

    DDRG = 0;
    PORTG = 0;

    // TODO: write data to EEPROM
    // time
    // temperature set-point
    // other settings should be saved when edited
}


/* TODO: This returns completely wrong values. The calculation seems to be OK, but the ADC are not.
 * Using the bandgap voltage as a reference is was measured at 1.099V at the Vref pin. */
void updateBattery(void)
{
    uint16_t adc = getAdc(ADC_CH_REF);
    /* Uin = scale/fullscale * Uref
     * -> here: Uref = ??; Uin = const
     * Uref = Uin/scale*fullscale
     * calculate at 32 bit
     */
    BatteryMV = (ADC_REF_MV * 1024UL / adc);
}
