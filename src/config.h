#ifndef CONFIG_H_
#define CONFIG_H_

/*************************************************************************
 **************************** Debug **************************************
 *************************************************************************/
#define DEBUG_BAUD 9600
#define DEBUG_ENABLED 1

/*************************************************************************
 **************************** Timer **************************************
 *************************************************************************/
#define F_TIMER 64 /* Hz, LCD frame IRQ */

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
#define MOTOR_SENSE_PIN PE3

#define ADC_CH_MOTOR 2
#define MOTOR_TIMEOUT_MS 300
#define MOTOR_MAX_RUNTIME_OPEN_S 30
#define MOTOR_MAX_RUNTIME_CLOSE_S 15
#define MOTOR_MIN_RANGE 300

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
#define POWERLOSS_PIN PE5

/*************************************************************************
 ******************************* NTC *************************************
 *************************************************************************/
#define NTC_PORT PORTF
#define NTC_DDR DDRF
#define NTC_PIN PF3
#define ADC_CH_NTC 1

/*************************************************************************
 ****************************** Radio ************************************
 *************************************************************************/
/* nRF24L01+ Mini module pinout:
 * Pin numbering starts at the side closer to the white dot on the PCB
 * 1: +3V3
 * 2: GND
 * 3: CE
 * 4: CSN
 * 5: SCK
 * 6: MOSI
 * 7: MISO
 * 8: IRQ
 */
#define NRF24L01_PORT_CE PORTF
#define NRF24L01_PIN_CE PF7
#define NRF24L01_PORT_CSN PORTF
#define NRF24L01_PIN_CSN PF5
#define SPI_PIN_SCK PB1
#define SPI_PIN_MOSI PB2
#define SPI_PIN_MISO PB3
#define SPI_DDR DDRB

#endif
