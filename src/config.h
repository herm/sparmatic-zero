#ifndef CONFIG_H_
#define CONFIG_H_

/*************************************************************************
 **************************** Debug **************************************
 *************************************************************************/
#define DEBUG_BAUD 9600
#define DEBUG_ENABLED 1

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

#define ADC_CH_MOTOR 2
#define ADC_CH_MOTOR_SENSE 0

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

/*************************************************************************
 *************************** Keys / Encoder ******************************
 *************************************************************************/
/* defines the number of steps per dent. #undef this if the device has no encoder. */
//#define ENCODER	2
#undef ENCODER

#ifndef ENCODER
#define KEY_PLUS  PB0
#define KEY_MINUS PB7
#else
#define ENCODER_A PB0
#define ENCODER_B PB7
#define ENCODER_DDR  DDRB
#define ENCODER_PORT PORTB
#define ENCODER_PIN  PINB
#endif

#define KEY_CLOCK PB5
#define KEY_OK    PB6
#define KEY_MENU  PB4

#define KEY_DDR  DDRB
#define KEY_PORT PORTB
#define KEY_PIN  PINB

/*************************************************************************
 ***************************** Power loss ********************************
 *************************************************************************/
#define POWERLOSS_PORTIN PINE
#define POWERLOSS_DDR DDRE
#define POWERLOSS_PIN PE0  //TODO: Is this the right pin?

/*************************************************************************
 *************************** Keys / Encoder ******************************
 *************************************************************************/

// #define RADIO	nRF24L01
// #define RADIO	RFM12

// #define BOOTLOADER

#endif
