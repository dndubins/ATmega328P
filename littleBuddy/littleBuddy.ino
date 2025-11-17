// littleBuddy.ino "Little Buddy Talker" Module Example Sketch (Pishop.ca)
// Example program for chat module. Plays single words and sentences.
// Author: D. Dubins
// Date: 13-Nov-25
// Adapted from: lbt_sentences.ino
// Downloaded from: http://www.engineeringshock.com/the-little-buddy-talker-project-page.html
//
// Wiring:
// -------
// Arduino Uno R3 - Little Buddy:
// Pin 13 - SC
// Pin 11 - D1
// Pin 10 (CS) To S1 on AP23
// 5V - 5V
// GND - GND
// Word table is at the bottom of this file.

#include <SPI.h>        // Include SPI library
#include "lbt_words.h"  // Include word library

byte CS = 10;  // Chip-select pin for SPI

void setup() {
  SPI.begin();                           // Initialize SPI
  SPI.setClockDivider(SPI_CLOCK_DIV32);  // low frequency SPI
  pinMode(CS, OUTPUT);                   // Chip select pins is an output
  digitalWrite(CS, HIGH);                // Set chip select to be HIGH (5v) by default.  The chip on the shield is selected when this line is brought low.
  SPI.setBitOrder(MSBFIRST);             // OTP requires MSB first
  SPI.setDataMode(SPI_MODE0);            // Use MODE0
  delay(1000);                           // One second delay
  // Ramp up LITTLE BUDDY TALKER
  digitalWrite(CS, LOW);
  SPI.transfer(0xA8);  //COUT ramp up
  SPI.transfer(0x00);
  digitalWrite(CS, HIGH);
}

void loop() {
  // One word at a time:
  readWord(W_SECURITY, 650);                                      // read "security", with 650 msec delay (from start of word)
  readWord(W_ALARM, 500);                                         // read "alarm" with 500 msec delay (from start of word)
  readWord(W_MESSAGE, 400);                                       // read "message" with 400 msec delay (from start of word)
  delay(1000);                                                    // wait a second
  // Declare sentence_words[] as array of bytes, using define statements in LBT_words.h:
  byte sentence_words[] = { W_WARNING, W_INTRUDER, W_DETECTED };  // "warning intruder detected" (using LBT_words.h)
  readSentence(sentence_words, 3);                                // read sentence_words (3 words long)

  // bundle readWords in a void function, and play them here:
  readCustomSentence();                                           // custom message "hello how are you" in a void function

  // Declare sentence_bytes[] as array of bytes:
  byte sentence_bytes[] = { 0x0a, 0x0d, 0xe8 };                   // "activate destruct sequence" (using byte table below)
  readSentence(sentence_bytes, 3);                                // read second sentence_bytes (3 words long)

  // Uncomment to read all words:
  //for(byte i=0;i<256;i++){
  //  readWord(i,700);
  //}
}

void readWord(byte w, int wait) {
  delay(7);
  // Transmit Data
  digitalWrite(CS, LOW);   // Select Little Buddy Talker
  SPI.transfer(0x98);      // Write play command (0x98)
  SPI.transfer(w);         // Send word address and play word
  digitalWrite(CS, HIGH);  // Unselect Little Buddy Talker
  delay(wait);             // Delay starts at beginning of word. Account for word length here.
}

void readSentence(byte words[], int n) {
  for (int i = 0; i < n; i++) {
    readWord(words[i], 700);
  }
  delay(1000);
}

// To read a custom sentence.
// Modify delay times as needed. 700 msec works for most words.
void readCustomSentence() {
  readWord(W_HELLO, 800); // read "hello", with 700 msec delay (from start of word)
  readWord(W_HOW, 400); // read "how", with 700 msec delay (from start of word)
  readWord(W_ARE, 400); // read "are", with 600 msec delay (from start of word)
  readWord(W_YOU, 500); // read "you", with 600 msec delay (from start of word)
  delay(1000);
}

