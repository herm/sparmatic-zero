#ifndef MAIN_H_
#define MAIN_H_

#define ADC_CH_REF 30
#define ADC_REF_MV 1100

#define SLEEP_POWERSAVE ((1 << SM1) | (1 << SM0))
#define SLEEP SLEEP_POWERSAVE

/* keypad changes temperature by this (1/100 degrees) */
#define MANUAL_TEMPERATURE_STEP		(50)

/* various input functions block for that time, not longer.
 * additionally, an okay press returns immediately.
 * time 256 ms steps (TIMER0 Overflow used).
 */
#define INPUT_TIMEOUT	(30)
#define TIMEOUT_READ		(Timer0H)

#define TIMEOUT_RENEW	timeout = TIMEOUT_READ + INPUT_TIMEOUT
#define TIMEOUT_INIT	uint8_t TIMEOUT_RENEW
#define TIMEOUT_OKAY	(timeout - TIMEOUT_READ < (INPUT_TIMEOUT + 1))

#define RF_STATUS_MESSAGES (15)

#endif
