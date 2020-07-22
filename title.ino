// -*-c-*-
#include <EEPROM.h>

// horizontal split/shift 
static void image_in(const unsigned char *p, uint8_t o) {
  // load logo
  for(uint8_t x=0;x<W;x++) {
    for(uint8_t y=0;y<H;y++) {
      int src;
      if(y & 1) src = 3*(H-y-1+H*(x+W-o));
      else      src = 3*(H-y-1+H*(x-W+o));
    
      if((src >= 0) && (src < 3*W*H)) {
	LED(x,y)[0] = pgm_read_byte_near(p+src+0);
	LED(x,y)[1] = pgm_read_byte_near(p+src+1);
	LED(x,y)[2] = pgm_read_byte_near(p+src+2);
      } else
	LED(x,y) = CRGB::Black;
    }
  }
}

void title_update() {
  uint8_t hs_base = (!game.game)?1:6;     // location of hiscore in eeprom for tetris and mario
  uint8_t name_base = (!game.game)?20:40; // location of name in eeprom for tetris and mario
  
  strcpy_P(game.title.score_msg, PSTR("HIGH SCORE BY 0123456789ABCDE: 1234567"));

  // load hi score
  uint32_t hi = 0;
  if(EEPROM.read(0) == 0x42) {
    EEPROM.get(hs_base, hi);

    // a mario score > 9999 is likely a leftover of a firmware which
    // didn't include mario and didn't initialize the mamory region
    if(game.game && (hi > 9999))
      hi = 0;

    // check if there's a user name in eeprom
    if(EEPROM.read(name_base) == 0x42) {
      uint8_t i = 0;

      // append the "BY " as it may have previously been removed
      // when there was no name so far
      strcpy_P(game.title.score_msg+10, PSTR(" BY "));

      // laod max 15 bytes to byte 14 in string
      do { 
	EEPROM.get(name_base+1+i, game.title.score_msg[14+i]);
	i++;
      } while((i < 16) && (game.title.score_msg[14+i-1]));
      
      strcat_P(game.title.score_msg, PSTR(": "));
    } else {
      // no name in eeprom: attach value directly
      strcpy_P(game.title.score_msg+10, PSTR(": "));
    }

    ltoa(hi, game.title.score_msg+strlen(game.title.score_msg), 10);
    game.title.score_len = text_str_len(game.title.score_msg);
  }
}
  
// setup title
void title_init() {
  LEDS.clear();
  LEDS.setBrightness(config_get_brightness());

  game.game = GAME_TETRIS;        // default tetris
  if(EEPROM.read(0) == 0x42) 
    EEPROM.get(5, game.game);
    
  game.title.scroll = -5*W;
  game.title.logo_fade = 0;
  game.title.timer = 0;

  title_update();
}

uint8_t title_process(uint8_t keys) {
  if((keys & (KEY_LEFT | KEY_RIGHT)) && (game.title.logo_fade == W)) {
    game.title.timer = 0;
    game.title.logo_fade = -W;
    game.title.scroll = -5*W;
  }

  // make logo (dis-)appear
  image_in((game.game == GAME_TETRIS)?logo:logo_m, abs(game.title.logo_fade));
  
  if(game.title.logo_fade < W) {
    if(++game.title.logo_fade == 0) {
      game.game = !game.game;
      title_update();
    }
  }
  
  // scroll two text alternating
  text_scroll(game.title.score_msg, game.title.scroll>>1, 0, W, 0, CRGB::White);

  if(++game.title.scroll > 2*(game.title.score_len+W))
    game.title.scroll = -2*W;

  // fire (rotate) starts game
  if(keys & KEY_ROTATE)
    EEPROM.put(5, game.game);

  return((keys & KEY_ROTATE)?1:0);
}
