/* 
   tetris.ino
   
   Real tetris for the 15*20 make:block
   eeprom usage: 
   [0] == magic 0x42, [1..4] = 32 bits hi score
   [40] == magic 0x42, ... pause state
*/

#include <EEPROM.h>
#include <FastLED.h>
#include "tetris.h"

#define INIT_LEVEL 0

CRGB leds[NUM_LEDS];

#define GAME_X     0
#define GAME_Y     1

#define PREVIEW_X  12
#define PREVIEW_Y  (H-2)

#define SCORE_X 11
#define SCORE_Y  1
#define SCORE_W  4

#define LEVEL_X  11
#define LEVEL_Y   8

// possible game states
typedef enum { STATE_CONFIG, STATE_TITLE, STATE_PAUSED,
	       STATE_GAME, STATE_SCORE, STATE_INITIALS } state_t;

state_t state;

// caution: this does not check for boundaries
void rect(int8_t x, int8_t y, uint8_t w, uint8_t h, CRGB c) {
  for(int8_t i=x;i<x+w;i++)
    for(int8_t j=y;j<y+h;j++)
      LED(i,j) = c;
}

// ------- game engine

// colors according to "tetris company standard"
// up to 16 colors possible with this engine
static const uint32_t tetromino_colors[] PROGMEM = {
  0x202020, 0x00ffff, 0xffa500, 0xffff00,   // empty, I, L, O
  0x4040ff, 0xff0000, 0x00ff00, 0xc000c0,   // J, Z, S, T
  0xffffff, 0x000000                        // highlight, closed
};

/* mapping of tetrominos under all four angles */
static const int8_t tetrominos[][4][4][2] PROGMEM = {  {  
  { {-1, 0},{ 0, 0},{ 1, 0},{ 2, 0} },   /* cyan    */
  { { 0,-1},{ 0, 0},{ 0, 1},{ 0, 2} },   /*  #*##   */
  { {-1, 0},{ 0, 0},{ 1, 0},{ 2, 0} },   /*         */
  { { 0,-1},{ 0, 0},{ 0, 1},{ 0, 2} }    /*         */
}, {  
  { {-1, 1},{-1, 0},{ 0, 0},{ 1, 0} },   /* orange  */
  { {-1,-1},{ 0,-1},{ 0, 0},{ 0, 1} },   /*  #*#    */
  { {-1, 0},{ 0, 0},{ 1, 0},{ 1,-1} },   /*  #      */
  { { 0,-1},{ 0, 0},{ 0, 1},{ 1, 1} }    /*         */
}, {  
  { { 0, 0},{ 1, 0},{ 0, 1},{ 1, 1} },   /* yellow  */
  { { 0, 0},{ 1, 0},{ 0, 1},{ 1, 1} },   /*   *#    */
  { { 0, 0},{ 1, 0},{ 0, 1},{ 1, 1} },   /*   ##    */
  { { 0, 0},{ 1, 0},{ 0, 1},{ 1, 1} }    /*         */
}, {  
  { {-1, 0},{ 0, 0},{ 1, 0},{ 1, 1} },   /* blue    */
  { { 0,-1},{ 0, 0},{ 0, 1},{-1, 1} },   /*  #*#    */
  { {-1,-1},{-1, 0},{ 0, 0},{ 1, 0} },   /*    #    */
  { { 1,-1},{ 0,-1},{ 0, 0},{ 0, 1} }    /*         */
}, {  
  { {-1, 0},{ 0, 0},{ 0, 1},{ 1, 1} },   /* red     */
  { { 0,-1},{ 0, 0},{-1, 0},{-1, 1} },   /*  #*     */
  { {-1, 0},{ 0, 0},{ 0, 1},{ 1, 1} },   /*   ##    */
  { { 0,-1},{ 0, 0},{-1, 0},{-1, 1} }    /*         */
}, {  
  { { 1, 0},{ 0, 0},{ 0, 1},{-1, 1} },   /* green   */
  { {-1,-1},{-1, 0},{ 0, 0},{ 0, 1} },   /*   *#    */
  { { 1, 0},{ 0, 0},{ 0, 1},{-1, 1} },   /*  ##     */
  { {-1,-1},{-1, 0},{ 0, 0},{ 0, 1} }    /*         */
}, {  
  { {-1, 0},{ 0, 0},{ 1, 0},{ 0, 1} },   /* purple  */
  { { 0,-1},{ 0, 0},{-1, 0},{ 0, 1} },   /*  #*#    */
  { {-1, 0},{ 0, 0},{ 0,-1},{ 1, 0} },   /*   #     */
  { { 0,-1},{ 0, 0},{ 1, 0},{ 0, 1} }    /*         */
} };  

