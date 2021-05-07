/*
  Shift Register Example

 This sketch cycles through pins and keeps track of what the current state is so that you can
 independently turn on and off a specific shift pin without resetting the rest of them.

 Hardware:
 * 74HC165 shift in register attached to pins 8,9,10 of the Arduino, (ground un-used inputs)
 * 74HC595 shift out register attached to pins 4,5, and 6 of the Arduino,
 as detailed below.
 * LEDs attached to each of the outputs of the shift register
 
*/
const byte OutClockPin = 4; //Pin connected to clock pin (SH_CP) of 74HC595 (pin 11 on chip)
const byte OutLatchPin = 5; //Pin connected to latch pin (ST_CP) of 74HC595 (pin 12 on chip)
const byte OutDataPin = 6;  //Pin connected to Data in (DS) of 74HC595 (pin 14 on chip)
//OUTPUTS on 74HC595 are (in order) pins 15,1,2,3,4,5,6,7

const byte InLoadPin = 8; //Pin connected to parallel load input (pin 1) of 74HC165
const byte InDataPin = 9; //Pin connected to Q7, Serial out pin (pin 9) of 74HC165
const byte InClockPin = 10; //Pin connected to clock (pin 2) of 74HC165
const byte InCEPin = 11; // Pin connected to clock_enable (pin 15 of 74HC165)
//INPUTS on 74HC165 are (in order) pins 11,12,13,14,3,4,5,6

byte ShiftByte;  //Define a byte for the shift registers. Make an array if more than one chip used.

void setup() {
  //set ShiftOUT pins:
  pinMode(OutLatchPin, OUTPUT);
  pinMode(OutDataPin, OUTPUT);  
  pinMode(OutClockPin, OUTPUT);
  //set ShiftIN pins:
  pinMode(InLoadPin, OUTPUT);
  pinMode(InDataPin, INPUT);  // data pin set as input
  pinMode(InClockPin, OUTPUT);  
  pinMode(InCEPin, OUTPUT); 
  ShiftByte=0b0; 
  Serial.begin(9600);
}

void loop() {
  ShiftByte=AshiftIn();
  AshiftOut(ShiftByte);  
  Serial.println(ShiftByte,BIN);
  delay(50);
}

byte AshiftIn() {
  byte temp = 0b0; //An 8-bit number to carry each bit value
  digitalWrite(InLoadPin, LOW); 
  delayMicroseconds(5); // 5 microsecond delay required from datasheet timing diagram
  digitalWrite(InLoadPin, HIGH);
  delayMicroseconds(5); // 5 microsecond delay required from datasheet timing diagram
  digitalWrite(InClockPin,HIGH);    //Pulse the clock to get the next bit 
  digitalWrite(InCEPin,LOW);    //Enable the clock 
  temp = shiftIn(InDataPin,InClockPin,MSBFIRST);
  digitalWrite(InCEPin,HIGH);    // Disable the clock
  return temp;
}

void AshiftOut(byte temp) {   
   digitalWrite(OutLatchPin, LOW);
   //Shift out in reverse order (2nd digit minute, 1st digit minute, 2nd digit hour, 1st digit hour)
   shiftOut(OutDataPin, OutClockPin, MSBFIRST, temp);
   digitalWrite(OutLatchPin, HIGH);
}
 
