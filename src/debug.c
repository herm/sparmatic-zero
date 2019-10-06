#include "debug.h"
#include "config.h"
#include <stdlib.h>
#include <avr/io.h>

#define UBRR_value (F_CPU/8/DEBUG_BAUD-1)
void debugInit(void)
{
    UBRR0 = UBRR_value;
    UCSR0A = _BV(U2X0);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); //8n1
}

void debugString(char *s)
{
    char c;
    while ((c = *s++)) {
        while (!(UCSR0A & _BV(UDRE0)));
        UDR0 = c;
    }
}

void debugNumber(uint16_t n)
{
    char buf[6];
    itoa(n, buf, 10);
    debugString(buf);
    debugString("\r\n");
}

void debugBinary(uint16_t n)
{
    char buf[17];
    itoa(n, buf, 2);
    debugString(buf);
    debugString("\r\n");
}

void debugHex(uint16_t n)
{
    char buf[5];
    itoa(n, buf, 16);
    debugString(buf);
    debugString("\r\n");
}
