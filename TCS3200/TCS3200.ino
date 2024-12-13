/* TCS3200 color recognition sensor 
 * This sketch spits out single reads from each channel of the TCS3200 (red, green, blue),
 * and it also has a function for multiple reads with data averaging.
 * Author: D. Dubins
 * Date: 13-Dec-24
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
#define NUMREADS 1000 // number of readings for data averaging

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
  Serial.print("One reading: ");
  readColour();              // take one colour reading
  Serial.print(reading[0]);  // output red channel
  Serial.print(",");
  Serial.print(reading[1]);  // output green channel
  Serial.print(",");
  Serial.println(reading[2]);  // output blue channel
  delay(500);
  readColourN(reading, 1000);
  Serial.print(NUMREADS);
  Serial.print(" readings: ");
  Serial.print(reading[0]);  // output red channel
  Serial.print(",");
  Serial.print(reading[1]);  // output green channel
  Serial.print(",");
  Serial.println(reading[2]);  // output blue channel
  delay(500);
}

// This is a simple function that takes only one reading to the global array reading[].
void readColour() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(100);                       // wait for reading to stabilize
  reading[0] = pulseIn(OUT, LOW);  //read red
  delay(100);                       // wait for reading to stabilize
  digitalWrite(S3, HIGH);
  reading[2] = pulseIn(OUT, LOW);  //read blue
  digitalWrite(S2, HIGH);
  delay(100);                       // wait for reading to stabilize
  reading[1] = pulseIn(OUT, LOW);  //read green
}

// This function takes an array as an input agument, and calculates the average 
// of n readings on each colour channel.
void readColourN(int colourArr[3], int n) {  // arrays are always passed by value
  unsigned long thisRead[3] = { 0, 0, 0 };   // for data averaging
  bool pinStates[3][2] = {
    { LOW, LOW },    // S2,S3 are LOW for RED
    { HIGH, HIGH },  // S2,S3 are HIGH for GREEN
    { LOW, HIGH }    // S2=LOW,S3=HIGH for BLUE
  };
  for (int i = 0; i < 3; i++) {          //i=0: red, i=1: green, i=2: blue
    digitalWrite(S2, pinStates[i][0]);   // set S2 to correct pin state
    digitalWrite(S3, pinStates[i][1]);   // set S3 to correct pin state
    thisRead[i] = 0;                     //initialize colour
    delay(100);                          // wait for reading to stabilize
    for (int j = 0; j < n; j++) {        // collect n readings on channel i
      thisRead[i] += pulseIn(OUT, LOW);  //read colour (iterative mean)
    }
    thisRead[i] /= n;            // report the average
    colourArr[i] = thisRead[i];  //write back to colourArr
  }
}
