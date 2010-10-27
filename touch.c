#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

extern uint8_t point;

/*        X     Y
 PC3  X1  5V    ADC
 PC2  Y2  ADC   5V
 PC1  X2  GND   FL
 PD4  Y1  FL    GND  //was PC0
*/

void init_x(void) {
  DDRC = ((1<<1) | (1<<3));  // set up for X
  PORTC = 0;
  PORTC = 1<<3;

}

void read_x(void) {
  ADMUX = (1 << REFS0) | (1 << ADLAR) | (1 << 1);
  ADCSRA |= (1 << ADSC);  // Start A2D Conversions 
  while(ADCSRA & (1 << ADSC));
  point = ADCH;
}

void read_y(void) {
  DDRC = 1<<2;  // set up for Y
  DDRD = 1<<4;
  PORTC = 0;
  PORTC = 1<<2;

  _delay_ms(1); //wait for screen to initialize

  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz 

  ADMUX |= (1 << REFS0); // Set ADC reference to AVCC 
  ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading 

  ADMUX |= (1<<1) | (1<<0);

  //ADCSRA |= (1 << ADATE);  // Set ADC to Free-Running Mode 

  ADCSRA |= (1 << ADEN);  // Enable ADC 
  ADCSRA |= (1 << ADSC);  // Start A2D Conversions 

  while(ADCSRA & (1 << ADSC));
  point = ADCH;
}
