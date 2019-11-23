#include "config.h"
#include <avr/signature.h>
#include <avr/lock.h>
#include <avr/io.h>

#if FUSE_MEMORY_SIZE == 1
FUSES =
    {
        .low = fuses_low,
    };
#elif FUSE_MEMORY_SIZE == 2
FUSES =
    {
        .low = fuses_low,
        .high = fuses_high,
    };
#elif FUSE_MEMORY_SIZE == 3
FUSES =
    {
        .low = fuses_low,
        .high = fuses_high,
        .extended = fuses_ext,
    };
#else
#error "Unsupported fuse length!"
#endif

// Lock bit mask is required because avrdude reads non-existing bits as 0
// The lock bit macros set these bits to 1
// As a result lock bit verification always fails
#if defined(__LOCK_BITS_EXIST)
#if defined (__AVR_ATmega8__) || defined(__AVR_ATmega169PA__)
#define LOCKBIT_MASK 0x3f
#else
#warning "No lock bit mask defined for this MCU"
#define LOCKBIT_MASK 0xff
#endif
#endif

#if defined(__LOCK_BITS_EXIST) && defined(lockbits)
LOCKBITS = lockbits & LOCKBIT_MASK;
#endif
