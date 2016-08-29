/////////////////////////////////////////////////////////////////////////7
//              ct'tres                                      /////////////
//              Ulrich Schmerold, 2014                       /////////////
//              Projekt für c't Hacks                        /////////////
//              Realisiert mit WS2812B Strips                /////////////
//////////////////////////////////////////////////////////////////////////

#include <EEPROM.h>           // für die Speicherung des Highscors
#include "FastLED.h"          // Bibliothek von GitHub für den WS2812B

#define DATA_PIN    12        // Datenaeingang des WS2812 am Arduino Port
#define KEY_LEFT    11        // links
#define KEY_RIGHT   10        // rechts
#define KEY_DOWN    8         // herabfallen
#define KEY_ROTATE  9         // drehen

#define field_width 10                      //Breite der LED Matrix
#define field_height 15                     //Höhe der LED Matrix
#define NUM_LEDS field_width*field_height   //Gesammtanzahl der LED's
#define Brightness 80                      //Helligkeit der Matrix

CRGB leds[NUM_LEDS];        // Arrray der LED'S

byte LED_Matrix[NUM_LEDS]; // LED-Feld, enthällt die Farbnummer(0-9)

int pos_x=4;        //Variable für die aktuelle Spalte des Bricks
int pos_y=1;        //Variable für die aktuelle Zeile des Bricks
byte current_color; //Variable für die aktuelle Farbe des Bricks

CRGB COLOR[10] =    // Auswahl der wichtigsten Farben definieren g, r, b,
  {           
        CRGB( 0  ,   0,   0),  // 0 = schwarz
        CRGB( 0  , 255,   0),  // 1 = rot
	CRGB( 255,   0,   0),  // 2 = grün
	CRGB( 0  ,   0, 255),  // 3 = blau
	CRGB( 255, 220,   0),  // 4 = gelb
	CRGB(   146, 66, 157), // 5 = hellblau
	CRGB( 120,  230, 23),  // 6 = orange 
	CRGB( 19,  221, 123),  // 7 = magenta
	CRGB( 0,  90, 255),    // 8 = violett
        CRGB( 255, 255, 255)   // 9 = weiß
  };

byte Brick[][3]={          // Bricks definieren
  {B001,B111,B000},
  {B010,B111,B000},
  {B011,B011,B000},
  {B000,B011,B000},        
  {B000,B111,B000},
  {B011,B010,B000},
  {B100,B111,B000},
  {B011,B110,B000},
  {B110,B011,B000}
  };
  
byte current_brick[3];          // der aktuell fallende Brick 
byte last_Brick[9];             // LED_Matrix und Farbe des letzten Bricks
unsigned long ticks = 0;        // Variable für Zeitsteuerung
boolean game_over=true;         // Variable für Time Over
byte brickMeasurement[3];       // Array für die Grenzen eines Bricks
boolean update_needed = true;   // Soll die Matrix neu "gezeichnet" werden? 
byte key=0;                     // Variable für die gedrückte Taste 
int level=0;                    // Variable für den aktuellen Level  
int score=0;                    // Variable für die Punkte 
int highscore=0;                // Variable für die Highscore 
boolean pause=false;            // Variable für Spielunterbrechung (Klopause)

const unsigned int level_ticks_timeout[ 10 ]	=  // Delayzeiten für das Fallen des Bricks
  { 300,150, 100, 80, 60, 50, 40, 35, 30, 20 };    // je nach Level


void setup() { 
    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS); // LED-Array initialisieren
    LEDS.setBrightness(Brightness);                          // Heligkeit setzen
    for (int i=0 ; i<NUM_LEDS ; i++) LED_Matrix[i]=0;        // Speicher des LED-Feldes(Bildschirm) löschen
    randomSeed(analogRead(0));                               // Zufallsfunktion initialisieren
    pinMode(KEY_LEFT, INPUT_PULLUP);                         // --------------------------------------
    pinMode(KEY_RIGHT, INPUT_PULLUP);                        // Tastenpins als Eingänge definieren und
    pinMode(KEY_ROTATE, INPUT_PULLUP);                       // internen Pullup Widerstand schalten
    pinMode(KEY_DOWN, INPUT_PULLUP);                         // --------------------------------------
    
      
//write_int_to_EEPROM(0);    //Muss einmal zum Löschen des Highscore aktiviert werden

 highscore = read_Highscore_from_EEPROM();  // Highscore aus dem EEPROM lesen
 key=get_key();                             // Taste, die beim Starten gedrückt wurde erkennen
 if (key!=2)                                // Vorspann überspringen mit rotate Taste
  {  
    Lauftext_blenden("MAKE",2,3,3,0,0,255);   // Text, x-Position,y-Position, Delay, r, g, b
    Lauftext_blenden(":",2,3,10,255,0,0);     // Text, x-Position,y-Position, Delay, r, g, b
    Lauftext_blenden("BLOCK",2,3,3,0,255,0); // Text, x-Position,y-Position, Delay, r, g, b
    delay(500);
    Lauftext_blenden("%",1,3,50,0,255,0);    // Text, x-Position, Delay, r, g, b   ==> Smillie
  }  
  new_game();
 }



