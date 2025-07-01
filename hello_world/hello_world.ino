// Arduino Hello World - Blink an LED
// This program blinks an LED connected to pin 13

int ledPin = 13;  // Pin number for the LED

void setup() {
  // Initialize the LED pin as an output
  pinMode(ledPin, OUTPUT);
  
  // Optional: Initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("Arduino Hello World - LED Blink Starting!");
}

void loop() {
  // Turn the LED on
  digitalWrite(ledPin, HIGH);
  Serial.println("LED ON");
  
  // Wait for 1 second (1000 milliseconds)
  delay(3000);
  
  // Turn the LED off
  digitalWrite(ledPin, LOW);
  Serial.println("LED OFF");
  
  // Wait for 1 second
  delay(3000);
  
  // The loop() function runs forever, so this will repeat
}
