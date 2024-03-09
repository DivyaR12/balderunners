//Store values from 4 ICP10125 sensors in data.csv file while monitoring timestamps and controlling the time intervals for readings
#include <Wire.h>
#include <icp101xx.h>
#include <SD.h>
#include <SPI.h>

ICP101xx sensor0; // Sensor connected to channel 0 of the multiplexer
ICP101xx sensor1; // Sensor connected to channel 1 of the multiplexer
ICP101xx sensor2; // Sensor connected to channel 2 of the multiplexer
ICP101xx sensor3; // Sensor connected to channel 3 of the multiplexer
File dataFile;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Initializing TCA9548A multiplexer...");

  if (!SD.begin()) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Time,Sensor0 Pressure,Sensor0 Temperature,Sensor1 Pressure,Sensor1 Temperature,Sensor2 Pressure,Sensor2 Temperature,Sensor3 Pressure,Sensor3 Temperature");
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }

  Serial.println("Initialization complete. Starting loop...");
}

void selectChannel(uint8_t channel) {
  Wire.beginTransmission(0x70);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void loop() {
  unsigned long period = 180000; // 3 minutes in milliseconds
  unsigned long startLoopTime = millis();
  unsigned long readingStartTime, readingEndTime, totalReadingTime;

  for (int channel = 0; channel < 4; channel++) {
    readingStartTime = millis();
    selectChannel(channel);

    ICP101xx* sensor = (channel == 0) ? &sensor0 :
                       (channel == 1) ? &sensor1 :
                       (channel == 2) ? &sensor2 :
                                         &sensor3; 

    sensor->measure();
    while (!sensor->dataReady()) {
      delay(10); // Wait for data to become ready
    }
    float temperature = sensor->getTemperatureC();
    float pressure = sensor->getPressurePa();
    readingEndTime = millis();
    totalReadingTime += readingEndTime - readingStartTime;

    // Only open the file once per loop to optimize SD card operations
    if (channel == 0) {
      dataFile = SD.open("data.csv", FILE_WRITE);
      if (!dataFile) {
        Serial.println("Error opening data.csv");
        return;
      }
      dataFile.print(String(millis()) + ","); // Timestamp for the first sensor
    }

    // Write sensor data to the file
    dataFile.print(String(pressure) + ",");
    dataFile.print(String(temperature));
    if (channel == 3) {
      dataFile.println();
      dataFile.close();
    } else {
      dataFile.print(","); // Prepare for the next sensor's data
    }
  }

  // Calculate and adjust delay to maintain the desired interval
  unsigned long endLoopTime = millis();
  unsigned long loopExecutionTime = endLoopTime - startLoopTime;
  unsigned long targetDelay = period / 60; // Aim for 60 readings in 3 minutes
  if (loopExecutionTime < targetDelay) {
    delay(targetDelay - loopExecutionTime);
  }
}
