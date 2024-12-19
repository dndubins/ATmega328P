/* TCH3200_4CH.ino
 * TCS3200 color recognition sensor 
 * This sketch takes readings from all 4 channels of the TCS3200 (RED, GREEN, BLUE, CLEAR),
 * It normalizes the readings using the CLEAR intensity, and identifies colour on the Serial Monitor.
 *
 * Author: D. Dubins
 * Date: 18-Dec-24
 * Sketch adapted from: https://electronicsforu.com/electronics-projects/rgb-color-detector-tcs3200-sensor-module
 * http://www.efymag.com/admin/issuepdf/RGB-Colour-Detection-Using-TCS3200_3-17.rar
 *
 * There are three functions presented here:
 * readColour() gives you one raw reading from the TCS3200.
 * readColourN() gives you an average of N readings.
 * decodeColour() normalizes the 4 channel reading, and converts it to a bucketed colour, defined by the enum "colour".
 * printColourArr(), printNormArr(), and printColour() are just for output to the Serial Monitor.
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

enum colour {  // define enum Colour with members RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET.
  RED,
  ORANGE,
  YELLOW,
  GREEN,
  BLUE,
  PURPLE,
  NOT_DETECTED
};

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
  readColourN(reading, 1000);  // take n colour readings
  Serial.print(NUMREADS);
  Serial.print(" readings: ");
  printColourArr(reading);  // print average of n readings
  printColour(decodeColour(reading));
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

colour decodeColour(int colourArr[4]) {                                                // decodes an RGB sensor reading into colour buckets defined in the enum "colour".
  if (colourArr[0] == 0 | colourArr[1] == 0 | colourArr[2] == 0) return NOT_DETECTED;  // if any channel times out, return a null reading and don't normalize.
  // Convert to Hz and normalize: RED/CLEAR, GREEN/CLEAR, BLUE/CLEAR:
  float norm[4] = { 0.0, 0.0, 0.0, 0.0 };  // to store normalized RED, GREEN, BLUE, CLEAR reading
  for (int i = 0; i < 4; i++) {
    norm[i] = 1000000.0 / (float)colourArr[i];  // calculate each signal in Hz (R, G, B, CLEAR)
  }
  for (int i = 0; i < 3; i++) {   // subtract each colour from CLEAR signal
    norm[i] = norm[i] / norm[3];  // calculate %signal of R, G, B (relative to CLEAR signal)
  }
  Serial.print("Normalized: ");
  printNormArr(norm);  // print the normalized array
  const float RED_THRESHOLD = 0.3;
  const float GREEN_THRESHOLD = 0.3;
  const float BLUE_THRESHOLD = 0.4;
  const float ORANGE_THRESHOLD = 0.2;                                                                         // this is the GREEN threshold for orange
  if (norm[0] >= RED_THRESHOLD) {                                                                             // if RED has a signal
    if (norm[1] < ORANGE_THRESHOLD && norm[2] < BLUE_THRESHOLD) return RED;                                   // RED is dominant
    if (norm[1] >= ORANGE_THRESHOLD && norm[1] < GREEN_THRESHOLD && norm[2] < BLUE_THRESHOLD) return ORANGE;  // RED and a little bit of GREEN make ORANGE
    if (norm[1] >= GREEN_THRESHOLD && norm[2] < BLUE_THRESHOLD) return YELLOW;                               // RED and GREEN make YELLOW
    if (norm[1] < GREEN_THRESHOLD && norm[2] >= BLUE_THRESHOLD) return PURPLE;                                // RED and BLUE make PURPLE
  }
  if (norm[0] < RED_THRESHOLD && norm[1] >= GREEN_THRESHOLD && norm[2] < BLUE_THRESHOLD) return GREEN;  // GREEN is dominant
  if (norm[0] < RED_THRESHOLD && norm[2] >= BLUE_THRESHOLD) return BLUE;                                 // BLUE is dominant. Ignore GREEN signal. (We are ignoring BLUE+GREEN=CYAN, and calling this "BLUE".)
  return NOT_DETECTED;                                                                                   // if the routine gets this far, we haven't identified a colour.
}

// Prints contents of colourArr[i]
void printColourArr(int colourArr[4]) {
  Serial.print(colourArr[0]);  // output RED channel
  Serial.print(",");
  Serial.print(colourArr[1]);  // output GREEN channel
  Serial.print(",");
  Serial.print(colourArr[2]);  // output BLUE channel
  Serial.print(",");
  Serial.println(colourArr[3]);  // output CLEAR channel
}

// Prints contents of colourArr[i]
void printNormArr(float normArr[3]) {
  Serial.print(normArr[0], 3);  // output RED channel
  Serial.print(",");
  Serial.print(normArr[1], 3);  // output GREEN channel
  Serial.print(",");
  Serial.print(normArr[2], 3);  // output BLUE channel
}

// Prints identified colour n
void printColour(colour n) {
  const char* colourNames[7] = { "RED", "ORANGE", "YELLOW", "GREEN", "BLUE", "PURPLE", "NOT DETECTED" };
  Serial.print(" - ");
  Serial.println(colourNames[n]);
}
