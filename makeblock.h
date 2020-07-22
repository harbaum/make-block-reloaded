//
// makeblock.h
//

#ifndef MAKEBLOCK_H
#define MAKEBLOCK_H

// possible game states
typedef enum { STATE_CONFIG, STATE_TITLE, STATE_PAUSED,
	       STATE_GAME, STATE_SCORE, STATE_INITIALS } state_t;

// enable LED_SERPENTINE if the stripe wiring used
// alternating directions
// #define LED_SERPENTINE

#ifndef LED_SERPENTINE
#define LED(x,y)  leds[H-(y)-1+H*(x)]
#else
#define LED(x,y)  leds[H-((x&1)?(H-(y)-1):(y))-1 + H*(x)]
#endif

// total display size
#define H 20
#define W 15

#define FPS      60    // 60Hz

#define GAME_CYCLE (1000/FPS)

#define NUM_LEDS (W*H)

#define TITLE_SCROLL_SPEED      2
#define GAME_SCORE_SCROLL_SPEED 5   // frames@60Hz

// pin mapping
#define KEY_DROP_PIN     7    // should be connected to "up"
#define KEY_LEFT_PIN     5 
#define KEY_RIGHT_PIN    6

#define LED_DATA_PIN    12

#define LED_PIN         13    // arduino on board led

#define AUDIO_PRESCALER  64LU
#define KEY_DOWN_PIN     8
#define KEY_ROTATE_PIN  11    // should be connected to "fire"
#define SPEAKER_PIN      9
#define SPEAKER_PIN_2   10    // second channel option

#define NO_DROP              // use drop key as second rotate

#define KEY_LEFT    0x01
#define KEY_RIGHT   0x02
#define KEY_ROTATE  0x04
#define KEY_DOWN    0x08
#define KEY_DROP    0x10
#define KEY_PAUSE   0x20    // generated from key combo

// ------------------- the entire in-game state ------------

#define GAME_TETRIS 0
#define GAME_MARIO  1

// title state
struct titleS {
  // variables required during the title
  int16_t score_len, scroll;
  int8_t logo_fade;
  uint32_t timer;      // to switch to plasma after some time ...
  // msg + max 15 chars user name
  char score_msg[40];  
};

// tetris constants

// standard tetris size as seen on gameboy

#define GAME_X   0
#define GAME_Y   1
#define GAME_W  10
#define GAME_H  18

// where pause logo appears in tetris
#define TETRIS_PAUSE_X GAME_X+2
#define TETRIS_PAUSE_Y GAME_Y+6

// tetris game state
struct tetrisS {
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
};

struct mario_levelS {
  uint8_t width;
  uint8_t flag;
  uint8_t castle;
  const uint8_t *gaps;
  const uint8_t (*clouds)[3];
  const uint8_t (*hills)[2];
  const uint8_t (*walls)[3];
  const uint8_t (*qmarks)[2];
  const uint8_t (*stairs)[2];
  const uint8_t (*pipes)[2];
  const uint8_t (*enemies)[4];
};

#define MARIO_MAX_ENEMY 4

// mario constants
struct marioS {
  uint8_t subcycle;

  int8_t speed;
  union {
    uint16_t w;
    struct { uint8_t l, h; };
  } x,y;

  uint8_t scroll;
  uint8_t jump, jump_press;

  struct {
    uint8_t x, y;
    uint8_t timer;
  } coin;

  struct {
    uint8_t x, y;
    uint8_t timer;
    uint8_t movement;
  } mushroom;

  struct {
    uint8_t x, y;
    uint8_t flags;
    uint8_t min, max;
  } enemy[MARIO_MAX_ENEMY];
  
  // blocks on current screen. Used for collision checks for mario and
  // the moving mushrooms. Enemies can actually move outside the visible
  // area and have their own collision system
  uint16_t block[W];

  uint32_t qmarks_used;
  uint32_t enemies_activated;

  uint8_t coins;
  uint16_t timer;
  uint32_t score;

  int8_t super;
  
  struct {
    uint8_t state;
    uint8_t cnt;
  } ending;
  
  const struct mario_levelS *level;
};

// where pause logo appears in mario
#define MARIO_PAUSE_X 4
#define MARIO_PAUSE_Y 4

// overall state
struct gameS {
  uint8_t game;   // 0 = tetris, 1 = mario
  union {
    struct titleS title;
    struct tetrisS tetris;
    struct marioS mario;
  };
};

extern struct gameS game;

#endif // MAKEBLOCK_H
