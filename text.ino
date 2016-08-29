#include <FastLED.h>
#include "tetris.h"

// extremely tiny font
const uint8_t font[][5] PROGMEM = {
  { 0b011110, 0b000101, 0b011110, 0b000000, 0b000000 },  // A
  { 0b011111, 0b010101, 0b001010, 0b000000, 0b000000 },  // B
  { 0b001110, 0b010001, 0b010001, 0b000000, 0b000000 },  // C
  { 0b011111, 0b010001, 0b001110, 0b000000, 0b000000 },  // D
  { 0b011111, 0b010101, 0b010001, 0b000000, 0b000000 },  // E
  { 0b011111, 0b000101, 0b000001, 0b000000, 0b000000 },  // F
  { 0b001110, 0b010001, 0b011010, 0b000000, 0b000000 },  // G
  { 0b011111, 0b000100, 0b011111, 0b000000, 0b000000 },  // H
  { 0b011111, 0b000000, 0b000000, 0b000000, 0b000000 },  // I
  { 0b010000, 0b001111, 0b000000, 0b000000, 0b000000 },  // J
  { 0b011111, 0b000100, 0b011011, 0b000000, 0b000000 },  // K
  { 0b011111, 0b010000, 0b010000, 0b000000, 0b000000 },  // L
  { 0b011111, 0b000010, 0b000100, 0b000010, 0b011111 },  // M
  { 0b011111, 0b000010, 0b000100, 0b011111, 0b000000 },  // N
  { 0b001110, 0b010001, 0b010001, 0b001110, 0b000000 },  // O
  { 0b011111, 0b000101, 0b000010, 0b000000, 0b000000 },  // P
  { 0b001110, 0b010001, 0b011110, 0b000000, 0b000000 },  // Q
  { 0b011111, 0b000101, 0b011010, 0b000000, 0b000000 },  // R
  { 0b010010, 0b010101, 0b001001, 0b000000, 0b000000 },  // S
  { 0b000001, 0b011111, 0b000001, 0b000000, 0b000000 },  // T
  { 0b001111, 0b010000, 0b011111, 0b000000, 0b000000 },  // U
  { 0b000111, 0b011000, 0b000111, 0b000000, 0b000000 },  // V
  { 0b001111, 0b010000, 0b001000, 0b010000, 0b001111 },  // W
  { 0b011011, 0b000100, 0b011011, 0b000000, 0b000000 },  // X
  { 0b000011, 0b011100, 0b000011, 0b000000, 0b000000 },  // Y
  { 0b011001, 0b010101, 0b010011, 0b000000, 0b000000 },  // Z
  { 0b001110, 0b010001, 0b001110, 0b000000, 0b000000 },  // 0
  { 0b000010, 0b011111, 0b000000, 0b000000, 0b000000 },  // 1
  { 0b011001, 0b010101, 0b010010, 0b000000, 0b000000 },  // 2
  { 0b010001, 0b010101, 0b001010, 0b000000, 0b000000 },  // 3
  { 0b001100, 0b001010, 0b011111, 0b000000, 0b000000 },  // 4
  { 0b010111, 0b010101, 0b001001, 0b000000, 0b000000 },  // 5
  { 0b001110, 0b010101, 0b001001, 0b000000, 0b000000 },  // 6
  { 0b000001, 0b011001, 0b000111, 0b000000, 0b000000 },  // 7
  { 0b001010, 0b010101, 0b001010, 0b000000, 0b000000 },  // 8
  { 0b010010, 0b010101, 0b001110, 0b000000, 0b000000 },  // 9
  { 0b010100, 0b000000, 0b000000, 0b000000, 0b000000 },  // :
  { 0b000100, 0b000100, 0b000000, 0b000000, 0b000000 },  // -
  { 0b000100, 0b001110, 0b010101, 0b000100, 0b000100 },  // <-
  { 0b001000, 0b010000, 0b001000, 0b000100, 0b000010 },  // OK
};

// origin is bottom left
void text_draw_pixel(uint8_t x, uint8_t y, CRGB c) {
  if((x<W)&&(y<H))
    LED(x,y) = c;
}

// display a single character at pos x,y
uint8_t text_draw_char(char chr, int8_t x, int8_t y, int8_t skip, uint8_t len, CRGB c) {
  const uint8_t *p;
  uint8_t l=0;

  // check if character is on screen
  if(x > W) return 0;

  if((chr >= 'A') && (chr <= 'Z')) p = font[chr-'A'];
  else if((chr >= '0') && (chr <= '9')) p = font[chr-'0'+'Z'-'A'+1];
  else if(chr == ':') p = font['Z'-'A'+11];
  else if(chr == '-') p = font['Z'-'A'+12];
  else if(chr == 1) p = font['Z'-'A'+13];
  else if(chr == 2) p = font['Z'-'A'+14];
  else return l+3;

  while(pgm_read_byte(p) && (l<5)) {
    if(skip > 0) skip--;
    else if(len > 0) {
      for(uint8_t b=0;b<6;b++)
	if(pgm_read_byte(p) & (0x20>>b)) 
	  text_draw_pixel(x, y+b-1, c);
    }
    
    len--;
    l++;
    x++;
    p++;
  }

  return l+1;  // x offset for next character
}

uint8_t text_char_width(char chr) {
  uint8_t x=0;
  const uint8_t *p;
  if((chr >= 'A') && (chr <= 'Z')) p = font[chr-'A'];
  else if((chr >= '0') && (chr <= '9')) p = font[chr-'0'+'Z'-'A'+1];
  else if(chr == ':') p = font['Z'-'A'+11];
  else if(chr == '-') p = font['Z'-'A'+12];
  else if(chr == 1) p = font['Z'-'A'+13];
  else if(chr == 2) p = font['Z'-'A'+14];
  else return 3;

  while(pgm_read_byte(p++) && (x<5)) x++;
  return x+1;
}

// draw string at x/y. skip the first "skip" pixels and draw at most
// len pixels
void text_str(const char *str, int8_t x, int8_t y, 
	      int8_t skip, int8_t len, CRGB c) {
  while(*str && x<W) {
    uint8_t w = text_draw_char(*str++, x, y, skip, len, c);
    x += w;
    len -= w;
    skip -= w;
  }
}

int16_t text_str_len(const char *str) {
  int16_t ret = 0;

  while(*str)
    ret += text_char_width(*str++);

  return ret-1;
}

void text_scroll(const char *str, int16_t offset, int8_t x, uint8_t len, int8_t y, CRGB c) {

  if(offset < 0) {
    // negative offset means text starts further to the right
    if(offset < -W) return;  // no text visible at all
  } else if(offset > 0) {
    // positive offset means display starts within text

    // skip "unused" characters
    uint8_t w = text_char_width(*str);
    while(*str && offset >= w) {
      offset -= w;
      str++;
      w = text_char_width(*str);
    }
  }

  // start text earlier by (remaining) offset
  text_str(str, x-offset, y, offset, len+offset, c);
}
