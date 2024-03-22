#include <Wire.h>
#include <icp101xx.h>
#include <SD.h>
#include <SPI.h>

ICP101xx sensor0; // Sensor connected to channel 0 of the multiplexer
ICP101xx sensor1; // Sensor connected to channel 1 of the multiplexer
ICP101xx sensor2; // Sensor connected to channel 2 of the multiplexer
ICP101xx sensor3; // Sensor connected to channel 3 of the multiplexer
String fileName;

// AD8232 pins
const int AD8232Output = A0; // AD8232 output pin connected to Arduino analog pin A0
const int LOPlus = 0; //pin LO+
const int LOMinus = 1; //pin LO-

void setup() {
  Serial.begin(9600);
  // delay(5000); // Consider removing or reducing this delay.
  
  Wire.begin();

  if (!SD.begin()) {
    // Use an LED or another indicator for failure
    Serial.println("Initialization of SD card failed!");
    while (true); // Halt indefinitely or implement a retry mechanism
  }
  Serial.println("SD card initialized.");

  pinMode(LOPlus, INPUT);
  pinMode(LOMinus, INPUT);

  // Find a new file name
  int fileIndex = 0;
  do {
    fileName = "data" + String(fileIndex) + ".csv";
    fileIndex++;
  } while (SD.exists(fileName));

  // Create a new CSV file and write the header
  File dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.println("Time,Sensor0 Pressure,Sensor0 Temperature,Sensor1 Pressure,Sensor1 Temperature,Sensor2 Pressure,Sensor2 Temperature,Sensor3 Pressure,Sensor3 Temperature,ECG Data");
    dataFile.close(); // Ensure data is saved
    Serial.println("Logging to: " + fileName);
  } else {
    // Consider an alternative error handling mechanism here as well
    Serial.println("Error creating new data file.");
    while(true); // Halt or handle error differently
  }

  Serial.println("Initialization complete. Starting data collection...");
}

void selectChannel(uint8_t channel) {
  Wire.beginTransmission(0x70); // Address of TCA9548A multiplexer
  Wire.write(1 << channel); // Enable channel
  Wire.endTransmission();
  delay(10); // Allow multiplexer to switch channels
}

void loop() {
  File dataFile = SD.open(fileName, FILE_WRITE);
  String dataString = String(millis()) + ","; // Time at the start of data string

  for (int channel = 0; channel < 4; channel++) {
    selectChannel(channel); // Select the correct sensor channel

    ICP101xx* sensor = (channel == 0) ? &sensor0 : 
                       (channel == 1) ? &sensor1 :
                       (channel == 2) ? &sensor2 : &sensor3;

    if (!sensor->begin()) {
      Serial.println("Sensor " + String(channel) + " not found. Check wiring.");
      dataString += "N/A,N/A,"; // Handle missing sensor data
      continue;
    }

    sensor->measure();
    if (sensor->dataReady()) {
      dataString += String(sensor->getPressurePa()) + "," + String(sensor->getTemperatureC()) + ",";
    } else {
      dataString += "N/A,N/A,"; // Handle unavailable sensor data
    }
  }

  bool leadsOff = digitalRead(LOPlus) == HIGH || digitalRead(LOMinus) == HIGH;
  dataString += leadsOff ? "LEADS_OFF" : String(analogRead(AD8232Output));

  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println("Data written to " + fileName);
  } else {
    Serial.println("Error opening " + fileName + " for writing.");
  }

  delay(1000); // 1 second delay between readings
}
