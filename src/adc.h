#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

static inline uint16_t getAdc(uint8_t channel)
{
    /* Use AVCC as voltage reference. Result is right-aligned. Prescaler: 16 => 62500kHz.
     * First conversation: 25 cycles => 400us
     * Normal conversation: 13 cycles => 200us
     * */
    ADMUX = (1 << REFS0) | (channel & 0x1F);
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2);
    while (ADCSRA & (1 << ADSC))
        ;
    return ADC;
}

extern uint16_t BatteryMV;

#define getBatteryVoltage() (BatteryMV)

#endif /* ADC_H_ */
