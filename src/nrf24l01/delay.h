#ifndef _NRF24L01_DELAY_H_
#define _NRF24L01_DELAY_H_
//This is a helper file to make compile the NRF24L01 driver library without the normally used build files.


#include <util/delay.h>
#define delay_ms _delay_ms
#define delay_us _delay_us

#endif
