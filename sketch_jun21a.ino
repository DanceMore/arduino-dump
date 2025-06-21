/*
 * Arduino IR Remote Decoder - Compatible with Flipper Zero ir rx format
 * Outputs same protocol format as Flipper Zero for drop-in replacement compatibility
 * Connect IR receiver OUTPUT pin to Arduino pin 2
 * 
 * Wiring for IR Receiver Module:
 * - VCC -> 5V or 3.3V
 * - GND -> GND  
 * - OUTPUT -> Pin 2 (IR_RECEIVE_PIN)
 */

#include <IRremote.h>

// Pin configuration
const int IR_RECEIVE_PIN = 2;    // Digital pin for IR receiver OUTPUT
const int STATUS_LED_PIN = 13;   // Built-in LED for status indication

// IR receiver object
IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

// Protocol name mapping to match Flipper Zero format
String getProtocolName(decode_type_t protocol) {
  switch (protocol) {
    case NEC: return "NEC";
    case SONY: return "SIRC";  // Flipper calls Sony "SIRC"
    case RC5: return "RC5";
    case RC6: return "RC6";
//    case DISH: return "DISH";
    case SHARP: return "SHARP";
    case PANASONIC: return "Panasonic";
    case JVC: return "JVC";
//    case SANYO: return "SANYO";
//    case MITSUBISHI: return "Mitsubishi";
    case SAMSUNG: return "Samsung32";  // Match Flipper naming
    case LG: return "LG";
    case WHYNTER: return "WHYNTER";
//    case AIWA_RC_T501: return "AIWA";
    case DENON: return "DENON";
    case LEGO_PF: return "LEGO";
    case BOSEWAVE: return "BOSE";
    case MAGIQUEST: return "MAGIQUEST";
    default: return "UNKNOWN";
  }
}

// Format address and command as hex strings to match Flipper output
String formatHex(unsigned long value, int minDigits = 2) {
  String hex = "0x";
  String hexValue = String(value, HEX);
  hexValue.toUpperCase();
  
  // Pad with zeros if needed
  while (hexValue.length() < minDigits) {
    hexValue = "0" + hexValue;
  }
  
  hex += hexValue;
  return hex;
}

// Extract address from different protocol formats
unsigned long extractAddress(decode_results *results) {
  switch (results->decode_type) {
    case NEC:
      // NEC sends address in first 8 bits
      return (results->value >> 16) & 0xFF;
    
    case SONY:
      // Sony SIRC has variable address length
      return (results->value >> 7) & 0x1F;  // 5-bit address
    
    case SAMSUNG:
      // Samsung sends 8-bit address
      return (results->value >> 16) & 0xFF;
    
    case RC5:
    case RC6:
      // RC5/RC6 have 5-bit address
      return (results->value >> 6) & 0x1F;
    
    default:
      // For unknown protocols, try to extract from upper bits
      return (results->value >> 8) & 0xFF;
  }
}

// Extract command from different protocol formats  
unsigned long extractCommand(decode_results *results) {
  switch (results->decode_type) {
    case NEC:
      // NEC command is in lower 8 bits
      return results->value & 0xFF;
    
    case SONY:
      // Sony SIRC command is in lower 7 bits
      return results->value & 0x7F;
    
    case SAMSUNG:
      // Samsung command is in lower 8 bits
      return results->value & 0xFF;
    
    case RC5:
    case RC6:
      // RC5/RC6 command is in lower 6 bits
      return results->value & 0x3F;
    
    default:
      // For unknown protocols, use lower 8 bits
      return results->value & 0xFF;
  }
}

void setup() {
  Serial.begin(115200);  // Match Flipper Zero baud rate
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Initialize IR receiver
  irrecv.enableIRIn();
  
  // Send Flipper-compatible startup messages
  Serial.println("ir rx");
  Serial.println("Receiving...");
  Serial.println("Press Ctrl+C to stop");
  
  // Flash LED to indicate ready (silent - no serial output)
  for(int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(200);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(200);
  }
}

void loop() {
  if (irrecv.decode(&results)) {
    // Flash LED on signal received
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    // Skip repeat codes (similar to Flipper behavior)
    if (results.value != REPEAT) {
      // Get protocol name
      String protocol = getProtocolName(results.decode_type);
      
      // Extract address and command based on protocol
      unsigned long address = extractAddress(&results);
      unsigned long command = extractCommand(&results);
      
      // Format in exact Flipper Zero format: "PROTOCOL, A:0xADDR, C:0xCMD"
      String output = protocol + ", A:" + formatHex(address) + ", C:" + formatHex(command);
      Serial.println(output);
      
      // Optional: Also output raw hex for debugging (comment out for production)
      // Serial.println("Raw: 0x" + String(results.value, HEX) + " (" + String(results.bits) + " bits)");
    }
    
    // Resume receiving
    irrecv.resume();
    
    digitalWrite(STATUS_LED_PIN, LOW);
  }
  
  // Small delay to prevent overwhelming the serial port
  delay(50);
}

/*
 * USAGE NOTES:
 * 
 * 1. This Arduino will output the exact same format as Flipper Zero "ir rx" command
 * 2. Connect to your Python script using: python3 ir_mapper.py --device /dev/ttyACM0
 * 3. The Python script will work unchanged - just swap the device path
 * 4. All your remote mappings, Easter eggs, and channel dialing will work identically
 * 
 * WIRING:
 * IR Receiver Module -> Arduino
 * VCC    -> 5V (or 3.3V)
 * GND    -> GND
 * OUTPUT -> Pin 2
 * 
 * EXAMPLE OUTPUT:
 * NEC, A:0x32, C:0x11
 * Samsung32, A:0x07, C:0x12
 * SIRC, A:0x01, C:0x10
 * 
 * This matches exactly what your Flipper Zero outputs, so your existing
 * Python script will work without any modifications!
 */
