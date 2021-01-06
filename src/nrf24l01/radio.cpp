#include "radio.h"
#include "nrf24l01.h"
#include "tinyudp.h"
#include "sensor.h"
#include "ntc.h"
#include "motor.h"
#include "power.h"
#include "debug.h"
#include <avr/pgmspace.h>
#include <avr/wdt.h>

namespace Radio {

static uint32_t uptime = 0, timestamp = 0; //TODO: Should be implemented using timers

struct sensor_data : public TinyUDP::Packet
{
    uint32_t timestamp;
    uint32_t uptime;
    int16_t temperature;
    int16_t valve_position;
    uint8_t battery_voltage;
};

struct control_data : public TinyUDP::Packet
{
    uint16_t bitmask;
    uint32_t timestamp;
    int16_t temperature;
    uint16_t valve_position;
};

/* Used to force a reset. */
struct debug_message : public TinyUDP::Packet
{
    uint16_t command;
    uint8_t data[6];
};

const PROGMEM sensor_info info_messages[] = {
     // Max text length: 10                        "0123456789"
     sinfo(0, st_unixtime,    ss_uint32, sc_1,     "Time"),
     sinfo(1, st_seconds,     ss_uint32, sc_1,     "Uptime"),
     sinfo(2, st_temperature, ss_int16,  sc_0_01,  "Temp"),
     sinfo(3, st_raw,         ss_int16,  sc_1,     "ValveP"),
     sinfo(4, st_voltage,     ss_uint8,  sc_0_1,   "Battery"),

     // Max text length: 10                                      "0123456789"
     cinfo(0, st_unixtime,    ss_uint32, sc_1,    0, 0xFFFFFFFF, "SetTime"),
     cinfo(1, st_temperature, ss_int16,  sc_0_01, 0, 3000,       "SetTemp"),
     cinfo(2, st_raw,         ss_uint16, sc_1,    0, 0xFFFF,     "SetValve"),

     ginfo(5, 3, "Heating"),
};

sensor_data sensors;
control_data controls;


radio_state_t state;
void startListening(void)
{
    state = RADIO_LISTENING;
    NRF24L01::start_receive();
}

void sendSensorDescriptions()
{
    for (uint8_t i=0; i<sizeof(info_messages)/sizeof(sensor_info); i++)
    {
        send_sensor_info_P(&(info_messages[i]));
    }
}

void sendSensorValues()
{
    sensors.set_payload_size(sizeof(sensor_data) - sizeof(TinyUDP::Packet));
    sensors.flags = 0;
    sensors.timestamp = timestamp;
    sensors.uptime = uptime;
    sensors.temperature = getNtcTemperature();
    sensors.valve_position = motorGetPosition();
    sensors.battery_voltage = updateBattery() / 100;
    send_sensor_data(sensors);
    NRF24L01::start_receive();
}


void receiveControlValues()
{
    debugString("Recv\r\n");
    if (!TinyUDP::receive(controls, sizeof(controls))) return;
    debugString("Packet\r\n");
    debugBinary(controls.bitmask);
    debugString("Port");
    debugHex(controls.port);
    debugNumber(controls.payload_size());
    debugNumber(sizeof(control_data) - sizeof(TinyUDP::Packet));
    if (controls.port == 0xfe) {
        debug_message &msg = (debug_message &)controls;
        if (msg.command == 0x2106) {
            //Force reset
            wdt_enable(WDTO_15MS);
            while (true);
        }
    }
    if (controls.port == 0 && (controls.payload_size() == sizeof(control_data) - sizeof(TinyUDP::Packet)))
    {
        if (controls.bitmask & _BV(0)) {
            //Time
            timestamp = controls.timestamp;
        }
        if (controls.bitmask & _BV(1)) {
            //Temperature
            //TODO
        }
        if (controls.bitmask & _BV(2)) {
            //Valve
            motorSetPosition(controls.valve_position);
        }
    }
}

void init(void)
{
    if (NRF24L01::init() != 0) {
        state = RADIO_DISABLED;
        return;
    }
    state = RADIO_IDLE;
    TinyUDP::init();
    startListening();
}

/* This function should be called once per second. */
void periodic(void)
{
    //TODO: Transmit in fixed intervals (=> Register timer callback)
    if (state == RADIO_DISABLED) return;
    uptime++;
    if ((uptime & 7) == 0) {
        sendSensorDescriptions();
    }
    if ((uptime & 3) == 0) {
        sendSensorValues();
    }
    receiveControlValues();
}
} //ns Radio
