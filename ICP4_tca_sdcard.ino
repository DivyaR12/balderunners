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

  // No need to check multiplexer response here as initialization success of sensors will implicitly confirm its presence and operation

  // Initialize sensors here (Note: Actual initialization occurs in loop due to channel switching)

  Serial.println("Initialization complete. Starting loop...");
}

void selectChannel(uint8_t channel) {
  Wire.beginTransmission(0x70); // Address of TCA9548A multiplexer
  Wire.write(1 << channel); // Enable channel (0-7)
  Wire.endTransmission();
}

void loop() {
  for (int channel = 0; channel < 4; channel++) {
    selectChannel(channel); // Select multiplexer channel for sensor

    ICP101xx* sensor;
    if (channel == 0) sensor = &sensor0;
    else if (channel == 1) sensor = &sensor1;
    else if (channel == 2) sensor = &sensor2;
    else if (channel == 3) sensor = &sensor3;

    Serial.print("Reading data from sensor ");
    Serial.println(channel);

    if (!sensor->begin()) {
      Serial.print("ICP-10125 sensor ");
      Serial.print(channel);
      Serial.println(" not found. Please check wiring.");
      continue; // Skip this sensor if not found
    }

    sensor->measure(); // Start measurement in NORMAL mode

    if (sensor->dataReady()) {
      float temperature = sensor->getTemperatureC(); // Get temperature in Celsius
      float pressure = sensor->getPressurePa(); // Get pressure in Pascals

      Serial.print("Sensor ");
      Serial.print(channel);
      Serial.print(" - Pressure: ");
      Serial.print(pressure);
      Serial.print(" Pa, Temperature: ");
      Serial.print(temperature);
      Serial.println(" C");

      // Write data to SD card
      File dataFile = SD.open("data.csv", FILE_WRITE);
      if (dataFile) {
        String dataString = "";
        dataString += String(millis()) + ",";
        dataString += String(pressure) + ",";
        dataString += String(temperature) + ",";
        dataFile.println(dataString);
        dataFile.close();
        Serial.println("Data written to SD card.");
      } else {
        Serial.println("Error opening data.csv");
      }
    } else {
      Serial.println("Waiting for data...");
    }
  }
  delay(1000); // Delay for 1 second before next read
}
