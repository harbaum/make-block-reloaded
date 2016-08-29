
//////////////////////////////////////////////////////////////////////
//
// Zeichentabelle
//
//////////////////////////////////////////////////////////////////////

byte ASCII[][8]=
{ 
 {0,0,0,0,0,0,0,0},                                //Space ==>ASCII 32
 { 96 , 96 , 96 , 96 , 96 , 0 , 96 , 0 },          //  ! 
 {160, 160 ,0,0,0,0,0,0},                          //  "
 {0,80 ,248 ,80 ,248 , 80 ,0,0},                   //  #
 {0,0,0,0,0,0,0,0},                                //  $ (Dollar)
{126, 129, 165, 129, 165, 153, 129, 126},          //  % (Smilie)
 {0, 108, 40, 130, 130, 68, 56, 0},                //  §
 {0,0,0,0,0,0,0,0},                                //  '
 {32, 64, 128, 128, 128, 64, 32, 0},               //  (
 {128, 64, 32, 32, 32, 64, 128, 0},                //  )
 {146, 68, 40, 146, 32, 68, 146, 0},               //  *
 {0,32, 32, 248, 32, 32,0,0},                      //  +
 { 0 , 0 , 0 , 0 , 0 , 128 , 128 , 0 },            //  ,  (Komma)
 { 0 , 0 , 0 , 112 , 0 , 0 , 0 , 0 },              //  -
 { 0 , 0 , 0 , 0 , 0 , 128 , 128 , 0 },            //  .  (Punkt)
 {1, 2, 4, 8, 16, 32, 64, 128},                    //  /
  ////////////////////// Zahlen //////////////////////
  { 112 , 136 , 152 , 168 , 200 , 136 , 112 , 0 }, // 0  ==>ASCII 48
 { 32 , 96 , 32 , 32 , 32 , 32 , 112 , 0 },        // 1
 { 112 , 136 , 8 , 16 , 32 , 64 , 248 , 0 },       // 2
 { 248 , 16 , 32 , 16 , 8 , 144 , 224 , 0 },       // 3
 { 16 , 48 , 80 , 144 , 248 , 16 , 16 , 0 },       // 4
 { 248 , 128 , 240 , 8 , 8 , 136 , 112 , 0 },      // 5
 { 48 , 64 , 128 , 240 , 136 , 136 , 112 , 0 },    // 6
 { 248 , 8 , 16 , 32 , 64 , 64 , 64 , 0 },         // 7
 { 112 , 136 , 136 , 112 , 136 , 136 , 112 , 0 },  // 8
 { 112 , 136 , 136 , 120 , 8 , 16 , 96 , 0 },      // 9
////////////////////////////////////////////////////////
 {0, 48, 48 , 0, 48, 48, 0, 0},                    //  :
 {0, 48, 48 , 0, 16, 32, 0, 0},                    //  ;
 {16 , 32, 64, 128, 64, 32, 16, 0},                //  <
 {0,0,240,0,240,0,0,0},                            //  =
 {128, 64, 32, 16, 32, 64, 128, 0},                //  >
 { 224 , 144 , 32 , 64 , 64 , 0 , 64 , 0 },        //  ?
 {0, 112, 136, 168, 152, 96, 0,0},                 //  @
 ////////////// Großbuchstaben //////////////////////
 { 112 , 136 , 136 , 136 , 248 , 136 , 136 , 0 },  // A  ==>ASCII 65
 { 240 , 136 , 136 , 240 , 136 , 136 , 240 , 0 },  // B
 { 112 , 136 , 128 , 128 , 128 , 136 , 112 , 0 },  // C
 { 224 , 144 , 136 , 136 , 136 , 144 , 224 , 0 },  // D
 { 248 , 128 , 128 , 248 , 128 , 128 , 248 , 0 },  // E
 { 248 , 128 , 128 , 240 , 128 , 128 , 128 , 0 },  // F
 { 112 , 136 , 128 , 184 , 136 , 136 , 120 , 0 },  // G
 { 136 , 136 , 136 , 248 , 136 , 136 , 136 , 0 },  // H
 { 112 , 32 , 32 , 32 , 32 , 32 , 112 , 0 },       // I
 { 56 , 8 , 8 , 8 , 8 , 72 , 48 , 0 },             // J
 { 136 , 144 , 160 , 192 , 160 , 144 , 136 , 0 },  // K
 { 128 , 128 , 128 , 128 , 128 , 128 , 248 , 0 },  // L
 { 136 , 216 , 168 , 168 , 136 , 136 , 136 , 0 },  // M
 { 136 , 136 , 200 , 168 , 152 , 136 , 136 , 0 },  // N
 { 112 , 136 , 136 , 136 , 136 , 136 , 112 , 0 },  // O
 { 240 , 136 , 136 , 240 , 128 , 128 , 128 , 0 },  // P
 { 112 , 136 , 136 , 136 , 168 , 144 , 104 , 0 },  // Q
 { 240 , 136 , 136 , 240 , 160 , 144 , 136 , 0 },  // R
 { 120 , 128 , 128 , 112 , 8 , 8 , 240 , 0 },      // S
 { 248 , 32 , 32 , 32 , 32 , 32 , 32 , 0 },        // T
 { 136 , 136 , 136 , 136 , 136 , 136 , 112 , 0 },  // U
 { 136 , 136 , 136 , 136 , 136 , 80 , 32 , 0 },    // V
 { 136 , 136 , 136 , 168 , 168 , 168 , 80 , 0 },   // W
 { 136 , 136 , 80 , 32 , 80 , 136 , 136 , 0 },     // X
 { 136 , 136 , 136 , 80 , 32 , 32 , 32 , 0 },      // J
 { 248 , 4 , 8 , 16 , 32 , 64 , 248 , 0 },         // Z
///////////////////////////////////////////////////////
 {0,96, 64, 64, 64, 64, 96, 0},                    //  [
 {0,96, 64, 64, 64, 64, 96, 0},                    //  [
 {0,64, 32, 16, 8, 4,0,0},                         //  \
 {0,96 ,32 ,32 ,32 ,32 ,96 ,0},                    //  ]
 {32, 108, 136 ,0,0,0,0,0},                        //  ^
 {0,0,0,0,0,0,255,0},                              //  _
 {0, 192, 192 , 0, 192, 192, 0, 0},     // :

////////////// Kleinbuchstaben //////////////////////  
 { 0 , 0 , 104 , 152 , 136 , 136 , 120 , 0 },      // a  ==>ASCII 97
 { 128 , 128 , 176 , 200 , 136 , 136 , 240 , 0 },  // b
 { 0 , 0 , 112 , 128 , 128 , 136 , 112 , 0 },      // c
 { 8 , 8 , 104 , 152 , 136 , 136 , 120 , 0 },      // d
 { 0 , 0 , 112 , 136 , 248 , 128 , 112 , 0 },      // e
 { 48 , 72 , 64 , 224 , 64 , 64 , 64 , 0 },        // f
 { 0 , 120 , 136 , 136 , 120 , 8 , 56 , 0 },       // g
 { 128 , 128 , 176 , 200 , 136 , 136 , 136 , 0 },  // h
 { 32 , 0 , 96 , 32 , 32 , 32 , 112 , 0 },         // i
 { 16 , 0 , 48 , 16 , 16 , 144 , 96 , 0 },         // j
 { 128 , 128 , 144 , 160 , 192 , 160 , 144 , 0 },  // k
 { 96 , 32 , 32 , 32 , 32 , 32 , 112 , 0 },        // l
 { 0 , 0 , 208 , 168 , 168 , 136 , 136 , 0 },      // m
 { 0 , 0 , 176 , 200 , 136 , 136 , 136 , 0 },      // n
 { 0 , 0 , 112 , 136 , 136 , 136 , 112 , 0 },      // o
 { 0 , 0 , 240 , 136 , 240 , 128 , 128 , 0 },      // p
 { 0 , 0 , 104 , 152 , 120 , 8 , 8 , 0 },          // q
 { 0 , 0 , 176 , 192 , 128 , 128 , 128 , 0 },      // r
 { 0 , 0 , 112 , 128 , 112 , 8 , 240 , 0 },        // s
 { 64 , 64 , 224 , 64 , 64 , 72 , 48 , 0 },        // t
 { 0 , 0 , 0 , 136 , 136 , 152 , 104 , 0 },        // u
 { 0 , 0 , 136 , 136 , 136 , 80 , 32 , 0 },        // v
 { 0 , 0 , 136 , 136 , 168 , 168 , 80 , 0 },       // w
 { 0 , 0 , 136 , 80 , 32 , 80 , 136 , 0 },         // x
 { 0 , 0 , 136 , 136 , 120 , 8 , 112 , 0 },        // y
 { 0 , 0 , 248 , 16 , 32 , 64 , 248 , 0 },         // z
};