/*
Word List for Little Buddy Talker
---------------------------------
Generated from: http://www.engineeringshock.com/uploads/1/2/5/2/12523657/library.xlsx

Address  Word        Category
-------  ----        --------
0x00     colour      Colours
0x01     black       Colours
0x02     white       Colours
0x03     blue        Colours
0x04     green       Colours
0x05     orange      Colours
0x06     red         Colours
0x07     yellow      Colours
0x08     purple      Colours
0x09     abort       Command
0x0a     activate    Command
0x0b     begin       Command
0x0c     deactivate  Command
0x0d     destruct    Command
0x0e     go          Command
0x0f     move        Command
0x10     off         Command
0x11     on          Command
0x12     open        Command
0x13     pull        Command
0x14     push        Command
0x15     reset       Command
0x16     run         Command
0x17     stop        Command
0x18     turn        Command
0x19     January     Months/Days/Time
0x1a     February    Months/Days/Time
0x1b     March       Months/Days/Time
0x1c     April       Months/Days/Time
0x1d     May         Months/Days/Time
0x1e     June        Months/Days/Time
0x1f     July        Months/Days/Time
0x20     August      Months/Days/Time
0x21     September   Months/Days/Time
0x22     October     Months/Days/Time
0x23     November    Months/Days/Time
0x24     December    Months/Days/Time
0x25     Monday      Months/Days/Time
0x26     Tuesday     Months/Days/Time
0x27     Wednesday   Months/Days/Time
0x28     Thursday    Months/Days/Time
0x29     Friday      Months/Days/Time
0x2a     Saturday    Months/Days/Time
0x2b     Sunday      Months/Days/Time
0x2c     A.M.        Months/Days/Time
0x2d     P.M.        Months/Days/Time
0x2e     date        Months/Days/Time
0x2f     day         Months/Days/Time
0x30     hours       Months/Days/Time
0x31     month       Months/Days/Time
0x32     o'clock     Months/Days/Time
0x33     time        Months/Days/Time
0x34     week        Months/Days/Time
0x35     year        Months/Days/Time
0x36     zero        Numbers
0x37     one         Numbers
0x38     two         Numbers
0x39     three       Numbers
0x3a     four        Numbers
0x3b     five        Numbers
0x3c     six         Numbers
0x3d     seven       Numbers
0x3e     eight       Numbers
0x3f     nine        Numbers
0x40     ten         Numbers
0x41     eleven      Numbers
0x42     twelve      Numbers
0x43     thirteen    Numbers
0x44     fourteen    Numbers
0x45     fifteen     Numbers
0x46     sixteen     Numbers
0x47     seventeen   Numbers
0x48     eighteen    Numbers
0x49     nineteen    Numbers
0x4a     twenty      Numbers
0x4b     thirty      Numbers
0x4c     forty       Numbers
0x4d     fifty       Numbers
0x4e     sixty       Numbers
0x4f     seventy     Numbers
0x50     eighty      Numbers
0x51     ninety      Numbers
0x52     hundred     Numbers
0x53     thousand    Numbers
0x54     million     Numbers
0x55     north       Directions
0x56     east        Directions
0x57     south       Directions
0x58     west        Directions
0x59     up          Directions
0x5a     down        Directions
0x5b     left        Directions
0x5c     right       Directions
0x5d     backward    Directions
0x5e     forward     Directions
0x5f     happy       Feelings
0x60     sad         Feelings
0x61     angry       Feelings
0x62     AC          Measurement
0x63     amps        Measurement
0x64     celsius     Measurement
0x65     centi       Measurement
0x66     cubic       Measurement
0x67     DC          Measurement
0x68     degrees     Measurement
0x69     diameter    Measurement
0x6a     fahrenheit  Measurement
0x6b     farads      Measurement
0x6c     feet        Measurement
0x6d     frequency   Measurement
0x6e     giga        Measurement
0x6f     gram        Measurement
0x70     height      Measurement
0x71     hertz       Measurement
0x72     humidity    Measurement
0x73     inches      Measurement
0x74     kilo        Measurement
0x75     length      Measurement
0x76     light       Measurement
0x77     litre       Measurement
0x78     mega        Measurement
0x79     meters      Measurement
0x7a     micro       Measurement
0x7b     milli       Measurement
0x7c     minutes     Measurement
0x7d     nano        Measurement
0x7e     Newton      Measurement
0x7f     night       Measurement
0x80     ohms        Measurement
0x81     per         Measurement
0x82     pico        Measurement
0x83     pitch       Measurement
0x84     pounds      Measurement
0x85     radius      Measurement
0x86     rate        Measurement
0x87     seconds     Measurement
0x88     sound       Measurement
0x89     speed       Measurement
0x8a     temperature Measurement
0x8b     volts       Measurement
0x8c     watts       Measurement
0x8d     weight      Measurement
0x8e     alarm       Security
0x8f     alert       Security
0x90     detected    Security
0x91     intruder    Security
0x92     security    Security
0x93     system      Security
0x94     warning     Security
0x95     clockwise   Math
0x96     counter     Math
0x97     divide      Math
0x98     equals      Math
0x99     minus       Math
0x9a     multiply    Math
0x9b     not         Math
0x9c     plus        Math
0x9d     square root Math
0x9e     a           Words General
0x9f     ahead       Words General
0xa0     air         Words General
0xa1     altitude    Words General
0xa2     an          Words General
0xa3     am          Words General
0xa4     and         Words General
0xa5     are         Words General
0xa6     area        Words General
0xa7     at          Words General
0xa8     axis        Words General
0xa9     back        Words General
0xaa     be          Words General
0xab     bearing     Words General
0xac     been        Words General
0xad     button      Words General
0xae     by          Words General
0xaf     can         Words General
0xb0     caution     Words General
0xb1     change      Words General
0xb2     check       Words General
0xb3     closed      Words General
0xb4     condition   Words General
0xb5     contact     Words General
0xb6     critical    Words General
0xb7     door        Words General
0xb8     empty       Words General
0xb9     end         Words General
0xba     environment Words General
0xbb     falling     Words General
0xbc     fast        Words General
0xbd     fatal       Words General
0xbe     feel        Words General
0xbf     first       Words General
0xc0     from        Words General
0xc1     front       Words General
0xc2     going       Words General
0xc3     goodbye     Words General
0xc4     he          Words General
0xc5     hello       Words General
0xc6     high        Words General
0xc7     how         Words General
0xc8     I           Words General
0xc9     in          Words General
0xca     incoming    Words General
0xcb     ing         Words General
0xcc     inside      Words General
0xcd     is          Words General
0xce     it          Words General
0xcf     laser       Words General
0xd0     last        Words General
0xd1     level       Words General
0xd2     locked      Words General
0xd3     low         Words General
0xd4     me          Words General
0xd5     message     Words General
0xd6     mode        Words General
0xd7     motor       Words General
0xd8     new         Words General
0xd9     next        Words General
0xda     nominal     Words General
0xdb     object      Words General
0xdc     obstacle    Words General
0xdd     of          Words General
0xde     out         Words General
0xdf     outside     Words General
0xe0     press       Words General
0xe1     process     Words General
0xe2     purge       Words General
0xe3     range       Words General
0xe4     rear        Words General
0xe5     received    Words General
0xe6     rising      Words General
0xe7     sent        Words General
0xe8     sequence    Words General
0xe9     she         Words General
0xea     sleep       Words General
0xeb     slow        Words General
0xec     stable      Words General
0xed     start       Words General
0xee     step        Words General
0xef     stepper     Words General
0xf0     switch      Words General
0xf1     tell        Words General
0xf2     the         Words General
0xf3     there       Words General
0xf4     they        Words General
0xf5     to          Words General
0xf6     total       Words General
0xf7     vent        Words General
0xf8     warning     Words General
0xf9     we          Words General
0xfa     will        Words General
0xfb     window      Words General
0xfc     you         Words General
0xfd     zone        Words General
*/
