/*
LEDMatrix_nchips.ino
Author: D. Dubins
AI Assist: ChatGPT, Claude.AI, Perplexity.AI
Date: 17-Jul-26
Last Revised: 28-Jul-26
Description: Drives a series of N 8x8 LED modules. Routines for displaying simple graphics, and scrolling text. Shift Register Example
 for 74HC595 shift register.

 Hardware:
 * 74HC595 shift register attached to pins 4,5, and 6 of the Arduino,
 as detailed below.
 * LEDs attached to each of the outputs of the shift register
 
First 74HC595 to Arduino:
SH_CP (pin 11) to to Ardunio DigitalPin 4
ST_CP (pin 12) to Ardunio DigitalPin 5
DS (pin 14) to Ardunio DigitalPin 6
Pins 10,16: +5V
Pins 8,13: GND

First 74HC595 to Second 74HC595:
Pin 11 to Pin 11
Pin 12 to Pin 12
Pin 9 to Pin 14

8x8 LED Matrix to first and second Shift Registers:
(1588BS - Pin 1 is on the far left of the side with the printed label)
Pin 1 to Chip 2, Pin 4
Pin 2 to Chip 2, Pin 6
Pin 3 to Chip 1, Pin 6
Pin 4 to Chip 1, Pin 5
Pin 5 to Chip 2, Pin 7
Pin 6 to Chip 1, Pin 3
Pin 7 to Chip 2, Pin 5
Pin 8 to Chip 2, Pin 2

Pin 9 to Chip 2, Pin 15
Pin 10 to Chip 1, Pin 4
Pin 11 to Chip 1, Pin 2
Pin 12 to Chip 2, Pin 3
Pin 13 to Chip 1, Pin 7
Pin 14 to Chip 2, Pin 1
Pin 15 to Chip 1, Pin 1
Pin 16 to Chip 1, Pin 15

displayBuffer[8][MODULES]: that's the image:
          Module0   Module1   Module2
Row0      10100110  01001001  00000000
Row1      11111111  00011000  01010101
Row2      ...
...
Row8
*/

#include "LEDfont.h"

const int clockPin = 4;
const int latchPin = 5;
const int dataPin = 6;

#define MODULES 5
#define CHIPNUM (MODULES * 2)
#define DISPLAY_WIDTH (MODULES * 8)

byte displayBuffer[8][MODULES];  // for the frame buffer (for more modules, Row0 will be [A][B][C][D][E])
#define ROTATE_90 true           // rotate screen 90 degrees (comment out if not needed)
#define SCROLLSPEED 20           // speed for scrolling text (shorter=faster) Default: 20

byte heart1[8] = {
  B00000000,  // the bit order is backwards here
  B00000000,  // so the image will be x-inverted
  B00000000,
  B00000000,
  B00111100,
  B00011000,
  B00000000,
  B00000000
};

byte heart2[8] = {
  B00000000,  // the bit order is backwards here
  B00000000,  // so the image will be x-inverted
  B01100110,
  B01011010,
  B01000010,
  B00100100,
  B00011000,
  B00000000
};

byte heart3[8] = {
  B00000000,  // the bit order is backwards here
  B01100110,  // so the image will be x-inverted
  B10011001,
  B10000001,
  B10000001,
  B01000010,
  B00100100,
  B00011000
};

byte skull[8] = {
  B01111100,
  B11111110,
  B10010010,
  B11111110,
  B11101110,
  B01111100,
  B01010100,
  B00000000
};

byte filled[8] = { // light all LEDs (diagnostic)
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111
};

byte happyface[8] = { 0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E };  //happy face

void setup() {
  //set shift register pins to OUTPUT mode
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  LEDMatrixClear();
  //Serial.begin(9600);
  randomSeed(analogRead(A0));  // seed random number generator
}

