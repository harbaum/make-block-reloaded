// audio.ino
//
// audio routines for the make:block:xl tetris
// all sound generation done through timer1 in CTC mode to
// allow led driver to disable irqs without impact on sound
// generation as happening with arduino tone() function

static bool audio_is_enabled = false;
void audio_on(bool on) {
  audio_is_enabled = on;
}

void audio_init() {
  // Timer1
  // OCR1A -> PB1 -> D9

  // CTC mode
  // -------------------------------------------
  // WGM13..10 = X100
  // FREQ = 16000000 / (2*X*(1+OCR))
  // OCR value = 16000000/(2*X*FREQ)-1
  
  // X=64: 125000/FREQ-1 
  // G4#: OCR=300, FREQ = 415.28 -> 0.004% error
  // ...
  // A5:  OCR=141, FREQ = 880.28 -> 0.03% error
  
  // COM1A1=0, COM1A0=1, WGM11=0, WGM10=0
  TCCR1A = 0;
  
  // WGM13=0, WGN12=1, CS12=0, CS11=1, CS10=1 (CS=3:64, CS=2:8)
  TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);

  TCCR1C = 0;
}

void audio_set(uint16_t ocr) {
  if((ocr==128) || !audio_is_enabled)
    TCCR1A = 0;              // output off
  else {
    TCCR1A = (1<<COM1A0);    // ocr toggle mode
    TCNT1 = 0;
    OCR1A = ocr;
  }
}
