/* 
   tetris.ino
   
   Real tetris for the 15*20 make:block
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
typedef enum { STATE_CONFIG, STATE_TITLE, 
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
static const uint32_t tetromino_colors[] = {
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

// two blocks are saved per byte -> 90 bytes
uint8_t game_area[GAME_W/2][GAME_H];

struct tetromino_S {
  uint8_t x, y;
  uint8_t rot, type, next;
} tetromino;

uint8_t game_step_cnt;
uint8_t game_level;
uint32_t game_score;
uint16_t game_lines;
uint8_t game_cont_drop;
uint32_t hi_score;

uint32_t row_remove;
uint8_t row_remove_timer;

uint8_t game_level_rate() {
  // speed table. taken from gameboy version. values in 60Hz steps
  static const uint8_t step_cnt_table[] PROGMEM = 
    // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
    { 53,49,45,41,37,33,28,22,17,11,10, 9, 8, 7, 6, 6, 5, 5, 4, 4, 3 }; 

  return pgm_read_byte(step_cnt_table+((game_level<20)?game_level:20));
}

// show level
// level: 0-9 = green digit 0-9, 10-19 = yellow digit 0-9, >= 20 = red X
void game_show_level() {
  rect(LEVEL_X, LEVEL_Y, 3, 5, CRGB::Black);
  if(game_level < 10)
    text_draw_char('0'+game_level, LEVEL_X, LEVEL_Y, 0, 3, CRGB(0x00ff00));
  else if(game_level < 20)
    text_draw_char('0'+game_level-10, LEVEL_X, LEVEL_Y, 0, 3, CRGB(0xffff00));
  else
    text_draw_char('X', LEVEL_X, LEVEL_Y, 0, 3, CRGB(0xff0000));
}

void game_tetromino_set_block(uint8_t x, uint8_t y, uint8_t col) {
  // we should never draw outside the game area
  if((x >= GAME_W) || (y >= GAME_H))
    return;

  if(x&1) game_area[x/2][y] = (game_area[x/2][y] & 0x0f) | (col<<4);
  else    game_area[x/2][y] = (game_area[x/2][y] & 0xf0) | (col&0x0f);
}

uint8_t game_tetromino_get_block(uint8_t x, uint8_t y) {
  // return 1 for all blocks outside game area so these seem to be
  // "occupied" to the game engine. Return 0 for one row above 
  // game area to ease implementation of scrolling down
  if((x >= GAME_W)||(y > GAME_H)) return 1;
  if(y == GAME_H) return 0;

  if(x&1) return game_area[x/2][y]>>4;
  return game_area[x/2][y]&0x0f;
}

void game_tetromino_draw(char show) {
  // get pointer to current tetromino shape at current angle
  int8_t const (*p)[2] = tetrominos[tetromino.type][tetromino.rot];

  // set all four blocks a tetromino consists of
  for(uint8_t i=0;i<4;i++) 
    game_tetromino_set_block(tetromino.x + pgm_read_byte(&p[i][0]), 
			     tetromino.y - pgm_read_byte(&p[i][1]), 
			     show?tetromino.type+1:0);
}

char game_tetromino_ok() {
  // get pointer to current tetromino shape at current angle
  int8_t const (*p)[2] = tetrominos[tetromino.type][tetromino.rot];
 
  // check all four blocks a tetromino consists of
  for(uint8_t i=0;i<4;i++) 
    if(game_tetromino_get_block(tetromino.x + pgm_read_byte(&p[i][0]), 
				tetromino.y - pgm_read_byte(&p[i][1])))
      return false;
  
  return true;
}

void game_tetromino_new() {
  tetromino.rot = 0;
  tetromino.type = tetromino.next;
  tetromino.next = random(0,7);
  tetromino.x = 4;

  // on gameboy the tetrominos spawn one row below top 
  // game area ...
  tetromino.y = GAME_H-2;

  // erase 4*2 preview area
  for(uint8_t y=0;y<2;y++)
    for(uint8_t x=0;x<4;x++)
      LED(PREVIEW_X+x-1, PREVIEW_Y-y) =
	CRGB::Black;

  // check if new tetromino can be placed on screen
  if(game_tetromino_ok()) {
    game_tetromino_draw(true);

    // and show next tetromino
    int8_t const (*p)[2] = tetrominos[tetromino.next][0];
    for(uint8_t i=0;i<4;i++)
      LED(PREVIEW_X+(int8_t)pgm_read_byte(&p[i][0]),
	  (PREVIEW_Y-(int8_t)pgm_read_byte(&p[i][1])))
	= CRGB(tetromino_colors[tetromino.next+1]);
  } else {
    // set current tetromino to 0xff indicating that
    // no game is in progress anymore
    tetromino.type = 0xff;
    row_remove_timer = 0;

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
  tetromino.x += x;
  tetromino.y += y;
  tetromino.rot = (tetromino.rot+rot)&3;

  // and check if it could be drawn
  if(!game_tetromino_ok()) {
    // restore old position
    tetromino.x -= x;
    tetromino.y -= y;
    tetromino.rot = (tetromino.rot-rot)&3;
    ret = false;
  }
    
  game_tetromino_draw(true);
  return ret;
}

void game_tetromino_locked() {
  // lock keys so the have to be released before auto repeat kicks in again
  keys_lock();

  // any manual drop before placement gives one extra point
  if(game_cont_drop) {
    game_score += game_cont_drop;
    game_cont_drop = 0;
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
      row_remove |= (1<<y);
    
      // line clear take 90 frames according to 
      // http://tetrisconcept.net/wiki/Tetris_%28Game_Boy%29
      row_remove_timer = 90;
    }
  }

  // no row removed: spawn new tetromino immediately
  if(!row_remove)
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
      game_area[x][y] = 0;
 
  row_remove = 0;  // no row being removed
  game_level = INIT_LEVEL;
  game_lines = 0;
  game_score = 0;
  game_cont_drop = 0;
  game_step_cnt = game_level_rate();

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
  tetromino.next = random(0,7);
  tetromino.next = random(0,7);
  game_tetromino_new();
}

#define PULSE_STEPS  60

void game_draw_score() {
  // draw score while game is running 
  if(tetromino.type != 0xff) {
    static uint8_t pulse_cnt;
    static uint32_t cur_score = 0;
    static char score_str[7] = "0";
    static uint8_t score_len = 3;
    CRGB color = CRGB::White;

    // let score "pulse" if hi score was exceeded
    if(game_score <= hi_score) {
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
    if(game_score != cur_score) {
      ltoa(game_score, score_str, 10);
      cur_score = game_score;
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

uint8_t game_process(uint8_t keys) {
  static const uint8_t score_step[] = { 4, 10, 30, 120 };

  // type == 0xff means game has ended
  if(tetromino.type == 0xff) {
    if(row_remove_timer <= GAME_H) {
      for(uint8_t x=0;x < GAME_W;x++) {
	game_tetromino_set_block(x, row_remove_timer-1, 9);
	game_tetromino_set_block(x, row_remove_timer, 8);
      }
      row_remove_timer++;
    } else {
      // game area is closed ..

      // update high score if necessary
      if(game_score > hi_score)
	EEPROM.put(1, game_score); // write new high score

      return 1;
    }
  } else if(row_remove) {
    // row removal is in progress
    game_cont_drop = 0;
    row_remove_timer--;

    if(!row_remove_timer) {
      uint8_t removed = 0;
      // finally remove the full rows

      for(uint8_t y=0;y<GAME_H;y++) { 
	if(row_remove & (1<<y)) {
	  uint8_t k=y;
	  // shift all lines above down one line
	  while(k < GAME_H) {
	    for(uint8_t x=0;x<GAME_W;x++)
	      game_tetromino_set_block(x, k,
	       game_tetromino_get_block(x, k+1));
	    k++;
	  }

	  removed++;
	  game_lines++;
	  if((game_lines % 10) == 0) {
	    game_level++;
	    game_show_level();
	  }

	  // also shift table of full rows down
	  row_remove = (row_remove & ~(1<<y))>>1;
	  y--;  // check same row again
	}
      }

      // update score
      game_score += 10l * score_step[removed-1] * (game_level+1);

      // limit score to 999999
      if(game_score > 999999)
        game_score = 999999;

      row_remove = 0;
      game_tetromino_new();
    }
  } else {
    if(keys & KEY_PAUSE) {
      keys &= ~(KEY_DOWN | KEY_DROP);
      keys_lock();
    }
  
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
	tetromino.y--;
	// twice the soft drop score for this
	game_cont_drop+=2;
      } while(game_tetromino_ok());

      // move up one again
      tetromino.y++;
      game_cont_drop-=2;
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
	  game_step_cnt = game_level_rate();
	  game_cont_drop++;
	} else 
	  game_tetromino_locked();
      }
      
      // advance tetromino by gravity
      if((tetromino.type != 0xff) && (!--game_step_cnt)) {
	if(!game_tetromino_move(0, -1, 0)) 
	  game_tetromino_locked();
	else
	  // clear "continous drop counter" if the tetromino drops by gravity
	  game_cont_drop = 0;
	
	game_step_cnt = game_level_rate();
      }
    }
  }

  // blit game_area to screen
  for(uint8_t y=0;y<GAME_H;y++) { 
    if((row_remove & (1<<y)) && (row_remove_timer & 16))
      for(uint8_t x=0;x<GAME_W;x++)
	LED(x+GAME_X,GAME_Y+y) = CRGB::White;
    else
      for(uint8_t x=0;x<GAME_W;x++) 
	LED(x+GAME_X,GAME_Y+y) =
	  CRGB(tetromino_colors[game_tetromino_get_block(x, y)]);
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
      
    case STATE_GAME:
      song_process(game_level+1);
      if(game_process(keys)) {
	if(game_score > hi_score) {
	  initials_init(game_score);
	  state = STATE_INITIALS;
	} else {
	  score_init(game_score, game_score > hi_score);
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
