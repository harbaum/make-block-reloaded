//
// tetris.h
//

#ifndef TETRIS_H
#define TETRIS_H

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

#endif // TETRIS_H
