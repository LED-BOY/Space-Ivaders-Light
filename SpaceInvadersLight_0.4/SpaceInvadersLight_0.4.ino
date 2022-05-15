/*SpaceInvadersLight  for LEDBOY and any Attiny series 1 compatible using neopixels
  Flash CPU Speed 5MHz.
  this code is released under GPL v3, you are free to use and modify.
  released on 2022.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    To contact us: ledboy.net
    ledboyconsole@gmail.com
*/
#include "tinyOLED.h"
#include "sprites.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

#if defined(MILLIS_USE_TIMERA0) || defined(MILLIS_USE_TIMERB0)
#error "This sketch does't use TCA0 nor TCB0 to save flash, select Disabled."
#endif

#define LEDON PORTA.OUT &= ~PIN3_bm;// led ON, also resets the oled screen.
#define LEDOFF PORTA.OUT |= PIN3_bm;// led OFF
#define BUTTONLOW !(PORTA.IN & PIN6_bm)// button press low
#define BUTTONHIGH (PORTA.IN & PIN6_bm)// button not pressed high
#define wdt_reset() __asm__ __volatile__ ("wdr"::) // watchdog reset macro

int8_t actualVoltage = 0;
uint8_t gameLevel = 1;
uint8_t tempArray [8];
uint16_t timer = 0;
uint16_t score = 0;
volatile uint16_t interruptTimer = 0;

void setup() {
  _PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_4KCLK_gc);// enable watchdog 1 sec. aprox.
  PORTA.DIR = 0b10000110; // setup ports in and out
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm;// button pullup

  TCB0.INTCTRL = TCB_CAPT_bm; // Setup Timer B as compare capture mode to trigger interrupt
  TCB0_CCMP = 5000;// CLK
  TCB0_CTRLA = (1 << 1) | TCB_ENABLE_bm;

  // We will be outputting PWM on PA3 on an 8-pin part PA3 - TCA0 WO0, pin 4 on 8-pin parts
  PORTMUX.CTRLC     = PORTMUX_TCA00_ALTERNATE_gc; // turn off PORTMUX, returning WO0 PORTMUX_TCA00_DEFAULT_gc for PA3. PORTMUX_TCA00_ALTERNATE_gc; for PA7
  takeOverTCA0();                               // this replaces disabling and resettng the timer, required previously.
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); // Single slope PWM mode, PWM on WO0
  TCA0.SINGLE.PER   = 255;                    // Count all the way up to (255) - 8-bit PWM. At 5MHz, this gives ~19.607kHz PWM
  TCA0.SINGLE.CMP0  = 115; // 45% duty cycle
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // enable the timer with no prescaler

  sei(); // enable interrupts
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);// configure sleep mode

  if (readSupplyVoltage() > 3800 && BUTTONHIGH) { // If voltage is above 3.8v the led is on to increase Amp consumption and lower batt.
    PORTA.DIR = (1 << 3);// pin3 (LED) as output
  }

  if (BUTTONHIGH) goToSleep(); // if button is high it is assumed that the watchdog bite and there is no intention to wake up.

  PORTA.DIR = (1 << 3); // pin3 (LED) as output
  LEDOFF
  oled.begin();// start oled screen
}

void loop() {
  buttonDebounce();
  clearScreen();
  introScreen();
  game();
}

