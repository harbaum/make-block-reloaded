// fake
#ifndef FASTLED_H
#define  FASTLED_H

#define MAX_BRIGHT  80   // assume client app never goes over this

#include <SDL.h>
#include <stdint.h>

extern "C" int usleep(__useconds_t __useconds);

#include "FastLED/pixeltypes.h"

void delay(int ms) { usleep(1000*ms); }

SDL_Surface* screen = NULL;

uint8_t gkey = 0;

#define W 10
#define H 15
#define SCALE 25

#define WS2812B 0

#define WS  (SCALE*W)
#define HS  (SCALE*H)

static CRGB *m_data = NULL;
static int m_size;
static int m_scale;

class CFastLED {
public:
  CFastLED() { 
  };

  ~CFastLED() {
    /* cleanup SDL */
    SDL_Quit();
  };

  static int addLeds(void *pLed, struct CRGB *data, int nLedsOrOffset, int nLedsIfOffset = 0);

  template<int CHIPSET,  uint8_t DATA_PIN, uint8_t CLOCK_PIN > static int &addLeds(struct CRGB *data, int nLedsOrOffset, int nLedsIfOffset = 0) { 
    m_data = data; m_size = nLedsOrOffset; 
    for(int i=0; i<nLedsOrOffset; i++) 
      data[i] = CRGB::Black; 

    if(nLedsOrOffset > W*H) {
      printf("too many leds\n");
      exit(-1);
    }

    m_scale = 100; 
    // create window
    
    /* initialize SDL */
    SDL_Init(SDL_INIT_VIDEO);
    
    /* set the title bar */
    SDL_WM_SetCaption("Makeblock", "Makeblock");
    
    /* create window */
    screen = SDL_SetVideoMode(WS, HS, 0, 0);

    
  };

  void setBrightness(uint8_t scale) { 
    m_scale = 100 * scale / MAX_BRIGHT; 
    if(m_scale > 100) m_scale = 100; 
  }

  /// Update all our controllers with the current led colors
  void show() { 
    SDL_Rect r;

    // draw all pixels
    r.x = 0;
    r.y = 0;
    r.w = 8*SCALE/10;
    r.h = 8*SCALE/10;

    for(int y=0;y<H;y++) {
      for(int x=0;x<W;x++) {
	CRGB c;
	if(x*H+y < m_size) c = m_data[x*H+y];
	else               c = CRGB::Black;
	r.x = SCALE*x + SCALE/10;
	r.y = SCALE*y + SCALE/10;
	SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 
		    m_scale*c.g/100, m_scale*c.r/100, m_scale*c.b/100));
      }
    }

    /* update the screen */
    SDL_UpdateRect(screen, 0, 0, 0, 0);

    // painting all leds takes some time (24*1.25=30us per led)
    usleep(30*m_size);

    SDL_Event event;
    while( SDL_PollEvent( &event ) ){
      /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
      switch( event.type ){
      case SDL_KEYDOWN:
	if(event.key.keysym.sym == SDLK_LEFT)   gkey |= 1;
	if(event.key.keysym.sym == SDLK_RIGHT)  gkey |= 2;
	if(event.key.keysym.sym == SDLK_UP)     gkey |= 4;
	if(event.key.keysym.sym == SDLK_DOWN)   gkey |= 8;
	if(event.key.keysym.sym == SDLK_ESCAPE) exit(-1);
        break;
	
      case SDL_KEYUP:
	if(event.key.keysym.sym == SDLK_LEFT)   gkey &= ~1;
	if(event.key.keysym.sym == SDLK_RIGHT)  gkey &= ~2;
	if(event.key.keysym.sym == SDLK_UP)     gkey &= ~4;
	if(event.key.keysym.sym == SDLK_DOWN)   gkey &= ~8;
        break;

      case SDL_QUIT:
	exit(-1);

      default:
        break;
      }
    }  
  }
  
  void clear() { for(int i=0; i<m_size; i++) m_data[i] = CRGB::Black; show(); }
};

CFastLED LEDS;
CFastLED &FastLED = LEDS;

#endif
