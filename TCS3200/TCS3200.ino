/* TCS3200 color recognition sensor 
 * This sketch spits out single reads from each channel of the TCS3200 (red, green, blue).
 * Sketch adapted from: https://electronicsforu.com/electronics-projects/rgb-color-detector-tcs3200-sensor-module
 * http://www.efymag.com/admin/issuepdf/RGB-Colour-Detection-Using-TCS3200_3-17.rar

Color Sensor - Arduino Uno
---------------------------
 VCC -- +5V
 GND -- GND
 S0 -- Pin 8
 S1 -- Pin 9
 S2 -- Pin 12
 S3 -- Pin 11
 OUT -- Pin 10
 OE -- GND
*/

#define S0 8  // declare pin numbers
#define S1 9
#define S2 12
#define S3 11
#define OUT 10

int reading[3] = { 0, 0, 0 };  // to store red, green, blue reading

void setup() {
  Serial.begin(9600);   // start the Serial Monitor
  pinMode(S0, OUTPUT);  // set pins to OUTPUT mode
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);     // set OUT pin to INPUT mode
  digitalWrite(S0, HIGH);  // set on LEDs on front to HIGH
  digitalWrite(S1, HIGH);  // (setting these both low turns LEDs off)
  Serial.println("R,G,B: ");
}

void loop() {
  readColour();  // take one colour reading
  Serial.print(reading[0]);   // output red channel
  Serial.print(",");
  Serial.print(reading[1]);   // output green channel
  Serial.print(",");
  Serial.println(reading[2]); // output blue channel
  delay(500);
}

void readColour() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(20);                                     // wait for reading to stabilize
  reading[0] = pulseIn(OUT, !digitalRead(OUT));  //read red
  delay(20);                                     // wait for reading to stabilize
  digitalWrite(S3, HIGH);
  reading[2] = pulseIn(OUT, !digitalRead(OUT));  //read blue
  digitalWrite(S2, HIGH);
  delay(20);                                     // wait for reading to stabilize
  reading[1] = pulseIn(OUT, !digitalRead(OUT));  //read green
}
