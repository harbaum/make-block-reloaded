// -*-c-*-
/* 
   score.ino
*/

#include <FastLED.h>
#include "makeblock.h"

static char score_msg[] = "NEW HI SCORE: xxxxxxx";
static char *score_ptr;
static int16_t score_len, score_scroll;
static uint16_t score_timeout;

void FillLEDsFromPaletteColors(uint8_t y, uint8_t colorIndex) {
  CRGBPalette16 currentPalette = RainbowColors_p;

  for( uint8_t x = 0; x < W; x++) {
    LED(x,y) = 
      ColorFromPalette( currentPalette, colorIndex, 255, LINEARBLEND);
    colorIndex += 3;
  }
}

void score_init(uint32_t score, int8_t is_hi) {
  LEDS.clear();
  LEDS.setBrightness(config_get_brightness());

  score_ptr = score_msg+(is_hi?0:7);
  ltoa(score, score_msg+14, 10);
  score_len = text_str_len(score_ptr);
  score_scroll = -2*W;
  score_timeout = 0;
}

#define SCORE_MSG_Y  8

#define SCORE_TIME 30
#define SCORE_FRAMES  (SCORE_TIME * FPS)

uint8_t score_process(uint8_t keys) {

  if(score_scroll & 1) {
    // clear text area
    for(uint8_t y=0;y<5;y++)
      for(uint8_t x=0;x<W;x++)
	LED(x, SCORE_MSG_Y+y) = CRGB::Black;
    
    text_scroll(score_ptr, score_scroll>>1, 0, W, SCORE_MSG_Y, CRGB::White);
  }

  if(++score_scroll > 2*(score_len+W))
    score_scroll = -2*W;

  // some colorful animation ...
  static uint8_t coff = 0;
  FillLEDsFromPaletteColors(SCORE_MSG_Y-2, coff);
  FillLEDsFromPaletteColors(SCORE_MSG_Y+6, coff);
  ++coff;

  score_timeout++;
  uint8_t score_brightness = config_get_brightness();
  if(score_timeout > SCORE_FRAMES) {    // 30 sec
    // fade out 
    if((score_timeout <= SCORE_FRAMES+score_brightness)&&
       (SCORE_FRAMES+score_brightness-score_timeout <= score_brightness))
      LEDS.setBrightness(SCORE_FRAMES+score_brightness-score_timeout);

    // 1 second of total darkness ...
    if(score_timeout == SCORE_FRAMES+score_brightness+FPS)
      return 2;
  }

  return keys?1:0;
}
