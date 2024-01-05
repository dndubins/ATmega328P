// Example: Wake on button push
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
byte buttonPin=2;

void setup(){
  Serial.begin(9600);
  pinMode(buttonPin,INPUT_PULLUP);
}
 
void loop(){
  Serial.println("Sleeping until button push.");
  sleep();  // run the sleep routine
  Serial.println("Ready to do your bidding!");
  delay(1000);
}

void sleep(){
  // Two options for ATmega328: 0 (Pin 2) or 1 (Pin 3)
  attachInterrupt(0, onWake, FALLING);
  delay(100); // allow serial commands to finish
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  /* set_sleep_mode() options:
     SLEEP_MODE_IDLE (least power savings)
     SLEEP_MODE_ADC
     SLEEP_MODE_PWR_SAVE
     SLEEP_MODE_STANDBY
     SLEEP_MODE_PWR_DOWN (most power savings) */
  sleep_enable();
  sleep_mode(); // MCU will sleep here
  sleep_disable(); // MCU will wake up here
  detachInterrupt(0); // detach INT0 while awake 
}

void onWake(){ // ISR
  // Commands could go here. If MCU just needs to
  // wake up, leave this empty.
}
