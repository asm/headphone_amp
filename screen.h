void screen_init(void);

void screen_clear(void);

void pen_fill(void);

void pen_line(void);

void draw_rec(
  unsigned char x1,
  unsigned char y1,
  unsigned char x2,
  unsigned char y2,
  uint16_t color);

void draw_s_bar(uint16_t vec, unsigned char x);

void draw_bar(uint8_t height, unsigned char pos, uint16_t color);

void screen_cp(
  unsigned char xs,
  unsigned char ys,
  unsigned char xd,
  unsigned char yd,
  unsigned char width, 
  unsigned char height);

void draw_text(
  unsigned char x,
  unsigned char y,
  unsigned char font,
  uint16_t color,
  unsigned char width,
  unsigned char height,
  unsigned char length,
  char *string);

void draw_point(uint8_t x, uint8_t y, uint16_t color);

void draw_wav(int16_t wav[64], uint16_t color);
