/// \file encoder.h
/// 
/// 
/// 

#ifndef ENCODER_H_
#define ENCODER_H_


#define ENCODER_A PB0
#define ENCODER_B PB7
#define ENCODER_ALL         ((1<<ENCODER_A)|(1<<ENCODER_B))

#define ENCODER_DDR         DDRB
#define ENCODER_PORT        PORTB
#define ENCODER_PIN         PINB
 
#define PHASE_A     (ENCODER_PIN & 1<<ENCODER_A)
#define PHASE_B     (ENCODER_PIN & 1<<ENCODER_B)

void		encoderInit(void);
void		encoderPeriodicScan(void);
int8_t	encoderRead(void);


#endif
