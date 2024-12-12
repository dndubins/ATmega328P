/* TCS3200 color recognition sensor 
 * This sketch spits out single reads from each channel of the TCS3200 (red, green, blue).
 * Sketch adapted from: https://electronicsforu.com/electronics-projects/rgb-color-detector-tcs3200-sensor-module
 * http://www.efymag.com/admin/issuepdf/RGB-Colour-Detection-Using-TCS3200_3-17.rar

Color Sensor - Arduino Uno
---------------------------
 VCC -- +5V
 GND -- GND
 S0 -- Pin 8
 S1 -- Pin 9
 S2 -- Pin 12
 S3 -- Pin 11
 OUT -- Pin 10
 OE -- GND
*/

#define S0 8  
#define S1 9
#define S2 12
#define S3 11
#define OUT 10

int reading[3]={0,0,0}; // to store red, green, blue reading
    
void setup(){   
  Serial.begin(9600); 
  pinMode(S0, OUTPUT);  
  pinMode(S1, OUTPUT);  
  pinMode(S2, OUTPUT);  
  pinMode(S3, OUTPUT);  
  pinMode(OUT, INPUT);  
  digitalWrite(S0, HIGH);  
  digitalWrite(S1, HIGH);  
}  
    
void loop(){
  readColour();
  Serial.print("R,G,B: ");
  Serial.print(reading[0]);
  Serial.println(",");
  Serial.print(reading[1]);
  Serial.println(",");
  Serial.println(reading[2]);
  delay(500);
 }  
    
void readColour(){ 
  digitalWrite(S2, LOW);  
  digitalWrite(S3, LOW);  
  reading[0] = pulseIn(OUT, !digitalRead(OUT)); //read red  
  digitalWrite(S3, HIGH);  
  reading[2] = pulseIn(OUT, !digitalRead(OUT)); //read blue
  digitalWrite(S2, HIGH);  
  reading[1] = pulseIn(OUT, !digitalRead(OUT)); //read green  
}
