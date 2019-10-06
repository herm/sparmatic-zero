/*
 * high layer rf protocol.
 */

#include <stdint.h>
#include "nRF24L01.h"
#include "radio.h"
#include "ntc.h"
#include "timer.h"
#include "lcd.h"
#include "motor.h"
#include "adc.h"
#include "control.h"
#include "programming.h"

/* from main */
extern uint16_t BatteryMV;


//TODO: This function seems to do nothing useful
void radioRxDataAvailable(void)
{
	uint8_t rxData[32];
	uint8_t readLen;
	readLen = nRF24L01_get_data(rxData);
    (void) readLen;
}

/**
 * initializes lower level rf protocol, sets high level address.
 * @param ownAddress high level address: 2-255
 */
void radioInit(void)
{
	nRF24L01_init();
	nRF24L01_set_RADDR_01(0, ThermostatAdr);
	nRF24L01_set_TADDR(ThermostatAdr);
	nRF24L01_set_rx_callback(&radioRxDataAvailable);
}


void radioSend(void)
{
	MSG_FROM_THRM msg;
	TIME time = getTime();
	msg.info.temperatureActual = getNtcTemperature();
	msg.info.valve = getMotorPosition();
	msg.info.battery = getBatteryVoltage();
	msg.info.temperatureNominal = targetTemperature;
	msg.time.day = time.weekday;
	msg.time.hour = time.hour;
	msg.time.minute = time.minute;

	nRF24L01_send((uint8_t *)&msg, sizeof(msg), 0);
}