void zeile_loeschen(int zeile){
  if (zeile>=0){
    for (int i=0 ; i<field_width ; i++) leds[zeile+i*field_height]=CRGB(0,0,0); 
  }
}
          
void Lauftext_von_oben(String txt, int x,int pause,int r, int g, int b)
{
  int anzahl = txt.length();                     // Anzahl der Zeichen im Text
  for (int i=0 ; i<field_height+8*anzahl-6 ; i++)  // Textzeichen haben eine Höhe von 8 Pixel
  {
    for (int ii=0;ii<=anzahl;ii++)
    {
     zeile_loeschen(i-9-ii*8);
     ZeichneBuchstaben(txt[anzahl-ii],x,i-ii*8,r,g,b);    
    }
  delay(pause);
  }
}


void Lauftext_von_unten(String txt, int x,int pause,int r, int g, int b)
{
  int anzahl = txt.length();                     // Anzahl der Zeichen im Text
  for (int i=field_height+8*anzahl ; i>0 ; i--)  // Textzeichen haben eine Höhe von 8 Pixel
  {
    for (int ii=0;ii<=anzahl;ii++)
    {
     ZeichneBuchstaben(txt[anzahl-ii],x,i-ii*8,r,g,b);    
    }
  delay(pause);
  }
}


void Lauftext_von_rechts(String txt, int y, int bremse, int r, int g, int b)
{
 int anzahl=txt.length();
 for (int i=8 ; i>anzahl*-6 ; i--)          //Länge des Lauftextes
 {  
   for (int ii=0-(i/6) ; ii<3-(i/6) ; ii++) //Buchstaben durchlaufen
   { 
    ZeichneBuchstaben(txt[ii],i+ii*6,y,r,g,b); 
   }
  delay(bremse);  
 }
}

