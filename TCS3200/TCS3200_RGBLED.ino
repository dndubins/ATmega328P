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
 * RGB LED Module - Arduino Uno
 * ---------------------------
 * Red   - 5
 * Green - 6
 * Blue  - 4
 * GND   - GND
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

// To hold readings
#define NUMREADS 1000            // number of readings for data averaging
int reading[3] = { 0, 0, 0 };  // to store RED, GREEN, BLUE reading

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
  lightLED(thisColour);
  if (thisColour == NOT_DETECTED) flashRed(5);  // flash the red light 5X
}

// This function takes an array as an input agument, and calculates the average
// of n readings on each colour channel.
void readColourN(int colourArr[3], int n) {  // arrays are always passed by value
#define TIMEOUT 1000                           // for timeout (in microseconds) on reading a colour
  unsigned long thisRead[3] = { 0, 0, 0 };     // for data averaging
  bool pinStates[3][2] = {
    { LOW, LOW },    // S2,S3 are LOW for RED
    { HIGH, HIGH },  // S2,S3 are HIGH for GREEN
    { LOW, HIGH }    // S2=LOW,S3=HIGH for BLUE
  };
  for (int i = 0; i < 3; i++) {                   // i=0: red, i=1: green, i=2: blue
    digitalWrite(S2, pinStates[i][0]);            // set S2 to correct pin state
    digitalWrite(S3, pinStates[i][1]);            // set S3 to correct pin state
    thisRead[i] = 0;                              // initialize colour
    delay(100);                                   // wait for reading to stabilize
    for (int j = 0; j < n; j++) {                 // collect n readings on channel i
      thisRead[i] += pulseIn(OUT, LOW, TIMEOUT);  // read colour
    }
    thisRead[i] /= n;                   // report the average
    colourArr[i] = thisRead[i];  //write back to colourArr
  }
}

colour decodeColour(int colourArr[3]) {
  float maxRead = 0.0;
  float norm[3] = { 0.0, 0.0, 0.0 };  // to store RED, GREEN, BLUE reading
    // normalize to highest intensity:
  for (int i = 0; i < 3; i++) {
    if (colourArr[i] > maxRead) maxRead = (float)colourArr[i];  // find maximum intensity
  }
  if (maxRead > 0) {                                                     // protect against dividing by zero
    for (int i = 0; i < 3; i++) {                                        // i=0: red, i=1: green, i=2: blue
      norm[i] = (maxRead - (float)colourArr[i]) / maxRead;  // Normalize here. Subtract from highest then divide by highest reading.
    }
  }
  Serial.print("Normalized: ");
  Serial.print(norm[0]);  // output RED channel
  Serial.print(",");
  Serial.print(norm[1]);  // output GREEN channel
  Serial.print(",");
  Serial.println(norm[2]);  // output BLUE channel
  const float RED_THRESHOLD = 0.25;
  const float GREEN_THRESHOLD = 0.25;
  const float BLUE_THRESHOLD = 0.25;
  const float ORANGE_THRESHOLD = 0.10;
  if (norm[0] > RED_THRESHOLD) {
    if (norm[1] < ORANGE_THRESHOLD && norm[2] < BLUE_THRESHOLD) return RED;
    if (norm[1] >= ORANGE_THRESHOLD && norm[1] < GREEN_THRESHOLD && norm[2] < BLUE_THRESHOLD) return ORANGE;
    if (norm[1] > GREEN_THRESHOLD && norm[2] < BLUE_THRESHOLD) return YELLOW;
    if (norm[1] < GREEN_THRESHOLD && norm[2] > BLUE_THRESHOLD) return PURPLE;
  }
  if (norm[1] > GREEN_THRESHOLD) {
    if (norm[0] < RED_THRESHOLD && norm[2] < BLUE_THRESHOLD) return GREEN;
    if (norm[0] < RED_THRESHOLD && norm[2] > BLUE_THRESHOLD) return BLUE;
  }
  if (norm[2] > BLUE_THRESHOLD) {
    if (norm[0] < RED_THRESHOLD && norm[1] < GREEN_THRESHOLD) return BLUE;  // just in case (blue colour usually triggers green)
  }
  return NOT_DETECTED;
}

void lightLED(colour c) {
  switch (c) {
    case RED:
      Serial.println("RED detected.");
      digitalWrite(REDPIN, HIGH);
      digitalWrite(GREENPIN, LOW);
      digitalWrite(BLUEPIN, LOW);
      break;
    case ORANGE:
      Serial.println("ORANGE detected.");
      digitalWrite(REDPIN, HIGH);
      analogWrite(GREENPIN, 50);  // less green for orange light
      digitalWrite(BLUEPIN, LOW);
      break;
    case YELLOW:
      Serial.println("YELLOW detected.");
      digitalWrite(REDPIN, HIGH);
      digitalWrite(GREENPIN, HIGH);
      digitalWrite(BLUEPIN, LOW);
      break;
    case GREEN:
      Serial.println("GREEN detected.");
      digitalWrite(REDPIN, LOW);
      digitalWrite(GREENPIN, HIGH);
      digitalWrite(BLUEPIN, LOW);
      break;
    case BLUE:
      Serial.println("BLUE detected.");
      digitalWrite(REDPIN, LOW);
      digitalWrite(GREENPIN, LOW);
      digitalWrite(BLUEPIN, HIGH);
      break;
    case PURPLE:
      Serial.println("PURPLE detected.");
      digitalWrite(REDPIN, HIGH);
      digitalWrite(GREENPIN, LOW);
      digitalWrite(BLUEPIN, HIGH);
      break;
    case NOT_DETECTED:
      Serial.println("No colour detected.");
      digitalWrite(REDPIN, LOW);
      digitalWrite(GREENPIN, LOW);
      digitalWrite(BLUEPIN, LOW);
      break;
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
