// OPB980P51.ino IR Limit Switch (Break Beam)
// Photologic Slotted Optical Switch
// Limit Switch Test Sketch: Testing the OPB9980P51
// Datasheet available at:
// https://www.ttelectronics.com/TTElectronics/media/ProductFiles/Datasheet/OPB960-990.pdf
// Author: D. Dubins
// Date: 22-Oct-26
// Libraries used: None
// Wiring:
// Facing you:
//  ____              _____
// |    |            |     |
// |    |            |     |
// |    |            |     |
// |____|            |_____|
//  |  |              | | |
//  1  2              3 4 5
// 1: IR Anode
// 2: IR Cathode
// Use 330R Resistor:   
// +5V -- 330R -- 1, 2 -- GND

// 3: Vcc - +5V
// 4: Output - Pin 7
// 5: GND - GND

const byte readPin=7;

void setup() {
  Serial.begin(9600); // start the Serial Monitor at 9600 bps
  pinMode(readPin,INPUT); // set readPin to output mode
}

void loop() {
  bool state=digitalRead(readPin); // read the state of readPin 
  Serial.println(state); // print to Serial Monitor
  delay(100); // short delay
}
