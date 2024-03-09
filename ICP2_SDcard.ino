//Save readings from 2 ICP sensors and old heart rate sensor in SD card
#include <Wire.h>
#include <icp101xx.h>
#include "DFRobot_Heartrate.h"
 
 
#include <SD.h>
#include <SPI.h>
DFRobot_Heartrate heartrateSensor(ANALOG_MODE); // Initialize the heart rate sensor in analog mode
 
 
ICP101xx sensor0; // Sensor connected to channel 0 of the multiplexer
 
 
ICP101xx sensor1; // Sensor connected to channel 1 of the multiplexer
File dataFile;
 
 
void setup() {
 
 
  Serial.begin(9600);
 
 
  Wire.begin();
 
 
  Serial.println("Initializing TCA9548A multiplexer...");
 
 
  Wire.beginTransmission(0x70); // Address of TCA9548A multiplexer
 
 
  Wire.write(1 << 0); // Enable channel 0 (SD0, SC0)
 
 
  Wire.endTransmission();
 
 
  delay(100); // Delay to allow sensor to initialize
 
 
  // Check if the multiplexer is responding
 
 
  Wire.requestFrom(0x70, 1);
 
 
  //Test sd card
   if (!SD.begin()) {
 
 
  Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");
 
 
  
  dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Time,Sensor0 Pressure,Sensor0 Temperature,Sensor1 Pressure,Sensor1 Temperature,Heart Rate");
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }
 
 
    //
  if (Wire.available()) { 
 
    Serial.println("TCA9548A multiplexer found."); 
 
  } else { 
 
    Serial.println("TCA9548A multiplexer not found. Please check wiring."); 
 
    while (1); // Stop the program if multiplexer is not found 
 
  }
 
 
  Serial.println("Initializing ICP-10125 sensors..."); 
 
  if (!sensor0.begin()) { 
 
    Serial.println("ICP-10125 sensor 0 not found. Please check wiring."); 
 
    while (1); // Stop the program if sensor is not found 
 
  }
 
 
  Serial.println("ICP-10125 sensor 0 initialized."); 
 
  if (!sensor1.begin()) { 
 
    Serial.println("ICP-10125 sensor 1 not found. Please check wiring."); 
 
    while (1); // Stop the program if sensor is not found 
 
  }
 
 
  Serial.println("ICP-10125 sensor 1 initialized."); 
 
  Serial.println("Heart rate sensor initialized in analog mode."); 
 
  Serial.println("Initialization complete. Starting loop..."); 
 
}
 
 
void loop() {
 
 
  // Read data from sensor 0
 
 
  Serial.println("Reading data from sensor 0...");
 
  sensor0.measure(); // Start measurement in NORMAL mode 
 
  if (sensor0.dataReady()) {
 
 
    float temperature = sensor0.getTemperatureC(); // Get temperature in Celsius 
 
    float pressure = sensor0.getPressurePa(); // Get pressure in Pascals 
 
    Serial.print("Sensor 0 - Pressure: "); 
 
    Serial.print(pressure); 
 
    Serial.print(" Pa, Temperature: "); 
 
    Serial.print(temperature); 
 
    Serial.println(" C"); 
 
  } else { 
 
    Serial.println("Sensor 0 - Waiting for data..."); 
 
  }
 
 
  // Read data from sensor 1
 
  Serial.println("Reading data from sensor 1..."); 
 
  sensor1.measure(); // Start measurement in NORMAL mode 
 
  if (sensor1.dataReady()) { 
 
    float temperature = sensor1.getTemperatureC(); // Get temperature in Celsius 
 
    float pressure = sensor1.getPressurePa(); // Get pressure in Pascals
 
    Serial.print("Sensor 1 - Pressure: "); 
 
    Serial.print(pressure); 
 
    Serial.print(" Pa, Temperature: ");
 
    Serial.print(temperature); 
 
    Serial.println(" C"); 
 
  } else {
 
    Serial.println("Sensor 1 - Waiting for data..."); 
 
  }
 
 
  // Read data from heart rate sensor 
 
  Serial.println("Reading heart rate..."); 
 
  uint16_t heartRate = heartrateSensor.getValue(A3); 
 
  Serial.print("Heart Rate: ");
 
  //int sensorValue = analogRead(A3);
  // print out the value you read:
  //Serial.println(sensorValue);
 
  Serial.println(heartRate); 
 
  delay(1000); // Delay for 1 second before reading again
 
 
  File dataFile = SD.open("data.csv", FILE_WRITE);
 
 
  if (dataFile) {
    String dataString = "";
    dataString += String(millis()) + ",";
    dataString += String(sensor0.getPressurePa()) + ",";
    dataString += String(sensor0.getTemperatureC()) + ",";
    dataString += String(sensor1.getPressurePa()) + ",";
    dataString += String(sensor1.getTemperatureC()) + ",";
    dataString += String(heartrateSensor.getValue(A3));
    dataFile.println(dataString);
    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Error opening data.csv");
  }
  delay(1000);
}
