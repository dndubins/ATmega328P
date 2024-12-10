/* SoftwareSerial example, MCU1 to MCU2. This sketch is for MCU1 - the master, sending commands to MCU2.
 * Author: D. Dubins
 * Date: 10-Dec-24

 * Description: This project illustrates how to get one MCU to send commands to another through a SoftwareSerial connection.
 * This sketch is for the first MCU (MCU1). The second MCU (MCU2) will be powered parasitically through the Vin and GND pins of MCU1.
 * This is hookup is optional though, they can be independently powered.
 * 
 * MCU1 reads an integer from the Serial Monitor, and sends a command to MCU2 through the SoftwareSerial connection.
 * MCU2 receives the command, executes a function, then reports back to MCU1.

 * Connections: (MCU1 - MCU2)
 * Vin - Vin (or power independently)
 * GND - GND
 * Pin 2 - Pin 3 (RX - TX)
 * Pin 3 - Pin 2 (TX - RX)
*/

#include <SoftwareSerial.h>       // include the software serial library
SoftwareSerial MCU2Serial(2, 3);  // define software serial connection to MCU2 (RX:2, TX:3)

void setup() {
  Serial.begin(57600);                                         // start serial connection
  MCU2Serial.begin(57600);                                     // start software serial connection with MCU2 (max speed for SoftwareSerial is 57600 for Uno)
  Serial.println("Enter number of flashes to send to MCU2:");  // ask user to enter a number
}

void loop() {
  int snd, rcv;               // define bytes for sending and receiving data
  if (Serial.available()) {   // if user has entered something on the Serial Monitor
    snd = Serial.parseInt();  // store it to snd
    MCU2Serial.print(snd);    // send snd to MCU2
  }
  if (MCU2Serial.available()) {             // listen for response from MCU2Serial
    rcv = MCU2Serial.parseInt();            // store it to rcv
    Serial.print("Receiving from MCU2: ");  // prompt user
    Serial.println(rcv);                    // send rcv to the regular Serial Monitor
  }
}
