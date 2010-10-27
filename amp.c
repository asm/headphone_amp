#define F_CPU 8000000
#define BAUD 57600 //250000
#define MYUBRR F_CPU/16/BAUD-1

#include <inttypes.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <math.h>

#include "TWI_driver.h"
#include "serial.h"
#include "touch.h"
#include "screen.h"
#include "fft.h"

#define NUM_SAMPLES  FFT_N
#define FFT_SIZE FFT_N / 2

int16_t capture[FFT_N];			  // Wave captureing buffer
complex_t bfly_buff[FFT_N];		// FFT buffer
uint16_t spektrum[FFT_N/2];		// Spectrum output buffer


uint8_t point = 0;
uint8_t screen_buffer[128];

void audio_setup(void) {
  ADCSRA = (1 << ADPS2);      // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz 
  ADCSRB = 0;

  ADMUX = 0;                  // channel 0 and 1
  ADMUX |= (1 << ADLAR);      // Left adjust ADC result to allow easy 8 bit reading 
  ADMUX |= (1 << REFS0);      // Set ADC reference to AVCC

  ADCSRA |= (1 << ADEN);      // Enable ADC 
}

SIGNAL(SIG_OVERFLOW0) {
	static uint16_t count = 0;
	static uint16_t offset=0;
	uint8_t adc_val;

	// we only want to run every clk/4 times  8khz
	count++;
	if (count % 4) // 2 = 16mhz, 4 = 8mhz, 8 = 4mhz
		return;

  ADMUX = 0 | (1 << ADLAR) | (1 << REFS0);
	ADCSRA |= (1 << ADSC);            // start ADC conversion
	while (ADCSRA & (1 << ADSC)) {;}; // wait for the result to be available
	adc_val = ADCH;
	
	capture[offset] = ((int16_t)(adc_val)-165)*15; // convert to signed linear
	offset++;

	if (offset == NUM_SAMPLES) { // buffer is full
    uint8_t i;
    unsigned int bin = 0;
    float cut = 50.0 / 44100.0;
    uint16_t bins[FFT_SIZE+5];

    read_x();
    if (point > 10) {
      unsigned char result;
      tx_type tx_frame[2];
      uint8_t vol[2];

      point = point >> 2;   // down-convert to 6bit
      point = point - 6;

      vol[0] = point;       // first digipot
      vol[1] = point | 64;  // second digipot

      tx_frame[0].slave_adr = COMMAND+W;
      tx_frame[0].size = 2;
      tx_frame[0].data_ptr = vol;

      tx_frame[1].slave_adr = OWN_ADR;

      result = Send_to_TWI(tx_frame);

      // Light up error LED if something went wrong
      PORTD = 0;
      if (result != SUCCESS) {
        PORTD = 1 << 2;
      }

      // Draw the volume bars seperately
      draw_bar(128-(point * 2.6), 0, 0xF800);
      draw_bar(128-(point * 2.6), 1, 0xF800);
    }

		fft_input(capture, bfly_buff);
		fft_execute(bfly_buff);
		fft_output(bfly_buff, spektrum);

    // Re-bin FFT into something that resembles actual western scale notes
    bins[bin] = 0;
    for (i = 1; i < FFT_SIZE; i++) {
      float f = (float)i / ((float)FFT_SIZE - 1.0) / 2.0;

      if (f > cut) {
        cut *= 1.06;
        bin++;
        bins[bin] = 0;
      }

      bins[bin] += spektrum[i]*2;
    }

    for (i = 2; i < 32; i++) {
      unsigned long int v = bins[i];
      draw_bar(v, i, 0x1F);  // blue
    }

		offset = 0; 
	}
}


int main (void) {
  /* IO pins used for the error LED */
  DDRD = ((1<<2) | (1<<3));  
  PORTD = 0;

  /* Initialize timers */
	TCCR0B |= (1<<CS00);   // clk prescale (clk/1)
	TIMSK0 |= (1<<TOIE0);  // enable timer/counter0 overflow interrupt 
	TCNT0 = 0;             // reset the timer

  /* Initialize two wire 8*/
  Init_TWI();

  /* Initialize ADC for x cood on touchscreen */
  init_x();

  _delay_ms(500);  // wait for screen to come up

  /* Initialize UART for screen */
  serial_init(MYUBRR);

  _delay_ms(500);

  /* Initialize screen 8*/
  screen_init();

  pen_fill();      // Set fill type

  /* Initialize ADC for audio */
  audio_setup();

  sei();           //enable global interrupts

  while(1) {
    // Magical while loop of bounty
  }

  return 0;
}

