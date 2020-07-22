// -*-c-*-
/* 
   makeblock.ino
   
   Real tetris for the 15*20 make:block

   eeprom usage: 
   [0] == magic 0x42, [1..4] = 32 bits tetris hi score, 
                       [5] game, [6..9] mario score
   [10] == magic 0x42, [11,12] config
   [20] == magic 0x42, [21..] tetris name
   [40] == magic 0x42, [41..] mario name
   [60] == magic 0x42, ... pause state

   EEPROM usage isn't perfect but i had to keep it compatible
   during development to keep the kids happy by preserving their
   hi scores and names ...
*/

#include <EEPROM.h>
#include <FastLED.h>
#include "makeblock.h"

CRGB leds[NUM_LEDS];

state_t state;

extern uint32_t tetris_hi_score, mario_hi_score;

// global game data for tetris. mario and the title
struct gameS game;

static const uint8_t pause_icon[] PROGMEM = {
  0x00, 0x7c, 0x14, 0x14, 0x08, 0x00
};

void game_pause(int8_t store) {

  // save in eeprom. Don't save when restoring
  // a pause state from eeprom
  if(store) {
    // save game state in eeprom
    EEPROM.write(60, 0x42);   // write magic marker
    EEPROM.put(61, game);     // write game state

    // mute audio while paused. Only do this when
    // paused from within the running game and not
    // when restoring a pause state.
    audio_set(128);	    
  }

  uint8_t px = 0, py = 0;
  if(game.game == GAME_TETRIS) {
    tetris_pause();
    px = TETRIS_PAUSE_X;
    py = TETRIS_PAUSE_Y;
  } else if(game.game == GAME_MARIO) {
    mario_pause();
    px = MARIO_PAUSE_X;
    py = MARIO_PAUSE_Y;
  }
    
  // and overlay "P"
  if(px && py) {
    for(uint8_t y=0;y<7;y++) { 
      for(uint8_t x=0;x<sizeof(pause_icon);x++) {
	if(pgm_read_byte_near(pause_icon+x) & (0x80>>y))
	  LED(px+x,py+y) = CRGB::Purple;
	else
	  LED(px+x,py+y) = CRGB::White;      
      }
    }
  }
}

unsigned long next_event;

void setup() {
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);

  pinMode(LED_PIN, OUTPUT);     
  pinMode(SPEAKER_PIN, OUTPUT);     
  keys_init();

  // init game cycle counter
  next_event = millis() + GAME_CYCLE;

  // load config (audio & brightness) from eeprom
  config_load();

  // check if user pressed a button at startup to enter
  // the config menu 
  if(keys_any_down()) {
    config_init();
    state = STATE_CONFIG;
  } else {
    // normal startup
    title_init();
    state = STATE_TITLE;

    // check for a save state
    if(EEPROM.read(60) == 0x42) {
      if(game.game == GAME_TETRIS) {
      
	// restore a tetris state
	game_init();

	// read game state from eeprom
	EEPROM.get(61, game);
      
	// update preview and level as loaded from save state
	game_tetromino_preview_draw(game.tetris.tetromino.next);
	game_show_level();
	
	// draw game area once      
	game_area_blit();
	game_pause(0);

	// and go into paused state
	state = STATE_PAUSED;
      }
    }    
  }

  song_init();
}

void loop() {

  // frame time hasn't elapsed yet?
  // the following will also work when millis() wraps (after 49 days :-)
  if( (long)(next_event - millis()) > 0 ) {
    // can do background stuff here like playing music ...

    delay(1);  // sleep a little bit
  } else {
    static uint8_t frame_cnt;

    // flash heartbeat led
    if(frame_cnt < FPS-2) digitalWrite(LED_PIN, LOW);
    else                  digitalWrite(LED_PIN, HIGH);
    if(++frame_cnt == FPS)
      frame_cnt = 0;
   
    // config has a faster key repeat for left/right
    // initials has constant repeat for up/down
    uint8_t keys = keys_get((state == STATE_CONFIG)?1:(state == STATE_INITIALS)?2:0);

    // game state
    switch(state) {
    case STATE_CONFIG:
      if(config_process(keys)) {
	title_init();
	state = STATE_TITLE;
      }
      break;

    case STATE_TITLE:
      switch(title_process(keys)) {
      case 1:
	if(game.game == GAME_TETRIS)
	  game_init();
	else
	  mario_init();
	
	state = STATE_GAME;
	break;

      default:
	break;
      }
      break;

    case STATE_PAUSED:
      if(game.game == GAME_TETRIS) {      
	// tetris score still scrolls
	game_draw_score();
      }
	
      // fire key unpauses
      if(keys & KEY_ROTATE) {
	  EEPROM.write(60, 0x00);   // clear magic save state marker
	  state = STATE_GAME;
	}
      break;
      
    case STATE_GAME:
      if(game.game == GAME_TETRIS) {
	song_process(0, game.tetris.level+1);
	if(game_process(keys)) {
	  keys_lock();   // prevent any further auto repeat
	  if(game.tetris.score > tetris_hi_score) {
	    initials_init(game.tetris.score);
	    state = STATE_INITIALS;
	  } else {
	    score_init(game.tetris.score, game.tetris.score > tetris_hi_score);
	    state = STATE_SCORE;
	  }
	  song_process(0,0);
	}
      } else if(game.game == GAME_MARIO) {
	song_process(1, 30);
	if(mario_process(keys)) {
	  keys_lock();   // prevent any further auto repeat

	  // a score of 0 means mario died without finishing the level
	  if(game.mario.score) {
	    if(game.mario.score > mario_hi_score) {
	      initials_init(game.mario.score);
	      state = STATE_INITIALS;
	    } else {
	      score_init(game.mario.score, game.mario.score > mario_hi_score);
	      state = STATE_SCORE;
	    }
	  } else {
	    title_init();
	    state = STATE_TITLE;
	  }
	  song_process(1,0);
	}
      }
      break;

    case STATE_SCORE:
      // don't jump directly into game again, since
      // the user may want to change the game
      if(score_process(keys)) {
	title_init();
	state = STATE_TITLE;
      }
      break;

    case STATE_INITIALS:
      if(initials_process(keys)) {
	title_init();
	state = STATE_TITLE;
      }
      break;

    default:
      break;
    }

    LEDS.show();
    
    next_event += GAME_CYCLE;
  }
}
  
