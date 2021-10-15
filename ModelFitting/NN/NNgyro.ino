/* NNsensor.ino
 * This sketch provides a serial menu, where you can train a sensor using a neural network algorithm, and save it to EPROM memory.
 * This can be retreived and used later. The menu also provides a reading mode to use your trained network on new colours.
 *
 * Neural network coding based on:
 * ArduinoANN - An artificial neural network for the Arduino
 * All basic settings can be controlled via the Network Configuration
 * section.
 * See http://robotics.hobbizine.com/arduinoann.html for details.
 * GY-521/MPU6050 6-axis gyroscopic sensor 
 * Sensor connection pins to Arduino are shown in comments
 * Neural network code and algorithm adapted from: http://robotics.hobbizine.com/arduinoann.html
 * Sketch: David Dubins
 * Date: 14-Oct-21
 * Library: Adafruit MPU6050 Version 2.0.5 (plus dependent libraries). Install through Library Manager.
 * 
 * Connections:
 * Gyroscopic Sensor --  Uno:
 * Vcc - +5V
 * GND - GND
 * SCL - SCL (or A5)
 * SDA - SDA (or A4)
 * XDA - (NC)
 * XCL - (NC)
 * ADO - GND
 * INT - Digital Pin 2
 */

//Library include statements required for the Adafruit library:
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

#include <EEPROM.h> // for saving AI matrix

// Global Network Configuration Variables
const int InputNodes = 3;            // The number of input neurons (can be sensor readings, <=7 for Arduino)
const int HiddenNodes = 8;           // The number of hidden neurons (> # output neurons, <=8 for Arduino)
const int OutputNodes = 5;           // The number of output neurons (<=4 for Arduino)
                                     // Note: Scroll down and adjust sizes of PatternCount, Input, and Target matrices
float Hidden[HiddenNodes];
float Output[OutputNodes];
float Accum;
String TargetNames[OutputNodes]={"rest","twist","shake","lift","drop"}; // titles to match training set

struct NNweights {
  char name[10];
  float HiddenWeights[InputNodes+1][HiddenNodes];
  float OutputWeights[HiddenNodes+1][OutputNodes];
} NN;

float reading[3]={0,0,0}; // to store x, y, z reading

char choice='\0'; // For serial menu. Initialize choice with NULL.

#include <math.h>

/******************************************************************
 * Network Configuration - customized per network 
 ******************************************************************/

const int PatternCount = 15;         // The number of training items (or rows) in the truth table
const float LearningRate = 0.3;      // Adjusts how much of the error is actually backpropagated. (lower = slower, less chance for oscillations)
const float Momentum = 0.9;          // Adjusts how much the results of the previous iteration affect the current iteration. (choose a value between 0 and 1)
const float InitialWeightMax = 0.5;  // Sets the maximum starting value for weights. (0.5 sets initial weights between -0.5 and 0.5.)
const float Success = 0.01;          // The threshold for error at which the network will be said to have solved the training set. (choose a value close to zero)

// For inputting training set manually:
float Input[PatternCount][InputNodes] = {
  { -0.05, 0.03, 0.03 },   // rest
  { -0.05, 0.04, 0.02 },   // rest
  { -0.04, 0.03, 0.02 },   // rest
  { -0.73, 0.99, 0.46 },   // twist
  { 0.17, 0.29, 0.20 },    // twist
  { -0.22, -0.04, -3.14 }, // twist
  { 1.20, -0.01, -2.76 },    // shake
  { 0.66, 0.04, -1.37 },    // shake 
  { 0.47, -0.23, 0.17 },    // shake 
  { 0.04, 0.06, 0.10 },    // lift
  { 0.25, 0.23, -0.07 },    // lift
  { 0.22, 0.01, -0.11 },    // lift
  { -0.26, -0.37, 0.05 },    // drop
  { -0.21, -0.59, 0.09 },    // drop
  { 0.56, 0.81, 0.24 }       // drop
}; 

const byte Target[PatternCount][OutputNodes] = {
  { 1, 0, 0, 0, 0 },  //rest
  { 1, 0, 0, 0, 0 },  
  { 1, 0, 0, 0, 0 },  
  { 0, 1, 0, 0, 0 },  //twist
  { 0, 1, 0, 0, 0 },  
  { 0, 1, 0, 0, 0 },  
  { 0, 0, 1, 0, 0 },  //shake
  { 0, 0, 1, 0, 0 },  
  { 0, 0, 1, 0, 0 },  
  { 0, 0, 0, 1, 0 },  //lift
  { 0, 0, 0, 1, 0 },  
  { 0, 0, 0, 1, 0 },  
  { 0, 0, 0, 0, 1 },  //drop
  { 0, 0, 0, 0, 1 },  
  { 0, 0, 0, 0, 1 }  
};

