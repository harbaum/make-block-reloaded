/* 
   config.ino
*/

#include <FastLED.h>
#include <EEPROM.h>
#include "tetris.h"

#define MIN_BRIGHTNESS  16   // too dark would be bad as config itself becomes invisible
#define MAX_BRIGHTNESS  255

uint8_t config_brightness;
bool config_audio;
bool config_key_released;

void config_load() {
  // load initial value from eeprom if availble
  if(EEPROM.read(10) == 0x42) {
    EEPROM.get(11, config_brightness);
    EEPROM.get(12, config_audio);
  } else {
    config_brightness = 128;  // default brightness
    config_audio = true;      // default audio is on
  }

  audio_on(config_audio);
}

static const uint8_t audio_on_icon[] PROGMEM = {
  0x0c, 0x0c, 0x1e, 0x3f, 0x3f, 0x00, 0x12, 0x0c, 0x21, 0x1e
};

static const uint8_t audio_off_icon[] PROGMEM = {
  0x0c, 0x0c, 0x1e, 0x3f, 0x3f
};

static const uint8_t brightness_icon[] PROGMEM = {
  0x08, 0x1c, 0x00, 0x08, 0x2a, 0x1c, 0x7f, 0x1c, 0x2a, 0x08, 0x00, 0x1c, 0x08
};

static const uint8_t ok_icon[] PROGMEM = {
  0x0e, 0x11, 0x11, 0x11, 0x0e, 0x00, 0x1f, 0x04, 0x0a, 0x11
};

uint8_t config_get_brightness() {
  return config_brightness;
}

void config_draw_icon(uint8_t x, uint8_t y, const uint8_t *d, uint8_t cnt, 
		      uint8_t h, uint8_t sel, int8_t bar) {
  uint8_t i, j;

  for(i=0;i<W;i++) {
    uint8_t pat = 0;
    if((i >= x) && (i < x+cnt))
      pat = pgm_read_byte(d+i-x);

    for(j=0;j<h;j++) {
      // background is dark grey or black (if icon is selected or not)
      CRGB pix = sel?0x202020:CRGB::Black;

      // the background acts as a "fill" bar
      if((bar >= 0) && (i <= bar))
	pix = sel?0x004000:CRGB::Black;

      if(pat & (1<<j)) pix = sel?CRGB::White:0x808080;
      LED(i,y+j) = pix;
    }
  }
}

static uint8_t config_entry = 2;

void config_draw_menu(uint8_t sel) {

  if(config_audio)
    config_draw_icon((W-sizeof(audio_on_icon))/2,14,
		     audio_on_icon,sizeof(audio_on_icon),6, sel == 0, -1);
  else
    config_draw_icon((W-sizeof(audio_off_icon))/2,14,
		     audio_off_icon,sizeof(audio_off_icon),6, sel == 0, -1);

  config_draw_icon((W-sizeof(brightness_icon))/2,6,
		   brightness_icon,sizeof(brightness_icon),7, sel==1, 
		   ((uint16_t)config_brightness * 14) / 255);

  config_draw_icon((W-sizeof(ok_icon))/2,0,
		   ok_icon,sizeof(ok_icon),5, sel == 2, -1);
}

void config_init() {
  LEDS.clear();
  LEDS.setBrightness(config_brightness);

  config_draw_menu(config_entry);
  config_key_released = false;
}

uint8_t config_process(uint8_t keys) {
  if(!keys_any_down())
    config_key_released = true;

  if(config_key_released) {
  
    if((keys & KEY_DROP) && (config_entry > 0)) {
      config_entry--;
      keys_lock();      // prevent auto repeat
    }
    
    if((keys & KEY_DOWN) && (config_entry < 2)) {
      config_entry++;
      keys_lock();      // prevent auto repeat
    }
    
    // handle menu entry
    switch(config_entry) {
    case 0:
      if(keys & KEY_ROTATE) 
	config_audio = !config_audio;
      break;
      
    case 1:
      // brightness
      if((keys & KEY_LEFT) && (config_brightness > MIN_BRIGHTNESS))
	LEDS.setBrightness(--config_brightness);
      
      if((keys & KEY_RIGHT) && (config_brightness < MAX_BRIGHTNESS))
	LEDS.setBrightness(++config_brightness);
      break;
      
    case 2:
      // "OK" -> start game
      if(keys & KEY_ROTATE) {
	audio_on(config_audio);

	// save config in eeprom
	EEPROM.write(10, 0x42);   // write magic marker
	EEPROM.put(11, config_brightness);
	EEPROM.put(12, config_audio);

	return 1;
      }
      break;
    }
    
    if(keys)
      config_draw_menu(config_entry);
  }
    
  return 0;
}
