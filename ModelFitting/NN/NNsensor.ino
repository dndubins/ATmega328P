/* NNsensor.ino
 * This sketch provides a serial menu, where you can train a sensor using a neural network algorithm, and save it to EPROM memory.
 * This can be retreived and used later. The menu also provides a reading mode to use your trained network on new colours.
 *
 * Neural network coding based on:
 * ArduinoANN - An artificial neural network for the Arduino
 * All basic settings can be controlled via the Network Configuration
 * section.
 * See http://robotics.hobbizine.com/arduinoann.html for details.
 * TCS230 color recognition sensor 
 * Sensor connection pins to Arduino are shown in comments
 * Neural network code and algorithm adapted from: http://robotics.hobbizine.com/arduinoann.html
 * Sketch: David Dubins
 * Date: 3-Feb-19
 * Last Updated: 13-Dec-24
 *
 * Connections:
 * TCS3200 - Arduino Uno
 * ---------------------
 *  VCC - 5V
 *  GND - GND
 *  s0  - 8
 *  s1  - 9
 *  s2  - 12
 *  s3  - 11
 *  OUT - 10
 *  OE  - GND
 */

#include <EEPROM.h>    // for saving AI matrix
#define NUMREADS 1000  // number of readings per colour reading (for averaging)

// Global Network Configuration Variables
const int InputNodes = 3;   // The number of input neurons (can be sensor readings, <=7 for Arduino)
const int HiddenNodes = 8;  // The number of hidden neurons (> # output neurons, <=8 for Arduino)
const int OutputNodes = 5;  // The number of output neurons (<=4 for Arduino)
                            // Note: Scroll down and adjust sizes of PatternCount, Input, and Target matrices
float Hidden[HiddenNodes];
float Output[OutputNodes];
float Accum;
const char* TargetNames[OutputNodes] = { "red", "yellow", "green", "blue", "purple" };  // titles to match training set

struct NNweights {
  char name[10];
  float HiddenWeights[InputNodes + 1][HiddenNodes];
  float OutputWeights[HiddenNodes + 1][OutputNodes];
} NN;

// Colour sensor module pins and setup
#define S0 8
#define S1 9
#define S2 12
#define S3 11
#define OUT 10
int reading[3] = { 0, 0, 0 };  // to store red, green, blue reading

char choice = '\0';  // For serial menu. Initialize choice with NULL.

#include <math.h>

/******************************************************************
 * Network Configuration - customized per network 
 ******************************************************************/

const int PatternCount = 15;         // The number of training items (or rows) in the truth table
const float LearningRate = 0.3;      // Adjusts how much of the error is actually backpropagated. (lower = slower, less chance for oscillations)
const float Momentum = 0.9;          // Adjusts how much the results of the previous iteration affect the current iteration. (choose a value between 0 and 1)
const float InitialWeightMax = 0.5;  // Sets the maximum starting value for weights. (0.5 sets initial weights between -0.5 and 0.5.)
const float Success = 0.01;          // The threshold for error at which the network will be said to have solved the training set. (choose a value close to zero)
                                     // This is a (somewhat) arbitrary relative measure. If the routine doesn't find a good solution, raise Success accordingly.
                                     // (If at first you don't succeed... lower your standards!)

// For inputting training set manually:
float Input[PatternCount][InputNodes] = {
  { 0.11, 0.45, 0.41 },  // red
  { 0.09, 0.38, 0.35 },  // red
  { 0.09, 0.39, 0.35 },  // red
  { 0.11, 0.19, 0.30 },  // yellow
  { 0.11, 0.18, 0.29 },  // yellow
  { 0.10, 0.14, 0.24 },  // yellow
  { 0.16, 0.13, 0.20 },  // green
  { 0.18, 0.15, 0.21 },  // green
  { 0.18, 0.15, 0.22 },  // green
  { 0.26, 0.21, 0.16 },  // blue
  { 0.25, 0.21, 0.15 },  // blue
  { 0.24, 0.21, 0.14 },  // blue
  { 0.27, 0.44, 0.25 },  // purple
  { 0.24, 0.38, 0.22 },  // purple
  { 0.20, 0.36, 0.19 }   // purple
};

const byte Target[PatternCount][OutputNodes] = {
  { 1, 0, 0, 0, 0 },  //red
  { 1, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0 },
  { 0, 1, 0, 0, 0 },  //yellow
  { 0, 1, 0, 0, 0 },
  { 0, 1, 0, 0, 0 },
  { 0, 0, 1, 0, 0 },  //green
  { 0, 0, 1, 0, 0 },
  { 0, 0, 1, 0, 0 },
  { 0, 0, 0, 1, 0 },  //blue
  { 0, 0, 0, 1, 0 },
  { 0, 0, 0, 1, 0 },
  { 0, 0, 0, 0, 1 },  //purple
  { 0, 0, 0, 0, 1 },
  { 0, 0, 0, 0, 1 }
};

