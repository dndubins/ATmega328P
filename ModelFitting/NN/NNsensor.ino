/* NNsensor.ino for TCS3200 Colour Sensor
 * This sketch provides a serial menu, where you can train a sensor using a neural network algorithm, and save it to EPROM memory.
 * This can be retreived and used later. The menu also provides a reading mode to use your trained network on new colours.
 *
 * Neural network coding based on:
 * ArduinoANN - An artificial neural network for the Arduino
 * All basic settings for the neural network can be controlled via the Network Configuration
 * section.
 * See http://robotics.hobbizine.com/arduinoann.html for more details.
 * Sketch: David Dubins
 * Date: 3-Feb-19
 * Last Updated: 18-Dec-24
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

#include <EEPROM.h>  // for saving AI matrix

// Global Network Configuration Variables
const int InputNodes = 3;   // The number of input neurons (can be sensor readings, <=7 for Arduino)
const int HiddenNodes = 8;  // The number of hidden neurons (> # output neurons, <=8 for Arduino)
const int OutputNodes = 6;  // The number of output neurons (<=4 for Arduino)
                            // Note: Scroll down and adjust sizes of PatternCount, Input, and Target matrices
float Hidden[HiddenNodes];
float Output[OutputNodes];
float Accum;
const char* TargetNames[OutputNodes] = { "red", "orange", "yellow", "green", "blue", "purple" };  // titles to match training set

struct NNweights {
  char name[10];
  float HiddenWeights[InputNodes + 1][HiddenNodes];
  float OutputWeights[HiddenNodes + 1][OutputNodes];
} NN;

// Colour sensor module pins and setup
#define S0 11
#define S1 12
#define S2 9
#define S3 10
#define OUT 8

// For reading data
#define NUMREADS 1000                       // number of readings per colour reading (for averaging)
float reading[4] = { 0.0, 0.0, 0.0, 0.0 };  // to store RED, GREEN, BLUE, CLEAR reading

char choice = '\0';  // For serial menu. Initialize choice with NULL.

#include <math.h>

/******************************************************************
 * Network Configuration - customized per network 
 ******************************************************************/

const int PatternCount = 18;         // The number of training items (or rows) in the truth table
const float LearningRate = 0.3;      // Adjusts how much of the error is actually backpropagated. (lower = slower, less chance for oscillations)
const float Momentum = 0.9;          // Adjusts how much the results of the previous iteration affect the current iteration. (choose a value between 0 and 1)
const float InitialWeightMax = 0.5;  // Sets the maximum starting value for weights. (0.5 sets initial weights between -0.5 and 0.5.)
const float Success = 0.01;          // The threshold for error at which the network will be said to have solved the training set. (choose a value close to zero)
                                     // This is a (somewhat) arbitrary relative measure. If the routine doesn't find a good solution, raise Success accordingly.
                                     // (If at first you don't succeed... lower your standards!)

// For inputting training set manually:
float Input[PatternCount][InputNodes] = {
  { 0.044, 0.506, 0.450 },  //RED
  { 0.035, 0.523, 0.442 },  //RED
  { 0.047, 0.496, 0.457 },  //RED
  { 0.079, 0.365, 0.556 },  //ORANGE
  { 0.063, 0.381, 0.556 },  //ORANGE
  { 0.066, 0.393, 0.541 },  //ORANGE
  { 0.200, 0.236, 0.564 },  //YELLOW
  { 0.161, 0.250, 0.589 },  //YELLOW
  { 0.164, 0.255, 0.582 },  //YELLOW
  { 0.483, 0.207, 0.310 },  //GREEN
  { 0.471, 0.216, 0.314 },  //GREEN
  { 0.479, 0.208, 0.312 },  //GREEN
  { 0.580, 0.300, 0.120 },  //BLUE
  { 0.556, 0.315, 0.130 },  //BLUE
  { 0.574, 0.296, 0.130 },  //BLUE
  { 0.254, 0.576, 0.169 },  //PURPLE
  { 0.259, 0.569, 0.172 },  //PURPLE
  { 0.309, 0.529, 0.162 }   //PURPLE
};