void loop() {
  // Fill all LEDs (diagnostic)
  //LEDshow_all(filled,1000);
  //delay(1000);

  // Test a character in context
  //char message0[] = "I'm your overlooked friend, the interrobang\x80\x80\x80";
  //LEDscrollPlay(message0, sizeof(message0), SCROLLSPEED);

  // Show all characters (diagnostic)
  /*for (int j = 0; j < LEDfontSize; j++) {
    LEDshow_all(LEDfont[j].bitmap, 250);
    //Serial.println("char: "+(String)LEDfont[j].key+" "+(String)font8Width(LEDfont[j].bitmap));
  }
  delay(1000);*/

  // Play sparkles
  LED_sparkles(displayBuffer, 8, MODULES, 5, 50, 3000);  // last number is # steps
  LEDMatrixClear();
  delay(1000);

  // Play hearts in all modules
  for (int i = 0; i < 5; i++) {
    LEDplayHearts_all(50); 
  }
  delay(1000);

  // Scroll message across multiple chips:
  char message[] = "Welcome Pharmacy Camp!!! ";  // remember to leave one extra space for string terminator
  LEDscrollPlay(message, sizeof(message), SCROLLSPEED);

  // Play beating hearts in separate modules:
  for (int i = 0; i < 3; i++) {
    LEDplayHearts(1, 20);        // play hearts in module 1
    LEDplayHearts(3, 20);        // play hearts in module 5
    LEDplayHearts(2, 20);        // play hearts in module 2
    LEDplayHearts(4, 20);        // play hearts in module 4
    LEDplayHearts(5, 20);        // play hearts in module 3
    LEDshow(happyface, 2, 250);  // play happy faces, staggered randomly
    LEDshow(happyface, 4, 250);
    LEDshow(happyface, 1, 250);
    LEDshow(happyface, 3, 250);
    LEDshow(happyface, 0, 250);
  }
  delay(1000);

  // Play scrolling message:
  char message2[] = "Leslie Dan Faculty of Pharmacy, University of Toronto";
  LEDscrollPlay(message2, sizeof(message2), SCROLLSPEED);

  // Play scrolling message
  char message3[] = "Remember to wear your PPE at all times: lab coat & safety glasses!";
  LEDscrollPlay(message3, sizeof(message3), SCROLLSPEED);

  // Play graphic (skulls):
  LEDplaySkulls_all(500);

  delay(1000);
}

void registerMultiplex(byte graphic[8][MODULES]) {  // this will take displayBuffer[8][MODULES]
  //Shifts out graphic[][]
  byte columnbyte = 0;
  for (int p = 0; p < 8; p++) {      // p is the row
    columnbyte = ~(B10000000 >> p);  // only turn on column "p" (e.g. B11101111 is column 4 ON, everything else off)
    digitalWrite(latchPin, LOW);
    for (int module = 0; module < MODULES; module++) {
      //for(int module = MODULES-1; module>=0; module--){   // to swap order of modules
      byte output;
#if ROTATE_90
      output = getRotatedByte(graphic, module, p);
#else
      output = graphic[p][module];
#endif
      shiftOut(dataPin, clockPin, LSBFIRST, output);
      shiftOut(dataPin, clockPin, LSBFIRST, columnbyte);
    }
    digitalWrite(latchPin, HIGH);  // Set the latch HIGH to trigger the bits shifting OUT:
  }
}

void LEDMatrixClear() {  // clear the LED screens
  reset_displayBuffer();
  registerMultiplex(displayBuffer);
}

void LEDshow(byte graphic[8], byte module, int wait) {  // show 8x8 graphic on one module (selected by "module")
  //Displays a single input graphic for "wait" msec then clears the screen (user friendly)
  reset_displayBuffer();  // reset the frame buffer (not needed?)
  //copy graphic to correct spot in displayBuffer:
  for (int row = 0; row < 8; row++) {  // p is the row
    displayBuffer[row][module] = graphic[row];
  }
  unsigned long timer = millis();
  while (millis() - timer < wait) {
    registerMultiplex(displayBuffer);
  }
  LEDMatrixClear();
}

void LEDshow_all(byte graphic[8], int wait) {
  //Displays a single input graphic for "wait" msec then clears the screen (user friendly)
  reset_displayBuffer();  // reset the frame buffer (not needed?)
  //copy graphic to correct spot in displayBuffer:
  for (int module = 0; module < MODULES; module++) {
    for (int row = 0; row < 8; row++) {  // p is the row
      displayBuffer[row][module] = graphic[row];
    }
  }
  long timer = millis();
  while (millis() - timer < wait) {
    registerMultiplex(displayBuffer);
  }
  LEDMatrixClear();
}

void LEDplayHearts(byte n, int wait) {
  LEDshow(heart1, n, wait);
  LEDshow(heart2, n, wait);
  LEDshow(heart3, n, wait);
  LEDshow(heart3, n, wait);
  LEDshow(heart2, n, wait);
  LEDshow(heart1, n, wait);
}

void LEDplayHearts_all(int wait) {
  LEDshow_all(heart1, wait);
  LEDshow_all(heart2, wait);
  LEDshow_all(heart3, wait);
  LEDshow_all(heart3, wait);
  LEDshow_all(heart2, wait);
  LEDshow_all(heart1, wait);
}

void LEDplaySkulls_all(int wait){
  for(int i=0;i<5;i++){
    LEDshow_all(skull, wait); 
    delay(wait);
  }
}

