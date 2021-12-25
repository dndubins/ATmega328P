/* Matrix keypad example (4x4 matrix keypad), SLAVE (no USB connection to computer, connected to MASTER through mySerial.)
 * Author: D. Dubins
 * Date: 24-Dec-21
 * Libraries: SendOnlySoftwareSerial.h v1.0 by Nick Gammon, available at: https://github.com/nickgammon/SendOnlySoftwareSerial

 * Description: This project illustrates how to get one MCU to send messages to another through a SoftwareSerial connection.
 * A matrix keypad is connected to each MCU. The MASTER reads its own keypad and sends the response to the regular Serial Monitor.
 * The SLAVE reads its own keypad, and sends the response to the Software Serial Monitor (mySerial).
 * The MASTER receives this message, and then sends the response to the regular Serial Monitor.
 * 
 * Since all the MASTER is doing is receiving data, and all the SLAVE is doing is sending serial data, only one wire is needed,
 * and the half-libraries are used (SoftwareSerialIn.h for the MASTER, and SendOnlySoftwareSerial.h for the SLAVE).
 *
 * This sketch is for the SLAVE MCU (connected to the MASTER).
 * The SLAVE MCU is powered parasitically through the MASTER (Vin - Vin, GND - GND).

 * Connections: 
 * Keypad row pins 1-4 to MCU pins 4-7
 * Kepad col pins 5-8 to MCU pins 8-11
 * Master Uno - Slave Uno:
 * Vin - Vin
 * GND - GND
 * Pin 2 - Pin 2 (RX - TX)
*/

#include <SendOnlySoftwareSerial.h> // include the SendOnlySoftwareSerial library
SendOnlySoftwareSerial mySerial(2); // define software serial connection TX:2

#define ROWS 4 //#rows on matrix keypad
#define COLS 4 //#cols on matrix keypad
// Connections:
// Keypad row pins 1-4 to MCU pins 4-7
// Kepad col pins 5-8 to MCU pins 8-11
byte rowPins[ROWS] = {4, 5, 6, 7}; 
byte colPins[COLS] = {8, 9, 10, 11}; 

char keys[ROWS][COLS] = {  //define keypad symbols
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

void setup(){
  //Serial.begin(57600); // start serial connection (for debugging)
  mySerial.begin(57600); // start software serial connection (max speed for SoftwareSerial is 57600 for Uno)
}

void loop(){
  char readKey=getKey();
  if(readKey!=0){
    //Serial.println(readKey); // print to regular serial monitor (for debug, comment out once keypad works)
    mySerial.write(readKey); // print to software serial monitor
    delay(100); //short debounce
    while(getKey()!=0); // wait until not pushed
    delay(100); // short debounce
  }
}

char getKey(){
  for(int i=0;i<ROWS;i++){
    pinMode(rowPins[i],INPUT_PULLUP);
    for(int j=0;j<COLS;j++){
      pinMode(colPins[j],OUTPUT);
      digitalWrite(colPins[j],LOW);
      if(digitalRead(rowPins[i])==LOW){
        return keys[i][j];
      }
      pinMode(colPins[j],INPUT);
    }
  }
  return 0; // if no key pushed, return null char.
}
