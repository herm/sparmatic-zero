#ifndef DEBUG_H_
#define DEBUG_H_
#include <stdint.h>

void debugInit(void);
void debugString(const char *s);
void debugNumber(int16_t n);
void debugBinary(uint16_t n);
void debugHex(uint16_t n);

#endif /* DEBUG_H_ */