void game (void) {
  int8_t enemiesPosX = 0;
  int8_t lastEnemiesPosX = 0;
  uint8_t playerPos = 20;
  uint8_t enemiesPosY = 0;
  uint8_t newEnemiesYPos = 0;
  uint8_t lastPlayerPos = 20;
  uint8_t playerShotState = 0;
  uint8_t enemiesShotState = 0;
  uint8_t playerFirePosX = 0;
  int8_t enemiesFirePosX = 0;
  int8_t playerFirePosY = 3;
  int8_t enemiesFirePosY = 0;
  uint8_t enemiesKilled [sizeof (enemiesPattern)];
  uint8_t enemiesKilledCounter = 0;
  int8_t enemiesDirection [] = { -1, 1};
  uint16_t enemiesFireTimer = 0;
  uint16_t playerFireTimer = 0;
  uint16_t enemiesTimer = 0;
  uint16_t playerTimer = 0;
  bool enemiesMoveRight = true;
  bool enemiesPosXEven = true;
  bool playerFireAgain = true;
  bool enemiesFireAgain = true;
  bool enemyIsDead = false;

  // draws some screen elements like score and batt display.
  clearScreen();
  getSpriteFromEeprom(16, 24);
  drawSprite(4, 0, tempArray, 0);
  getSpriteFromEeprom(8, 16);
  drawSprite(4, 1, tempArray, 0);
  drawDecimal (0, 2, voltageReading()); // display batt %
  showScore(score);
  getSpriteFromEeprom(0, 8);// get Ship srite

  for ( uint8_t x = 0; x < 4; x++) { // draws screen bars
    oled.drawLine(18, x, 1, 0b11111111);
    oled.drawLine(110, x, 1, 0b11111111);
  }

  for ( uint8_t x = 0; x < sizeof (enemiesKilled); x++) {// clears enemies killed array.
    enemiesKilled [x] = 0;
  }

  while (true) {

    if ((interruptTimer - timer) > 8000) {
      timer = interruptTimer;
      drawDecimal (0, 2, voltageReading()); // display batt %
      for ( uint8_t x = 0; x < 2; x++) {
        oled.drawLine(20, (newEnemiesYPos + x), 90, 0x00); // clear  enemy position
      }
      newEnemiesYPos++;
    }

    if ((interruptTimer - enemiesTimer) > (80 - gameLevel)) { // enemies movement and kill logic
      enemiesPosX += enemiesDirection [enemiesMoveRight];



      for ( uint8_t x = 0; x < sizeof (enemiesPattern); x++) {// defines enemies positions.
        enemyIsDead = false;

        for ( uint8_t y = 0; y < sizeof (enemiesKilled); y++) {// checks if the enemy is dead

          if (enemiesKilled [y] == enemiesPattern [x]) {
            enemyIsDead = true;
          }
        }

        if (!enemyIsDead) {// only if the enemy isn't dead continue.

          if ( (enemiesPattern [x] + enemiesPosX) > 102 && enemiesMoveRight == true) { // chesks boundaries
            enemiesMoveRight = false;
          }

          if ( (enemiesPattern [x] + enemiesPosX) < 26 && enemiesMoveRight == false) {
            enemiesMoveRight = true;
          }

          if (enemiesFireAgain) { // choose wich enemy fires, randomizer based on timer press.
            if (interruptTimer % 2 == 0) {
              enemiesFirePosX = (enemiesPattern [x] + enemiesPosX);
              enemiesFireAgain = false;
            }
          }

          if (x % 2 == 0) {// if enemy pattern is even or odd draws it in the corresponding screen page (y Positon)
            enemiesPosY = 0;
          } else {
            enemiesPosY = 1;
          }

          enemiesPosY += newEnemiesYPos;

          if (playerFirePosX > (enemiesPattern [x] + enemiesPosX) && playerFirePosX < (enemiesPattern [x] + enemiesPosX) + 8 && playerFirePosY == enemiesPosY) { // if palyer hits an enemy
            enemiesKilled [enemiesKilledCounter] = enemiesPattern [x];
            enemiesKilledCounter++;
            score += 2;
            playerFireAgain = true;
            oled.drawLine((enemiesPattern [x] + lastEnemiesPosX), enemiesPosY, 8, 0x00); // clear  enemy position
            showScore(score);
          } else {
            oled.drawLine((enemiesPattern [x] + lastEnemiesPosX), enemiesPosY, 8, 0x00); // clear last enemy position

            if (enemiesPosX % 2 == 0) { // choose enemies animation based on if the pos is even or odd.
              drawSprite(enemiesPattern [x] + enemiesPosX, enemiesPosY, enemy1A, 0); // draws the enemy on screen.
            } else {
              drawSprite(enemiesPattern [x] + enemiesPosX, enemiesPosY, enemy1B, 0); // draws the enemy on screen.
            }
          }
        }
      }
      lastEnemiesPosX = enemiesPosX;
      if (enemiesKilledCounter > 5) {
        gameLevel++;
        break;
      }
      enemiesTimer = interruptTimer;
    }

    if ((interruptTimer - playerFireTimer) > 70) { // player fire logic
      playerFireTimer = interruptTimer;
      oled.drawLine (playerFirePosX, playerFirePosY, 1, 0b00000000); // clear player shoot

      if (playerFireAgain) {
        playerFirePosX = (playerPos + 4);
        playerFirePosY = 3;
        playerShotState = 6;
        playerFireAgain = false;
      }

      if (playerShotState > 6) {
        playerShotState = 0;
        playerFirePosY--;

        if (playerFirePosY < 0) {
          playerFireAgain = true;
        }
      }

      if (!playerFireAgain) {
        oled.drawLine (playerFirePosX, playerFirePosY, 1, (0b11000000 >> playerShotState));
        playerShotState++;
      }
    }

    if ((interruptTimer - enemiesFireTimer) > 50 && !enemiesFireAgain) { // enemies fire logic
      enemiesFireTimer = interruptTimer;

      oled.drawLine (enemiesFirePosX, enemiesFirePosY, 1, 0b00000000);

      if (enemiesShotState > 6) {
        enemiesShotState = 0;
        enemiesFirePosY++;

        if (enemiesFirePosY > 3) {
          enemiesFirePosY = enemiesPosY;
          enemiesShotState = 6;
          enemiesFireAgain = true;
        }
      }

      if (!enemiesFireAgain) {
        oled.drawLine (enemiesFirePosX, enemiesFirePosY, 1, (0b00000011 << enemiesShotState));
        enemiesShotState++;
      }
    }

    if ((interruptTimer - playerTimer) > 15) { // player movement logic.
      playerTimer = interruptTimer;

      if (lastPlayerPos != playerPos) {// clear last player position
        oled.drawLine(20, 3, 90, 0x00);
      }
      lastPlayerPos == playerPos;

      if (BUTTONLOW && playerPos < 103) {
        playerPos++;
      } else if (playerPos > 20) {
        playerPos--;
      }
      drawSprite(playerPos, 3, tempArray, 0);// in this case tempArray contains ship sprite
    }

    if (enemiesFirePosX > playerPos && enemiesFirePosX < (playerPos + 8) && enemiesFirePosY == 3 || enemiesPosY > 2) { // checks if player is hit or not.
      gameLevel = 1;
      score = 0;
      break;
    }
  }
}

