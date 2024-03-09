#include <Wire.h>
#include <icp101xx.h>
#include <SD.h>
#include <SPI.h>

ICP101xx sensor0; // Sensor connected to channel 0 of the multiplexer
ICP101xx sensor1; // Sensor connected to channel 1 of the multiplexer
ICP101xx sensor2; // Sensor connected to channel 2 of the multiplexer
ICP101xx sensor3; // Sensor connected to channel 3 of the multiplexer

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only.
  }
  Serial.println("Starting...");

  Wire.begin();

  if (!SD.begin()) {
    Serial.println("Initialization of SD card failed!");
    return; // Don't proceed further if the SD card initialization fails.
  }
  Serial.println("SD card initialized.");

  // Create or open the CSV file and write the header
  File dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    if (dataFile.size() == 0) {
      // Write the header if the file is new
      dataFile.println("Time,Sensor0 Pressure,Sensor0 Temperature,Sensor1 Pressure,Sensor1 Temperature,Sensor2 Pressure,Sensor2 Temperature,Sensor3 Pressure,Sensor3 Temperature");
    }
    dataFile.close();
  } else {
    Serial.println("Error opening data.csv");
  }

  Serial.println("Initialization complete. Starting data collection...");
}

void selectChannel(uint8_t channel) {
  Wire.beginTransmission(0x70); // Address of TCA9548A multiplexer
  Wire.write(1 << channel); // Enable channel (0-7)
  Wire.endTransmission();
}

void loop() {
  String dataString = String(millis()) + ","; // Start the data string with the current time

  for (int channel = 0; channel < 4; channel++) {
    selectChannel(channel); // Select multiplexer channel for sensor

    ICP101xx* sensor;
    if (channel == 0) sensor = &sensor0;
    else if (channel == 1) sensor = &sensor1;
    else if (channel == 2) sensor = &sensor2;
    else sensor = &sensor3;

    if (!sensor->begin()) {
      Serial.print("ICP-10125 sensor ");
      Serial.print(channel);
      Serial.println(" not found. Please check wiring.");
      dataString += "N/A,N/A,"; // Append N/A for missing sensors
      continue; // Skip this sensor if not found
    }

    sensor->measure(); // Start measurement in NORMAL mode

    if (sensor->dataReady()) {
      float temperature = sensor->getTemperatureC(); // Get temperature in Celsius
      float pressure = sensor->getPressurePa(); // Get pressure in Pascals

      dataString += String(pressure) + "," + String(temperature) + ",";
    } else {
      dataString += "N/A,N/A,"; // In case data is not ready, append N/A
    }
  }

  // Remove the last comma
  dataString.remove(dataString.length() - 1);

  // Write the data string to the SD card
  File dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Error opening data.csv");
  }

  delay(1000); // Delay for 1 second before next read
}
