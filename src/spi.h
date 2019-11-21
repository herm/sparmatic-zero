#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>
#include <stdint.h>

/* Configure SPI in master mode, maximum speed (clock/2). */
static inline void spiInit(void)
{
    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR = (1 << SPI2X);
}

static inline uint8_t spiWrite(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & _BV(SPIF)));
    return SPDR;
}

#endif /* SPI_H_ */
