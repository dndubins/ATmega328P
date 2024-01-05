#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

void setup(){
  Serial.begin(9600);
  configure_wdt(); // configure watchdog timer
}

void loop(){
  Serial.println("Sleeping for 5 seconds.");
  sleep(5);  // sleep for 5 seconds
  Serial.println("Ready to do your bidding!");
}
ISR(WDT_vect){ // define the watchdog ISR
  wdt_reset(); // reset watchdog timer
}

void configure_wdt(){
  cli(); // clear (disable) interrupts
  MCUSR=0; // the following are ATmega328-specific cmds
  WDTCSR |= 0b00011000; // set WDCE and WDE high
  //uncomment for your desired timeout:  
  //WDTCSR = 0b01000000 | 0b000000; //16 msec timeout
  //WDTCSR = 0b01000000 | 0b000100; //0.25 sec
  //WDTCSR = 0b01000000 | 0b000101; //0.5 sec
  WDTCSR = 0b01000000 | 0b000110;   //1 sec
  //WDTCSR = 0b01000000 | 0b000111; //2 sec
  //WDTCSR = 0b01000000 | 0b100000; //4 sec
  //WDTCSR = 0b01000000 | 0b100001; //8 sec
  //From: Table 8-2, Watchdog Timer Prescale Select,
  //ATmega328 datasheet
  sei(); // set (re-enable) interrupts
}

void sleep(unsigned long n){
  delay(100); // wait for serial commands to finish
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //max power down
  power_adc_disable(); // turn off adc
  for(int i=0;i<n;i++){
    sleep_mode(); // go to sleep here
    sleep_disable(); // wake up here
  }
  power_all_enable();
}
