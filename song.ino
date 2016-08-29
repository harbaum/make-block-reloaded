// song.ino

// when using 16 bit timer 1 the values exceed 255 and we need to add
// an offset to store the ocr values in bytes
#define AUDIO_OFF 128

// convert frequencies to timer values
#define FREQ2OCR(a) ((int)(F_CPU/(2*AUDIO_PRESCALER*(a))-0.5)-AUDIO_OFF)

// Frequencies/notes required for Korobeiniki (Tetris theme):
#define G4S FREQ2OCR(415.30)
#define A4  FREQ2OCR(440.00)
#define B4  FREQ2OCR(493.88)
#define C5  FREQ2OCR(523.25)
#define D5  FREQ2OCR(587.33)
#define E5  FREQ2OCR(659.25)
#define F5  FREQ2OCR(698.46)
#define G5  FREQ2OCR(783.99)
#define G5S FREQ2OCR(830.61)
#define A5  FREQ2OCR(880.00)
#define P   (0)

// Korobeiniki
const struct song_S { uint8_t ocr; uint8_t len; } song[] PROGMEM = {
  { E5,2}, { B4,1}, { C5,1}, { D5,2}, { C5,1}, { B4,1}, 
  { A4,2}, { A4,1}, { C5,1}, { E5,2}, { D5,1}, { C5,1}, 
  { B4,3}, { C5,1}, { D5,2}, { E5,2}, 
  { C5,2}, { A4,2}, { A4,2}, {  P,2}, 

  { D5,3}, { F5,1}, { A5,2}, { G5,1}, { F5,1}, 
  { E5,3}, { C5,1}, { E5,2}, { D5,1}, { C5,1}, 
  { B4,2}, { B4,1}, { C5,1}, { D5,2}, { E5,2}, 
  { C5,2}, { A4,2}, { A4,2}, {  P,2}, 

  { E5,2}, { B4,1}, { C5,1}, { D5,2}, { C5,1}, { B4,1}, 
  { A4,2}, { A4,1}, { C5,1}, { E5,2}, { D5,1}, { C5,1}, 
  { B4,3}, { C5,1}, { D5,2}, { E5,2}, 
  { C5,2}, { A4,2}, { A4,2}, {  P,2}, 

  { D5,3}, { F5,1}, { A5,2}, { G5,1}, { F5,1}, 
  { E5,3}, { C5,1}, { E5,2}, { D5,1}, { C5,1}, 
  { B4,2}, { B4,1}, { C5,1}, { D5,2}, { E5,2}, 
  { C5,2}, { A4,2}, { A4,2}, {  P,2}, 

  { E5,4}, { C5,4}, 
  { D5,4}, { B4,4}, 
  { C5,4}, { A4,4}, 
  {G4S,4}, { B4,2}, {  P,2}, 

  { E5,4}, { C5,4}, 
  { D5,4}, { B4,4}, 
  { C5,2}, { E5,2}, { A5,4}, 
  {G5S,4}, { P,4}, 

  {  P,0}
};

#define PAUSE 1
#define SPEED (FPS/5)

void song_init() {
  audio_init();
}

void song_process(int8_t speed) {
  // This routine is called at 60Hz. The music runs at 150BPM = 2.5BPS.
  // The shortest note is a half beat, so we need to handle 5 events/sec
  // This is 12 frames at 60 Hz
  static uint8_t next_event = 0;
  static uint8_t current_note = 0;
  static uint8_t pause = 0;

  // stop any running playback
  if(!speed) {
    audio_set(AUDIO_OFF); // turn audio off
    current_note = 0;     // reset song pointer
    return;
  }

  if(!next_event) {
    
    if(pause) {
      audio_set(AUDIO_OFF);
      next_event = pause-1;
      pause = 0;
    } else {
      // speed 1..99 -> 1..8

      // reduce speed increase
      speed = (speed+1)/2;    // 1,2,3,4,5,6,7,... -> 1,1,2,2,3,3,4,...

      // limit playback speed to 8
      if(speed > 8) speed = 8;

      audio_set(pgm_read_byte(&song[current_note].ocr)+AUDIO_OFF);
      // relative inter-note pause length
      next_event = (SPEED-PAUSE-speed) * pgm_read_byte(&song[current_note].len);
      pause      =              PAUSE  * pgm_read_byte(&song[current_note].len);
      
      current_note++;
      if(!pgm_read_byte(&song[current_note].len))
        current_note = 0;
    }
  } else
    next_event--;
}

