/************************************************************************/
/*                                                                      */
/*                      Debouncing 8 Keys                               */
/*                      Sampling 4 Times                                */
/*                      With Repeat Function                            */
/*                                                                      */
/*              Author: Peter Dannegger                                 */
/*                      danni@specs.de                                  */
/*                                                                      */
/************************************************************************/

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#ifdef ENCODER
#include "encoder.h"
#endif
#include "keys.h"

volatile uint8_t key_state; // debounced and inverted key state:
// bit = 1: key pressed
volatile uint8_t key_press; // key press detect

volatile uint8_t key_rpt; // key long press and repeat

uint8_t key_irq_turn_off_delay;

uint8_t keyPeriodicScan(void)
{
    static uint8_t ct0, ct1, rpt;
    uint8_t i;

    i = key_state ^ ~KEY_PIN; // key changed ?
    ct0 = ~(ct0 & i); // reset or count ct0
    ct1 = ct0 ^ (ct1 & i); // reset or count ct1
    i &= ct0 & ct1; // count until roll over ?
    key_state ^= i; // then toggle debounced state
    key_press |= key_state & i; // 0->1: key press detect

    if ((key_state & REPEAT_MASK) == 0) // check repeat function
        rpt = REPEAT_START; // start delay
    if (--rpt == 0) {
        rpt = REPEAT_NEXT; // repeat delay
        key_rpt |= key_state & REPEAT_MASK;
    }
    if (key_state & KEY_ALL) {
        key_irq_turn_off_delay = 0;
    } else if (key_irq_turn_off_delay < 255) {
        key_irq_turn_off_delay++;
    }
    return key_irq_turn_off_delay < 5;
}

///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed. Each pressed key is reported
// only once
//
uint8_t get_key_press(uint8_t key_mask)
{
    cli(); // read and clear atomic !
    key_mask &= key_press; // read key(s)
    key_press ^= key_mask; // clear key(s)
    sei();
    return key_mask;
}

///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed long enough such that the
// key repeat functionality kicks in. After a small setup delay
// the key is reported being pressed in subsequent calls
// to this function. This simulates the user repeatedly
// pressing and releasing the key.
//
uint8_t get_key_rpt(uint8_t key_mask)
{
    cli(); // read and clear atomic !
    key_mask &= key_rpt; // read key(s)
    key_rpt ^= key_mask; // clear key(s)
    sei();
    return key_mask;
}

///////////////////////////////////////////////////////////////////
//
uint8_t get_key_short(uint8_t key_mask)
{
    cli(); // read key state and key press atomic !
    uint8_t tmp = get_key_press(~key_state & key_mask);
    sei();
    return tmp;
}

///////////////////////////////////////////////////////////////////
//
uint8_t get_key_long(uint8_t key_mask)
{
    return get_key_press(get_key_rpt(key_mask));
}

/*
 * get increment/decrement keys.
 * This may be extended for rotary encoders in future version.
 */
int8_t get_key_increment(void)
{
#ifdef ENCODER
    int8_t keys = encoderRead();
    // limit
    if (keys > 0)
        return 1;
    if (keys < 0)
        return -1;
#else
    uint8_t keys = get_key_press((1 << KEY_MINUS) | (1 << KEY_PLUS));
    if (keys & (1 << KEY_PLUS))
        return 1;
    if (keys & (1 << KEY_MINUS))
        return -1;
#endif
    return 0;
}

void keyInit(void)
{
    // Configure debouncing routines
    KEY_DDR &= ~KEY_ALL; // configure key port for input
    KEY_PORT |= KEY_ALL; // and turn on pull up resistors

    // Enable interrupt as a wake-up source
    EIMSK |= (1 << PCIE1); //PC-INT 8..15
    PCMSK1 |= KEY_ALL; // Enable all switches PC-INT
}

ISR(PCINT1_vect)
{
    key_irq_turn_off_delay = 0;
    /* used for waking up the device by key press*/
    LCDCRA |= (1 << LCDIE);

#ifdef ENCODER
    encoderPeriodicScan();
#endif
}
