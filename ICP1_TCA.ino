//Code for reading from 1 ICP10125 sensor using TCA9548A mux
#include <Wire.h>
#include <icp101xx.h>
 
#define TCAADDR 0x70
ICP101xx sensor0;
 
void tcaSelect(uint8_t i) {
  if (i > 7) return; // The TCA9548A has 8 channels, numbered 0-7
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i); // Select channel i
  Wire.endTransmission();
}
 
void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud rate
  Wire.begin(); // Initialize I2C
  Serial.println("Initializing ICP10125 on channel 0...");
 
  tcaSelect(3); // Select channel 0 to communicate with the ICP10125
  if (!sensor0.begin()) {
    Serial.println("ICP10125 sensor not found. Please check wiring.");
    while (1); // Stop the program if the sensor is not found
  } else {
    Serial.println("ICP10125 sensor initialized.");
  }
}
 
void loop() {
  tcaSelect(3); // Ensure channel 0 is selected before reading from the sensor
 
  sensor0.measure(); // Start a measurement
  if (sensor0.dataReady()) {
    float temperature = sensor0.getTemperatureC(); // Get temperature in Celsius
    float pressure = sensor0.getPressurePa(); // Get pressure in Pascals
 
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" Pa");
  } else {
    Serial.println("Waiting for data...");
  }
 
  delay(2000); // Wait for 2 seconds before reading again
}
