//Read from 4 ICP sensors using TCA9548A mux and display readings
#include <Wire.h>
#include <icp101xx.h>

#define TCAADDR 0x70
ICP101xx sensor0; // Sensor connected to channel 0
ICP101xx sensor1; // Sensor connected to channel 1
ICP101xx sensor2; // Sensor connected to channel 2
ICP101xx sensor3; // Sensor connected to channel 3

void tcaSelect(uint8_t i) {
  if (i > 7) return; // The TCA9548A has 8 channels, numbered 0-7

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i); // Select channel i
  Wire.endTransmission();
}

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud rate
  Wire.begin(); // Initialize I2C
  
  // Initialize each sensor by selecting its channel and checking if it's connected
  for (uint8_t i = 0; i < 4; i++) {
    tcaSelect(i);
    Serial.print("Initializing ICP10125 on channel ");
    Serial.print(i);
    Serial.println("...");
    
    if (!sensor0.begin()) {
      Serial.print("ICP10125 sensor on channel ");
      Serial.print(i);
      Serial.println(" not found. Please check wiring.");
      while (1); // Stop the program if a sensor is not found
    } else {
      Serial.print("ICP10125 sensor on channel ");
      Serial.print(i);
      Serial.println(" initialized.");
    }
  }
}

void readSensor(ICP101xx& sensor, uint8_t channel) {
  tcaSelect(channel); // Select the appropriate channel
  
  sensor.measure(); // Start a measurement
  if (sensor.dataReady()) {
    float temperature = sensor.getTemperatureC(); // Get temperature in Celsius
    float pressure = sensor.getPressurePa(); // Get pressure in Pascals

    Serial.print("Channel ");
    Serial.print(channel);
    Serial.print(" - Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
    Serial.print("Channel ");
    Serial.print(channel);
    Serial.print(" - Pressure: ");
    Serial.print(pressure);
    Serial.println(" Pa");
  } else {
    Serial.println("Waiting for data...");
  }
}

void loop() {
  // Read from all four sensors
  readSensor(sensor0, 0);
  readSensor(sensor1, 1);
  readSensor(sensor2, 2);
  readSensor(sensor3, 3);
 
  delay(2000); // Wait for 2 seconds before reading again
}
