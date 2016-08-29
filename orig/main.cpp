/*
  makeblock linux wrapper

  run the makeblock sketch under Linux
*/

#include <stdio.h>
#include <stdlib.h>
#include <WString.h>
#include "FastLED.h"

typedef char boolean;
typedef unsigned char byte;
enum { B000=0, B001, B010, B011, B100, B101, B110, B111 };

#define LOW 0
#define HIGH 1
int digitalRead(int i) { 
  if(i == 8) return !(gkey & 8);    // Down
  if(i == 9) return !(gkey & 1);    // Rot
  if(i == 10) return !(gkey & 2);   // Right
  if(i == 11) return !(gkey & 4);   // Left
  return 1; 
}

int analogRead(int i) { return 0; }
long int random(int min, int max) { return min+(random()%(max-min)); }
#define INPUT_PULLUP 0
void pinMode(int , int) { };
void randomSeed(int) { };

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

char * utoa(unsigned int val, char * s, int radix) { printf("utoa\n"); };
char * ultoa(unsigned long val, char * s, int radix) { printf("ultoa\n"); };
char * itoa(int val, char * s, int radix) { sprintf(s, "%d", val); return s; };
char * ltoa(long val, char * s, int radix) { printf("ltoa\n"); };

// arduino adds declarations automagically
void write_Highscore_to_EEPROM( int value);
int read_Highscore_from_EEPROM();
void Zeile_loeschen(int zeile,byte color);
void show_score();
void fade_out();
void show_game_over();
void show_highscore();
boolean move_brick();
boolean Kollision(int Spalte, int Zeile);
void new_game();
byte get_key();
void move_down();
void move_left();
void move_right();
void rotate();
void check_score();
void reihe_voll();
void draw_Brick(int Spalte, int Zeile, byte Farbe );
void display_matrix();
boolean Is_Game_Over();
void new_Brick();
void get_brick_measurement();
void del_old_Brick();

void Lauftext_von_oben(String txt, int x,int pause,int r, int g, int b);
void Lauftext_blenden(String txt, int x, int y, int warten, int r, int g, int b);
void Lauftext_von_unten(String txt, int x,int pause,int r, int g, int b);
void Lauftext_von_rechts(String txt, int y, int bremse, int r, int g, int b);
void ZeichneBuchstaben(char ch, int x, int y, int r, int g, int b);

#include "MAKEBlOCK_WS2812B.ino"
#include "ASCII.ino"
#include "WString.cpp"

int main() {
  setup();

  while(1)
    loop();

  return 0;
}
