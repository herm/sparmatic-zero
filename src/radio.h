#ifndef _RADIO_H_
#define _RADIO_H_
/* This file defines a generic interface for all radios. Each supported radio should live in its own directory. */
namespace Radio {
void init(void);
void periodic(void);
enum radio_state_t {
    RADIO_DISABLED,
    RADIO_IDLE,
    RADIO_LISTENING,
    RADIO_TRANSMITTING
};

extern radio_state_t state;
} //ns Radio
#endif
