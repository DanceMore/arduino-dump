/*
 * Simplified Arduino IR Receiver
 * Focus on clean data capture using IRremote.h best practices
 * 
 * Wiring:
 * IR Receiver Module -> Arduino
 * VCC    -> 5V
 * GND    -> GND
 * OUTPUT -> Pin 2
 */

#include <IRremote.h>

// Pin configuration
const int IR_RECEIVE_PIN = 2;
const int STATUS_LED_PIN = 13;

void setup() {
  Serial.begin(115200);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Initialize IR receiver with proper settings
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
  // Flipper Zero compatible startup messages
  Serial.println("ir rx");
  Serial.println("Receiving...");
  Serial.println("Press Ctrl+C to stop");
  
  // Ready indicator (silent)
  digitalWrite(STATUS_LED_PIN, HIGH);
  delay(500);
  digitalWrite(STATUS_LED_PIN, LOW);
}

// Convert protocol names to match Flipper Zero format
String getFlipperProtocolName(String arduinoProtocol) {
  if (arduinoProtocol == "Sony") return "SIRC";
  if (arduinoProtocol == "Samsung32") return "Samsung32";
  if (arduinoProtocol == "Samsung") return "Samsung32";
  // Most others match directly
  return arduinoProtocol;
}

void loop() {
  // Check if we received IR data
  if (IrReceiver.decode()) {
    
    // Get basic info
    String protocol = getFlipperProtocolName(IrReceiver.getProtocolString());
    uint16_t address = IrReceiver.decodedIRData.address;
    uint16_t command = IrReceiver.decodedIRData.command;
    
    // Only process non-repeat signals
    if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
      
      // Flash LED briefly
      digitalWrite(STATUS_LED_PIN, HIGH);
      
      // Output in exact Flipper Zero format: "PROTOCOL, A:0xADDR, C:0xCMD"
      Serial.print(protocol);
      Serial.print(", A:0x");
      if (address < 0x10) Serial.print("0");  // Ensure 2-digit hex
      Serial.print(address, HEX);
      Serial.print(", C:0x");
      if (command < 0x10) Serial.print("0");  // Ensure 2-digit hex
      Serial.println(command, HEX);
      
      digitalWrite(STATUS_LED_PIN, LOW);
    }
    
    // Enable receiving of the next IR signal
    IrReceiver.resume();
  }
  
  delay(50);
}

/*
 * USAGE NOTES:
 * 
 * This Arduino now outputs EXACTLY the same format as Flipper Zero "ir rx":
 * 
 * EXPECTED OUTPUT:
 * ir rx
 * Receiving...
 * Press Ctrl+C to stop
 * NEC, A:0x32, C:0x12
 * NEC, A:0x32, C:0x10
 * SIRC, A:0x01, C:0x75
 * Samsung32, A:0x07, C:0x12
 * 
 * INTEGRATION:
 * 1. Connect to your Python script: python3 ir_mapper.py --device /dev/ttyACM0
 * 2. Your existing Python script should work without ANY modifications
 * 3. Just change the device path from Flipper to Arduino serial port
 * 
 * SERIAL PORTS:
 * - Linux: /dev/ttyACM0 or /dev/ttyUSB0
 * - Windows: COM3, COM4, etc.
 * - Mac: /dev/cu.usbmodem* or /dev/cu.usbserial*
 */