void loop() 
{
         if (pause==false) ticks++;       // Spieler muss mal aufs Clo ==> drehen und down gleichzeitig gedrückt
         key=get_key();                   // Tasten abfragen
         if (game_over==false)
         {                                // Aktionen den Tasten zuweisen
           if (key==1)   move_right();    // Brick nach rechts verschieben
           if (key==2)   rotate();        // Brick drehen 
           if (key==4)   move_left();     // Brick nach links verschieben
           if (key==8)   move_down();     // Brick schnell anch unten bewegen
           if (key==10)  pause=true;      // rotate und move_down gleichzeitig gedrückt = Pause 
         } else
        {                                 // -------- Tasten nach dem Spiel -----
           if (key==4)  show_score();     // Punktzahl vom letzetn Spiel anzeigen  
           if (key==1)  show_highscore(); // Highscore anzeigen
           if (key==8)  new_game();       // Neues Spiel starten
        }
         if (key==15)  new_game();        //alle 4 Tasten gleichzeitig startet das Spiel sofort neu

	if( (ticks >= level_ticks_timeout[level]) and (game_over==false)  )
	{
            check_score();
            ticks = 0;
            pos_y++;
            if (Kollision(pos_x,pos_y))
            {
              display_matrix();
              reihe_voll();
              game_over=Is_Game_Over();
              if (game_over==false) new_Brick();
            } 
            else
            {
             move_brick(); 
            }
	}
  if(update_needed)
  {
    display_matrix();
  }   
}

void check_score()
{
 int faktor = 100;
 byte tmp_level = level;
 if (score > 1*faktor) level=1;
 if (score > 2*faktor) level=2;
 if (score > 3*faktor) level=3;
 if (score > 4*faktor) level=4;
 if (score > 5*faktor) level=5;
 if (score > 6*faktor) level=6;
 if (score > 7*faktor) level=7;
 if (score > 8*faktor) level=8;
 if (score > 9*faktor) level=9;
 if (level!=tmp_level)
 {
   fade_out();
   LEDS.clear();                                    //schaltet nur die LED's aus
   Lauftext_blenden(String(level),2,4,10,0,255,0);  // nächsten Level einblenden
 }
}


void rotate()
{
  byte tmp_Brick[3];  
  for (int i=0 ; i<3 ; i++) tmp_Brick[i]=current_brick[i]; // Bitmuster temporär speichern
  for (int i=0 ; i<3 ; i++)                                // Bitmuster drehen
      current_brick[2-i]=bitRead(tmp_Brick[0], i)
                        + (bitRead(tmp_Brick[1], i)*2) 
                        + (bitRead(tmp_Brick[2], i)*4);
  get_brick_measurement();                              // wie breit ist der Brick?
  if ((pos_x-brickMeasurement[2])+1 < 0) pos_x++;       // falls der Brick beim Drehen anstösst, nach rechts verschieben
  if (Kollision(pos_x,pos_y) == true) pos_x--;          // falls der Brick beim Drehen anstösst, nach links verschieben
  move_brick();  
 }
 
void move_down()
{
    pos_y++;                              // Brick versuchsweise nach unten verschieben
    if (Kollision(pos_x,pos_y)) pos_y--;  // Kollidiert der Brick, dann wieder zurück
      else move_brick();                  // Wenn nicht dann tatsächlich verschieben
    pause=false;                          // falls Pause dann Spiel fortsetzen
}

void move_left()
{
  pos_x--;                                // Brick versuchsweise nach links verschieben
  if  (Kollision(pos_x,pos_y)) pos_x++;   // Kollidiert der Brick, dann wieder zurück
   else  move_brick();                    // Wenn nicht dann tatsächlich verschieben
}

void move_right()
{
    pos_x++;                              // Brick versuchsweise nach rechts verschieben
   if (Kollision(pos_x,pos_y))  pos_x--;  // Kollidiert der Brick, dann wieder zurück
    else  move_brick();                   // Wenn nicht dann tatsächlich verschieben
}

  
void display_matrix()
{
 for (int i=0 ; i<NUM_LEDS ; i++)
 {
   leds[i]=CRGB (COLOR[LED_Matrix[i]]);
 }
 LEDS.show(); 
}