/******************************************************************
 * End Network Configuration
 ******************************************************************/

void setup(){
  Serial.begin(115200);

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println(F("Failed to find MPU6050 chip"));
    while (1) {
      delay(10);
    }
  }
  Serial.println(F("MPU6050 Found!"));
  //Options: 
  //MPU6050_RANGE_2_G
  //MPU6050_RANGE_4_G
  //MPU6050_RANGE_8_G
  //MPU6050_RANGE_16_G
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  //Options: (+/-)
  //MPU6050_RANGE_250_DEG
  //MPU6050_RANGE_500_DEG
  //MPU6050_RANGE_1000_DEG
  //MPU6050_RANGE_2000_DEG
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  //Options:
  //MPU6050_BAND_260_HZ
  //MPU6050_BAND_184_HZ
  //MPU6050_BAND_94_HZ
  //MPU6050_BAND_44_HZ
  //MPU6050_BAND_21_HZ
  //MPU6050_BAND_10_HZ
  //MPU6050_BAND_5_HZ
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
  
  Serial.println(F("Loading weights form EEPROM."));
  EEPROM.get(0, NN); // get weights from EEPROM
  Serial.println(F("Weights loaded.")); 
  printMenu(); //reprint user menu    
}  

void loop (){
  if(Serial.available()){
    choice=Serial.read();
  } else if(choice=='r'){ // what happens with s
    Serial.println(F("\nReading sample:"));
    readSample();
    choice='\0'; //erase choice
    printMenu(); //reprint user menu
  } else if(choice=='c'){
    readSample();
    delay(100); //short delay
  } else if(choice=='t'){
    for(int i=0;i<OutputNodes;i++){
      for(int j=0;j<3;j++){
        Serial.println("Enter 'r' to read response "+(String)(j+1)+" for "+TargetNames[i]+">");
        while(!Serial.available()){;} // wait for input
        choice=Serial.read();
        readSensor(); // read sensor
        Serial.println("Reading: "+(String)reading[0]+","+(String)reading[1]+","+(String)reading[2]);
        Input[j+(3*i)][0]=(float)reading[0]/100.0; //x
        Input[j+(3*i)][1]=(float)reading[1]/100.0; //y
        Input[j+(3*i)][2]=(float)reading[2]/100.0; //z
      }
    }
    solveNN();
    Serial.println(F("Saving weights to EEPROM."));
    EEPROM.put(0, NN); // write weights to EEPROM
    Serial.println(F("Weights saved."));
    choice='\0'; //erase choice
    printMenu(); //reprint user menu
  } else if(choice!='\0'){
    Serial.println(F("\nInvalid option."));
    choice='\0'; //erase choice
    printMenu(); //reprint user menu    
  }
}

