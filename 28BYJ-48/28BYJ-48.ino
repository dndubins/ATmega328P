/* Test sketch: 28BYJ-48 Stepper with ULN2003
 * David Dubins 15-Dec-20
 * Last Updated: 25-Dec-21
 *
 * Connections:
 * Plug the connector of the 28BYJ-48 stepper motor directly into the socket of the ULN2003.
 *
 * ULN2003 to MCU:
 * IN1 to D8
 * IN2 to D9
 * IN3 to D10
 * IN4 to D11
 * +5V to MCU +5V (or external supply Vcc)
 * GND to MCU GND (and external supply GND if used)
*/

const unsigned long stepsPerRev=4096;
const byte IN[4]={8,9,10,11}; // define motor pin array

void setup() {
  Serial.begin(9600);
  for(int i=0;i<4;i++){
    pinMode(IN[i],OUTPUT); // set pins to output mode
  }
}

void loop() {
  Serial.println("Stepping clockwise.");
  motorStep(4096,15);  // CW 4096 steps @15rpm
  delay(1000);
  Serial.println("Stepping counter-clockwise.");
  motorStep(-4096,15); // CCW 4096 steps @15rpm
  delay(1000);
}

void motorStep(int mSteps, float rpm){
  //convert rpm to time delay in microsec per step:
  unsigned long t=60000000/(rpm*stepsPerRev);
  const bool mSequence[8][4]={
    {1, 0, 0, 1}, // step 0
    {1, 0, 0, 0}, // step 1
    {1, 1, 0, 0}, // step 2
    {0, 1, 0, 0}, // step 3
    {0, 1, 1, 0}, // step 4
    {0, 0, 1, 0}, // step 5
    {0, 0, 1, 1}, // step 6
    {0, 0, 0, 1}  // step 7    
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
    if(t<16383){ // delayMicroseconds() becomes less accurate after 16383 uS
      delayMicroseconds(t); // use microsec delay if t < 16383
    }else{
      delay(t/1000); // use to millisec delay if t >= 16383
    }
  }
}
