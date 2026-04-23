//triacDimmer.ino: Example of forward-phase dimming
//Set up a 10K pot as a voltage divider on pin A0
byte crossPin=2; // Uno pin 2 -- H11AA1 pin 5
byte triacPin=3; // Uno pin 3 -- 400R -- MOC3010 pin 1
int dur=0; // duration for dimming 

void setup(){
  pinMode(crossPin,INPUT_PULLUP);
  pinMode(triacPin,OUTPUT);
  Serial.begin(9600); // start the Serial Monitor
}

void loop(){
  triacON(analogRead(A0)); // use 10K pot on A0
  Serial.println(dur);
}

void triacISR(){
  delayMicroseconds(dur);
  digitalWrite(triacPin,HIGH); // turn on TRIAC
  delayMicroseconds(10);
  digitalWrite(triacPin,LOW); // turn off TRIAC
}

void triacON(int drive){  // drive range: 0 to 1023
  int tMIN=1300; //min delay for triac (exptl, ~350)
  int tMAX=7660; //max delay for triac (exptl, ~8100)
  dur=map(drive,0,1023,tMIN,tMAX); // scale dur
  attachInterrupt(0,triacISR,RISING); // ISR to pin 2
}

void triacOFF(){ // use this to shut triac off
  detachInterrupt(0);
  digitalWrite(triacPin,LOW);
}
