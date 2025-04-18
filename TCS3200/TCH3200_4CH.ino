/* TCH3200_4CH.ino
 * TCS3200 color recognition sensor 
 * This sketch spits out single reads from each channel of the TCS3200, including clear (RED, GREEN, BLUE, CLEAR),
 * and it also has a function for multiple reads with data averaging.
 *
 * Author: D. Dubins
 * Date: 19-Dec-24
 * Sketch adapted from: https://electronicsforu.com/electronics-projects/rgb-color-detector-tcs3200-sensor-module
 * http://www.efymag.com/admin/issuepdf/RGB-Colour-Detection-Using-TCS3200_3-17.rar
 * There are three functions presented here:
 * readColour() gives you one raw reading from the TCS3200.
 * readColourN() gives you an average of N readings.
 *
 * Connections:
 * TCS3200 - Arduino Uno
 * ---------------------
 *  VCC - 5V
 *  OUT - 8
 *  S2  - 9
 *  S3  - 10
 *  S0  - 11
 *  S1  - 12
 *  OE  - GND
 *  GND - GND
*/

// Colour sensor module pins and setup
#define S0 11
#define S1 12
#define S2 9
#define S3 10
#define OUT 8

// For reading data
#define NUMREADS 1000             // number of readings for data averaging
int reading[4] = { 0, 0, 0, 0 };  // to store RED, GREEN, BLUE, CLEAR reading

void setup() {
  Serial.begin(9600);   // start the Serial Monitor
  pinMode(S0, OUTPUT);  // set pins to OUTPUT mode
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);     // set OUT pin to INPUT mode
  digitalWrite(S0, HIGH);  // set on LEDs on front to HIGH
  digitalWrite(S1, HIGH);  // (setting S0 & S1 LOW turns LEDs off)
  Serial.println("R,G,B,CL: ");
}

void loop() {
  Serial.print("One reading: ");
  readColour();              // take one colour reading
  Serial.print(reading[0]);  // output RED channel
  Serial.print(",");
  Serial.print(reading[1]);  // output GREEN channel
  Serial.print(",");
  Serial.print(reading[2]);  // output BLUE channel
  Serial.print(",");
  Serial.println(reading[3]);  // output CLEAR channel
  delay(500);
  readColourN(reading, 1000);  // take n colour readings
  Serial.print(NUMREADS);
  Serial.print(" readings: ");
  Serial.print(reading[0]);  // output RED channel
  Serial.print(",");
  Serial.print(reading[1]);  // output GREEN channel
  Serial.print(",");
  Serial.print(reading[2]);  // output BLUE channel
  Serial.print(",");
  Serial.println(reading[3]);  // output CLEAR channel
  delay(500);
}

// This is a simple function that takes only one reading to the global array reading[].
void readColour() {
  digitalWrite(S2, LOW);  // S2,S3 are LOW for RED
  digitalWrite(S3, LOW);
  delay(100);                      // wait for reading to stabilize
  reading[0] = pulseIn(OUT, LOW);  // read RED channel
  delay(100);                      // wait for reading to stabilize
  digitalWrite(S2, HIGH);          // S2,S3 are HIGH for GREEN
  digitalWrite(S3, HIGH);
  reading[1] = pulseIn(OUT, LOW);  // read GREEN channel
  digitalWrite(S2, LOW);           // S2=LOW,S3=HIGH for BLUE
  digitalWrite(S3, HIGH);
  delay(100);                      // wait for reading to stabilize
  reading[2] = pulseIn(OUT, LOW);  // read BLUE channel
  digitalWrite(S2, HIGH);          // S2=HIGH,S3=LOW for CLEAR (no filter)
  digitalWrite(S3, LOW);
  delay(100);                      // wait for reading to stabilize
  reading[3] = pulseIn(OUT, LOW);  // read CLEAR channel
}

// This function takes an array as an input agument, and calculates the average
// of n readings on each colour channel.
void readColourN(int colourArr[4], int n) {    // arrays are always passed by value
#define TIMEOUT 200                            // timeout for pulseIN() routine
  unsigned long thisRead[4] = { 0, 0, 0, 0 };  // for data averaging
  bool pinStates[4][2] = {
    { LOW, LOW },    // S2,S3 are LOW for RED
    { HIGH, HIGH },  // S2,S3 are HIGH for GREEN
    { LOW, HIGH },   // S2=LOW,S3=HIGH for BLUE
    { HIGH, LOW }    // S2=HIGH,S3=LOW for CLEAR
  };
  for (int i = 0; i < 4; i++) {                   //i=0:RED, i=1:GREEN, i=2:BLUE
    digitalWrite(S2, pinStates[i][0]);            // set S2 to correct pin state
    digitalWrite(S3, pinStates[i][1]);            // set S3 to correct pin state
    thisRead[i] = 0;                              //initialize colour
    delay(100);                                   // wait for reading to stabilize
    for (int j = 0; j < n; j++) {                 // collect n readings on channel i
      thisRead[i] += pulseIn(OUT, LOW, TIMEOUT);  //read colour
    }
    thisRead[i] /= n;            // report the average
    colourArr[i] = thisRead[i];  //write values back to colourArr
  }
}