void introScreen (void) {

  if (score > readScoreFromEEPROM(126)) {
    writeScoreToEEPROM(126, score);
  }

  getSpriteFromEeprom(24, 32);// letters L
  drawSprite (25, 1, tempArray, 0);
  drawSprite (65, 1, tempArray, 0);
  getSpriteFromEeprom(32, 40);// letters E
  drawSprite (35, 1, tempArray, 0);
  drawSprite (55, 1, tempArray, 0);
  getSpriteFromEeprom(40, 48);// letters E
  drawSprite (45, 1, tempArray, 0);
  drawDecimal (40, 2, gameLevel);
  showScore(readScoreFromEEPROM(126));

  timer = interruptTimer;
  while (BUTTONHIGH) {

    if ((interruptTimer - timer) > 4000) {
      clearScreen();
      oled.ssd1306_send_command(0xAE);
      goToSleep();
    }
  }
}

void getSpriteFromEeprom (uint8_t addressStart, uint8_t addressFinish) {
  uint8_t y = 0;
  for (uint8_t x = addressStart; x < addressFinish; x++) {
    tempArray [y]  = eeprom_read_byte((uint8_t*)x);
    y++;
  }
}

void showScore(uint16_t score) {// draws all ledInvadersScore using showTime function
  uint16_t valueDecimals = score;

  if (score < 100) { // functions to draw units in digit 3, decimals in digit 2 and hundredths in digit 1
    drawDecimal (112, 2, score);
    drawDecimal (112, 0, 0);
  } else {
    score /= 100;
    drawDecimal (112, 2, score);
    score *= 100;

    for (uint16_t x = 0; x < score; x++) {
      valueDecimals--;
    }
    drawDecimal (112, 0, valueDecimals);
  }
}

