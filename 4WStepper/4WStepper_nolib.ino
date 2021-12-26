/* 4-Wire Generic Stepper (capable of full and half-steps)
 * David Dubins 12-Dec-18
 * see: http://www.hurst-motors.com/Technical_Help.html
 * for full and half-step sequences, and more info
 * 
 * Motor Driver to MCU:
 * IN1 to D8
 * IN2 to D9
 * IN3 to D10
 * IN4 to D11
 * +5V to MCU +5V (or external supply Vcc)
 * GND to MCU GND (and external supply GND if used)
*/
const int stepsPerRev=20; // change as needed for motor
const byte IN[4]={8,9,10,11}; // define motor pins as array

void setup() {
  Serial.begin(9600);
  for(int i=0;i<4;i++){
    pinMode(IN[i],OUTPUT);
  }
}

void loop() {
  Serial.println("Stepping clockwise in full steps.");
  motorStep(200,10);  // clockwise 200 steps @10rpm
  delay(1000);
  Serial.println("Stepping counter-clockwise in full steps.");
  motorStep(-200,10); // counter-clockwise 200 steps @10rpm
  delay(1000);
  Serial.println("Stepping clockwise in half steps.");
  motorStepHalf(200,10);  // CW 200 half steps @10rpm
  delay(1000);
  Serial.println("Stepping counter-clockwise in half steps.");
  motorStepHalf(-200,10); // CCW 200 half steps @10rpm
  delay(1000);
}

void motorStep(int mSteps, float rpm){
  //convert rpm to time delay:
  float t=60000.0/(rpm*stepsPerRev);
  const bool mSequence[4][4]={
    {1, 0, 0, 1}, // step 0
    {1, 0, 1, 0}, // step 1
    {0, 1, 1, 0}, // step 2
    {0, 1, 0, 1}  // step 3
  };
  static int mStep; // remember last val of mStep
  for(int i=0;i<abs(mSteps);i++){ // STEP pulses
    if(mSteps>0){ // clockwise
      mStep++;
      if(mStep>3)mStep=0;
    }else{  // counter-clockwise
      mStep--;
      if(mStep<0)mStep=3;      
    }
    for(int j=0;j<4;j++){      
      digitalWrite(IN[j],mSequence[mStep][j]);
    }
    delay_(t);
  }
}

void motorStepHalf(int mSteps, float rpm){
  float t=60000.0/(rpm*stepsPerRev*2.0);
  const bool mSequence[8][4]={ // for motor sequence
    {0, 1, 0, 1}, // step 0
    {0, 0, 0, 1}, // step 1    
    {1, 0, 0, 1}, // step 2
    {1, 0, 0, 0}, // step 3
    {1, 0, 1, 0}, // step 4
    {0, 0, 1, 0}, // step 5
    {0, 1, 1, 0}, // step 6
    {0, 1, 0, 0}  // step 7
  };
  static int mStep; // remember last val of mStep
  for(int i=0;i<abs(mSteps);i++){ // STEP pulses
    if(mSteps>0){ // clockwise
      mStep++;
      if(mStep>7)mStep=0;
    }else{  // counter-clockwise
      mStep--;
      if(mStep<0)mStep=7;      
    }
    for(int j=0;j<4;j++){
      digitalWrite(IN[j],mSequence[mStep][j]);
    }
    delay_(t);
  }
}

void delay_(float x){ // allows for delays <1ms
  if(x<16.383){ // delayMicroseconds() becomes less accurate after 16383 uS
    delayMicroseconds(t*1000.0); // use microsec delay if t < 16383
  }else{
    delay(t); // use to millisec delay if t >= 16.383 (will be important at speeds under 1 rpm)
  }  
}
