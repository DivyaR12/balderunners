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
  
  Wire.begin();

  if (!SD.begin()) {
    Serial.println("Initialization of SD card failed!");
    while (true); // Halt indefinitely or implement a retry mechanism
  }
  Serial.println("SD card initialized.");

  pinMode(LOPlus, INPUT);
  pinMode(LOMinus, INPUT);

  // Find a new file name
  int fileIndex = 0;
  do {
    fileName = "ECG_250Hz_data" + String(fileIndex) + ".csv"; // File name now indicates sampling rate
    fileIndex++;
  } while (SD.exists(fileName));

  // Create a new CSV file and write the header and sampling rate note
  File dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.println("Sampling Rate: 250 Hz"); // Note about the sampling rate
    dataFile.println("Time,Sensor0 Pressure,Sensor0 Temperature,Sensor1 Pressure,Sensor1 Temperature,Sensor2 Pressure,Sensor2 Temperature,Sensor3 Pressure,Sensor3 Temperature,ECG Data");
    dataFile.close();
    Serial.println("Logging to: " + fileName);
  } else {
    Serial.println("Error creating new data file.");
    while(true);
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
  unsigned long startTime = millis();
  
  File dataFile = SD.open(fileName, FILE_WRITE);
  String dataString = String(millis()) + ","; // Time at the start of data string

  // Loop through channels, collect data
  for (int channel = 0; channel < 4; channel++) {
    selectChannel(channel);

    ICP101xx* sensor = (channel == 0) ? &sensor0 : 
                       (channel == 1) ? &sensor1 :
                       (channel == 2) ? &sensor2 : &sensor3;

    if (!sensor->begin()) {
      Serial.println("Sensor " + String(channel) + " not found. Check wiring.");
      dataString += "N/A,N/A,";
      continue;
    }

    sensor->measure();
    if (sensor->dataReady()) {
      dataString += String(sensor->getPressurePa()) + "," + String(sensor->getTemperatureC()) + ",";
    } else {
      dataString += "N/A,N/A,";
    }
  }

  // Read ECG data, check for lead-off condition
  bool leadsOff = digitalRead(LOPlus) == HIGH || digitalRead(LOMinus) == HIGH;
  dataString += leadsOff ? "LEADS_OFF" : String(analogRead(AD8232Output));

  // Write data to file
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println("Data written to " + fileName);
  } else {
    Serial.println("Error opening " + fileName + " for writing.");
  }

  // Attempt to maintain a 250Hz sampling rate
  while (millis() - startTime < 4); // Loop cycle target time for 250 Hz sampling rate
}