void writeScoreToEEPROM(uint8_t address, uint16_t number) { // ledInvadersScore is an uint16_t so needs to be stored in 2 eeprom slots of 8bits
  eeprom_write_byte((uint8_t*)address, number >> 8);// shift all the bits from the number to the right, 8 times
  eeprom_write_byte((uint8_t*)(address + 1), number & 0xFF);// only store 8bits of the right
}

uint16_t readScoreFromEEPROM(uint8_t address) { // reads and reconstruct the number
  byte byte1 = eeprom_read_byte((uint8_t*)address);
  byte byte2 = eeprom_read_byte((uint8_t*)address + 1);

  return (byte1 << 8) + byte2;
}

uint8_t voltageReading(void) {
  actualVoltage = map(readSupplyVoltage(), 2600, 3805, 0, 99);
  if (actualVoltage < 0) actualVoltage = 0;
  return actualVoltage;
}

void clearScreen (void) {
  for ( uint8_t x = 0; x < 4; x++) {
    oled.drawLine(0, x, 128, 0x00);
  }
}

void buttonDebounce(void) {
  for (uint16_t x = 0; x < 60000; x++) {
    while (BUTTONLOW); // super simple button debounce
  }
}

void drawSprite (uint8_t column, uint8_t page, uint8_t sprite[], bool digit) {
  oled.setCursor(column, page);// position cursor column - page
  oled.ssd1306_send_data_start();

  for (uint8_t x = 0; x < 8; x++) {
    oled.ssd1306_send_data_byte(sprite[x]);
  }
  oled.ssd1306_send_data_stop();

  if (digit) {
    oled.setCursor(column, (page + 1));
    oled.ssd1306_send_data_start();

    for (uint8_t x = 8; x < 16; x++) {
      oled.ssd1306_send_data_byte(sprite[x]);
    }
    oled.ssd1306_send_data_stop();
  }
}

void drawDecimal (uint8_t firstPixel, uint8_t page, uint8_t value) {// this function takes the digit and value from characters.h and draws it without the 0 to the left in hours
  uint8_t valueUnits = value;                      // always draws 2 digits.

  if (value < 10) {
    drawSprite(firstPixel, page, numbers[0], true);
    drawSprite((firstPixel + 8), page, numbers[value], true);
  } else {
    value /= 10; // some math to substract the 0 from the left in hours digits
    drawSprite(firstPixel, page, numbers[value], true);
    value *= 10;

    for (uint8_t x = 0; x < value ; x++) {
      valueUnits--;
    }
    drawSprite((firstPixel + 8), page, numbers[valueUnits], true);
  }
}

void goToSleep (void) {
  PORTA.PIN6CTRL  |= PORT_ISC_BOTHEDGES_gc; //attach interrupt to portA pin 3 keeps pull up enabled
  //_PROTECTED_WRITE(WDT.CTRLA, 0);
  TCA0.SPLIT.CTRLA = 0; //disable TCA0 and set divider to 1
  //TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESET_gc | 0x03; //set CMD to RESET to do a hard reset of TCA0.
  PORTA.OUT |= PIN7_bm;// P CHANNEL mosfet High to deactivate
  sleep_enable();
  sleep_cpu();// go to sleep
}

uint16_t readSupplyVoltage() { //returns value in millivolts  taken from megaTinyCore example
  analogReference(VDD);
  VREF.CTRLA = VREF_ADC0REFSEL_1V5_gc;
  // there is a settling time between when reference is turned on, and when it becomes valid.
  // since the reference is normally turned on only when it is requested, this virtually guarantees
  // that the first reading will be garbage; subsequent readings taken immediately after will be fine.
  // VREF.CTRLB|=VREF_ADC0REFEN_bm;
  // delay(10);
  uint16_t reading = analogRead(ADC_INTREF);
  reading = analogRead(ADC_INTREF);
  uint32_t intermediate = 1023UL * 1500;
  reading = intermediate / reading;
  return reading;
}

ISR(TCB0_INT_vect) {// timmer
  wdt_reset(); // reset watchdog
  interruptTimer++;
  TCB0_INTFLAGS = 1; // clear flag
}

ISR(PORTA_PORT_vect) {
  sleep_disable();
  _PROTECTED_WRITE(RSTCTRL.SWRR, 1);
}
