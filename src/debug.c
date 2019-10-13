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

#ifdef DEBUG_IRQS
#include <avr/interrupt.h>
ISR(BADISR_vect)
{
    debugString("Bad ISR\r\n");
}

ISR( INT0_vect )
{
    debugString(" INT0_vect \r\n");
}
//ISR( PCINT0_vect )
//{
//    debugString(" PCINT0_vect \r\n");
//}
//ISR( PCINT1_vect )
//{
//    debugString(" PCINT1_vect \r\n");
//}
ISR( TIMER2_COMP_vect )
{
    debugString(" TIMER2_COMP_vect \r\n");
}
//ISR( TIMER2_OVF_vect )
//{
//    debugString(" TIMER2_OVF_vect \r\n");
//}
ISR( TIMER1_CAPT_vect )
{
    debugString(" TIMER1_CAPT_vect \r\n");
}
ISR( TIMER1_COMPA_vect )
{
    debugString(" TIMER1_COMPA_vect \r\n");
}
ISR( TIMER1_COMPB_vect )
{
    debugString(" TIMER1_COMPB_vect \r\n");
}
ISR( TIMER1_OVF_vect )
{
    debugString(" TIMER1_OVF_vect \r\n");
}
//ISR( TIMER0_COMP_vect )
//{
//    debugString(" TIMER0_COMP_vect \r\n");
//}
//ISR( TIMER0_OVF_vect )
//{
//    debugString(" TIMER0_OVF_vect \r\n");
//}
ISR( SPI_STC_vect )
{
    debugString(" SPI_STC_vect \r\n");
}
ISR( USART0_RX_vect )
{
    debugString(" USART0_RX_vect \r\n");
}
ISR( USART0_UDRE_vect )
{
    debugString(" USART0_UDRE_vect \r\n");
}
ISR( USART0_TX_vect )
{
    debugString(" USART0_TX_vect \r\n");
}
ISR( USI_START_vect )
{
    debugString(" USI_START_vect \r\n");
}
ISR( USI_OVERFLOW_vect )
{
    debugString(" USI_OVERFLOW_vect \r\n");
}
ISR( ANALOG_COMP_vect )
{
    debugString(" ANALOG_COMP_vect \r\n");
}
ISR( ADC_vect )
{
    debugString(" ADC_vect \r\n");
}
ISR( EE_READY_vect )
{
    debugString(" EE_READY_vect \r\n");
}
ISR( SPM_READY_vect )
{
    debugString(" SPM_READY_vect \r\n");
}
//ISR( LCD_vect)
//{
//    debugString(" LCD_vect\r\n");
//}
#endif
