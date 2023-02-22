// GY-521_CSV.ino
// Basic demo for accelerometer readings from Adafruit MPU6050
// Library: Adafruit MPU6050 by Adafruit, v.2.2.4
// Adapted from Adafruit MPU6050 basic_readings.ino sketch by Adafruit, v 2.2.4 by D. Dubins
// Output in CSV format (x,y,z acceleration, then x,y,z rotation)
//
// Wiring:
// GY-521 - Uno
// ------------
// INT - Pin 2
// ADO - GND
// XCL - (NC)
// XDA - (NC)
// SDA - SDA (or A4)
// SCL - SCL (or A5)
// GND - GND
// VCC - +5V

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setup(void) {
  Serial.begin(115200);
  // Initialize sensor
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
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
}

void loop() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  //Print out the acceleration values (in m/s^2)
  Serial.print(a.acceleration.x);
  Serial.print(", ");
  Serial.print(a.acceleration.y);
  Serial.print(", ");
  Serial.print(a.acceleration.z);
  Serial.print(",      ");

  //Print out the gyroscopic values (in rad/s)
  Serial.print(g.gyro.x);
  Serial.print(", ");
  Serial.print(g.gyro.y);
  Serial.print(", ");
  Serial.print(g.gyro.z);
  Serial.println("");

  delay(100); // set the delay here
}
