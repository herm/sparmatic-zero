#ifndef CONFIG_H_
#define CONFIG_H_

/*************************************************************************
 **************************** Motor **************************************
 *************************************************************************/
#define MOTOR_PORT PORTE
#define MOTOR_DDR DDRE
#define MOTOR_PIN_L PE7
#define MOTOR_PIN_R PE6

#define MOTOR_SENSE_DDR DDRE
#define MOTOR_SENSE_PORT PORTE
#define MOTOR_SENSE_PORT_IN PINE
#define MOTOR_SENSE_LED_PIN PE2

#define ADC_CH_MOTOR (2)
#define ADC_CH_MOTOR_SENSE (0)

/* hardcoded blocking current limit. Currents are in ADC digits.
 * For real current the formular is as follows:
 * I = (1024 - ADCval) * Ubat / 2.2 */
#define MOTOR_CURRENT_BLOCK 930
/* harder limit for detecting open end position */
#define MOTOR_CURRENT_BLOCK_OPEN 950
/* minimum change in current to detect valve */
#define MOTOR_CURRENT_VALVE_DETECT 5
/* all speeds are given in ms per tick.
 * A tick is a period on motor sense pin: LH for Forward, HL for Backwards.
 */
/* block: Stop if no pulse is received in that interval */
#define MOTOR_SPEED_BLOCK 110
/* faster turn off at open end position */
#define MOTOR_SPEED_BLOCK_OPEN 60

#define MOTOR_POSITION_MAX 380
/* ventOpen - ventClosed has to be min. X motor steps */
#define MOTOR_VENT_RANGE_MIN 70

/* stop earlier than target */
#define MOTOR_TARGET_STOP_EARLY 6
#define MOTOR_TARGET_HYSTERESIS 10


#define ENCODER	2
// #define ENCODERTEST

// #define RADIO	nRF24L01
// #define RADIO	RFM12

// #define BOOTLOADER

#endif