/******************************************************************
 * End Network Configuration
 ******************************************************************/

void setup() {
  Serial.begin(9600);
  Serial.println("Loading weights form EEPROM.");
  EEPROM.get(0, NN);  // get weights from EEPROM
  Serial.println("Weights loaded.");
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
    Serial.println("\nReading sample:");
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
        readColourN(reading, NUMREADS);  // read sensor
        //Serial.println("Reading: " + (String)reading[0] + "," + (String)reading[1] + "," + (String)reading[2]);
        Input[j + (3 * i)][0] = (float)reading[0] / 100.0;  //red
        Input[j + (3 * i)][1] = (float)reading[1] / 100.0;  //green
        Input[j + (3 * i)][2] = (float)reading[2] / 100.0;  //blue
        Serial.println("Reading: " + (String)Input[j + (3 * i)][0] + "," + (String)Input[j + (3 * i)][1] + "," + (String)Input[j + (3 * i)][2]);
      }
    }
    solveNN();
    Serial.println("Saving weights to EEPROM.");
    EEPROM.put(0, NN);  // write weights to EEPROM
    Serial.println("Weights saved.");
    choice = '\0';  //erase choice
    printMenu();    //reprint user menu
  } else if (choice != '\0') {
    Serial.println("\nInvalid option.");
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
  Serial.println("Initial/Untrained Outputs: ");
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
      Serial.print("TrainingCycle: ");
      Serial.print(TrainingCycle);
      Serial.print("  Error = ");
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
  Serial.print("TrainingCycle: ");
  Serial.print(TrainingCycle);
  Serial.print("  Error = ");
  Serial.println(Error, 5);
  toTerminal();

  /******************************************************************
* Send HiddenWeights and OutputWeights to Serial
******************************************************************/
  Serial.println();
  Serial.println("  HiddenWeights: ");
  for (int j = 0; j <= InputNodes; j++) {
    for (int i = 0; i < HiddenNodes; i++) {
      Serial.print(NN.HiddenWeights[j][i], DEC);
      Serial.print(", ");
    }
    Serial.println("");
  }
  Serial.println();
  Serial.println("  OutputWeights: ");
  for (int j = 0; j <= HiddenNodes; j++) {
    for (int i = 0; i < OutputNodes; i++) {
      Serial.print(NN.OutputWeights[j][i], DEC);
      Serial.print(", ");
    }
    Serial.println("");
  }
  Serial.println("\n\nTraining Set Solved! ");
  Serial.println("--------\n\n");
  ReportEvery1000 = 1;
}

void toTerminal() {
  for (int p = 0; p < PatternCount; p++) {
    Serial.println();
    Serial.print("  Training Pattern: ");
    Serial.println(p);
    Serial.print("  Input ");
    for (int i = 0; i < InputNodes; i++) {
      Serial.print(Input[p][i], DEC);
      Serial.print(" ");
    }
    Serial.print("  Target ");
    for (int i = 0; i < OutputNodes; i++) {
      Serial.print(Target[p][i], DEC);
      Serial.print(" ");
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
    Serial.print("  Output ");
    for (int i = 0; i < OutputNodes; i++) {
      Serial.print(Output[i], 5);
      Serial.print(" ");
    }
  }
  Serial.println("\nSolving neural network. Please wait.");
}

void useNN(int R, int G, int B) {  // use NN hidden and output weights to compute outcome
  float measuredInput[3];
  measuredInput[0] = R / 100.0;
  measuredInput[1] = G / 100.0;
  measuredInput[2] = B / 100.0;
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
void readColourN(int colourArr[3], int n) {  // arrays are always passed by value
  unsigned long thisRead[3] = { 0, 0, 0 };   // for data averaging
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
    thisRead[i] /= n;            // report the average
    colourArr[i] = thisRead[i];  // write back to colourArr
  }
}

void readSample() {                // take one reading and apply neural network weights to calculate output
  readColourN(reading, NUMREADS);  // take measurement
  useNN(reading[0], reading[1], reading[2]);
  //Serial.print ("  Output ");
  float maxC = 0.0;
  byte maxIdx = 0;
  for (int i = 0; i < OutputNodes; i++) {
    Serial.print(Output[i], 3);
    if (Output[i] > maxC) {
      maxC = Output[i];
      maxIdx = i;
    }
    Serial.print(" ");
  }
  if (maxC > 0.8) {
    Serial.print(" <--- " + (String)TargetNames[maxIdx] + " detected");
  }
  Serial.println("");
}

void printMenu() {  // user menu
  Serial.println("r: Read one sample");
  Serial.println("c: Continuous reads");
  Serial.println("t: train neural network with new values");
  Serial.println("Enter choice: ");
}
