// HC-SR04.ino
// Sketch to take a distance reading from the HC-SR04 device
// Source: https://pimylifeup.com/arduino-distance-sensor-hc-sr04/
// Author: D. Dubins, 22-Feb-23
//
// Wiring:
// HC-SR04 - Uno:
// --------------
// GND - GND
// Echo - Pin 3
// Trig - Pin 2
// Vcc - +5V

#define TRIG 2 // pin 2 is trigger
#define ECHO 3 // pin 3 is echo

void setup() {
  Serial.begin(9600); // start the serial monitor
  pinMode(TRIG,OUTPUT); // set TRIG to output mode
  pinMode(ECHO,INPUT);  // set ECHO to input mode
  digitalWrite(TRIG,LOW); // set TRIG low
}

void loop() {
  float reading_cm = getDist_cm(TRIG,ECHO);
  float reading_in = getDist_in(TRIG,ECHO);
  Serial.print(reading_cm);
  Serial.print(", ");
  Serial.println(reading_in);
  delay(100);
}

float getDist_cm(byte tPin, byte ePin){
  digitalWrite(tPin, HIGH); // set trigger pin high
  delayMicroseconds(10);    // wait 10 microseconds
  digitalWrite(tPin, LOW);  // set trigger pin low
  unsigned long dur = pulseIn(ePin, HIGH, 3000); // get duration, timeout = 3000 usec
  float dist = dur*0.01715; // calculate distance in cm
  return dist;
}

// Function for distance in inches:
float getDist_in(byte tPin, byte ePin){ 
  digitalWrite(tPin, HIGH); // set trigger pin high
  delayMicroseconds(10);    // wait 10 microseconds
  digitalWrite(tPin, LOW);  // set trigger pin low
  unsigned long dur = pulseIn(ePin, HIGH, 24000); // get duration, timeout = 24000 usec
  float dist = round(dur*0.006752); // calculate distance in inches
  return dist;
}