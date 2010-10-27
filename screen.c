#define F_CPU 8000000

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "screen.h"

uint16_t screen_buffer[8];

void screen_tx(unsigned char byte) {
  /* Wait for empty transmit buffer */
  while ( !( UCSR0A & (1<<UDRE0)) )
    ;

  /* Put data into buffer, sends the data */
  UDR0 = byte;
}

unsigned char screen_rx(void) {
  /* Wait for incomming data */
	while ( !(UCSR0A & (1<<RXC0)) ) 	
		;

	return UDR0;
}

unsigned char screen_ack(void) {
  if (screen_rx() != 0x06) {
    PORTD = 1 << 2; // error!
    return 0;
  }

  return 1;
}

void screen_init(void) {
  char* startup = "Booting...";
  int i;

  for (i = 0; i < 10; i++) {
    screen_tx(0x55);
    screen_ack();
  }

  draw_text(0, 0, 0, 0x001F, 1, 1, strlen(startup), startup);

  _delay_ms(1000);

  screen_clear();
}

void draw_s_bar(uint16_t vec, unsigned char pos) {
  unsigned char i;
  unsigned char x;
  unsigned char y;
  uint16_t color;
  
  x = pos * 4; 

  for (i = 0; i < 32; i++) {
    if ((screen_buffer[pos] & (1<<i)) != (vec & (1<<i))) {
      y = i * 8;

      if (vec & (1<<i)) {
        color = 0x001F;
      } else {
        color = 0x00;
      }

      draw_rec(x ,y, x + 6, y + 6, color);
    }
  }

  screen_buffer[pos] = vec;
}

void draw_bar(uint8_t height, unsigned char pos, uint16_t color) {
  unsigned char x;

  if (height > 128) {
    height = 128;
  }

  x = pos * 4;
  if (screen_buffer[pos] > height) {
    if (pos > 2) {
      screen_buffer[pos]-=2;
    } else {
      screen_buffer[pos] = height;
    }
    draw_rec(x + 2, 128-screen_buffer[pos], x, 0, 0x0);
  } else {
    draw_rec(x + 2, 128 - height, x, 128, color);
    screen_buffer[pos] = height;
  }
}

void draw_point(uint8_t x, uint8_t y, uint16_t color) {
  screen_tx(0x50); // cmd
  screen_tx(x);
  screen_tx(y);

  screen_tx(color >> 8);
  screen_tx((unsigned char)color);

  screen_ack();
}

void draw_wav(int16_t wav[64], uint16_t color) {
  uint8_t i;

  for (i = 0; i < 64; i++) {
    draw_point(i, (uint8_t)wav[i], color);
  }
}

void screen_clear(void) {
  unsigned char i;
  screen_tx(0x45);

  screen_ack();

  for (i = 0; i < 8; i++) {
    screen_buffer[i] = 0;
  }

}

void pen_fill(void) {
  screen_tx(0x70);  // cmd
  screen_tx(0x00);

  screen_ack();
}

void pen_line(void) {
  screen_tx(0x70);  // cmd
  screen_tx(0x01);

  screen_ack();
}

void draw_rec(
  unsigned char x1,
  unsigned char y1,
  unsigned char x2,
  unsigned char y2,
  uint16_t color) {

  screen_tx(0x72); // cmd
  screen_tx(x1);
  screen_tx(y1);
  screen_tx(x2);
  screen_tx(y2);

  screen_tx(color >> 8);
  screen_tx((unsigned char)color);

  screen_ack();
}

void draw_text(
  unsigned char x,
  unsigned char y,
  unsigned char font,
  uint16_t color,
  unsigned char width,
  unsigned char height,
  unsigned char length,
  char *string) {
  unsigned char i;

  screen_tx(0x53);  // cmd
  screen_tx(x);
  screen_tx(y);
  screen_tx(font);

  screen_tx(color >> 8);
  screen_tx((unsigned char)color);

  screen_tx(width);
  screen_tx(height);

  for (i = 0; i < length; i++) {
    screen_tx(string[i]);
  }

  screen_tx(0);

  screen_ack();
}

void screen_cp(
  unsigned char xs,
  unsigned char ys,
  unsigned char xd,
  unsigned char yd,
  unsigned char width, 
  unsigned char height) {

  screen_ack();
}
