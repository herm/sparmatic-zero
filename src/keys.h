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

#ifndef KEYS_H_
#define KEYS_H_

#define REPEAT_MASK     KEY_ALL       // repeat: key1, key2
#define REPEAT_START    30            // after N scans
#define REPEAT_NEXT     9             // every M scans

#ifdef ENCODER
#define KEY_ALL (                                     (1 << KEY_CLOCK) | (1 << KEY_MENU) | (1 << KEY_OK))
#else
#define KEY_ALL ((1 << KEY_PLUS) | (1 << KEY_MINUS) | (1 << KEY_CLOCK) | (1 << KEY_MENU) | (1 << KEY_OK))
#endif

void keyInit(void);
uint8_t keyPeriodicScan(void);
uint8_t get_key_press(uint8_t key_mask);
uint8_t get_key_rpt(uint8_t key_mask);
uint8_t get_key_short(uint8_t key_mask);
uint8_t get_key_long(uint8_t key_mask);
int8_t get_key_increment(void);
static inline void keyWaitFor(uint8_t key)
{
    while (!get_key_press(1 << key)) {}
}

extern volatile uint8_t key_state; // debounced and inverted key states

#endif /* KEYS_H_ */
