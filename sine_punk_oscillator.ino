/*

  S     N      I     D
  SINE PUNK OSCILLATOR
  v 0.1

  sine wave phase multiplying oscillator thing
  -- for use with ATtiny 85 running @ 8 MHz --

  thank you to the following sources from which much of this code is derived: 
  - technoblogy for the waveform generation info
  - miniMO for the sinewave stuff
  - bastl instruments for the kastle synth
  
*/

unsigned int acc1, acc2, freq1, freq2, d, e;

// miniMO sine wavetable
const char PROGMEM sinetable[128] = {
  0,   0,   0,   0,   1,   1,   1,   2,   2,   3,   4,   5,   5,   6,   7,   9,
  10,  11,  12,  14,  15,  17,  18,  20,  21,  23,  25,  27,  29,  31,  33,  35,
  37,  40,  42,  44,  47,  49,  52,  54,  57,  59,  62,  65,  67,  70,  73,  76,
  79,  82,  85,  88,  90,  93,  97,  100, 103, 106, 109, 112, 115, 118, 121, 124,
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
  176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
  218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244,
  245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
};

unsigned char wavetable[256];

void setup() {
  sineWave();

  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);

  PLLCSR |= (1 << PLLE);               // Enable PLL (64 MHz)
  delay(100);
  while (!(PLLCSR & (1 << PLOCK)));    // Ensure PLL lock
  PLLCSR |= (1 << PCKE);               // Enable PLL as clock source for timer 1
  PLLCSR |= (1 << LSM);                //low speed mode 32mhz
  cli();                               // Interrupts OFF (disable interrupts globally)

  TCCR0A = 2 << COM0A0 | 2 << COM0B0 | 3 << WGM00;
  TCCR0B = 0 << WGM02 | 1 << CS00;

  //  setup timer 0 to run fast for audiorate interrupt
  TCCR1 = 0;                           //stop the timer
  TCNT1 = 0;                           //zero the timer
  GTCCR = _BV(PSR1);                   //reset the prescaler
  OCR1A = 255;                         //set the compare value
  OCR1C = 255;
  TIMSK = _BV(OCIE1A);                 //interrupt on Compare Match A
  TCCR1 = _BV(CTC1) | _BV(CS12);
  sei();
}


// more miniMO stuff
void sineWave() {
  for (int i = 0; i < 128; ++i) {
    wavetable[i] = pgm_read_byte_near(sinetable + i);
  }
  wavetable[128] = 255;
  for (int i = 129; i < 256; ++i) {
    wavetable[i] = wavetable[256 - i] ;
  }
}

void loop() {
  if (analogRead(0) > 800) {           // check switch
    freq1 = analogRead(3) * 8;         // wave mult osc frequency
    freq2 = analogRead(2) * 4;         // sine wave osc frequency
  }
  else {

    freq1 = analogRead(3) * 8;
    freq2 = analogRead(2) / 5 + freq1 / 2;
  }

  e = analogRead(1) / 2 - 256;         // comparator
  if (e > 20) {
    d = e - 20;
  }
  else if (e < -20) {
    d = e + 20;
  }
  else {
    d = 0;
  }
}

ISR(TIMER1_COMPA_vect) {
  // sine wave osc
  acc2 += freq2;
  
  // wave mult osc
  // weird stuff when accumulator is less than comparartor
  if ((acc1 >> 8) < d) {
    acc1 += (freq1 * d);
  }
  // sine wave when accumulator is greater than comparartor
  else {
    acc1 += freq1;
  }

  OCR0A = wavetable[acc1 >> 8];

  OCR0B = wavetable[acc2 >> 8];
}
