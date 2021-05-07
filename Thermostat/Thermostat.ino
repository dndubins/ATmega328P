/* Thermostat.ino: Simple thermostat program (no sensor data  
 * averaging)
 * Author: David Dubins
 * Date: Sep 2018
 * Connections:
 * +3.3V--10K resistor--pin A1--10K thermistor--GND
 * 3.3V to AREF pin
 * Pin 6 to relay module 
 * (or Pin 6 to LED--220R resistor--GND to test)
*/

byte tempPin=A1;       // Declaring the Analog input to be A1
                       // of Arduino board.
byte relayPin=6;       // For an LED (or relay)
float tempC=0.0;       // For holding Celcius temp (floating
                       // for decimal points precision)
float R = 0.0;         // Variable for reading the measured
                       // resistance of the thermistor
float R0 = 11930;      // Resistance (Ohms) of thermistor at
                       // room temperature
float T0 = 273.15 + 22.5;  //Room temperature in Kelvin
float R1 = 10000.0;    // Resistance (Ohms) of sense resistor
                       // in voltage divider (should be ~10000
                       // Ohms, measure for better accuracy)
float volts = 0.0;     // Variable to read in voltage (to be
                       // converted to resistance)
float B = 3672.433;    // B Coefficient of Thermistor. Enter B
                       // coefficient here.
float setTemp = 37.0;  // Setpoint temperature for thermostat                                            

void setup(){
  analogReference(EXTERNAL);  // Use external (3.3V) AREF.
                       // Make sure 3.3V connected to AREF pin
                       // or or analogRead won't work!
  Serial.begin(9600);  // Open serial port, set to 9600 bps
  Serial.println("Volts(V), Resistance(Ohm), Temperature(C)");
                       // Set up column titles
  pinMode(relayPin, OUTPUT);  // Set up relayPin as output to
                       // send a digital signal to the relay
                       // module.
}

void loop(){
  readTemp();          // Read the temperature (void fn below)
 // Now ouput the results to the serial monitor:
  Serial.print(volts); // Print Volts
  Serial.print(",");   // Print a comma for CSV file
  Serial.print(R);     // Print Resistance
  Serial.print(",");   // Print a comma for CSV file
  Serial.println(tempC,2); // Print Temperature and new line
  if(tempC < setTemp){ // if measured temp less than set temp
    digitalWrite(relayPin, HIGH); // then turn on relay
  } else {
    digitalWrite(relayPin, LOW); // otherwise turn off relay
  }
  delay(1000);         // Wait 1sec before taking next measure
}

void readTemp(){
  volts = analogRead(tempPin) * 3.3 / 1023.0;  // Take sensor
                       // reading from tempPin in divs (Scale
                       // 0-1023), and convert to volts
  R = volts*R1/(3.3-volts); // Convert voltage to resistance
  tempC = (1/T0)+((1/B)*log(R/R0)); // Use the simplified B-
                       // coefficient thermistor formula to
                       // calculate temperature
  tempC = 1.0/tempC;   // invert the answer
  tempC = tempC - 273.15; // Convert from Kelvin to Celsius
}
