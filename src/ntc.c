#include <avr/io.h>
#include <avr/pgmspace.h>
#include "adc.h"
#include "config.h"
#include "debug.h"

/* all resistances in 10 Ohms */
#define VOLTAGE_DIVIDER_RES 1200000000UL
#define NTC_START_DEGREE 0
#define NTC_DEGREE_STEPS 5

static const uint16_t NtcRes[] PROGMEM = { 34090, //  0°C
        26310, //  5°C
        20440, // 10°C
        16000, // 15°C
        12610, // 20°C
        10000, // 25°C
        7981, // 30°C
        6408, // 35°C
        5174, // 40°C
        4202, // 45°C
        3431, // 50°C
        2816, // 55°C
        2322, // 60°C
        1925, // 65°C
        1603, // 70°C
        1340, // 75°C
        1126, // 80°C
        949, // 85°C
        804, // 90°C
        684, // 95°C
        0 };

int16_t Temperature; /* in 0.01°C */
int16_t NTCOffset = 0; /* in 0.01°C */

void ntcInit(void)
{
    NTC_DDR |= (1 << NTC_PIN);
}

static uint16_t getNtcAdc(void)
{
    uint16_t ntc;
    NTC_PORT |= (1 << NTC_PIN);
    ntc = getAdc(ADC_CH_NTC);
    NTC_PORT &= ~(1 << NTC_PIN);
    return ntc;
}

/** returns temperature in 0.01°C.
 * TODO: Measured voltage is correct, calculated temperature a bit to high. */
void updateNtcTemperature(void)
{
    uint16_t ntcVoltage = getNtcAdc();
    uint16_t ntcRes = VOLTAGE_DIVIDER_RES / (102300000UL / ntcVoltage - 100000);
    uint16_t ntcResTbl;
    uint8_t i = 0;
    int16_t temperature;
    while ((ntcResTbl = pgm_read_word(&NtcRes[i])) > ntcRes)
        ++i;

    temperature = NTC_START_DEGREE + i * NTC_DEGREE_STEPS * 100UL
            - (((ntcRes - ntcResTbl) * NTC_DEGREE_STEPS * 100UL) / (pgm_read_word(&NtcRes[i-1]) - ntcResTbl));
    Temperature = temperature - NTCOffset;
}
