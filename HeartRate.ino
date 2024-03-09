//Raeding heart reate from new sesnor-AD8232 and displaying it 
const int heartPin = A1; // AD8232 output connected to Analog pin A1

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud
  pinMode(heartPin, INPUT); // Initialize the heart rate sensor pin as an input
}

void loop() {
  int heartRateValue = analogRead(heartPin); // Read the value from the heart rate sensor
  unsigned long timestamp = millis(); // Get the current time
  
  // Print the heart rate value with a timestamp
  Serial.print("Heart rate at time ");
  Serial.print(timestamp);
  Serial.print("ms is ");
  Serial.println(heartRateValue);
  
  delay(1000); // Delay for 1 second before the next read
}
