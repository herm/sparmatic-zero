#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"
#include "keys.h"
#include "motor.h"
#include "adc.h"
#include "config.h"
#include "debug.h"

#define MOTOR_DEBUG_POWER
#define MOTOR_DEBUG_ADAPT_ONE_WAY

/* Motor characteristics @ 3.3V:
 * No load:
 * ENC: 70-75 counts/s
 * ADC: 977
 *
 * Medium load:
 * ENC: ~30 counts/s
 * ADC: ~950-960
 *
 * High load:
 * < 5counts/s
 * ADC: <945
 *
 * ADC value requires about 60ms after motor enable to reach a stable value. Readings seem to vary quite a bit
 * with voltage and model variations.
 */
#define MOTOR_TIMEOUT ((uint16_t)((uint32_t)F_TIMER * MOTOR_TIMEOUT_MS / 1000 + 1))
#define MOTOR_MAX_RUNTIME_OPEN ((uint16_t)((uint32_t)F_TIMER * MOTOR_MAX_RUNTIME_OPEN_S))
#define MOTOR_MAX_RUNTIME_CLOSE ((uint16_t)((uint32_t)F_TIMER * MOTOR_MAX_RUNTIME_CLOSE_S))
#define DIR_OPEN 1
#define DIR_CLOSE -1
#define DIR_DISABLED 0

volatile int8_t motor_direction;
volatile uint8_t motor_timeout;
volatile uint16_t motor_runtime;
volatile int16_t motor_position;
volatile int16_t motor_position_max;
volatile int16_t motor_position_target;

/* TODO: Reduce number of accesses to volatile variable.
 * TODO: Make sure all 16 bit accesses are atomic.
 */

#define motor_running (motor_direction != DIR_DISABLED)

static void motorEnable(void)
{
    motor_runtime = 0;
    motor_timeout = 0;
    MOTOR_SENSE_PORT |= (1 << MOTOR_SENSE_LED_PIN);
    MOTOR_DDR |= (1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R);
    LCDCRA |= (1 << LCDIE); //Enable timer IRQ
#ifdef MOTOR_DEBUG_POWER
    displaySymbols(LCD_LOCK, LCD_LOCK);
#endif
}

static force_inline void motorDisable(void)
{
    motor_direction = DIR_DISABLED;
    MOTOR_PORT &= ~((1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R));
    MOTOR_DDR &= ~(1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R); //TODO: Does this actually conserve power?
    MOTOR_SENSE_PORT &= ~(1 << MOTOR_SENSE_LED_PIN);
#ifdef MOTOR_DEBUG_POWER
    displaySymbols(LCD_NONE, LCD_LOCK);
#endif
}

static force_inline void motorOpen(void)
{
    motor_direction = DIR_OPEN;
    motorEnable();
    MOTOR_PORT |= (1 << MOTOR_PIN_L);
}

static force_inline void motorClose(void)
{
    motor_direction = DIR_CLOSE;
    motorEnable();
    MOTOR_PORT |= (1 << MOTOR_PIN_R);
}

static force_inline void motorStop(void)
{
    MOTOR_PORT &= ~((1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R));
    /* Motor will be disabled by the timer after a timeout.
     * It might still move a bit after disabling its power.
     * Disabling the encoder right here might lose some steps.
     * This is also the reason why motor_direction must not be modified here
     */
}

void motorInit(void)
{
    motorDisable();
    MOTOR_SENSE_DDR |= (1 << MOTOR_SENSE_LED_PIN);
    PCMSK0 |= (1 << MOTOR_SENSE_PIN);
}

void motorIrq(void)
{
    motor_position += motor_direction;
    motor_timeout = 0;
}

uint8_t motorTimer(void)
{
    if (++motor_timeout > MOTOR_TIMEOUT) {
        motorStop();
        /* Note: This timeout also expires if the motor was stopped intentionally and is used to disable the driver in this case. */
        motorDisable();
    }
    if (motor_running) {
        motor_runtime++;
    }
    if (motor_position_max) {
        // Already adapted => Check position
        // Calling motorStop() when reaching the exact value is usually enough to stop within +-1 count.
        // However sometimes we move fast enough to not see the value at all. Therefore we must check if we exceeded the value.
        if (motor_direction == DIR_OPEN) {
             if (motor_position >= motor_position_target) {
                 motorStop();
             }
        } else if (motor_direction == DIR_CLOSE) {
            if (motor_position <= motor_position_target) {
                motorStop();
            }
        }
    }
    //TODO: Check current
    return motor_running;
}

uint8_t motorAdaptOpen(void)
{
    motorOpen();
    while (motor_running) {
//        debugNumber(motor_position);
        if (motor_runtime > MOTOR_MAX_RUNTIME_OPEN) {
            motorStop();
        }
    }
    motor_position = 0;
    return motor_runtime <= MOTOR_MAX_RUNTIME_OPEN;
}

uint8_t motorAdaptClose(void)
{
    motorClose();
    while (motor_running) {
//        debugNumber(motor_position);
        if (motor_runtime > MOTOR_MAX_RUNTIME_CLOSE) {
            motorStop();
        }
    }

    if (motor_runtime <= MOTOR_MAX_RUNTIME_CLOSE) {
        motor_position_max = -motor_position;
        motor_position = 0;
        return 1;
    } else {
        return 0;
    }
}

#ifdef MOTOR_DEBUG_ADAPT_ONE_WAY
void motorAdapt()
{
    displayString("DBG ");
    motorAdaptOpen();
    motor_position = MOTOR_MIN_RANGE;
    motor_position_max = MOTOR_MIN_RANGE;
}
#else
void motorAdapt()
{
    displayString(" -> ");
    if (!motorAdaptOpen()) {
        goto error;
    }
    displayString("ADAP");
    keyWaitFor(KEY_OK);
    displayString(" <- ");
    if (!motorAdaptClose()) {
        goto error;
    }
    if (motor_position_max < MOTOR_MIN_RANGE) {
        motor_position_max = 0;
        goto error;
    }
    return;
error:
    debugString("Adapt error\r\n");
    displayString("ERR1");
    keyWaitFor(KEY_OK);
}
#endif

uint8_t motorIsAdapted(void)
{
    return motor_position_max != 0;
}

void motorSetPosition(int16_t position)
{
    if (position < 0) position = 0;
    if (position > motor_position_max) position = motor_position_max;
    if (position > motor_position) {
        motorOpen();
    } else if (position < motor_position) {
        motorClose();
    }
    motor_position_target = position;
    while (motor_running) {
        debugNumber(motor_position);
    }
}

int16_t motorGetPosition(void)
{
    return motor_position;
}
