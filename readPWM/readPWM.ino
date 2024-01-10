// readPWM.ino
// Author: D. Dubins
// Date: 09-Jan-24
// Libraries Required: None
// Description: This sketch sends a PWM signal on Pin 5, and reads the pulse width on Pin 3.
// Both are output to the Serial Monitor for comparison.
// Interrupts are used to measure pulse width. Since the width is calculated as a proportion,
// the calculation doesn't depend on the frequency of the signal.
// For the Arudino Uno, Pin 2 is int_0, Pin 3 is int_1.

// Wiring:
// Wire a pot as a voltage divider, and wire the wiper to pin A0
// Connect pin 3 to pin 5 to generate a PWM signal from pin 5, and read it from pin 3 using interrupts.
// This routine is ok,  but at the limits it kind of craps out (where there's no pulse). The lowest number I can get
// is 12. This could be taken care of in the software, but if there isn't a pulse this doesn't really matter, and
// the signal could be remapped to a useful range anyway.

byte pwmSend = 5;  //to send PWM signal
byte pwmRead = 3;  //to read PWM signal
byte potPin = A0;  //to read the wiper of a 10K pot, to generate PWM

volatile long timer0 = 0;   //last rising edge
volatile long timer1 = 0;   //rising edge
volatile long timer2 = 0;   //falling edge
volatile long width = 0;    //to store width of pulse

void setup() {
  Serial.begin(9600);
  attachInterrupt(1, risingISR, RISING);  // attach interrupt to pin 3 (int_1), on CHANGE. Use ISR called pwmISR.
  pinMode(pwmRead, INPUT);                // set the pinMode for pin 3
}

void loop() {
  int vPot = analogRead(potPin) / 4;  // take a reading from the potentiometer, rescale to 0-255.
  analogWrite(pwmSend, vPot);         // send a PWM signal to the pwmSend pin
  Serial.print(" PWM Sent: ");
  Serial.print(vPot);
  Serial.print(", PWM Read: ");
  Serial.println(width);
  delay(500);  // wait a bit
}

void fallingISR() {                        // This ISR will run with a falling signal on Pin 3 when attached.
  timer2 = micros();  // if falling edge
  attachInterrupt(1, risingISR, RISING);
}

void risingISR(){  // This ISR will run with a rising signal on Pin 3 when attached.
  timer0 = timer1;  // store last value of timer1 as timer0
  timer1 = micros();
  width = 256 * (timer2 - timer0) / (timer1 - timer0);  // gives a number from ~0-255
  attachInterrupt(1, fallingISR, FALLING); 
}
