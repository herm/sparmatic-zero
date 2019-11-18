#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"
#include "keys.h"
#include "motor.h"
#include "adc.h"
#include "config.h"
#include "debug.h"

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
#define DIR_STOP 0

#define force_inline inline __attribute__((always_inline))
uint8_t motor_enabled;
uint8_t motor_running;
int8_t motor_direction;
uint8_t motor_timeout;
uint16_t motor_runtime;
int16_t motor_position;
int16_t motor_position_max;
int16_t motor_position_target;

static void motorEnable(void)
{
    motor_enabled = 1;
    motor_runtime = 0;
    motor_timeout = 0;
    MOTOR_SENSE_PORT |= (1 << MOTOR_SENSE_LED_PIN);
    MOTOR_DDR |= (1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R);
    LCDCRA |= (1 << LCDIE); //Enable timer IRQ
}

static force_inline void motorDisable(void)
{
    motor_enabled = 0;
    MOTOR_DDR &= ~(1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R);
    MOTOR_SENSE_PORT &= ~(1 << MOTOR_SENSE_LED_PIN);
}

static force_inline void motorOpen(void)
{
    MOTOR_PORT |= (1 << MOTOR_PIN_L);
    motor_direction = DIR_OPEN;
    motor_running = 1;
}

static force_inline void motorClose(void)
{
    MOTOR_PORT |= (1 << MOTOR_PIN_R);
    motor_direction = DIR_CLOSE;
    motor_running = 1;
}

static force_inline void motorStop(void)
{
    MOTOR_PORT &= ~((1 << MOTOR_PIN_L) | (1 << MOTOR_PIN_R));
    motor_direction = DIR_STOP;
    motor_running = 0;
}

void motorInit(void)
{
    motorStop();
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
    motor_timeout++; //is reset in motorIrq()
    if (motor_timeout > MOTOR_TIMEOUT) {
        motorStop();
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
    return motor_enabled;
}

uint8_t motorAdaptOpen(void)
{
    motorEnable();
    motorOpen();
    while (motor_running) {
        debugNumber(motor_position);
        if (motor_runtime > MOTOR_MAX_RUNTIME_OPEN) {
            motorStop();
        }
    }
    motor_position = 0;
    motorDisable();
    return motor_runtime <= MOTOR_MAX_RUNTIME_OPEN;
}

uint8_t motorAdaptClose(void)
{
    motorEnable();
    motorClose();
    while (motor_running) {
        debugNumber(motor_position);
        if (motor_runtime > MOTOR_MAX_RUNTIME_CLOSE) {
            motorStop();
        }
    }

    motorDisable();

    if (motor_runtime <= MOTOR_MAX_RUNTIME_CLOSE) {
        motor_position_max = -motor_position;
        motor_position = 0;
        return 1;
    } else {
        return 0;
    }
}

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

uint8_t motorIsAdapted(void)
{
    return motor_position_max != 0;
}

void motorSetPosition(int16_t position)
{
    motorEnable();
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
