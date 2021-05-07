/*  A4988 test sketch
 *  David Dubins 12-Dec-18
 *  
 *  Connections:
 *  ------------
 *  ENABLE -- Pin 11
 *  MS1 -- Pin 10
 *  MS2 -- Pin 9
 *  MS3 -- Pin 8
 *  (connect RESET to SLEEP)
 *  STEP -- Pin 7
 *  DIR -- Pin 6
 *    
 *  VMOT -- motor supply +8-32V (100 uF cap)
 *  GND -- motor supply GND
 *  2B -- B-
 *  2A -- B+
 *  1A -- A+
 *  1B -- A-
 *  VDD -- MCU +5V
 *  GND -- MCU GND
 */
const int stepsPerRev=200; //change as needed for motor
byte n;             // multiplier for microstepping
const byte ENA=11;  // set ENA low to enable motor
const byte MS1=10;  // microstepping pins
const byte MS2=9;
const byte MS3=8;
const byte STEP=7;  // sends a pulse for each step
const byte DIR=6;   // changes direction of stepper

void setup() {
  Serial.begin(9600);
  pinMode(ENA,OUTPUT);// set pins to output
  pinMode(MS1,OUTPUT);
  pinMode(MS2,OUTPUT);
  pinMode(MS3,OUTPUT);
  pinMode(STEP,OUTPUT);
  pinMode(DIR,OUTPUT);
  setMicroStep(0);  // 0:full step mode (choose 0-4)
}

void loop() {
  Serial.println("Stepping clockwise.");
  motorStep(200,10);  // clockwise 200 steps @10rpm
  delay(1000);
  Serial.println("Stepping counter-clockwise.");
  motorStep(-200,10); // CCW 200 steps @10rpm
  delay(1000);
}

void setMicroStep(byte RES){
  const bool MSvals[5][3]={
    {0, 0, 0}, // RES=0: full steps
    {1, 0, 0}, // RES=1: 1/2 steps
    {0, 1, 0}, // RES=2: 1/4 steps
    {1, 1, 0}, // RES=3: 8th steps
    {1, 1, 1}  // RES=4: 16th steps
  };
  digitalWrite(MS1,MSvals[RES][0]);
  digitalWrite(MS2,MSvals[RES][1]);
  digitalWrite(MS3,MSvals[RES][2]);
  n=pow(2,RES); // calculate multiplier for steps/rev
}

void motorStep(int mSteps, float rpm){
  //convert rpm to time delay:
  float t=60000.0/(rpm*stepsPerRev*n*2.0);
  unsigned int timer=millis();
  if(mSteps<0){ // set direction (use mSteps>0 to reverse)
    digitalWrite(DIR,0); // counter-clockwise
  }else{
    digitalWrite(DIR,1); // clockwise
  }
  digitalWrite(ENA,LOW);  // enable motor  
  for(int i=0;i<abs(mSteps);i++){ // STEP pulses
    digitalWrite(STEP,HIGH);
    delay_(t);
    digitalWrite(STEP,LOW);
    delay_(t);
  }
  digitalWrite(ENA,HIGH); // disable motor
}

void delay_(float x){ // allows for delays <1ms
  if(x>1.0){
    delay(x);
  }else{
    delayMicroseconds(x*1000.0); //convert to usec
  }  
}