const byte Target[PatternCount][OutputNodes] = {
  { 1, 0, 0, 0, 0, 0 },  //RED
  { 1, 0, 0, 0, 0, 0 },  //RED
  { 1, 0, 0, 0, 0, 0 },  //RED
  { 0, 1, 0, 0, 0, 0 },  //ORANGE
  { 0, 1, 0, 0, 0, 0 },  //ORANGE
  { 0, 1, 0, 0, 0, 0 },  //ORANGE
  { 0, 0, 1, 0, 0, 0 },  //YELLOW
  { 0, 0, 1, 0, 0, 0 },  //YELLOW
  { 0, 0, 1, 0, 0, 0 },  //YELLOW
  { 0, 0, 0, 1, 0, 0 },  //GREEN
  { 0, 0, 0, 1, 0, 0 },  //GREEN
  { 0, 0, 0, 1, 0, 0 },  //GREEN
  { 0, 0, 0, 0, 1, 0 },  //BLUE
  { 0, 0, 0, 0, 1, 0 },  //BLUE
  { 0, 0, 0, 0, 1, 0 },  //BLUE
  { 0, 0, 0, 0, 0, 1 },  //PURPLE
  { 0, 0, 0, 0, 0, 1 },  //PURPLE
  { 0, 0, 0, 0, 0, 1 }   //PURPLE
};

/******************************************************************
 * End Network Configuration
 ******************************************************************/

void setup() {
  Serial.begin(9600);
  Serial.println(F("Loading weights form EEPROM."));
  EEPROM.get(0, NN);  // get weights from EEPROM
  Serial.println(F("Weights loaded."));
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
  printMenu();  //reprint user menu
}

void loop() {
  if (Serial.available()) {
    choice = Serial.read();
  } else if (choice == 'r') {  // what happens with s
    Serial.println(F("\nReading sample:"));
    readSample();
    choice = '\0';  //erase choice
    printMenu();    //reprint user menu
  } else if (choice == 'c') {
    readSample();
    delay(100);  //short delay
  } else if (choice == 't') {
    for (int i = 0; i < OutputNodes; i++) {
      for (int j = 0; j < 3; j++) {
        Serial.println("Enter 'r' to read response " + (String)(j + 1) + " for " + TargetNames[i] + ">");
        while (!Serial.available()) { ; }  // wait for input
        choice = Serial.read();
        readColourN_norm(reading, NUMREADS);  // read sensor
        //Serial.println("Reading: " + (String)reading[0] + "," + (String)reading[1] + "," + (String)reading[2]);
        Input[j + (3 * i)][0] = reading[0];  //red
        Input[j + (3 * i)][1] = reading[1];  //green
        Input[j + (3 * i)][2] = reading[2];  //blue
        Serial.println("Reading: " + (String)Input[j + (3 * i)][0] + "," + (String)Input[j + (3 * i)][1] + "," + (String)Input[j + (3 * i)][2]);
      }
    }
    solveNN();
    Serial.println(F("Saving weights to EEPROM."));
    EEPROM.put(0, NN);  // write weights to EEPROM
    Serial.println(F("Weights saved."));
    choice = '\0';  //erase choice
    printMenu();    //reprint user menu
  } else if (choice == 'h') {
    solveNN();
    Serial.println(F("Saving weights to EEPROM."));
    EEPROM.put(0, NN);  // write weights to EEPROM
    Serial.println(F("Weights saved."));
    choice = '\0';  //erase choice
    printMenu();    //reprint user menu
  } else if (choice != '\0') {
    Serial.println(F("\nInvalid option."));
    choice = '\0';  //erase choice
    printMenu();    //reprint user menu
  }
}

