// Mechanical rotary encoder in Eurotronic Sparmatic Comet / Aldi Thermy 2011
// 
// The encoder has detents on states 00 and 11 (PB7 PB0)
// Reading direction +: left to right, -: right to left
//                .. 00 10 11 01 ..
// 
// Credits:
//  Reading rotary encoder  / one, two and four step encoders supported / Author: Peter Dannegger
//  http://www.mikrocontroller.net/articles/Drehgeber#Beispielcode_in_C (German)

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "encoder.h"

volatile int8_t enc_delta;          // -128 ... 127
static volatile int8_t last;



/// \brief .
/// 
/// 
void encoderInit( void )
{
  int8_t new;
	
	
	ENCODER_DDR &= ~ENCODER_ALL;                // configure key port for input
  ENCODER_PORT |= ENCODER_ALL;                // and turn on internal pull-up resistors

	// wake up from sleep (and scan) in interrupt
	EIMSK |= (1 << PCIE1);	//PC-INT 8..15
	PCMSK1 |= ENCODER_ALL; // Enable all switches PC-INT
 
  new = 0;
  if( PHASE_A )
    new = 3;
  if( PHASE_B )
    new ^= 1;                   // convert gray to binary
  last = new;                   // power on state
  enc_delta = 0;

}



/// \brief .
/// 
/// 
void encoderPeriodicScan(void)
{
  int8_t new, diff;
 
  new = 0;
  if( PHASE_A )
    new = 3;
  if( PHASE_B )
    new ^= 1;                   // convert gray to binary
  diff = last - new;                // difference last - new
  if( diff & 1 ){               // bit 0 = value (1)
    last = new;                 // store new as next last
    enc_delta += (diff & 2) - 1;        // bit 1 = direction (+/-)
  }
}



/// \brief .
/// 
/// 
#if ENCODER == 1
#warning encoder 1
int8_t encoderRead( void )         // read single step encoders
{
  int8_t val;
 
  cli();
  val = enc_delta;
  enc_delta = 0;
  sei();
  return val;                   // counts since last call
}
#endif

#if ENCODER == 2 
// #warning encoder 2
int8_t encoderRead( void )         // read two step encoders
{
  int8_t val;
 
  cli();
  val = enc_delta;
  enc_delta = val & 1;
  sei();
  return val >> 1;
}
#endif

#if ENCODER == 4
#warning encoder 4
int8_t encoderRead( void )         // read four step encoders
{
  int8_t val;
 
  cli();
  val = enc_delta;
  enc_delta = val & 3;
  sei();
  return val >> 2;
}
#endif
