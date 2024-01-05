#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

byte buttonPin=2;     //push button - goes LOW to wake up (PD2 will be interrupt, which is PCINT0) - PD2 is physical pin 4 on ATmega328 - FIX THIS ROUTINE

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin,INPUT_PULLUP);
}
 
void loop() {
  Serial.println("Going to sleep to save power.");
  sleep();  // run the sleep routine
  Serial.println("Ready to do your bidding!!");
  delay(1000);
}

void sleep() {
  // Two pin options for ATmega328: 0 (Pin 2) or 1 (Pin 3)
  attachInterrupt(0, onWake, LOW);
  delay(100); // warm-up time
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode(); // processor will sleep after this command.
  sleep_disable(); // enable interrupts
  detachInterrupt(0); // detach interrupt 0 during awake period 
}

void onWake()
{
  // Commands could go here, but if MCU just needs to wake up,
  // leave empty.
}
