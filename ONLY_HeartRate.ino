// Define the AD8232 output pin
const int AD8232Output = A1;
// Define the AD8232 LO+ and LO- pins (Leads off detection)
const int LOPlus = 10;
const int LOMinus = 5;

void setup() {
  // Initialize Serial communication
  Serial.begin(9600);
  // Configure the leads off detection pins as inputs
  pinMode(LOPlus, INPUT);
  pinMode(LOMinus, INPUT);
}

void loop() {
  // Check if leads are off
  if (digitalRead(LOPlus) == HIGH || digitalRead(LOMinus) == HIGH) {
    Serial.println("Leads off detected");
  } else {
    // Read the analog value from sensor
    int ecgValue = analogRead(AD8232Output);
    // Print the analog value
    Serial.println(ecgValue);
  }
  // Small delay to stabilize readings
  delay(10);
}
