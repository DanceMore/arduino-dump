/*
 * Arduino IR Receiver with Jumper-Controlled Debug Mode
 * Compatible with Flipper Zero ir rx format + optional debugging
 * 
 * Debug Control:
 * - Insert jumper between Pin 10 and GND for DEBUG_MODE=true
 * - Remove jumper for production/Flipper Zero compatibility
 * - No recompilation needed!
 * 
 * Wiring:
 * IR Receiver Module -> Arduino
 * VCC    -> 5V
 * GND    -> GND
 * OUTPUT -> Pin 2
 * 
 * Debug Jumper:
 * Pin 10 -> GND (jumper wire or actual jumper)
 * 
 * LEDs:
 * - Your Arduino board has a built-in LED (usually on pin 13)
 * - Your IR receiver module has its own indicator LED
 */

#include <IRremote.h>

// Configuration (will be overridden by jumper detection)
bool DEBUG_MODE = false;                 // Determined by jumper on pin 10
const bool SHOW_REPEATS = false;        // Set to true to show repeat signals in debug mode
const bool SHOW_RAW_DATA = false;       // Set to true to show raw hex values in debug mode
const unsigned long BAUD_RATE = 115200; // Serial communication speed

// Pin configuration
const int IR_RECEIVE_PIN = 2;
const int DEBUG_JUMPER_PIN = 10;         // Pin 10 to GND = debug mode

// Statistics tracking
unsigned long totalSignals = 0;
unsigned long validSignals = 0;
unsigned long lastSignalTime = 0;

// Check if debug jumper is installed
bool isDebugJumperInstalled() {
  pinMode(DEBUG_JUMPER_PIN, INPUT_PULLUP);
  delay(10); // Small delay for stable reading
  return digitalRead(DEBUG_JUMPER_PIN) == LOW; // LOW = jumper to ground
}

void setup() {
  Serial.begin(BAUD_RATE);
  delay(2000); // sleep so serial port can wake up...

  // DIAGNOSTIC: Check jumper status with detailed info
  pinMode(DEBUG_JUMPER_PIN, INPUT_PULLUP);
  delay(100); // Longer delay for stable reading

  bool jumperDetected = (digitalRead(DEBUG_JUMPER_PIN) == LOW);

  // Print diagnostic info
  //Serial.println("=== JUMPER DIAGNOSTIC ===");
  //Serial.print("Pin 10 raw reading: ");
  //Serial.println(digitalRead(DEBUG_JUMPER_PIN));
  //Serial.print("Pin 10 voltage: ");
  //Serial.println(digitalRead(DEBUG_JUMPER_PIN) == HIGH ? "HIGH (3.3V/5V)" : "LOW (0V/GND)");
  //Serial.print("Jumper detected: ");
  //Serial.println(jumperDetected ? "YES" : "NO");
  //Serial.println("========================");

  DEBUG_MODE = jumperDetected;
  
  // Initialize IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
  if (DEBUG_MODE) {
    // Debug mode startup
    Serial.println("=== Arduino IR Receiver (DEBUG MODE - JUMPER DETECTED) ===");
    Serial.println("Configuration:");
    Serial.print("  - Debug Jumper: "); Serial.println("INSTALLED (Pin 10 -> GND)");
    Serial.print("  - Debug Mode: "); Serial.println("ON");
    Serial.print("  - Show Repeats: "); Serial.println(SHOW_REPEATS ? "ON" : "OFF");
    Serial.print("  - Show Raw Data: "); Serial.println(SHOW_RAW_DATA ? "ON" : "OFF");
    Serial.print("  - Baud Rate: "); Serial.println(BAUD_RATE);
    Serial.println();
    Serial.println("Remove jumper and restart for production mode");
    Serial.println("Waiting for IR signals...");
    Serial.println("Format: Protocol | Address | Command | Raw Value | Bits | Time");
    Serial.println("---");
  } else {
    // Flipper Zero compatible startup
    Serial.println("ir rx");
    Serial.println("Receiving...");
    Serial.println("Press Ctrl+C to stop");
    Serial.println("(Insert Pin 10->GND jumper and restart for debug mode)");
  }
}

// Convert protocol names to match Flipper Zero format
String getFlipperProtocolName(String arduinoProtocol) {
  if (arduinoProtocol == "Sony") return "SIRC";
  if (arduinoProtocol == "Samsung32") return "Samsung32";
  if (arduinoProtocol == "Samsung") return "Samsung32";
  // Most others match directly
  return arduinoProtocol;
}

// Format hex values with proper padding
String formatHex(uint32_t value, int minDigits = 2) {
  String hex = String(value, HEX);
  hex.toUpperCase();
  while (hex.length() < minDigits) {
    hex = "0" + hex;
  }
  return hex;
}