void solveNN(){ // neural network fitting routine
  int p, q, r;  // counter variables
  int ReportEvery1000;
  int RandomizedIndex[PatternCount];
  float Rando;
  float Error;
  float HiddenDelta[HiddenNodes];
  float OutputDelta[OutputNodes];
  float ChangeHiddenWeights[InputNodes+1][HiddenNodes];
  float ChangeOutputWeights[HiddenNodes+1][OutputNodes];
  unsigned long TrainingCycle=0;

  randomSeed(analogRead(3));
  ReportEvery1000 = 1;
  for( p = 0 ; p < PatternCount ; p++ ) {    
    RandomizedIndex[p] = p ;
  }
  
/******************************************************************
* Initialize HiddenWeights and ChangeHiddenWeights 
******************************************************************/
  for( int i = 0 ; i < HiddenNodes ; i++ ) {    
    for( int j = 0 ; j <= InputNodes ; j++ ) { 
      ChangeHiddenWeights[j][i] = 0.0 ;
      Rando = float(random(100))/100;
      NN.HiddenWeights[j][i] = 2.0 * ( Rando - 0.5 ) * InitialWeightMax ;
    }
  }
  
/******************************************************************
* Initialize OutputWeights and ChangeOutputWeights
******************************************************************/
  for( int i = 0 ; i < OutputNodes ; i ++ ) {    
    for( int j = 0 ; j <= HiddenNodes ; j++ ) {
      ChangeOutputWeights[j][i] = 0.0 ;  
      Rando = float(random(100))/100;        
      NN.OutputWeights[j][i] = 2.0 * ( Rando - 0.5 ) * InitialWeightMax ;
    }
  }
  Serial.println(F("Initial/Untrained Outputs: "));
  toTerminal();
/******************************************************************
* Begin training 
******************************************************************/
  for( TrainingCycle = 1 ; TrainingCycle < 2147483647 ; TrainingCycle++) {    

/******************************************************************
* Randomize order of training patterns
******************************************************************/
    for(p = 0 ; p < PatternCount ; p++) {
      q = random(PatternCount);
      r = RandomizedIndex[p] ; 
      RandomizedIndex[p] = RandomizedIndex[q] ; 
      RandomizedIndex[q] = r ;
    }
    Error = 0.0 ;
    
/******************************************************************
* Cycle through each training pattern in the randomized order
******************************************************************/
    for( int q = 0 ; q < PatternCount ; q++ ) {    
      p = RandomizedIndex[q];

/******************************************************************
* Compute hidden layer activations
******************************************************************/
      for( int i = 0 ; i < HiddenNodes ; i++ ) {    
        Accum = NN.HiddenWeights[InputNodes][i] ;
        for( int j = 0 ; j < InputNodes ; j++ ) {
          Accum += Input[p][j] * NN.HiddenWeights[j][i] ;
        }
        Hidden[i] = 1.0/(1.0 + exp(-Accum)) ;
      }

/******************************************************************
* Compute output layer activations and calculate errors
******************************************************************/
      for( int i = 0 ; i < OutputNodes ; i++ ) {    
        Accum = NN.OutputWeights[HiddenNodes][i] ;
        for( int j = 0 ; j < HiddenNodes ; j++ ) {
          Accum += Hidden[j] * NN.OutputWeights[j][i] ;
        }
        Output[i] = 1.0/(1.0 + exp(-Accum)) ;   
        OutputDelta[i] = (Target[p][i] - Output[i]) * Output[i] * (1.0 - Output[i]) ;   
        Error += 0.5 * (Target[p][i] - Output[i]) * (Target[p][i] - Output[i]) ;
      }

/******************************************************************
* Backpropagate errors to hidden layer
******************************************************************/
      for( int i = 0 ; i < HiddenNodes ; i++ ) {    
        Accum = 0.0 ;
        for( int j = 0 ; j < OutputNodes ; j++ ) {
          Accum += NN.OutputWeights[i][j] * OutputDelta[j] ;
        }
        HiddenDelta[i] = Accum * Hidden[i] * (1.0 - Hidden[i]) ;
      }


/******************************************************************
* Update Inner-->Hidden Weights
******************************************************************/
      for( int i = 0 ; i < HiddenNodes ; i++ ) {     
        ChangeHiddenWeights[InputNodes][i] = LearningRate * HiddenDelta[i] + Momentum * ChangeHiddenWeights[InputNodes][i] ;
        NN.HiddenWeights[InputNodes][i] += ChangeHiddenWeights[InputNodes][i] ;
        for( int j = 0 ; j < InputNodes ; j++ ) { 
          ChangeHiddenWeights[j][i] = LearningRate * Input[p][j] * HiddenDelta[i] + Momentum * ChangeHiddenWeights[j][i];
          NN.HiddenWeights[j][i] += ChangeHiddenWeights[j][i] ;
        }
      }

/******************************************************************
* Update Hidden-->Output Weights
******************************************************************/
      for( int i = 0 ; i < OutputNodes ; i ++ ) {    
        ChangeOutputWeights[HiddenNodes][i] = LearningRate * OutputDelta[i] + Momentum * ChangeOutputWeights[HiddenNodes][i] ;
        NN.OutputWeights[HiddenNodes][i] += ChangeOutputWeights[HiddenNodes][i] ;
        for( int j = 0 ; j < HiddenNodes ; j++ ) {
          ChangeOutputWeights[j][i] = LearningRate * Hidden[j] * OutputDelta[i] + Momentum * ChangeOutputWeights[j][i] ;
          NN.OutputWeights[j][i] += ChangeOutputWeights[j][i] ;
        }
      }
    }

/******************************************************************
* Every 1000 cycles send data to terminal for display
******************************************************************/
    ReportEvery1000 = ReportEvery1000 - 1;
    if (ReportEvery1000 == 0)
    {
      Serial.println(); 
      Serial.println(); 
      Serial.print (F("TrainingCycle: "));
      Serial.print (TrainingCycle);
      Serial.print (F("  Error = "));
      Serial.println (Error, 5);
      toTerminal();
      if (TrainingCycle==1)
      {
        ReportEvery1000 = 999;
      }else{
        ReportEvery1000 = 1000;
      }
    }    

/******************************************************************
* If error rate is less than pre-determined threshold then end
******************************************************************/
    if( Error < Success ) break ;  
  }
  Serial.println ();
  Serial.println(); 
  Serial.print (F("TrainingCycle: "));
  Serial.print (TrainingCycle);
  Serial.print (F("  Error = "));
  Serial.println (Error, 5);
  toTerminal();
  
/******************************************************************
* Send HiddenWeights and OutputWeights to Serial
******************************************************************/
  Serial.println(); 
  Serial.println(F("  HiddenWeights: "));      
  for( int j = 0 ; j <= InputNodes ; j++ ) { 
    for( int i = 0 ; i < HiddenNodes ; i++ ) {
      Serial.print (NN.HiddenWeights[j][i], DEC);
      Serial.print (F(", "));
    }
    Serial.println(F(""));
  }
  Serial.println(); 
  Serial.println(F("  OutputWeights: "));  
  for( int j = 0 ; j <= HiddenNodes ; j++ ) { 
    for( int i = 0 ; i < OutputNodes ; i++ ) {
      Serial.print (NN.OutputWeights[j][i], DEC);
      Serial.print (F(", "));
    }
    Serial.println(F(""));
  }
  Serial.println (F("\n\nTraining Set Solved! "));
  Serial.println (F("--------\n\n")); 
  ReportEvery1000 = 1;  
}

