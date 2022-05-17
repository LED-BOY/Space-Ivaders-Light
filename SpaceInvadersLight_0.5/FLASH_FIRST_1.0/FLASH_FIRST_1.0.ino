#include <avr/eeprom.h>

const uint8_t sprites[] = {
  0xe0, 0xf0, 0xf0, 0xfc, 0xfc, 0xf0, 0xf0, 0xe0, // player ship 0-8
  0x00, 0x42, 0x20, 0x10, 0x08, 0x04, 0x42, 0x00, // percent simbol 8-16
  0x3f, 0x21, 0x21, 0x21, 0x21, 0x21, 0x33, 0x1e, // bat icon 16-24
  0xff, 0xff, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, // letter L 24-32
  0xff, 0xff, 0xdb, 0xdb, 0xdb, 0x00, 0x00, 0x00, // letter E 32-40
  0x0f, 0x3f, 0x78, 0xe0, 0xe0, 0x78, 0x3f, 0x0f, // letter V 40-48
  0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x00, // letter H 48-56
  0x00, 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 0x00 // letter I  56-64
};

void setup() {
  // We will be outputting PWM on PA3 on an 8-pin part PA3 - TCA0 WO0, pin 4 on 8-pin parts
  PORTMUX.CTRLC     = PORTMUX_TCA00_ALTERNATE_gc; // turn off PORTMUX, returning WO0 PORTMUX_TCA00_DEFAULT_gc for PA3. PORTMUX_TCA00_ALTERNATE_gc; for PA7
  takeOverTCA0();                               // this replaces disabling and resettng the timer, required previously.
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); // Single slope PWM mode, PWM on WO0
  TCA0.SINGLE.PER   = 255;                    // Count all the way up to (255) - 8-bit PWM. At 5MHz, this gives ~19.607kHz PWM
  TCA0.SINGLE.CMP0  = 115; // 45% duty cycle
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // enable the timer with no prescaler


  for (uint16_t x = 0; x < sizeof(sprites); x++) {
    eeprom_write_byte((uint8_t*)x, sprites[x]);
    eeprom_write_byte((uint8_t*)126, 0); // score address initialization.
    eeprom_write_byte((uint8_t*)127, 0);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