// standard tetris size as seen on gameboy
#define GAME_W  10
#define GAME_H  18

// ------------------- the entire in-game state ------------

struct {
  uint8_t step_cnt;
  uint8_t level;
  uint32_t score;
  uint16_t lines;
  uint8_t cont_drop;

  struct {
    uint8_t x, y;
    uint8_t rot, type, next;
  } tetromino;

  uint32_t row_remove;
  uint8_t row_remove_timer;
  
  // two blocks are saved per byte -> 90 bytes
  uint8_t area[GAME_W/2][GAME_H];
} game;

uint32_t hi_score;
  
uint8_t game_level_rate() {
  // speed table. taken from gameboy version. values in 60Hz steps
  static const uint8_t step_cnt_table[] PROGMEM = 
    // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
    { 53,49,45,41,37,33,28,22,17,11,10, 9, 8, 7, 6, 6, 5, 5, 4, 4, 3 }; 

  return pgm_read_byte(step_cnt_table+((game.level<20)?game.level:20));
}

// show level
// level: 0-9 = green digit 0-9, 10-19 = yellow digit 0-9, >= 20 = red X
void game_show_level() {
  rect(LEVEL_X, LEVEL_Y, 3, 5, CRGB::Black);
  if(game.level < 10)
    text_draw_char('0'+game.level, LEVEL_X, LEVEL_Y, 0, 3, CRGB(0x00ff00));
  else if(game.level < 20)
    text_draw_char('0'+game.level-10, LEVEL_X, LEVEL_Y, 0, 3, CRGB(0xffff00));
  else
    text_draw_char('X', LEVEL_X, LEVEL_Y, 0, 3, CRGB(0xff0000));
}

void game_tetromino_set_block(uint8_t x, uint8_t y, uint8_t col) {
  // we should never draw outside the game area
  if((x >= GAME_W) || (y >= GAME_H))
    return;

  if(x&1) game.area[x/2][y] = (game.area[x/2][y] & 0x0f) | (col<<4);
  else    game.area[x/2][y] = (game.area[x/2][y] & 0xf0) | (col&0x0f);
}

uint8_t game_tetromino_get_block(uint8_t x, uint8_t y) {
  // return 1 for all blocks outside game area so these seem to be
  // "occupied" to the game engine. Return 0 for one row above 
  // game area to ease implementation of scrolling down
  if((x >= GAME_W)||(y > GAME_H)) return 1;
  if(y == GAME_H) return 0;

  if(x&1) return game.area[x/2][y]>>4;
  return game.area[x/2][y]&0x0f;
}

void game_tetromino_draw(char show) {
  // get pointer to current tetromino shape at current angle
  int8_t const (*p)[2] = tetrominos[game.tetromino.type][game.tetromino.rot];

  // set all four blocks a tetromino consists of
  for(uint8_t i=0;i<4;i++) 
    game_tetromino_set_block(game.tetromino.x + pgm_read_byte(&p[i][0]), 
			     game.tetromino.y - pgm_read_byte(&p[i][1]), 
			     show?game.tetromino.type+1:0);
}

char game_tetromino_ok() {
  // get pointer to current tetromino shape at current angle
  int8_t const (*p)[2] = tetrominos[game.tetromino.type][game.tetromino.rot];
 
  // check all four blocks a tetromino consists of
  for(uint8_t i=0;i<4;i++) 
    if(game_tetromino_get_block(game.tetromino.x + pgm_read_byte(&p[i][0]), 
				game.tetromino.y - pgm_read_byte(&p[i][1])))
      return false;
  
  return true;
}

void game_tetromino_preview_draw(uint8_t type) {
  // erase 4*2 preview area
  for(uint8_t y=0;y<2;y++)
    for(uint8_t x=0;x<4;x++)
      LED(PREVIEW_X+x-1, PREVIEW_Y-y) =
	CRGB::Black;
  
  if(type != 0xff) {
    int8_t const (*p)[2] = tetrominos[type][0];
    for(uint8_t i=0;i<4;i++) {
      uint32_t c = pgm_read_dword_near(tetromino_colors + type + 1);
      LED(PREVIEW_X+(int8_t)pgm_read_byte(&p[i][0]),
	  (PREVIEW_Y-(int8_t)pgm_read_byte(&p[i][1]))) = CRGB(c);
    }
  }
}

