/* SoftwareSerial example, MCU1 to MCU2. This sketch is for MCU1.
 * Author: D. Dubins
 * Date: 25-Dec-21

 * Description: This project illustrates how to get one MCU to send messages to another through a SoftwareSerial connection.
 * This sketch is for the first MCU (MCU1). The second MCU (MCU2) will be powered parasitically through the Vin and GND pins of MCU1. 
 * This is hookup is optional though, they can be independently powered.
 * 
 * Each MCU receives an integer from the other MCU, and flashes that number of times + 1.

 * Connections: (MCU1 - MCU2)
 * Vin - Vin (or power independently)
 * GND - GND (or power independently)
 * Pin 2 - Pin 3 (RX - TX)
 * Pin 3 - Pin 2 (TX - RX)
*/

#include <SoftwareSerial.h> // include the software serial library
SoftwareSerial mySerial(2,3); // define software serial connection RX:2, TX:3

#define LEDPIN 13

void setup(){
  Serial.begin(57600); // start serial connection
  mySerial.begin(57600); // start software serial connection (max speed for SoftwareSerial is 57600 for Uno)
  pinMode(LEDPIN,OUTPUT); // set LEDPIN to OUTPUT mode
  delay(1000);            // wait for both MCUs to boot up
  Serial.println(1);      // send 1 to the Serial Monitor
  flashLED(1);            // flash onboard LED once
  mySerial.print(1);      // To start the ball rolling...
}

void loop(){
  if(mySerial.available()){    // if there's something received by mySerial
    int i=mySerial.parseInt(); // store it to readKey
    Serial.println(i);         // send i to the regular Serial Monitor
    flashLED(i);               // flash onboard LED i times
    mySerial.print(i+1);       // send i+1 to the other MCU
  }
}

void flashLED(int n){ // flash the LED n times
  for(int i=0;i<n;i++){
    digitalWrite(LEDPIN,HIGH); // turn on LED
    delay(200);                // wait a bit
    digitalWrite(LEDPIN,LOW);  // turn off LED
    delay(200);                // wait a bit
  }
}
