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
const int LOPlus = 10; //pin LO+
const int LOMinus = 5; //pin LO-

void setup() {
  Serial.begin(9600);
  delay(5000); // Start delay to ensure SD card is ready
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only.
  }
  Serial.println("Starting...");

  Wire.begin();

  if (!SD.begin()) {
    Serial.println("Initialization of SD card failed!");
    return;
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

  // Create a new CSV file and write the header, including ECG Data column
  File dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    Serial.print("Logging to: ");
    Serial.println(fileName);
    dataFile.println("Time,Sensor0 Pressure,Sensor0 Temperature,Sensor1 Pressure,Sensor1 Temperature,Sensor2 Pressure,Sensor2 Temperature,Sensor3 Pressure,Sensor3 Temperature,ECG Data");
    dataFile.close(); // Ensure data is saved
  } else {
    Serial.println("Error creating new data file.");
  }

  Serial.println("Initialization complete. Starting data collection...");
}

void selectChannel(uint8_t channel) {
  Wire.beginTransmission(0x70); // Address of TCA9548A multiplexer
  Wire.write(1 << channel); // Enable channel (0-7)
  Wire.endTransmission();
  delay(10); // Short delay to ensure the multiplexer has time to switch channels
}

void loop() {
  // Open the new data file for this session
  File dataFile = SD.open(fileName, FILE_WRITE);
  
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

  // Check leads off status before reading ECG
  bool leadsOff = digitalRead(LOPlus) == HIGH || digitalRead(LOMinus) == HIGH;
  if (leadsOff) {
    // If leads are off, log a specific value or message
    dataString += "LEADS_OFF";
  } else {
    // If leads are not off, read the ECG value
    int ecgValue = analogRead(AD8232Output);
    // Append the ECG value to the data string
    dataString += String(ecgValue);
  }

  if (dataFile) {
    data    dataFile.println(dataString);
    dataFile.close(); // Close the file to ensure data is saved
    Serial.println("Data written to " + fileName);
  } else {
    Serial.println("Error opening " + fileName + " for writing.");
  }

  delay(1000); // Delay for 1 second before next read
}