void get_brick_measurement()
{
 for (int i=0;i<3;i++)
 {
    brickMeasurement[i]=0;
    for (int ii=0;ii<3;ii++)
    {
       if (current_brick[ii] & (1<<(i)))  brickMeasurement[i]=1;
    }
 } 
}


boolean move_brick()
{
  if (Kollision(pos_x,pos_y)==false)
  {     
        del_old_Brick();
        draw_Brick(pos_x,pos_y,current_color); // Spalte, Zeile, Farbe
  }
  update_needed=true;
}


void new_Brick()
{
 delay(500);
 for (int z=0 ; z<9 ; z++) last_Brick[z]=-1; 
   int Brick_art=random(0,9);                                     // Zufallsfunktion für Brickart 
   current_color=Brick_art+1;                                     // Jede Brickart erhält immer die gleiche Farbe
   for (int i=0 ; i<3 ; i++)current_brick[i]=Brick[Brick_art][i]; // Brick zuweisen
   get_brick_measurement();                                       // Wie breit ist der Brick
   pos_x=random(brickMeasurement[2],field_width-1-brickMeasurement[0]);// Zufallsfunktion für Spalte  
   if (Brick[Brick_art][0]>0) pos_y=0; else pos_y=-1;  // Bei einem flachen Brick in der obersten Zeile beginnen
   score++;                                            //Punktzahl erhöhen
}


byte get_key() // Steuertasten auslesen
{ 
  static byte last_key;
  static unsigned int repeat;
  byte  key=0;
   if( digitalRead(KEY_LEFT  ) == LOW) key = key + 2;   // links
   if( digitalRead(KEY_RIGHT ) == LOW) key = key + 1;   // rechts
   if( digitalRead(KEY_ROTATE) == LOW) key = key + 4;   // drehen
   if( digitalRead(KEY_DOWN  ) == LOW) key = key + 8;   // herabfallen
   if ((key != last_key)  or (repeat > 20))
   { 
     repeat=0;
     last_key = key;
     return key;
   } else 
   {
     if (key != 2)repeat++;
     return 0;
   }  
   
}



// Schaltet eine LED in der Spalte, Zeile mit der Farbe (0-9) ein
void Set(int Spalte, int Zeile, CRGB COLOR)
{
 leds[ Zeile-1+(Spalte-1)*field_height] = CRGB(COLOR);
 //LEDS.show();
}

// errechnet die Nummer der LED aus Spalte und Zeile
int Pos(int Spalte, int Zeile)
{ 
  int ergebnis =  Zeile-1+(Spalte-1)*field_height;
  return ergebnis;
}



void del_old_Brick()
{
  for (int z=0 ; z<9 ; z++)
  {
    if (last_Brick[z] > -1) 
     {
       LED_Matrix[last_Brick[z]] = 0; 
     } 
  }
}


void draw_Brick(int Spalte, int Zeile, byte Farbe )
{
  for (int z=0 ; z<9 ; z++) last_Brick[z] = -1; 
  Spalte=Spalte+3;
  for( int ii=0 ; ii<3 ; ii++){ // Brickhöhe
  for (int i=0 ; i<3 ; i++)     // Brickbreite
  { 
   if (current_brick[ii] & (1<<(i)))
   if ((Zeile+ii) > 0) 
   { 
     LED_Matrix[Pos(Spalte-i ,Zeile+ii)]=Farbe;
     last_Brick[ii*3+i]=Pos(Spalte-i , Zeile+ii);
   }  
  }
 }
}

boolean Kollision(int Spalte, int Zeile)
{
  boolean belegt=false; 
  Spalte=Spalte+3;
 
  for( int ii=0 ; ii<3 ; ii++) // Schleife über Brickhöhe
  { 
   for (int i=0 ; i<3 ; i++)   // Schleife über Brickbreite
   {
    if (current_brick[ii] & (1<<(i)))
    {  
     if (((Spalte-i)<=0 ) or ((Spalte-i)>field_width) or ((Zeile+ii)>field_height) ) belegt=true; // Grenzen des Spielfeldes?
     if( LED_Matrix[Pos(Spalte-i ,Zeile+ii)]>0) //LED_Matrix belegt? 
     { 
      boolean aktuelle_LED_Matrix=false;
      for (int iii=0 ; iii<field_width ; iii++)  
        if (last_Brick[iii]==Pos(Spalte-i ,Zeile+ii)) aktuelle_LED_Matrix=true;     //aktuelle LED_Matrix nicht beachten
      if (aktuelle_LED_Matrix == false) belegt=true;
     }   
    }  
   }
 }
 return belegt; 
}