void game_tetromino_new() {
  game.tetromino.rot = 0;
  game.tetromino.type = game.tetromino.next;
  game.tetromino.next = random(0,7);
  game.tetromino.x = 4;

  // on gameboy the tetrominos spawn one row below top 
  // game area ...
  game.tetromino.y = GAME_H-2;

  // check if new tetromino can be placed on screen
  if(game_tetromino_ok()) {
    game_tetromino_draw(true);
    
    // and show next tetromino
    game_tetromino_preview_draw(game.tetromino.next);
  } else {
    game_tetromino_preview_draw(0xff);

    // set current tetromino to 0xff indicating that
    // no game is in progress anymore
    game.tetromino.type = 0xff;
    game.row_remove_timer = 0;

    // remove level indicator
    rect(LEVEL_X, LEVEL_Y, 3, 5, CRGB::Black);

    // remove score
    rect(SCORE_X, SCORE_Y, SCORE_W, 5, CRGB::Black);
  }
}

// move tetromino in the given direction or return false
// if impossible
char game_tetromino_move(int8_t x, int8_t y, int8_t rot) {
  char ret = true;

  // remove current tetromino
  game_tetromino_draw(false);

  // advance tetromino
  game.tetromino.x += x;
  game.tetromino.y += y;
  game.tetromino.rot = (game.tetromino.rot+rot)&3;

  // and check if it could be drawn
  if(!game_tetromino_ok()) {
    // restore old position
    game.tetromino.x -= x;
    game.tetromino.y -= y;
    game.tetromino.rot = (game.tetromino.rot-rot)&3;
    ret = false;
  }
    
  game_tetromino_draw(true);
  return ret;
}

void game_tetromino_locked() {
  // lock keys so the have to be released before auto repeat kicks in again
  keys_lock();

  // any manual drop before placement gives one extra point
  if(game.cont_drop) {
    game.score += game.cont_drop;
    game.cont_drop = 0;
  }

  // check if a row was filled
  for(uint8_t y=0;y<GAME_H;y++) {
    // a row is full if no block is empty
    char row_full = true;
    for(uint8_t x=0;x<GAME_W;x++) 
      if(!game_tetromino_get_block(x, y))
	row_full = false;

    if(row_full) { 
      // trigger removal of that row
      game.row_remove |= (1<<y);
    
      // line clear take 90 frames according to 
      // http://tetrisconcept.net/wiki/Tetris_%28Game_Boy%29
      game.row_remove_timer = 90;
    }
  }

  // no row removed: spawn new tetromino immediately
  if(!game.row_remove)
    game_tetromino_new();
}

void game_init() {
  LEDS.clear();
  LEDS.setBrightness(config_get_brightness());

  // the microseconds since startup are a perfect seed as 
  // the user has pressed a button since boot time
  randomSeed(micros());   // init rng

  // clear game area
  for(uint8_t x=0;x<GAME_W/2;x++)
    for(uint8_t y=0;y<GAME_H;y++)
      game.area[x][y] = 0;
 
  game.row_remove = 0;  // no row being removed
  game.level = INIT_LEVEL;
  game.lines = 0;
  game.score = 0;
  game.cont_drop = 0;
  game.step_cnt = game_level_rate();

  // load hi score from eeprom
  // check if eeprom marker is valid
  if(EEPROM.read(0) == 0x42)
    EEPROM.get(1, hi_score);
  else {
    hi_score = 0;            // no high score yet
    EEPROM.write(0, 0x42);   // write magic marker
    EEPROM.put(1, hi_score); // write (clear) hi score
  }

  game_show_level();

  // for some reason the first call to random always returns 0 ...
  game.tetromino.next = random(0,7);
  game.tetromino.next = random(0,7);
  game_tetromino_new();
}

#define PULSE_STEPS  60

