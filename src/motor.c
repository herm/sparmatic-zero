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
#define MOTOR_TIMEOUT 10 /* timer cycles => Less than (f_timer/MOTOR_TIMEOUT) counts/s. (f_timer = 64Hz) */

#define DIR_OPEN 1
#define DIR_CLOSE -1
#define DIR_STOP 0

#define force_inline inline __attribute__((always_inline))
uint8_t motor_enabled;
uint8_t motor_running;
int8_t motor_direction;
uint8_t motor_timeout;
int16_t motor_position;
int16_t motor_position_max;

static force_inline void motorEnable(void)
{
    motor_enabled = 1;
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
    //TODO: Check current
    return motor_enabled;
}

void motorAdaptOpen(void)
{
    motorEnable();
    motorOpen();
    //TODO: Timeout if no stop is found
    while (motor_running) {
        debugNumber(motor_position);
        _delay_ms(30);
    }
    motor_position = 0;
    motorDisable();
}

void motorAdaptClose(void)
{
    motorEnable();
    motorClose();
    //TODO: Timeout if no stop is found
    while (motor_running) {
        debugNumber(motor_position);
        _delay_ms(30);
    }
    motorStop();
    motor_position_max = -motor_position;
    motor_position = 0;
    motorDisable();
}

void motorAdapt()
{
    //TODO: Error checking
    displayString(" -> ");
    motorAdaptOpen();
    displayString("ADAP");
    while (!get_key_press(1 << KEY_OK)) {}
    displayString(" <- ");
    motorAdaptClose();
}

uint8_t motorIsAdapted(void)
{
    return motor_position_max != 0;
}


#if 0


typedef enum
{
    STOP_NULL = 0, STOP_TIMEOUT = 1, STOP_CURRENT = 2, STOP_POSITION = 4, STOP_TARGET = 8
} MOTOR_STOP_SOURCE;

volatile static uint8_t MotorStopSource;

volatile static int8_t Direction = 0;
volatile static uint8_t PWM;
volatile int16_t MotorPosition = 0;
int16_t PositionValveOpen;
int16_t PositionValveClosed;

/* used as stop condition, initialized to very slow speed stop/block stop */
static uint8_t MotorTimeout = 95;
static uint16_t CurrentLimit = 940;
static int16_t TargetPosition = -1;

/* Motorgeschwindigkeit Leerlauf ~ 70-90 Schritte pro Sekunde
 * Motorgeschwindigkeit Last ~40-80 Schritte pro Sekunde
 * Motorstrom Leerlauf ~30 mA
 * Motorstrom Kurzschluss ~180 mA
 * Motorstrom starke Belastung
 */



static uint16_t getCurrent(void)
{
    return getAdc(ADC_CH_MOTOR);
}

void motorStopMove(void)
{
    disableTimeout();
    MOTOR_STOP;
    MOTOR_DIR_IN;
    _delay_ms(800); /* todo think about different implementation */
    MOTOR_SENSE_OFF;
    Direction = DIR_STOP;
}

void motorStopTimeout(void)
{
    MotorStopSource |= STOP_TIMEOUT;
    motorStopMove();
}

static void motorMove(int8_t dir)
{
    MOTOR_SENSE_ON;
    MOTOR_STOP;
    MOTOR_DIR_OUT;
    MotorStopSource = STOP_NULL;
    enableTimeout(motorStopTimeout, 255); /* first timeout long to allow startup */
    switch (dir)
    {
    case DIR_OPEN:
        MOTOR_OPEN;
    break;
    case DIR_CLOSE:
        MOTOR_CLOSE;
    break;
    default:
        MOTOR_SENSE_OFF;
    break;
    }
    Direction = dir;
    PWM = 0;
    MOTOR_TIMER_START;
}

/**
 * drives valve position to given value.
 * @param valve valve opening from 0..255
 */
void motorMoveTo(uint8_t valve)
{
    int16_t newPosition = PositionValveClosed + ((PositionValveOpen - PositionValveClosed) * valve) / 255;
    TargetPosition = newPosition;
    if (newPosition > MotorPosition + MOTOR_TARGET_HYSTERESIS) {
        TargetPosition -= MOTOR_TARGET_STOP_EARLY;
        motorMove(DIR_OPEN);
    } else if (newPosition < MotorPosition - MOTOR_TARGET_HYSTERESIS) {
        TargetPosition += MOTOR_TARGET_STOP_EARLY;
        motorMove(DIR_CLOSE);
    }

}

