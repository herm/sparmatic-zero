#ifndef MOTOR_H_
#define MOTOR_H_
#include <stdint.h>

void motorInit(void);
void motorIrq(void);
/* Returns 1 if it wants to be called again. 0 otherwise. This function
 * might also be called when the timer is not explicitly enabled.*/
uint8_t motorTimer(void);
void motorAdapt(void);
#if 0
uint8_t motorStep(void);
void motorMoveTo(uint8_t valve);
uint8_t motorFullOpen(void);
uint8_t motorAdapt(void);
uint8_t motorIsRunning(void);
void motorTimer(void);

extern volatile int16_t MotorPosition;
extern int16_t PositionValveOpen;
extern int16_t PositionValveClosed;

#define getMotorPosition(x) ((const volatile int16_t)MotorPosition)
#endif

#endif /* MOTOR_H_ */
