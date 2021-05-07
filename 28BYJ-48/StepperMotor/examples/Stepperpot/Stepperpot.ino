/* StepperMotor.h Library: Potentiometer control for 28BYJ-48 5V DC Stepper Motor 
 *  with ULN2003 Driver
 *  
 * Wire ULN2003 to Arduino Uno:
 * ----------------------------
 * Blue wire (A) - IN1 to Pin 8
 * Pink Wire (B) - IN2 to Pin 9 
 * Yellow Wire (C) IN3 to Pin 10
 * Orange Wire (D) IN4 to Pin 11
 * - to GND
 * + to +5V
 */

const byte potPin=A0;
int v=0;
int last=0;

#include <StepperMotor.h>

StepperMotor motor(8,9,10,11);

void setup(){
  motor.setStepDuration(1);  // stepper motor speed (fastest:1, larger=longer)
}

void loop(){
  v=5*analogRead(potPin);
  motor.step(v-last);
  last=v;
  delay(1000);
}
