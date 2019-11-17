#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

// Timer 0
volatile uint8_t Timer0H;
static TimerCallback TimeoutCallback;

// Timer 2
volatile uint32_t SystemTime; //monotonic time in seconds
static volatile date_t Time;

// Timer 0
ISR(TIMER0_OVF_vect)
{
    ++Timer0H;
}

ISR(TIMER0_COMP_vect)
{
    if (TimeoutCallback) {
        TimeoutCallback();
        TimeoutCallback = 0;
        TIMSK0 &= ~(1 << OCIE0A);
    }
}

void enableTimeout(TimerCallback cbk, uint8_t timeout)
{
    TimeoutCallback = cbk;
    OCR0A = TCNT0 + timeout;
    TIFR0 |= (1 << OCF0A);
    TIMSK0 |= (1 << OCIE0A);
}

void setTimeout(uint8_t timeout)
{
    OCR0A = TCNT0 + timeout;
}

void disableTimeout()
{
    TIMSK0 &= ~(1 << OCIE1A);
    TimeoutCallback = 0;
}

// Timer 2
/* used for waking up the device periodically */
ISR(TIMER2_OVF_vect)
{
    /* TODO: Check the assembly code of this function. Copying "Time" twice seems to be inefficient. */
    /* TODO: Use full date (with day and month) to allow programming holidays. */
    date_t tTime = Time;

    tTime.second += 8;
    SystemTime += 8;
    if (tTime.second > 59) {
        tTime.second -= 60;
        tTime.minute += 1;
        if (tTime.minute > 59) {
            tTime.minute = 0;
            tTime.hour += 1;
            if (tTime.hour > 23) {
                tTime.hour = 0;
                tTime.weekday += 1;
                if (tTime.weekday > 6) {
                    tTime.weekday = 0;
                }
            }
        }
    }

    Time = tTime;
}

date_t getTime(void)
{
    date_t res;
    cli();
    res = Time;
    sei();
    return res;
}

// Both timers
void timerInit(void)
{
    ASSR |= (1 << AS2);
    TCCR2A = (1 << CS20) | (1 << CS21) | (1 << CS22); /* normal mode, 32768/1024 = 32 Hz, 1/8 Hz Interrupt rate */
    TCNT2 = 0;
    OCR2A = 0;
    while (ASSR & ((1 << TCN2UB) | (1 << OCR2UB) | (1 << TCR2UB)))
        ;
    TIFR2 = (1 << OCF2A) | (1 << TOV2);
    TIMSK2 = (1 << TOIE2);

    //TODO: Timer 0 is always running and generating interrupts at a high rate even when it is not used.
    TCCR0A = (1 << CS00) | (1 << CS02); /* CLK / 1024 -> 1ms */
    TIMSK0 = (1 << TOIE0);
}

// Utility functions
void addToTime(date_t *time, uint8_t hours, uint8_t minutes)
{
    uint16_t tempMinute = time->minute + minutes;
    time->minute = tempMinute % 60;
    time->hour += tempMinute / 60 + hours;
    time->weekday += time->hour / 24;
    time->hour %= 24;
}
