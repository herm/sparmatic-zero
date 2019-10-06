#ifndef ENCODER_H_
#define ENCODER_H_

#define ENCODER_ALL         ((1<<ENCODER_A)|(1<<ENCODER_B))

#define PHASE_A     (ENCODER_PIN & 1<<ENCODER_A)
#define PHASE_B     (ENCODER_PIN & 1<<ENCODER_B)

void encoderInit(void);
void encoderPeriodicScan(void);
int8_t encoderRead(void);

#endif