void toTerminal(){
  for( int p = 0 ; p < PatternCount ; p++ ) { 
    Serial.println(); 
    Serial.print (F("  Training Pattern: "));
    Serial.println (p);      
    Serial.print (F("  Input "));
    for( int i = 0 ; i < InputNodes ; i++ ) {
      Serial.print (Input[p][i], DEC);
      Serial.print (F(" "));
    }
    Serial.print (F("  Target "));
    for( int i = 0 ; i < OutputNodes ; i++ ) {
      Serial.print (Target[p][i], DEC);
      Serial.print (F(" "));
    }
/******************************************************************
* Compute hidden layer activations
******************************************************************/
    for( int i = 0 ; i < HiddenNodes ; i++ ) {    
      Accum = NN.HiddenWeights[InputNodes][i] ;
      for( int j = 0 ; j < InputNodes ; j++ ) {
        Accum += Input[p][j] * NN.HiddenWeights[j][i] ;
      }
      Hidden[i] = 1.0/(1.0 + exp(-Accum)) ;
    }

/******************************************************************
* Compute output layer activations and calculate errors
******************************************************************/
    for( int i = 0 ; i < OutputNodes ; i++ ) {    
      Accum = NN.OutputWeights[HiddenNodes][i] ;
      for( int j = 0 ; j < HiddenNodes ; j++ ) {
        Accum += Hidden[j] * NN.OutputWeights[j][i] ;
      }
      Output[i] = 1.0/(1.0 + exp(-Accum)) ; 
    }
    Serial.print (F("  Output "));
    for( int i = 0 ; i < OutputNodes ; i++ ) {       
      Serial.print (Output[i], 5);
      Serial.print (F(" "));
    }
  }
  Serial.println(F("\nSolving neural network. Please wait."));
}

void useNN(int R, int G, int B){  // use NN hidden and output weights to compute outcome
  float measuredInput[3];
  measuredInput[0]=R/100.0;
  measuredInput[1]=G/100.0;
  measuredInput[2]=B/100.0;
  /******************************************************************
* Compute hidden layer activations
******************************************************************/
      for( int i = 0 ; i < HiddenNodes ; i++ ) {    
        Accum = NN.HiddenWeights[InputNodes][i] ;
        for( int j = 0 ; j < InputNodes ; j++ ) {
          Accum += measuredInput[j] * NN.HiddenWeights[j][i] ;
        }
        Hidden[i] = 1.0/(1.0 + exp(-Accum)) ;
      }

/******************************************************************
* Compute output layer activations and calculate errors
******************************************************************/
      for( int i = 0 ; i < OutputNodes ; i++ ) {    
        Accum = NN.OutputWeights[HiddenNodes][i] ;
        for( int j = 0 ; j < HiddenNodes ; j++ ) {
          Accum += Hidden[j] * NN.OutputWeights[j][i] ;
        }
        Output[i] = 1.0/(1.0 + exp(-Accum)) ;   
    }
}

void readSensor(){
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float thisread[3]={0,0,0}; 
  thisread[0] = g.gyro.x; 
  thisread[1] = g.gyro.y; 
  thisread[2] = g.gyro.z; 
  for(int i=0;i<3;i++){
    reading[i]=thisread[i]; 
  }
}

void readSample(){ // take one reading and apply neural network weights to calculate output
  readSensor(); // take measurement
  useNN(reading[0],reading[1],reading[2]);
  //Serial.print ("  Output ");
  float maxC=0.0;
  byte maxIdx=0;
  for( int i = 0 ; i < OutputNodes ; i++ ) {       
    Serial.print (Output[i], 3);
    if(Output[i]>maxC){
      maxC=Output[i];
      maxIdx=i;
    }
    Serial.print (F(" "));
  }
  if(maxC>0.8){
        Serial.print(" <--- "+TargetNames[maxIdx]+" detected");
  }
  Serial.println(F(""));
}

void printMenu(){ // user menu
  Serial.println(F("r: Read one sample"));
  Serial.println(F("c: Continuous reads"));
  Serial.println(F("t: train neural network with new values"));
  Serial.println(F("Enter choice: "));
}