// Print detailed debug information
void printDebugInfo(String protocol, uint16_t address, uint16_t command, 
                   uint32_t rawValue, uint8_t bits, bool isRepeat) {
  
  totalSignals++;
  unsigned long currentTime = millis();
  
  if (!isRepeat) {
    validSignals++;
  }
  
  // Main data line
  Serial.print(protocol);
  Serial.print(" | A:0x");
  Serial.print(formatHex(address));
  Serial.print(" | C:0x");
  Serial.print(formatHex(command));
  
  if (SHOW_RAW_DATA) {
    Serial.print(" | Raw:0x");
    Serial.print(formatHex(rawValue, 8));
  }
  
  Serial.print(" | ");
  Serial.print(bits);
  Serial.print(" bits");
  
  if (isRepeat) {
    Serial.print(" | REPEAT");
  }
  
  // Timing info
  if (lastSignalTime > 0) {
    Serial.print(" | +");
    Serial.print(currentTime - lastSignalTime);
    Serial.print("ms");
  }
  
  Serial.println();
  
  // Diagnostic warnings
  if (bits == 0) {
    Serial.println("  ^ WARNING: 0 bits received - possible noise or interference");
  }
  if (protocol == "UNKNOWN") {
    Serial.println("  ^ UNKNOWN protocol - may need raw timing analysis");
  }
  if (address == 0 && command == 0 && protocol != "UNKNOWN") {
    Serial.println("  ^ Both address and command are 0 - unusual for this protocol");
  }
  
  lastSignalTime = currentTime;
  
  // Periodic statistics
  if (totalSignals % 50 == 0) {
    Serial.print("  [Stats: ");
    Serial.print(validSignals);
    Serial.print("/");
    Serial.print(totalSignals);
    Serial.print(" valid signals, ");
    Serial.print((validSignals * 100) / totalSignals);
    Serial.println("% success rate]");
  }
}

// Print simple Flipper-compatible output
void printFlipperOutput(String protocol, uint16_t address, uint16_t command) {
  Serial.print(protocol);
  Serial.print(", A:0x");
  Serial.print(formatHex(address));
  Serial.print(", C:0x");
  Serial.println(formatHex(command));
}

void loop() {
  // Check if we received IR data
  if (IrReceiver.decode()) {
    
    // Extract all signal information
    String protocol = getFlipperProtocolName(IrReceiver.getProtocolString());
    uint16_t address = IrReceiver.decodedIRData.address;
    uint16_t command = IrReceiver.decodedIRData.command;
    uint32_t rawValue = IrReceiver.decodedIRData.decodedRawData;
    uint8_t bits = IrReceiver.decodedIRData.numberOfBits;
    bool isRepeat = (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT);
    
    // Decide whether to process this signal
    bool shouldProcess = !isRepeat || (DEBUG_MODE && SHOW_REPEATS);
    
    if (shouldProcess) {
      if (DEBUG_MODE) {
        // Detailed debug output
        printDebugInfo(protocol, address, command, rawValue, bits, isRepeat);
      } else {
        // Simple Flipper-compatible output (skip repeats)
        if (!isRepeat) {
          printFlipperOutput(protocol, address, command);
        }
      }
    }
    
    // Enable receiving of the next IR signal
    IrReceiver.resume();
  }
  
  delay(50);
}

/*
 * JUMPER CONFIGURATION GUIDE:
 *
 * === Production Mode (No Jumper) ===
 * Pin 10: No connection (or leave floating)
 * Result: DEBUG_MODE = false
 *
 * Output:
 * ir rx
 * Receiving...
 * Press Ctrl+C to stop
 * (Insert Pin 10->GND jumper and restart for debug mode)
 * NEC, A:0x32, C:0x12
 * SIRC, A:0x01, C:0x75
 *
 * === Debug Mode (Jumper Installed) ===
 * Pin 10: Connected to GND (use jumper wire or actual jumper)
 * Result: DEBUG_MODE = true
 *
 * Output:
 * === Arduino IR Receiver (DEBUG MODE - JUMPER DETECTED) ===
 * Configuration:
 *   - Debug Jumper: INSTALLED (Pin 10 -> GND)
 *   - Debug Mode: ON
 *   - Show Repeats: OFF
 *   - Show Raw Data: OFF
 *   - Baud Rate: 115200
 *
 * Remove jumper and restart for production mode
 * Waiting for IR signals...
 * Format: Protocol | Address | Command | Raw Value | Bits | Time
 * ---
 * NEC | A:0x32 | C:0x12 | 32 bits | +150ms
 * SIRC | A:0x01 | C:0x75 | 12 bits | +250ms
 *
 * === Hardware Notes ===
 * - Use any jumper wire or actual jumper connector
 * - Most Arduino boards have a built-in LED (usually on pin 13)
 * - IR receiver modules typically have their own indicator LEDs
 * - External status LED on pin 12 is optional (set USE_STATUS_LED = true)
 * - Jumper detection uses internal pullup resistor
 * - Changes take effect immediately on restart (no recompilation!)
 *
 * === LED Indicators You'll See ===
 * - Arduino built-in LED: May blink during startup and operation
 * - IR receiver LED: Typically flashes when receiving IR signals
 * - Optional external LED: Only if USE_STATUS_LED = true
 *
 * === Field Usage ===
 * 1. Upload this code once to your Arduino
 * 2. For normal use: Remove any jumper, restart Arduino
 * 3. For debugging: Insert jumper (Pin 10 to GND), restart Arduino
 * 4. Switch modes anytime by adding/removing jumper and restarting
 */
