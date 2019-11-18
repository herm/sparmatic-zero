#ifndef TIMER_H_
#define TIMER_H_

/* long system timer, incremented every 8 seconds. Starts at 0 on every system reset. */
extern volatile uint32_t SystemTime;

typedef struct
{
    uint8_t weekday;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} date_t;

void timerInit(void);

date_t getTime(void);

void addToTime(date_t *time, uint8_t hours, uint8_t minutes);

static inline uint8_t compareTime(date_t *a, date_t *b)
{
    if (a->weekday < b->weekday) return -1;
    if (a->weekday > b->weekday) return 1;
    if (a->hour < b->hour) return -1;
    if (a->hour > b->hour) return 1;
    if (a->minute < b->minute) return -1;
    if (a->minute > b->minute) return 1;
    return 0;
}

#endif /* TIMER_H_ */