void Lauftext2(String txt, int x, int y, int bremse, int r, int g, int b) // Nacheinander anzeigen
{
  int anzahl=txt.length();
  for (int i=0 ; i<anzahl ; i++)
  {
    ZeichneBuchstaben(txt[i],x,y,r,g,b);  
    delay(bremse);  
  }
}

void Lauftext_blenden(String txt, int x, int y, int warten, int r, int g, int b) // Nacheinander einblenden
{ 
  LEDS.setBrightness(0);
  int anzahl=txt.length();
  for (int i=0 ; i<anzahl ; i++)
  {
    
  ZeichneBuchstaben(txt[i],x,y,r,g,b);  
  for(int ii=0 ; ii<Brightness ; ii++)
  {
    LEDS.setBrightness(ii);
    delay(warten);
    LEDS.show();
  }
    
  for(int ii=Brightness ; ii>0 ; ii--)
  {
    LEDS.setBrightness(ii);
    delay(warten);
    LEDS.show();
  }
  }
   LEDS.setBrightness(Brightness);
}


void zeichnenPos(byte zeichen[], int posx, int posy, int r, int g, int b)
{
 for (int spalte=0 ; spalte<8 ; spalte++)  //8 pixel breite
 { 
  for (int zeile=0 ; zeile<8 ; zeile++)   //7 pixel höhe
  {     
    if ((zeile+posy>=0) &&(zeile+posy<field_height))
    {
     if ( zeichen[zeile] & (1 << (7-spalte)) ) //if ((bitRead(zeile,A[spalte]))==true)
      { 
        if (((spalte+posx)<field_width) && ((spalte+posx)>=0) ) leds[zeile+spalte*field_height+posx*field_height+posy] = CRGB(r,b, g); 
      }
      else    
      {
      if (((spalte+posx)<field_width) && ((spalte+posx)>=0) ) {leds[zeile+spalte*field_height+posx*field_height+posy] = CRGB(0,0, 0);} 
      }
    }
   }
  }
LEDS.show();
}



void ZeichneBuchstaben(char ch, int x, int y, int r, int g, int b)
{ 
  if (ch-32>0) zeichnenPos(ASCII[ch-32],x,y,r,g,b);  
}