void reihe_voll()
{
 for (int i=0; i<field_height ; i++)
{
 boolean voll=true;
 byte color=LED_Matrix[i] ;

 if (color != 0)
 {
  for (int ii=0 ; ii<field_width ; ii++)
   { 
    if (LED_Matrix[ii*field_height+i]==0)voll=false; 
   }
 }else voll=false;
  
 if (voll==true) 
 {
   Zeile_loeschen(i+1,color);
   score=score+10;
 }
}
}


void flashLine(byte line,byte color){  // ------------Zeile Binken lassen
   
 for (int ii=0 ; ii<3 ; ii++)
 {
   for (int i=1 ; i<field_width+1 ; i++) Set(i, line, COLOR[0]);
   LEDS.show();
   delay(200);
   for (int i=1 ; i<field_width+1 ; i++) Set(i, line, COLOR[color]);
   LEDS.show();
   delay(200);
 }
}  

void Zeile_loeschen(int zeile,byte color)
{
flashLine( zeile, color); 
 for (int i=(zeile-1) ; i>0 ; i--)
 for (int ii=0 ; ii<field_width ; ii++)
 {
   leds[ii*field_height+i]=COLOR[LED_Matrix[ii*field_height+(i-1)]];   // field_height nach unten verschieben
   LED_Matrix[ii*field_height+i]=LED_Matrix[ii*field_height+(i-1)];    // und LED_Matrix speichern 
 }
 for (int i=0 ; i<field_width ; i++) // oberste Zeile löschen
 {  
   LED_Matrix[i*field_height]=0;      
   leds[i*field_height] = CRGB(0, 0, 0); //oberste Reihe ausschalten
 } 
 LEDS.show();
}

boolean Is_Game_Over()
{
   int z=0;
   for (int i=0 ; i<field_width ; i++) if (LED_Matrix[i*field_height]>0)  z++;
   if (z>0)
  { 
      if (highscore<score) //Highscore in EEPROM speichern
      {
        write_Highscore_to_EEPROM(score);
        highscore=score;
        show_highscore();
      }
      show_game_over();
      return true;
  } else return false;
}

void show_game_over()
{
     LEDS.setBrightness(0); 
     LEDS.show();
     delay(300);
     LEDS.setBrightness(Brightness);
     LEDS.show(); 
     delay(1000);  
     fade_out();
     LEDS.clear();
     LEDS.setBrightness(Brightness);
    Lauftext_von_unten("GAME OVER",2,100,0,255,0);   // Text, y-Position, Delay, r, g, b  
    show_score();
}

void show_score()
{
   LEDS.clear();
   Lauftext_von_rechts("Score:",2,100,255,0,0);       // Text, y-Position, Delay, r, g, b   
   Lauftext_von_rechts(String(score),2,100,255,0,0);  // Text, y-Position, Delay, r, g, b   
}

void show_highscore()
{
   LEDS.clear();
   Lauftext_von_rechts("Highscore:",2,100,0,0,255);      // Text, y-Position, Delay, r, g, b   
   Lauftext_von_rechts(String(highscore),2,100,0,0,255); // Text, y-Position, Delay, r, g, b   
}


void new_game()
{
  LEDS.clear();
  Lauftext_von_unten("NEW GAME",2,100,255,0,0);  // Text, y-Position, Delay, r, g, b
  for (int i=0 ; i<NUM_LEDS ; i++) 
  {
    LED_Matrix[i]=0;           // Speicher des LED-Feldes(Bildschirm) löschen 
    leds[i] = CRGB(50, 0, 0);  //alle LED nacheinander einschalten
    LEDS.show(); 
  } 
  for (int i=0 ; i<NUM_LEDS ; i++) 
  {
     leds[i] = CRGB(0, 0, 0); //alle LED nacheinander ausschalten
     LEDS.show(); 
  }
  game_over=false;
  pos_y=0;
  level=0;
  score=0;
  Lauftext_blenden("0",3,3,10,0,255,0); // Text, x-Position, Delay, r, g, b
  new_Brick();
}

void fade_out()
{
 for (int i=Brightness ; i>0 ; i--)
 {
    LEDS.setBrightness(i);
    delay(20);
    LEDS.show();  
 }
}

void fade_in()
{
 for (int i=0 ; i<=Brightness ; i++)
 {
    LEDS.setBrightness(i);
    delay(20);
    LEDS.show();  
 }
}



void clear_Matrix()
{
 LEDS.clear();           //alle LED ausschalten
 for (int i=0 ; i< NUM_LEDS ; i++)
 {   
   LED_Matrix[i]=0;     //Matrix löschen      
 }
}

void write_Highscore_to_EEPROM( int value)
{
 byte a = value/256;
 byte b = value % 256;
 EEPROM.write(0,a);
 EEPROM.write(1,b); 
}

int read_Highscore_from_EEPROM()
{
 byte a=EEPROM.read(0);
 byte b=EEPROM.read(1);
 return  (a*256+b); 
}