void reset_displayBuffer() {
  for (int row = 0; row < 8; row++) {  // initialize the displayBuffer
    for (int module = 0; module < MODULES; module++) {
      displayBuffer[row][module] = 0;
    }
  }
}

void addCharacterColumn(byte graphic[], byte column) {
  for (int row = 0; row < 8; row++) {
    byte pixel = (graphic[row] >> column) & 1;
    // put pixel into the incoming right edge
    if (pixel) {
      displayBuffer[row][0] |= 0x01;  // New pixels always enter at the right-hand edge of the virtual display,
    } else {                          // which corresponds to bit 0 of module 0 before shifting.
      displayBuffer[row][0] &= ~0x01;
    }
  }
}

void shiftDisplayLeft() {
  for (int row = 0; row < 8; row++) {
    for (int module = MODULES - 1; module >= 0; module--) {
      byte carry = 0;
      if (module > 0) {
        carry = displayBuffer[row][module - 1] & 0x80;
      }
      displayBuffer[row][module] <<= 1;
      if (carry) {
        displayBuffer[row][module] |= 0x01;
      }
    }
  }
}

void LEDscrollPlay(char msg[], int len, int duration) {
  reset_displayBuffer();
  for (int i = 0; i < DISPLAY_WIDTH; i++) {  // blank lead-in, once
    shiftDisplayLeft();
    long timer = millis();
    while (millis() - timer < duration) registerMultiplex(displayBuffer);
  }
  for (int j = 0; j < len; j++) {
    int idx = LEDlookup(msg[j]);
    if (idx >= 0) LEDscrollChar(LEDfont[idx].bitmap, idx, duration);
  }
  for (int i = 0; i < DISPLAY_WIDTH; i++) {  // blank exit
    shiftDisplayLeft();
    long timer = millis();
    while (millis() - timer < duration) registerMultiplex(displayBuffer);
  }
}

void LEDscrollChar(byte graphic[], byte ID, int wait) {
  for (int column = 7; column >= (7-font8Width(graphic)); column--) {
    shiftDisplayLeft();
    addCharacterColumn(graphic, column);
    long timer = millis();
    while (millis() - timer < wait) registerMultiplex(displayBuffer);
  }
}

byte getRotatedByte(byte graphic[8][MODULES], int module, int column) {
  byte result = 0;
  for (int row = 0; row < 8; row++) {
    if (graphic[row][module] & (1 << column)) {
      result |= (1 << (7 - row));
    }
  }
  return result;
}

// Latin Hypercube Sampling for a 2D Array of Bytes
// Algorithm: https://www.numberanalytics.com/blog/latin-hypercube-sampling-guide#sampling-algorithm
// Set seg = modules in this strategy for the algorithm to work.
void LatinHypercube_2D(byte graphic[8][MODULES], int rows, int cols, int n) {
  // First clear the array
  for (int row = 0; row < rows; row++) {  // initialize the displayBuffer
    for (int col = 0; col < cols; col++) {
      graphic[row][col] = 0;
    }
  }
  // Select n pixels randomly, n/seg of them in each segment.
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < cols; j++) {             // in a true latin hypercube sampling this would be segments
      int randrow = random(0, 8);                // random row number between 0 and 7
      graphic[randrow][j] |= 1 << random(0, 8);  // random shift between 0 and 7
    }
  }
}

// Make the LED screen sparkle
void LED_sparkles(byte graphic[8][MODULES], int rows, int cols, int n, int dur_step, int dur_total) {
  // rows: 8
  // cols: MODULES. We are using MODULES as our # segments in this algorithm, to work nicely with our array structure.
  // n: # random points to select in total (at least 1 per segment)
  // dur_step: duration of one step (use ~50 msec for glittery transitions)
  // dur_total: total time for this effect

  unsigned long timer = millis();
  int steps = dur_total / dur_step;  // calculate # steps
  for (int k = 0; k < steps; k++) {
    LatinHypercube_2D(displayBuffer, 8, MODULES, n);  // rows, cols, segments, #random points
    while (millis() - timer < dur_step) {
      registerMultiplex(displayBuffer);
    }
    timer = millis();  //reset the timer
  }
}

byte font8Width(byte graphic[]){ // calculate the width of the graphic. If a space (empty), return a width of 4.
  #define KERNING 1    // use this to change spacing between characters (default: 1)
  byte b=0;
  for(byte i=0;i<8;i++){
    b|=graphic[i]; // flatten graphic vertically
  }
  for(byte i=0;i<8;i++){
    if(b>>i&1){ // Check status of bit.
      return (7-i+KERNING);
    }
  }
  return 4; // if you made it this far, it's a space!
}