void game_draw_score() {
  // draw score while game is running 
  if(game.tetromino.type != 0xff) {
    static uint8_t pulse_cnt;
    static uint32_t cur_score = 0;
    static char score_str[7] = "0";
    static uint8_t score_len = 3;
    CRGB color = CRGB::White;

    // let score "pulse" if hi score was exceeded
    if(game.score <= hi_score) {
      pulse_cnt = 0;
      color = CRGB::White;
    } else {
      uint8_t shade;
      pulse_cnt++;
      if(pulse_cnt < PULSE_STEPS)
	 shade = 255*pulse_cnt/PULSE_STEPS;
      else 
	shade = 255*(2*PULSE_STEPS-pulse_cnt)/PULSE_STEPS;

      color = CRGB(shade, 255-shade, 255);

      if(pulse_cnt == 2*PULSE_STEPS-1)
	pulse_cnt = 0;
    }

    // update score string if necessary
    if(game.score != cur_score) {
      ltoa(game.score, score_str, 10);
      cur_score = game.score;
      score_len = text_str_len(score_str);
    }

    // scroll score
    static int8_t score_scroll = 0, sub_score_scroll = GAME_SCORE_SCROLL_SPEED;
    if(sub_score_scroll == 0) {
      rect(SCORE_X, SCORE_Y, SCORE_W, 5, CRGB::Black);
      // if only one digit: don't scroll at all
      // otherwise only scroll up to the last digit and stay
      // there for a moment
      text_scroll(score_str, (score_len == 3)?0:
		  (score_scroll > score_len-3)?score_len-3:score_scroll, 
		  SCORE_X, SCORE_W, SCORE_Y, color);
      score_scroll++;
      if(score_scroll == score_len+20)
	score_scroll = -5;
      
      sub_score_scroll = GAME_SCORE_SCROLL_SPEED;
    } else
      sub_score_scroll--;
  }
}

static const uint8_t pause_icon[] PROGMEM = {
  0x00, 0x7c, 0x14, 0x14, 0x08, 0x00
};

void game_pause(int8_t store) {

  // save in eeprom. Don't save when restoring
  // a pause state from eeprom
  if(store) {
    // save game state in eeprom
    EEPROM.write(40, 0x42);   // write magic marker
    EEPROM.put(41, game);     // write game state

    // mute audio while paused. Only do this when
    // paused from within the running game and not
    // when restoring a pause state.
    audio_set(128);	    
  }
    
  // make whole game area darker
  for(uint8_t y=0;y<GAME_H;y++) { 
    for(uint8_t x=0;x<GAME_W;x++) {
      LED(x+GAME_X,GAME_Y+y)[0] = LED(x+GAME_X,GAME_Y+y)[0] >> 1;
      LED(x+GAME_X,GAME_Y+y)[1] = LED(x+GAME_X,GAME_Y+y)[1] >> 1;
      LED(x+GAME_X,GAME_Y+y)[2] = LED(x+GAME_X,GAME_Y+y)[2] >> 1;
    }
  }

  // and overlay "P"
  for(uint8_t y=0;y<7;y++) { 
    for(uint8_t x=0;x<sizeof(pause_icon);x++) {
      if(pgm_read_byte(pause_icon+x) & (0x80>>y))
	LED(2+x+GAME_X,6+GAME_Y+y) = CRGB::Purple;
      else
	LED(2+x+GAME_X,6+GAME_Y+y) = CRGB::White;      
    }
  }
}

void game_area_blit(void) {
  
  for(uint8_t y=0;y<GAME_H;y++) { 
    if((game.row_remove & (1<<y)) && (game.row_remove_timer & 16))
      for(uint8_t x=0;x<GAME_W;x++)
	LED(x+GAME_X,GAME_Y+y) = CRGB::White;
    else
      for(uint8_t x=0;x<GAME_W;x++) {
	uint32_t c = pgm_read_dword_near(tetromino_colors +
					 game_tetromino_get_block(x, y));
	LED(x+GAME_X,GAME_Y+y) = CRGB(c);
      }
  }
}