void solveNN() {  // neural network fitting routine
  int p, q, r;    // counter variables
  int ReportEvery1000;
  int RandomizedIndex[PatternCount];
  float Rando;
  float Error;
  float HiddenDelta[HiddenNodes];
  float OutputDelta[OutputNodes];
  float ChangeHiddenWeights[InputNodes + 1][HiddenNodes];
  float ChangeOutputWeights[HiddenNodes + 1][OutputNodes];
  unsigned long TrainingCycle = 0;

  randomSeed(analogRead(3));
  ReportEvery1000 = 1;
  for (p = 0; p < PatternCount; p++) {
    RandomizedIndex[p] = p;
  }

  /******************************************************************
* Initialize HiddenWeights and ChangeHiddenWeights 
******************************************************************/
  for (int i = 0; i < HiddenNodes; i++) {
    for (int j = 0; j <= InputNodes; j++) {
      ChangeHiddenWeights[j][i] = 0.0;
      Rando = float(random(100)) / 100;
      NN.HiddenWeights[j][i] = 2.0 * (Rando - 0.5) * InitialWeightMax;
    }
  }

  /******************************************************************
* Initialize OutputWeights and ChangeOutputWeights
******************************************************************/
  for (int i = 0; i < OutputNodes; i++) {
    for (int j = 0; j <= HiddenNodes; j++) {
      ChangeOutputWeights[j][i] = 0.0;
      Rando = float(random(100)) / 100;
      NN.OutputWeights[j][i] = 2.0 * (Rando - 0.5) * InitialWeightMax;
    }
  }
  Serial.println(F("Initial/Untrained Outputs: "));
  toTerminal();
  /******************************************************************
* Begin training 
******************************************************************/
  for (TrainingCycle = 1; TrainingCycle < 2147483647; TrainingCycle++) {

    /******************************************************************
* Randomize order of training patterns
******************************************************************/
    for (p = 0; p < PatternCount; p++) {
      q = random(PatternCount);
      r = RandomizedIndex[p];
      RandomizedIndex[p] = RandomizedIndex[q];
      RandomizedIndex[q] = r;
    }
    Error = 0.0;

    /******************************************************************
* Cycle through each training pattern in the randomized order
******************************************************************/
    for (int q = 0; q < PatternCount; q++) {
      p = RandomizedIndex[q];

      /******************************************************************
* Compute hidden layer activations
******************************************************************/
      for (int i = 0; i < HiddenNodes; i++) {
        Accum = NN.HiddenWeights[InputNodes][i];
        for (int j = 0; j < InputNodes; j++) {
          Accum += Input[p][j] * NN.HiddenWeights[j][i];
        }
        Hidden[i] = 1.0 / (1.0 + exp(-Accum));
      }

      /******************************************************************
* Compute output layer activations and calculate errors
******************************************************************/
      for (int i = 0; i < OutputNodes; i++) {
        Accum = NN.OutputWeights[HiddenNodes][i];
        for (int j = 0; j < HiddenNodes; j++) {
          Accum += Hidden[j] * NN.OutputWeights[j][i];
        }
        Output[i] = 1.0 / (1.0 + exp(-Accum));
        OutputDelta[i] = (Target[p][i] - Output[i]) * Output[i] * (1.0 - Output[i]);
        Error += 0.5 * (Target[p][i] - Output[i]) * (Target[p][i] - Output[i]);
      }

      /******************************************************************
* Backpropagate errors to hidden layer
******************************************************************/
      for (int i = 0; i < HiddenNodes; i++) {
        Accum = 0.0;
        for (int j = 0; j < OutputNodes; j++) {
          Accum += NN.OutputWeights[i][j] * OutputDelta[j];
        }
        HiddenDelta[i] = Accum * Hidden[i] * (1.0 - Hidden[i]);
      }


      /******************************************************************
* Update Inner-->Hidden Weights
******************************************************************/
      for (int i = 0; i < HiddenNodes; i++) {
        ChangeHiddenWeights[InputNodes][i] = LearningRate * HiddenDelta[i] + Momentum * ChangeHiddenWeights[InputNodes][i];
        NN.HiddenWeights[InputNodes][i] += ChangeHiddenWeights[InputNodes][i];
        for (int j = 0; j < InputNodes; j++) {
          ChangeHiddenWeights[j][i] = LearningRate * Input[p][j] * HiddenDelta[i] + Momentum * ChangeHiddenWeights[j][i];
          NN.HiddenWeights[j][i] += ChangeHiddenWeights[j][i];
        }
      }

      /******************************************************************
* Update Hidden-->Output Weights
******************************************************************/
      for (int i = 0; i < OutputNodes; i++) {
        ChangeOutputWeights[HiddenNodes][i] = LearningRate * OutputDelta[i] + Momentum * ChangeOutputWeights[HiddenNodes][i];
        NN.OutputWeights[HiddenNodes][i] += ChangeOutputWeights[HiddenNodes][i];
        for (int j = 0; j < HiddenNodes; j++) {
          ChangeOutputWeights[j][i] = LearningRate * Hidden[j] * OutputDelta[i] + Momentum * ChangeOutputWeights[j][i];
          NN.OutputWeights[j][i] += ChangeOutputWeights[j][i];
        }
      }
    }

    /******************************************************************
* Every 1000 cycles send data to terminal for display
******************************************************************/
    ReportEvery1000 = ReportEvery1000 - 1;
    if (ReportEvery1000 == 0) {
      Serial.println();
      Serial.println();
      Serial.print(F("TrainingCycle: "));
      Serial.print(TrainingCycle);
      Serial.print(F("  Error = "));
      Serial.println(Error, 5);
      toTerminal();
      if (TrainingCycle == 1) {
        ReportEvery1000 = 999;
      } else {
        ReportEvery1000 = 1000;
      }
    }

    /******************************************************************
* If error rate is less than pre-determined threshold then end
******************************************************************/
    if (Error < Success) break;
  }
  Serial.println();
  Serial.println();
  Serial.print(F("TrainingCycle: "));
  Serial.print(TrainingCycle);
  Serial.print(F("  Error = "));
  Serial.println(Error, 5);
  toTerminal();

  /******************************************************************
* Send HiddenWeights and OutputWeights to Serial
******************************************************************/
  Serial.println();
  Serial.println(F("  HiddenWeights: "));
  for (int j = 0; j <= InputNodes; j++) {
    for (int i = 0; i < HiddenNodes; i++) {
      Serial.print(NN.HiddenWeights[j][i], DEC);
      Serial.print(F(", "));
    }
    Serial.println("");
  }
  Serial.println();
  Serial.println(F("  OutputWeights: "));
  for (int j = 0; j <= HiddenNodes; j++) {
    for (int i = 0; i < OutputNodes; i++) {
      Serial.print(NN.OutputWeights[j][i], DEC);
      Serial.print(F(", "));
    }
    Serial.println("");
  }
  Serial.println(F("\n\nTraining Set Solved! "));
  Serial.println(F("--------\n\n"));
  ReportEvery1000 = 1;
}

