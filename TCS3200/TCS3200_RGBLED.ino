/* TCS3200_RGBLED.ino
 * TCS3200 color recognition sensor 
 * This sketch measures colour and lights up an RGB LED module accordingly.
 * Author: D. Dubins
 * Date: 17-Dec-24
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
 *
 * RGB LED - Arduino Uno
 * ---------------------
 * Red - 1K - 5
 * Green - 220R - 6
 * Blue - 220R - 7
 * GND - GND
*/

// Colour sensor module pins and setup
#define S0 11
#define S1 12
#define S2 9
#define S3 10
#define OUT 8

// RGB LED pins
#define REDPIN 5
#define GREENPIN 6
#define BLUEPIN 4

float reading[3] = { 0, 0, 0 };  // to store RED, GREEN, BLUE reading

enum colour {  // define enum Colour with members RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET.
  RED,
  ORANGE,
  YELLOW,
  GREEN,
  BLUE,
  PURPLE,
  NOT_DETECTED
};

colour thisColour = NOT_DETECTED;  // initialized thisColour as NOT_DETECTED

#define NUMREADS 100  // number of readings for data averaging

void setup() {
  Serial.begin(9600);   // start the Serial Monitor
  pinMode(S0, OUTPUT);  // set pins to OUTPUT mode
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);      // set OUT pin to INPUT mode
  pinMode(REDPIN, OUTPUT);  // set LED pins to OUTPUT mode
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(7, OUTPUT);  // set up pin 7 as a GND from the RGB module
  digitalWrite(7, LOW);
  digitalWrite(S0, HIGH);  // set on LEDs on front to HIGH
  digitalWrite(S1, HIGH);  // (setting S0 & S1 LOW turns LEDs off)
  Serial.println("R,G,B: ");
}

void loop() {
  readColourN(reading, 1000);  // take n colour readings
  Serial.print("Reading: ");
  Serial.print(reading[0]);  // output RED channel
  Serial.print(",");
  Serial.print(reading[1]);  // output GREEN channel
  Serial.print(",");
  Serial.println(reading[2]);  // output BLUE channel
  thisColour = decodeColour(reading);
  if (thisColour == NOT_DETECTED) {
    flashRed(5);  // flash the red light 5X
  } else {
    lightLED(thisColour);
  }
}

// This function takes an array as an input agument, and calculates the average
// of n readings on each colour channel.
void readColourN(float colourArr[3], int n) {  // arrays are always passed by value
  unsigned long thisRead[3] = { 0, 0, 0 };     // for data averaging
  int maxRead = 0;                             // maximum reading (for normalizing the colour signal to the highest intensity)
  bool pinStates[3][2] = {
    { LOW, LOW },    // S2,S3 are LOW for RED
    { HIGH, HIGH },  // S2,S3 are HIGH for GREEN
    { LOW, HIGH }    // S2=LOW,S3=HIGH for BLUE
  };
  for (int i = 0; i < 3; i++) {          // i=0: red, i=1: green, i=2: blue
    digitalWrite(S2, pinStates[i][0]);   // set S2 to correct pin state
    digitalWrite(S3, pinStates[i][1]);   // set S3 to correct pin state
    thisRead[i] = 0;                     // initialize colour
    delay(100);                          // wait for reading to stabilize
    for (int j = 0; j < n; j++) {        // collect n readings on channel i
      thisRead[i] += pulseIn(OUT, LOW);  // read colour (iterative mean)
    }
    thisRead[i] /= n;                                  // report the average
    colourArr[i] = (float)thisRead[i];                 //write back to colourArr
    if (thisRead[i] > maxRead) maxRead = thisRead[i];  // find maximum intensity
  }
  // normalize to highest intensity:
  for (int i = 0; i < 3; i++) {                                // i=0: red, i=1: green, i=2: blue
    colourArr[i] = (maxRead - colourArr[i]) / (float)maxRead;  // Normalize here. Subtract from highest then divide by highest reading.
  }
}

colour decodeColour(float colourArr[3]) {
  if (colourArr[0] > 0.25) {                                                               // strong red channel
    if (colourArr[1] < 0.15 && colourArr[2] < 0.25) return RED;                            // 0 green channel, weak blue channel
    if (colourArr[1] >= 0.15 && colourArr[1] < 0.25 && colourArr[2] < 0.25) return ORANGE;  // weak green channel, 0 blue channel
    if (colourArr[1] > 0.25 && colourArr[2] < 0.25) return YELLOW;                         // weak green channel, 0 blue channel
    if (colourArr[1] < 0.25 && colourArr[2] > 0.25) return PURPLE;                         // 0 green channel, strong blue channel
  }
  if (colourArr[1] > 0.25) {                                       // strong green channel
    if (colourArr[0] < 0.25 && colourArr[2] < 0.25) return GREEN;  // strong green channel
    if (colourArr[0] < 0.25 && colourArr[2] > 0.25) return BLUE;   // strong blue channel
  }
  return NOT_DETECTED;  // if no colour was detected
}

void lightLED(colour c) {
  if (c == RED) {
    Serial.println("RED detected.");
    digitalWrite(REDPIN, HIGH);
    digitalWrite(GREENPIN, LOW);
    digitalWrite(BLUEPIN, LOW);
  }
  if (c == ORANGE) {
    Serial.println("ORANGE detected.");
    digitalWrite(REDPIN, HIGH);
    analogWrite(GREENPIN, 50);  //less green for orange light
    digitalWrite(BLUEPIN, LOW);
  }
  if (c == YELLOW) {
    Serial.println("YELLOW detected.");
    digitalWrite(REDPIN, HIGH);
    digitalWrite(GREENPIN, HIGH);
    digitalWrite(BLUEPIN, LOW);
  }
  if (c == GREEN) {
    Serial.println("GREEN detected.");
    digitalWrite(REDPIN, LOW);
    digitalWrite(GREENPIN, HIGH);
    digitalWrite(BLUEPIN, LOW);
  }
  if (c == BLUE) {
    Serial.println("BLUE detected.");
    digitalWrite(REDPIN, LOW);
    digitalWrite(GREENPIN, LOW);
    digitalWrite(BLUEPIN, HIGH);
  }
  if (c == PURPLE) {
    Serial.println("PURPLE detected.");
    digitalWrite(REDPIN, HIGH);
    digitalWrite(GREENPIN, LOW);
    digitalWrite(BLUEPIN, HIGH);
  }
  if (c == NOT_DETECTED) {
    Serial.println("No colour detected.");
  }
}

void flashRed(byte n) {
  digitalWrite(GREENPIN, LOW);
  digitalWrite(BLUEPIN, LOW);
  for (byte i = 0; i < n; i++) {
    digitalWrite(REDPIN, HIGH);
    delay(100);
    digitalWrite(REDPIN, LOW);
    delay(100);
  }
}
