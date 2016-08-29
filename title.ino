#include <EEPROM.h>

// variables required during the title 
int16_t title_score_len, title_scroll;
uint8_t title_logo_fade;

// horizontal split/shift 
static void image_in(const unsigned char *p, uint8_t o) {
  // load logo
  for(uint8_t x=0;x<W;x++) {
    for(uint8_t y=0;y<H;y++) {
      int src;
      if(y & 1) src = 3*(H-y-1+H*(x+W-o));
      else      src = 3*(H-y-1+H*(x-W+o));
    
      if((src >= 0) && (src < 3*W*H)) {
	LED(x,y)[0] = pgm_read_byte(p+src+0);
	LED(x,y)[1] = pgm_read_byte(p+src+1);
	LED(x,y)[2] = pgm_read_byte(p+src+2);
      } else
	LED(x,y) = CRGB::Black;
    }
  }
}

// max 15 chars user name
static char title_score_msg[] = "HIGH SCORE BY 0123456789ABCDE: 1234567";

// setup title
void title_init() {
  LEDS.clear();
  LEDS.setBrightness(config_get_brightness());

  // check if there's a user name in eeprom
  if(EEPROM.read(20) == 0x42) {
    uint8_t i = 0;

    // append the "BY " as it may have previously been removed
    // when there was no name so far
    strcpy(title_score_msg+10, " BY ");

    // laod max 15 bytes to byte 14 in string
    do { 
      EEPROM.get(21+i, title_score_msg[14+i]);
      i++;
    } while((i < 16) && (title_score_msg[14+i-1]));
	 
    strcat(title_score_msg, ": ");
  } else {
    // no name in eeprom: attach value directly
    strcpy(title_score_msg+10, ": ");
  }

  // load hi score
  uint32_t hi = 0;
  if(EEPROM.read(0) == 0x42)
    EEPROM.get(1, hi);

  ltoa(hi, title_score_msg+strlen(title_score_msg), 10);
  title_score_len = text_str_len(title_score_msg);

  title_scroll = -5*W;
  title_logo_fade = 0;
}

uint8_t title_process(uint8_t keys) {
  // make logo appear
  if(title_logo_fade <= W)
    image_in(logo, title_logo_fade++);
  
  // draw scrolling text every second frame
  if(title_scroll & 1) {
    // clear text area
    for(uint8_t y=0;y<5;y++)
      for(uint8_t x=0;x<W;x++)
	LED(x,y) = CRGB::Black;

    // scroll two text alternating
    text_scroll(title_score_msg, title_scroll>>1, 0, W, 0, CRGB::White);
  }

  if(++title_scroll > 2*(title_score_len+W))
    title_scroll = -2*W;

 return keys;
}
