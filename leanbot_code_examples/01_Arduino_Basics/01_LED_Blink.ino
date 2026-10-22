int ledPin = 13; // LED connected to digital pin 13
 
void setup() {
  pinMode(ledPin, OUTPUT); // Set the pin as an output pin
}
 
void loop() {
  digitalWrite(ledPin, HIGH); // Turn the LED on
  delay(1000); // Wait for 1 second
  digitalWrite(ledPin, LOW); // Turn the LED off
  delay(1000); // Wait for 1 second
}