void toTerminal() {
  for (int p = 0; p < PatternCount; p++) {
    Serial.println();
    Serial.print(F("  Training Pattern: "));
    Serial.println(p);
    Serial.print(F("  Input "));
    for (int i = 0; i < InputNodes; i++) {
      Serial.print(Input[p][i], DEC);
      Serial.print(F(" "));
    }
    Serial.print(F("  Target "));
    for (int i = 0; i < OutputNodes; i++) {
      Serial.print(Target[p][i], DEC);
      Serial.print(F(" "));
    }
    /******************************************************************
* Compute hidden layer activations
******************************************************************/
    for (int i = 0; i < HiddenNodes; i++) {
      Accum = NN.HiddenWeights[InputNodes][i];
      for (int j = 0; j < InputNodes; j++) {
        Accum += Input[p][j] * NN.HiddenWeights[j][i];
      }
      Hidden[i] = 1.0 / (1.0 + exp(-Accum));
    }

    /******************************************************************
* Compute output layer activations and calculate errors
******************************************************************/
    for (int i = 0; i < OutputNodes; i++) {
      Accum = NN.OutputWeights[HiddenNodes][i];
      for (int j = 0; j < HiddenNodes; j++) {
        Accum += Hidden[j] * NN.OutputWeights[j][i];
      }
      Output[i] = 1.0 / (1.0 + exp(-Accum));
    }
    Serial.print(F("  Output "));
    for (int i = 0; i < OutputNodes; i++) {
      Serial.print(Output[i], 5);
      Serial.print(F(" "));
    }
  }
  Serial.println(F("\nSolving neural network. Please wait."));
}

