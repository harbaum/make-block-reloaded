#include <SDL/SDL_audio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <cmath>
#include <assert.h>

#include "makeblock.h"
#include "FastLED.h"

// required for compatibility with FastLED 3.0
#ifndef LINEARBLEND
#define LINEARBLEND  BLEND
#endif

class Beeper {
private:
  double phase;
  double frequency;
public:
  Beeper();
  ~Beeper();
  void beep(double toneFrequency);
  void generateSamples(Uint8 *stream, int length);
};

void audio_callback(void*, Uint8*, int);

Beeper::Beeper() {
  SDL_AudioSpec desiredSpec;

  desiredSpec.freq = 8000;
  desiredSpec.format = AUDIO_U8;
  desiredSpec.channels = 1;
  desiredSpec.samples = 128;     // small buffer for low delay
  desiredSpec.callback = audio_callback;
  desiredSpec.userdata = this;

  // open audio
  if(SDL_OpenAudio(&desiredSpec, NULL) < 0) {
    printf("Failed to open audio\n");
    return;
  }

  // immediately start producing audio.
  SDL_PauseAudio(0);
}

Beeper::~Beeper() {
  SDL_CloseAudio();
}

void Beeper::generateSamples(Uint8 *stream, int length) {
  if (frequency == 0) {
    // silence
    phase = 0;
    memset(stream, 128, length);
  } else {
    // generate/continue sinus wave
    for(int i=0;i<length;i++) {
      stream[i] = 128 + (int)(127 * std::sin(phase * 2 * M_PI / 8000));
      phase += frequency;
    }
  }
}

void Beeper::beep(double toneFrequency) {
  SDL_LockAudio();
  frequency = toneFrequency;
  SDL_UnlockAudio();
}

void audio_callback(void *beeper, Uint8 *stream, int length) {
  ((Beeper*)beeper)->generateSamples(stream, length);
}

static bool audio_is_enabled = false;
void audio_on(bool on) {
  audio_is_enabled = on;
}

Beeper *b = NULL;
void audio_init() {
  b = new Beeper();
}

#define F_CPU 16000000

void audio_set(uint16_t ocr) {
  if((ocr == 128) || !audio_is_enabled) {
    b->beep(0); 
    return;
  }

  if(ocr == 129) ocr = 119;  // Hack for C6
  
  b->beep(F_CPU/(2 * AUDIO_PRESCALER  * (ocr + 0.5))); 
}

unsigned long millis() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_usec/1000)+(tv.tv_sec*1000ll);
}

unsigned long micros() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec;
}

char * ltoa(int32_t val, char * s, int radix) { 
  sprintf(s, "%d", val); return s; 
};

#define LOW 0
#define HIGH 1
int key_setup = 0;

int digitalRead(int i) { 
  if((i == KEY_ROTATE_PIN) && (key_setup > 0)) {
    key_setup--;
    return 0;
  }
  
  if(i == KEY_LEFT_PIN)   return !(gkey & 1);
  if(i == KEY_RIGHT_PIN)  return !(gkey & 2);
  if(i == KEY_ROTATE_PIN) return !(gkey & 4);
  if(i == KEY_DOWN_PIN)   return !(gkey & 8);
  if(i == KEY_DROP_PIN)   return !(gkey & 16);
  return 1; 
}

void digitalWrite(int, int) { }

void randomSeed(int a) { srandom(a); };
long int random(int min, int max) { return min+(random()%(max-min)); }
#define INPUT_PULLUP 0
#define OUTPUT       1
void pinMode(int , int) { };

extern CRGB leds[NUM_LEDS];

extern const TProgmemRGBPalette16 RainbowColors_p PROGMEM = {
    0xFF0000, 0xD52A00, 0xAB5500, 0xAB7F00,
    0xABAB00, 0x56D500, 0x00FF00, 0x00D52A,
    0x00AB55, 0x0056AA, 0x0000FF, 0x2A00D5,
    0x5500AB, 0x7F0081, 0xAB0055, 0xD5002B
};

extern state_t state;
extern void game_pause(int8_t store);

extern const struct mario_levelS level_1_1;

#include "FastLED/colorutils.cpp"
#include "FastLED/hsv2rgb.cpp"

#include "text.ino"
#include "keys.ino"
#include "logo.ino"

#include "song.ino"
#include "config.ino"
#include "initials.ino"
#include "title.ino"
#include "score.ino"
#include "tetris.ino"
#include "mario.ino"
#include "mario_lvl.ino"
#include "make-block-reloaded.ino"

int main(int argc, char **argv) {
  if(argc != 1)
    key_setup = 100;

  setup();

  while(1)
    loop();

  return 0;
}
