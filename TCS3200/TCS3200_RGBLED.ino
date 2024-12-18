/* TCS3200_RGBLED.ino
 * TCS3200 color recognition sensor 
 * This sketch measures colour and lights up an RGB LED module accordingly.
 * Author: D. Dubins
 * Date: 18-Dec-24
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
  pinMode(7, OUTPUT);  // set up pin 7 as a GND for the RGB module
  digitalWrite(7, LOW);
  digitalWrite(S0, HIGH);  // set on LEDs on front to HIGH
  digitalWrite(S1, HIGH);  // (setting S0 & S1 LOW turns LEDs off)
  Serial.println("R,G,B,CL: ");
}

void loop() {
  readColourN(reading, 1000);  // take n colour readings
  Serial.print("Reading: ");
  printColourArr(reading);   // print contents of reading
  thisColour = decodeColour(reading);
  lightLED(thisColour);
  if (thisColour == NOT_DETECTED) flashRed(5);  // flash the red light 5X
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

colour decodeColour(int colourArr[4]) {  // decodes an RGB sensor reading into colour buckets defined in the enum "colour".
  float norm[3] = { 0.0, 0.0, 0.0 };     // to store RED, GREEN, BLUE, CLEAR reading
    // Normalize: (CLEAR - COLOUR)/[(CLEAR-RED)+(CLEAR-GREEN)+(CLEAR-BLUE)]
  if (colourArr[0] == 0 | colourArr[1] == 0 | colourArr[2] == 0) return NOT_DETECTED;  // if any channel times out, return a null reading and don't normalize.
  float total = 0.0;                                                                   // to store total
  for (int i = 0; i < 3; i++) {                                                        // subtract each colour from CLEAR signal
    norm[i] = (float)(colourArr[i] - colourArr[3]);                                    // subtract W from each signal
    total += norm[i];                                                                  // add signal to total
  }
  for (int i = 0; i < 3; i++) {
    norm[i] = norm[i] / total;  // divide by total
  }
  Serial.print("Normalized: ");
  printNormArr(norm);  // print the normalized array
  const float RED_THRESHOLD = 0.3;
  const float GREEN_THRESHOLD = 0.3;
  const float BLUE_THRESHOLD = 0.2;
  const float ORANGE_THRESHOLD = 0.45;                                                                        // this is the GREEN threshold for orange (orange happens when green signal is between ~0.3-0.45)
  if (norm[0] < RED_THRESHOLD) {                                                                              // if RED has a signal
    if (norm[1] > ORANGE_THRESHOLD && norm[2] > BLUE_THRESHOLD) return RED;                                   // RED is dominant
    if (norm[1] <= ORANGE_THRESHOLD && norm[1] > GREEN_THRESHOLD && norm[2] > BLUE_THRESHOLD) return ORANGE;  // RED and a little bit of GREEN make ORANGE
    if (norm[1] < GREEN_THRESHOLD && norm[2] > BLUE_THRESHOLD) return YELLOW;                                 // RED and GREEN make YELLOW
    if (norm[1] > GREEN_THRESHOLD && norm[2] < BLUE_THRESHOLD) return PURPLE;                                 // RED and BLUE make PURPLE
  }
  if (norm[0] > RED_THRESHOLD && norm[1] < GREEN_THRESHOLD && norm[2] > BLUE_THRESHOLD) return GREEN;  // GREEN is dominant
  if (norm[0] > RED_THRESHOLD && norm[2] < BLUE_THRESHOLD) return BLUE;                                // BLUE is dominant. Ignore GREEN signal. (We are ignoring BLUE+GREEN=CYAN, and calling this "BLUE".)
  return NOT_DETECTED;                                                                                 // if the routine gets this far, we haven't identified a colour.
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
  Serial.println(normArr[2], 3);  // output BLUE channel
}