void useNN(float R, float G, float B) {  // use NN hidden and output weights to compute outcome
  float measuredInput[3];
  measuredInput[0] = R;
  measuredInput[1] = G;
  measuredInput[2] = B;

  /******************************************************************
* Compute hidden layer activations
******************************************************************/
  for (int i = 0; i < HiddenNodes; i++) {
    Accum = NN.HiddenWeights[InputNodes][i];
    for (int j = 0; j < InputNodes; j++) {
      Accum += measuredInput[j] * NN.HiddenWeights[j][i];
    }
    Hidden[i] = 1.0 / (1.0 + exp(-Accum));
  }

  /******************************************************************
* Compute output layer activations and calculate errors
******************************************************************/
  for (int i = 0; i < OutputNodes; i++) {
    Accum = NN.OutputWeights[HiddenNodes][i];
    for (int j = 0; j < HiddenNodes; j++) {
      Accum += Hidden[j] * NN.OutputWeights[j][i];
    }
    Output[i] = 1.0 / (1.0 + exp(-Accum));
  }
}

// This function takes an array as an input agument, and calculates the average
// of n readings on each colour channel.
void readColourN_norm(float colourArr[4], int n) {  // arrays are always passed by value
#define TIMEOUT 200                                 // timeout for pulseIN() routine
  unsigned long thisRead[4] = { 0, 0, 0, 0 };       // for data averaging
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
    colourArr[i] = thisRead[i];  // write values back to colourArr
  }
  // Normalize: (CLEAR - COLOUR)/[(CLEAR-RED)+(CLEAR-GREEN)+(CLEAR-BLUE)]
  float total = 0.0;                             // to store total
  for (int i = 0; i < 3; i++) {                  // subtract each colour from CLEAR signal
    colourArr[i] = colourArr[i] - colourArr[3];  // subtract W from each signal
    total += colourArr[i];                       // add signal to total
  }
  for (int i = 0; i < 3; i++) {
    colourArr[i] = colourArr[i] / total;  // divide by total
  }
}

void readSample() {                // take one reading and apply neural network weights to calculate output
  readColourN_norm(reading, NUMREADS);  // take measurement
  useNN(reading[0], reading[1], reading[2]);
  float maxC = 0.0;
  byte maxIdx = 0;
  for (int i = 0; i < OutputNodes; i++) {
    Serial.print(Output[i], 3);
    if (Output[i] > maxC) {
      maxC = Output[i];
      maxIdx = i;
    }
    Serial.print(F(" "));
  }
  if (maxC > 0.8) {
    Serial.print(" <--- " + (String)TargetNames[maxIdx] + " detected");
  }
  Serial.println("");
}

void printMenu() {  // user menu
  Serial.println(F("r: Read one sample"));
  Serial.println(F("c: Continuous reads"));
  Serial.println(F("h: train neural network with historical values"));
  Serial.println(F("t: train neural network with new values"));

  Serial.println(F("Enter choice: "));
}
