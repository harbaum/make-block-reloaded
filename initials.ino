/* 
   initials.ino

   eeprom usage: [20] == magic 0x42, [21..36] = 16 bytes name
*/

#include <FastLED.h>
#include <EEPROM.h>
#include "tetris.h"

#define INITIALS_BACK  1
#define INITIALS_DONE  2

#define INITIALS_SCORE_Y 2
#define INITIALS_Y 12

static char initials_str[16]; // 15 chars
static char initials_score_str[] = "NEW HIGH SCORE: xxxxxxx";
static int16_t initials_score_len, initials_score_scroll;
static int16_t initials_x;
static uint8_t initials_cur_chr;
static uint8_t initials_when_entered;

void initials_colorbar(uint8_t y, uint8_t colorIndex) {
  CRGBPalette16 currentPalette = RainbowColors_p;

  for( uint8_t x = 0; x < W; x++) {
    LED(x,y) = 
      ColorFromPalette( currentPalette, colorIndex, 255, LINEARBLEND);
    colorIndex += 3;
  }
}

void initials_entry_draw() {
  // erase area
  for(uint8_t y=0;y<7;y++)
    for(uint8_t x=0;x<W;x++)
      LED(x, INITIALS_Y-1+y) = CRGB::Black;

  // determine x offset
  int16_t off = -1-(W/2);  // char to edit is in the middle of the screen
  // calculate offset of char in string
  for(uint8_t i=0;i<initials_cur_chr;i++)
    off += text_char_width(initials_str[i]);

  // current char is centered
  uint8_t cwid = text_char_width(initials_str[initials_cur_chr]);
  off += cwid/2;

  // draw highlight
  for(uint8_t y=0;y<7;y++) 
    for(uint8_t x=0;x<cwid+1;x++) 
      LED(W/2-cwid/2+x, INITIALS_Y-1+y) = 0x800000;  // 50% red

  // draw text
  text_scroll(initials_str, off, 0, W, INITIALS_Y, CRGB::White);
}

void initials_init(uint32_t score) {
  LEDS.clear();
  LEDS.setBrightness(config_brightness);

  // load previous name from eeprom
  // this is stored from byte 20 to 36
  memset(initials_str, 0, 16);
  if(EEPROM.read(20) == 0x42) {
    for(uint8_t i=0;i<15;i++)
      EEPROM.get(21+i, initials_str[i]);
  } else
    strcpy(initials_str, "A");

  ltoa(score, initials_score_str+16, 10);
  initials_score_len = text_str_len(initials_score_str);
  initials_score_scroll = -2*W;

  // editing starts with first char
  initials_cur_chr = 0;

  initials_entry_draw();
}

uint8_t initials_process(uint8_t keys) {
  // some colorful animation ...
  static uint8_t coff = 0;
  initials_colorbar(INITIALS_SCORE_Y-2, coff);
  initials_colorbar(INITIALS_SCORE_Y+6, coff);
  ++coff;

  // process name entry field
  if((keys & KEY_LEFT) && initials_cur_chr > 0) {
    // going left from BACK/OK? Erase that
    if((initials_str[initials_cur_chr] == '\x01') ||
       (initials_str[initials_cur_chr] == '\x02'))
      initials_str[initials_cur_chr] = initials_when_entered;

    initials_cur_chr--;
    initials_entry_draw();
  }

  if(((keys & KEY_ROTATE) || (keys & KEY_RIGHT))
     && (initials_cur_chr < 14)) {
    // can't go right from BACK or OK
    if((initials_str[initials_cur_chr] != '\x01') &&
       (initials_str[initials_cur_chr] != '\x02')) {
      initials_cur_chr++;
      initials_when_entered = initials_str[initials_cur_chr];
      
      // check if there isn't already a character
      if(!initials_str[initials_cur_chr])
	initials_str[initials_cur_chr] = '\x02';
      
      initials_entry_draw();

      keys = 0;  // prevent key from fire again
    }
  }

  if(keys & KEY_ROTATE) {
    if(initials_str[initials_cur_chr] == INITIALS_BACK) {
      initials_cur_chr--;
      for(uint8_t i=initials_cur_chr;i<15;i++)
	initials_str[i] = initials_str[i+1];
	
      // deleted second char? Then make first char a 'A' instead
      // of the del char
      if(!initials_cur_chr)
	initials_str[initials_cur_chr] = 'A';

      initials_entry_draw();
    }

    if(initials_str[initials_cur_chr] == INITIALS_DONE) {
      // fill current string up with zeros
      for(uint8_t i=initials_cur_chr;i<16;i++)
	initials_str[i] = 0;

      EEPROM.write(20, 0x42);   // write magic marker
      for(uint8_t i=0;i<15;i++)
	EEPROM.put(21+i, initials_str[i]);

      return 1;
    }
  }

  // "drop" is actually up
  if((keys & KEY_DROP) || (keys & KEY_DOWN)) {
    // keys are A-Z/0-9/-/SPC/BACK/OK
    char c = initials_str[initials_cur_chr];
    if(keys & KEY_DROP) {
      if((c >= 'A') && (c < 'Z')) c++;
      else if(c == 'Z') c = '0';
      else if((c >= '0') && (c < '9')) c++;
      else if(c == '9') c = '-';
      else if(c == '-') c = ' ';
      // first char cannot be removed and also not completed
      else if((c == ' ') && (initials_cur_chr == 0)) c = 'A';
      else if(c == ' ') c = INITIALS_BACK;
      else if(c == INITIALS_BACK) c = INITIALS_DONE;
      else if(c == INITIALS_DONE) c = 'A';
    } else {
      if((c == 'A') && (initials_cur_chr == 0)) c=' ';
      else if(c == 'A') c = INITIALS_DONE;
      else if((c > 'A') && (c <= 'Z')) c--;
      else if(c == '0') c = 'Z';
      else if((c > '0') && (c <= '9')) c--;
      else if(c == '-') c = '9';
      else if(c == ' ') c = '-';
      else if(c == INITIALS_BACK) c = ' ';
      else if(c == INITIALS_DONE) c = INITIALS_BACK;
    }
    initials_str[initials_cur_chr] = c;
    initials_entry_draw();
  }

  // draw scrolling score, scroll at 30Hz
  if(initials_score_scroll & 1) {
    // clear text area
    for(uint8_t y=0;y<5;y++)
      for(uint8_t x=0;x<W;x++)
	LED(x,INITIALS_SCORE_Y+y) = CRGB::Black;
    
    text_scroll(initials_score_str, initials_score_scroll>>1, 0, W, 
		INITIALS_SCORE_Y, CRGB::White);
  }

  if(++initials_score_scroll > 2*(initials_score_len+W))
    initials_score_scroll = -2*W;

  return 0;
}