/*
 * resets motor calibration and fully opens until block.
 * @return not zero on hardware failure (no other errors possible)
 */
uint8_t motorFullOpen(void)
{
    MotorTimeout = MOTOR_SPEED_BLOCK_OPEN;
    CurrentLimit = MOTOR_CURRENT_BLOCK_OPEN;
    MotorPosition = 0;
    TargetPosition = -1;
    motorMove(DIR_OPEN);
    while (motorIsRunning())
        ;
    MotorPosition = MOTOR_POSITION_MAX;
    if (MotorStopSource & ~(STOP_CURRENT | STOP_TIMEOUT))
        return 1; /* current detection and stop detection are okay */

    return 0;
}

/*
 * Closes until detection of touching the vent. close further until fully closed.
 * @return not zero on error
 */
uint8_t motorAdapt(void)
{
    uint16_t currentNormal;

    MotorTimeout = MOTOR_SPEED_BLOCK;
    CurrentLimit = MOTOR_CURRENT_BLOCK;

    motorMove(DIR_CLOSE);
    /* let vent start moving */
    while (motorIsRunning() && MotorPosition > MOTOR_POSITION_MAX - 11)
        ;
    currentNormal = getCurrent();

    if (!motorIsRunning())
        return 1;

    /* wait for a small increase in motor power consumption -> vent touched */
    while (motorIsRunning() && getCurrent() > (currentNormal - MOTOR_CURRENT_VALVE_DETECT))
        ;
    PositionValveOpen = MotorPosition;

    if (!motorIsRunning())
        return 1;

    /* wait for motor turning off -> vent closed*/
    while (motorIsRunning())
        ;

    PositionValveClosed = MotorPosition;

    /* check for min. vent range, we may have an error in detection */
    if (PositionValveOpen - PositionValveClosed < MOTOR_VENT_RANGE_MIN)
        return 1;

    if (MotorStopSource & ~(STOP_CURRENT | STOP_TIMEOUT))
        return 1;

    return 0;
}

/// \brief Check and update motor position.
/// 
/// called by opto sensor interrupt
/// 
uint8_t motorStep(void)
{
    // mask sensor pin because pin change interrupt is triggered by all pins on port
    uint8_t state = MOTOR_SENSE_PORT_IN & (1 << MOTOR_SENSE_PIN);

    /* TODO: Why is direction important here? */
    if (((Direction == DIR_OPEN) && state) || ((Direction == DIR_CLOSE) && !state)) {
        setTimeout(MotorTimeout);
        MotorPosition += Direction;

        // limits
        if (MotorPosition < 0 || MotorPosition > MOTOR_POSITION_MAX) {
            MotorStopSource |= STOP_POSITION;
            motorStopMove();
        }

        // target
        if (TargetPosition >= 0
                && ((Direction == DIR_OPEN && MotorPosition >= TargetPosition) || (Direction == DIR_CLOSE && MotorPosition <= TargetPosition))) {
            MotorStopSource |= STOP_TARGET;
            TargetPosition = -1;
            motorStopMove();
        }
        return 1;
    }
    return 0;
}

uint8_t motorIsRunning(void)
{
    return (Direction != DIR_STOP);
}

/**
 * periodic callback as long as the motor is running.
 * used for current limit or stop condition tests.
 */
void motorTimer(void)
{
    if (motorIsRunning()) {
        uint16_t current = getCurrent();
        if (current < CurrentLimit) {
            MotorStopSource |= STOP_CURRENT;
            motorStopMove();
        } else {
            if (PWM == 0) {
                switch (Direction)
                {
                case DIR_OPEN:
                    MOTOR_OPEN;
                break;
                case DIR_CLOSE:
                    MOTOR_CLOSE;
                break;
                default:
                break;
                }
                PWM = 0;
            } else {
                MOTOR_STOP;
                ++PWM;
            }
        }
    }
}
#endif
