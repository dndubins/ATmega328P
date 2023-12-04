// BMP280.ino
// Library: BMx280MI v1.2.3 by Gregor Christandl
// Original Sketch: BMX280_I2C.ino by Gregor Christandl
// Modified by: D. Dubins
// Date: 04-Dec-23
//
// BMP280 - Uno:
// VIN -- +5V
// GND -- GND
// SDA -- SDA
// SCL -- SCL

#include <BMx280I2C.h>
BMx280I2C BMP280(0x76); // Default bit address is 0x76. use I2C_Scanner.ino to find bit address if this is incorrect
float temperature=0.0;
float pressure=0.0;
float humidity=0.0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Initializing...");
  if (!BMP280.begin()) {
    Serial.println("Failed to initialize. Check wiring and I2C address.");
    while (1);
  }
  BMP280.resetToDefaults();
  BMP280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
  BMP280.writeOversamplingTemperature(BMx280MI::OSRS_T_x16);
  BMP280.writeOversamplingHumidity(BMx280MI::OSRS_H_x16);
}

void loop() {
  getReading(pressure, temperature, humidity); // get a reading
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  delay(1000);  // time between measurements
}

void getReading(float &p, float &t, float &h){
  BMP280.measure(); // take a measurement
  while (!BMP280.hasValue()); // wait for measurement to finish
  p=BMP280.getPressure(); //for 64 bit pressure, replace with BMP280.getPressure64()
  t=BMP280.getTemperature();
  h=BMP280.getHumidity();
}
