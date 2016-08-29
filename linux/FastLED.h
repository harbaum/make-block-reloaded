// fake
#ifndef FASTLED_H
#define FASTLED_H

#include <SDL.h>
#include <stdint.h>

#define ENABLE_POWER_VIEW 1

// height of power display area
#if ENABLE_POWER_VIEW == 1
#define POWER_H   100
#else
#define POWER_H   0
#endif

extern "C" int usleep(__useconds_t __useconds);

// include the required parts of FastLED
#include "FastLED/pixeltypes.h"
#include "FastLED/colorutils.h"
#include "FastLED/colorpalettes.h"

void delay(int ms) { usleep(1000*ms); }

SDL_Surface* screen = NULL;
SDL_Surface* power = NULL;

uint8_t gkey = 0;

#define W 15
#define H 20
#define SCALE 20

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
    screen = SDL_SetVideoMode(WS, HS+POWER_H, 0, 0);

#if ENABLE_POWER_VIEW == 1
    // erase power area
    SDL_Rect r;
    r.y = r.x = 0; r.w = WS; r.h = POWER_H;
    SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 255, 255));
#endif

    delay(500);
  };

  void setBrightness(uint8_t scale) { 
    m_scale = scale;
  }

  /// Update all our controllers with the current led colors
  void show() { 
    SDL_Rect r,s;

    // draw all pixels
    r.x = 0;
    r.y = 0;
    r.w = 8*SCALE/10;
    r.h = 8*SCALE/10;

    double current = 0;

    for(int y=0;y<H;y++) {
      for(int x=0;x<W;x++) {
#ifndef LED_SERPENTINE
	CRGB c = m_data[x*H+y];
#else
	// simulate the serpentine mapping
	int y_ser = (x&1)?(H-y-1):y;
	CRGB c = m_data[x*H+y_ser];
#endif
	r.x = SCALE*x + SCALE/10;
	r.y = POWER_H + SCALE*y + SCALE/10;
	SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 
		    m_scale*c.r/255, m_scale*c.g/255, m_scale*c.b/255));

	// current estimation
	// asumming a max of 60mA per LED (20mA per color)
	current += m_scale * c.r * (20.0 / 255.0 / 255.0);
	current += m_scale * c.g * (20.0 / 255.0 / 255.0);
	current += m_scale * c.b * (20.0 / 255.0 / 255.0);
      }
    }

    static double max_current = 0;
    if(current > max_current) {
      char str[32];
      max_current = current;
      sprintf(str, "max %.2fA/%.1fW", current/1000, current/200);
      SDL_WM_SetCaption(str, str);
    }

#if ENABLE_POWER_VIEW == 1
    // scroll one pixel to the left
    s.x = 1; s.y=0; s.w = WS-1; s.h = POWER_H;
    r.x = 0; r.y=0; r.w = WS-1; r.h = POWER_H;
    SDL_BlitSurface(screen, &s, screen, &r);

    // erase rightmost column
    r.y = 0; r.x = WS-1; r.w = 1; r.h = POWER_H;
    SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 255, 255));

    // draw 5A bars
    int ph = (5 * POWER_H) / 18;
    r.y = POWER_H-ph; r.h = ph;
    SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 230, 230, 230));
    r.y = POWER_H-3*ph;
    SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 230, 230, 230));

    // max current = 300*0.06A = 18A
    ph = (current/1000 * POWER_H) / 18;
    r.y = POWER_H-ph; r.h = ph;
    SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 0, 0));
#endif

    /* update the screen */
    SDL_UpdateRect(screen, 0, 0, 0, 0);

    // painting all leds takes some time (24*1.25=30us per led)
    usleep(30*m_size);  // total delay is ~9ms for 300 leds

    SDL_Event event;
    while( SDL_PollEvent( &event ) ){
      /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
      switch( event.type ){
      case SDL_KEYDOWN:
	if(event.key.keysym.sym == SDLK_LEFT)   gkey |= 1;
	if(event.key.keysym.sym == SDLK_RIGHT)  gkey |= 2;
	if(event.key.keysym.sym == SDLK_RSHIFT) gkey |= 4;
	if(event.key.keysym.sym == SDLK_DOWN)   gkey |= 8;
	if(event.key.keysym.sym == SDLK_UP)     gkey |= 16;
	if(event.key.keysym.sym == SDLK_ESCAPE) exit(-1);
        break;
	
      case SDL_KEYUP:
	if(event.key.keysym.sym == SDLK_LEFT)   gkey &= ~1;
	if(event.key.keysym.sym == SDLK_RIGHT)  gkey &= ~2;
	if(event.key.keysym.sym == SDLK_RSHIFT) gkey &= ~4;
	if(event.key.keysym.sym == SDLK_DOWN)   gkey &= ~8;
	if(event.key.keysym.sym == SDLK_UP)     gkey &= ~16;
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
