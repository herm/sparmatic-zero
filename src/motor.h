#ifndef MOTOR_H_
#define MOTOR_H_
#include <stdint.h>

void motorInit(void);
void motorIrq(void);
/* Returns 1 if it wants to be called again. 0 otherwise. This function
 * might also be called when the timer is not explicitly enabled.*/
uint8_t motorTimer(void);
void motorAdapt(void);
uint8_t motorIsAdapted(void);


#endif /* MOTOR_H_ */
