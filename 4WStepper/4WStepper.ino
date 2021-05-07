// Generic 4-Wire Stepper Motor Control
#include <Stepper.h>   // Arduino IDE built-in library
const byte stepsPerRev = 200; // change as needed for motor
// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRev, 8, 9, 10, 11);

void setup(){
  myStepper.setSpeed(60);  // set speed to 60 rpm
  Serial.begin(9600);
}

void loop(){
  myStepper.step(200); // clockwise one rotation
  delay(1000);
  myStepper.step(-200); // counterclockwise one rotation
  delay(1000);
}
