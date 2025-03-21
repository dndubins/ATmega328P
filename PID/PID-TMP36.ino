/*
PID.ino: Control a drive element using a PID strategy
Author: David Dubins
Date: September 2018
Last Updated: 12-Mar-25
Connections:
+5V to TMP36 Pin 1
Arduino Pin A0 to TMP36 Pin 2
GND to TMP36 Pin 3
Arduino Pin 11 to the base of an NPN transistor
DC fan red wire to ATX +9V
DC fan black wire to the collector of an NPN transistor
Connect arduino GND to 9V battery GND
*/

byte DRIVEPin = 11;    // Digital pin for PWM fan control
byte MESPin = A0;      // Analog pin A0 for temperature
                       // measurement
float SETPOINT = 22.0; // Setpoint temperature: what you would
                       // like temperature to be
float MEASURED = 0.0;  // Variable to store measured
                       // temperature

void setup() {
  Serial.begin(9600);  // Open serial port, set to 9600 bps
  pinMode(DRIVEPin, OUTPUT); // Generally not needed for PWM
}

void loop(){
  myPID(10.0, 1.0, 0.0); // call PID control here, entering
                       // values for kP, kI, and kD (other
                       // options available in PID subroutine)
  delay(500);          // add a short delay here
  // Other code can go here.
}

void myPID(float kP, float kI, float kD) {
  /*
  PID Algorithm:
  MEASURED: Measured value for feedback 
  MESpin: pin to measure 
  SETPOINT: value you would like the system to converge to
  DRIVEpin: pin to send drive signal
  TOLERANCE: Acceptable range from SETPOINT (adjust the system until Error is within TOLERANCE).
  kP: Proportional gain; kI: Integrator gain; kD: Derivative Gain
  IntThresh: Only run integrator only if Error < IntThresh (if control is almost done). Make this smallish, but greater than TOLERANCE (otherwise it will never engage).
  ScaleFactor: factor to scale P+I+D to a response (then after, impose 0-255 limits). 
               A negative scale factor will engage DRIVE if MEASURED < SETPOINT. 
               A positive scale factor will engage DRIVE if MEASURED > SETPOINT.
  */
  float TOLERANCE = 1.0; // tolerance of control (in this
                       // case, 1 means that PID won't do
                       // anything if measured temperature
                       // is within 1 degC of the setpoint)
  float IntThresh=3.0; // narrow region of integral term
                       // (proximity to SET)
  float ScaleFactor=-1.0; // Use this to change direction, and
                       // rescale DRIVE if necessary
                       // ScaleFactor=-1.0: Cooling.
                       // ScaleFactor=1.0: Heating.
  float Error = 0.0;    
  static float Integral = 0.0; // static remembers Integral on exit 
  float P = 0.0;
  float I = 0.0;
  float D = 0.0;
  static float LAST;    // static remembers LAST on exit 
  long DRIVE = 0;      // to store DRIVE value
  // -0.5V here is the offset for TMP36
  MEASURED = ((analogRead(MESPin)*5.0/1023.0)-0.5)*100.0; // convert
                     // from divs to degC
  Error = SETPOINT - MEASURED;
  if (abs(Error)<TOLERANCE){
    LAST = MEASURED;
    I = 0;
    Serial.println("SETPOINT within TOLERANCE.");
    return; // leave function
  }
  if (abs(Error) < IntThresh){ // prevent integral wind-up by
                     // only engaging it close to SET.
    Integral = Integral + Error; // add to Error Integral
  } else {     
    Integral=0.0;    // zero Integral if out of bounds    
  }        
  P = Error*kP;      // calculate proportional term      
  I = Integral*kI;   // integral term      
  D = (LAST-MEASURED)*kD; // derivative term     
  DRIVE = P + I + D; // Total DRIVE = P+I+D      
  DRIVE = (long)(DRIVE*ScaleFactor); // used to reverse
                  // direction, and scale DRIVE to be
                  // in the range 0-255. 
  DRIVE = constrain(DRIVE,0,255); // make sure DRIVE is an
                     // integer between 0 and 255. An offset
                     // can be used to get the DRIVE moving,
                     // e.g. constrain(68+DRIVE,0,255).
  Serial.print("SETPOINT: "); // info to serial monitor
  Serial.print(SETPOINT);  
  Serial.print(", MEASURED: ");
  Serial.print(MEASURED);
  Serial.print(", ERROR: ");
  Serial.print(Error);
  Serial.print(", P: ");  
  Serial.print(P);  
  Serial.print(", I: ");  
  Serial.print(I);  
  Serial.print(", D: ");  
  Serial.print(D);  
  Serial.print(", DRIVE: ");  
  Serial.println(DRIVE);
                     // replace all Serial.print commands
                     // with the following command to
                     // observe the response on the serial
                     // plotter:
  //Serial.println((String)SETPOINT+", "+(String)MEASURED);
  LAST = MEASURED;   // save current value for next time
  analogWrite(DRIVEPin, DRIVE); // send DRIVE as PWM signal
}