uint8_t game_process(uint8_t keys) {
  static const uint8_t score_step[] = { 4, 10, 30, 120 };

  // type == 0xff means game has ended
  if(game.tetromino.type == 0xff) {
    if(game.row_remove_timer <= GAME_H) {
      for(uint8_t x=0;x < GAME_W;x++) {
	game_tetromino_set_block(x, game.row_remove_timer-1, 9);
	game_tetromino_set_block(x, game.row_remove_timer, 8);
      }
      game.row_remove_timer++;
    } else {
      // game area is closed ..

      // update high score if necessary
      if(game.score > hi_score)
	EEPROM.put(1, game.score); // write new high score

      return 1;
    }
  } else if(game.row_remove) {
    // row removal is in progress
    game.cont_drop = 0;
    game.row_remove_timer--;

    if(!game.row_remove_timer) {
      uint8_t removed = 0;
      // finally remove the full rows

      for(uint8_t y=0;y<GAME_H;y++) { 
	if(game.row_remove & (1<<y)) {
	  uint8_t k=y;
	  // shift all lines above down one line
	  while(k < GAME_H) {
	    for(uint8_t x=0;x<GAME_W;x++)
	      game_tetromino_set_block(x, k,
	       game_tetromino_get_block(x, k+1));
	    k++;
	  }

	  removed++;
	  game.lines++;
	  if((game.lines % 10) == 0) {
	    game.level++;
	    game_show_level();
	  }

	  // also shift table of full rows down
	  game.row_remove = (game.row_remove & ~(1<<y))>>1;
	  y--;  // check same row again
	}
      }

      // update score
      game.score += 10l * score_step[removed-1] * (game.level+1);

      // limit score to 999999
      if(game.score > 999999)
        game.score = 999999;

      game.row_remove = 0;
      game_tetromino_new();
    }
  } else {
    // advance tetromino manually
    int8_t x=0, y=0, rot=0;
    if(keys & KEY_LEFT)   x--;
    if(keys & KEY_RIGHT)  x++;
    if(keys & KEY_ROTATE) rot=-1;
    if(keys & KEY_DOWN)   y=-1;

#ifdef NO_DROP
    // drop acts like rotate
    if(keys & KEY_DROP) 
      rot=-1;
#else
    // hard drop: a gameboy doesn't do this ... Tanja likes it
    if(keys & KEY_DROP) {
      // remoove from current position
      game_tetromino_draw(false);
      
      // move down until it cannot be draw anymore
      do {
	game.tetromino.y--;
	// twice the soft drop score for this
	game.cont_drop+=2;
      } while(game_tetromino_ok());

      // move up one again
      game.tetromino.y++;
      game.cont_drop-=2;
      game_tetromino_draw(true);

      game_tetromino_locked();
    } else 
#endif

    {
      // do manual movement
      if(x || rot)
	game_tetromino_move(x, 0, rot);
      
      // y movement needs to be handles seperately since only
      // this will cause the tetromino to lock
      if(y) {
	if(game_tetromino_move(0, y, 0)) {
	  game.step_cnt = game_level_rate();
	  game.cont_drop++;
	} else 
	  game_tetromino_locked();
      }
      
      // advance tetromino by gravity
      if((game.tetromino.type != 0xff) && (!--game.step_cnt)) {
	if(!game_tetromino_move(0, -1, 0)) 
	  game_tetromino_locked();
	else
	  // clear "continous drop counter" if the tetromino drops by gravity
	  game.cont_drop = 0;
	
	game.step_cnt = game_level_rate();
      }
    }
  }

  // blit game_area to screen
  game_area_blit();
    
  // check if user just pressed pause key 
  if(keys & KEY_PAUSE) {
    // go into pause state and save game
    // state to eeprom
    game_pause(1);
      
    // pause key has just been pressed
    state = STATE_PAUSED;
    keys_lock();
  }

  game_draw_score();

  return 0;
}

unsigned long next_event;

void setup() {
  //  Serial.begin(9600);
  //  Serial.println("Tetris");

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
    // check if there's a high score in eeprom but no name. In that case
    // ask the user to set a name. This is for boards that previously had
    // a firmware that wouldn't let the user set a name
    if((EEPROM.read(0) == 0x42) && (EEPROM.read(20) != 0x42)) {
      uint32_t hi;
      EEPROM.get(1, hi);
      initials_init(hi);
      state = STATE_INITIALS;
    } else {
      // normal startup
      title_init();
      state = STATE_TITLE;
    }

    // check for a save state
    if(EEPROM.read(40) == 0x42) {
      game_init();

      // read game state from eeprom
      EEPROM.get(41, game);
      
      // update preview and level as loaded from save state
      game_tetromino_preview_draw(game.tetromino.next);
      game_show_level();

      // draw game area once      
      game_area_blit();
      game_pause(0);

      // and go into paused state
      state = STATE_PAUSED;
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
      if(title_process(keys)) {
	game_init();
	state = STATE_GAME;
      }
      break;

    case STATE_PAUSED:
      // score still scrolls
      game_draw_score();
      
      // fire key unpauses
      if(keys & KEY_ROTATE) {
	EEPROM.write(40, 0x00);   // clear magic save state marker
	state = STATE_GAME;
      }
      break;
      
    case STATE_GAME:
      song_process(game.level+1);
      if(game_process(keys)) {
	if(game.score > hi_score) {
	  initials_init(game.score);
	  state = STATE_INITIALS;
	} else {
	  score_init(game.score, game.score > hi_score);
	  state = STATE_SCORE;
	}
        song_process(0);
      }
      break;

    case STATE_SCORE:
      switch(score_process(keys)) {
      case 1:
	// user pressed a key -> jump directly into
	// next game
	game_init();
	state = STATE_GAME;
	break;

      case 2:
	// timeout, jump to title screen
	title_init();
	state = STATE_TITLE;
	break;
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
