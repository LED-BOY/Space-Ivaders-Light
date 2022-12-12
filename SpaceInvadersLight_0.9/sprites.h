/*
    This file is part of SpaceInvadersLight x.x.

    Foobar is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.

    To contact us: ledboy.net
    ledboyconsole@gmail.com
*/

const  uint8_t enemy1A[] = {
  0b00010001, 0b11111001, 0b11101111, 0b00111000, 0b00111000, 0b11101111, 0b11111001, 0b00010001
};

const  uint8_t enemy1B[] = {
  0b00001010, 0b00111010, 0b11101110, 0b11111000, 0b11111000, 0b11101110, 0b00111010, 0b00100010
};

const  uint8_t enemiesPattern[] = {
  20, 28, 36, 44, 52, 60
};

const  uint8_t ship[] = {
0xe0, 0xf0, 0xf0, 0xfc, 0xfc, 0xf0, 0xf0, 0xe0
};

const  uint8_t percentSimbol[] = {
0x00, 0x42, 0x20, 0x10, 0x08, 0x04, 0x42, 0x00
};

const  uint8_t battIcon[] = {
  0x3f, 0x21, 0x21, 0x21, 0x21, 0x21, 0x33, 0x1e
};

//letters
const  uint8_t letterL[] = {
  0xff, 0xff, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00
};

const  uint8_t letterE[] = {
0xff, 0xff, 0xdb, 0xdb, 0xdb, 0x00, 0x00, 0x00
};

const  uint8_t letterV[] = {
0x0f, 0x3f, 0x78, 0xe0, 0xe0, 0x78, 0x3f, 0x0f
};

const  uint8_t letterH[] = {
0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x00
};

const  uint8_t letterI[] = {
0x00, 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 0x00
};

//digits.
const  uint8_t number0 [] = {
  0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
};

const  uint8_t number1[] = {
  0x00, 0x00, 0x42, 0x7F, 0x40, 0x00, // 1
};

const  uint8_t number2[] = {
  0x00, 0x42, 0x61, 0x51, 0x49, 0x46, // 2
};

const  uint8_t number3[] = {
  0x00, 0x21, 0x41, 0x45, 0x4B, 0x31, // 3
};

const  uint8_t number4[] = {
  0x00, 0x18, 0x14, 0x12, 0x7F, 0x10, // 4
};

const  uint8_t number5[] = {
  0x00, 0x27, 0x45, 0x45, 0x45, 0x39, // 5
};

const  uint8_t number6[] = {
  0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
};

const  uint8_t number7[] = {
  0x00, 0x01, 0x71, 0x09, 0x05, 0x03, // 7
};

const  uint8_t number8[] = {
  0x00, 0x36, 0x49, 0x49, 0x49, 0x36, // 8
};

const  uint8_t number9[] = {
  0x00, 0x06, 0x49, 0x49, 0x29, 0x1E, // 9
};

// Array containing all numbers array
const uint16_t numbers[] = { number0, number1, number2, number3, number4,
                             number5, number6, number7, number8, number9
                           };
