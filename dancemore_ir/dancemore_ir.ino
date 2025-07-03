#include <IRremote.h>
#include "LEDAnimations.h"
#include "DisplayController.h"
#include "StringConstants.h"

// TM1637 Display Configuration
const int CLK = 5;
const int DIO = 4;
DisplayController displayController(CLK, DIO);

// RGB LED Configuration
const int RED_PIN = 9;
const int GREEN_PIN = 6;
const int BLUE_PIN = 3;

// LED Animation object
LEDAnimations ledAnimations(RED_PIN, GREEN_PIN, BLUE_PIN);

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

// Process serial commands for display and LED control
// Updated main loop command processing - much cleaner now:
void processSerialCommand() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    // Try display controller first
    if (displayController.processCommand(command)) {
      return; // Command was handled by display controller
    }

    // Try LED controller
    if (ledAnimations.processCommand(command)) {
      return; // Command was handled by LED controller
    }

    // Unknown command
    if (DEBUG_MODE) {
      Serial.println(F("Commands: DISP:text, DISP:CLR, DISP:ON, DISP:OFF, DISP:BRT:n"));
      ledAnimations.printHelp();
    }
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  delay(2000); // sleep so serial port can wake up...

  // Initialize RGB LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // DIAGNOSTIC: Check jumper status with detailed info
  pinMode(DEBUG_JUMPER_PIN, INPUT_PULLUP);
  delay(100); // Longer delay for stable reading

  bool jumperDetected = (digitalRead(DEBUG_JUMPER_PIN) == LOW);
  DEBUG_MODE = jumperDetected;

  // Initialize display controller
  displayController.setDebugMode(DEBUG_MODE);
  displayController.begin();

  // Initialize LED animations
  ledAnimations.begin(DEBUG_MODE);

  // Initialize IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  if (DEBUG_MODE) {
    // Debug mode startup
    Serial.println(F("=== Arduino IR Receiver + TM1637 Display + Enhanced RGB LED (DEBUG MODE) ==="));
    Serial.println(F("Configuration:"));
    Serial.print(F("  - Debug Jumper: ")); Serial.println("INSTALLED (Pin 10 -> GND)");
    Serial.print(F("  - Debug Mode: ")); Serial.println("ON");
    Serial.print(F("  - Show Repeats: ")); Serial.println(SHOW_REPEATS ? "ON" : "OFF");
    Serial.print(F("  - Show Raw Data: ")); Serial.println(SHOW_RAW_DATA ? "ON" : "OFF");
    Serial.print(F("  - Baud Rate: ")); Serial.println(BAUD_RATE);
    Serial.print(F("  - Display: ")); Serial.println("TM1637 4-Digit 7-Segment");
    Serial.print(F("  - RGB LED: ")); Serial.println("Common Anode (Pins 9,6,3)");
    Serial.println();
    Serial.println(F("Display Commands:"));
    StringManager::printDisplayHelp();
    Serial.println();
    Serial.println(F("Enhanced LED Animation Commands:"));
    StringManager::printLEDHelp();
    Serial.println();
    Serial.println(F("Remove jumper and restart for production mode"));
    Serial.println(F("Waiting for IR signals and commands..."));
    Serial.println(F("Format: Protocol | Address | Command | Raw Value | Bits | Time"));
    Serial.println(F("---"));

    // Show "REDY" on display in debug mode
    displayController.showReady();

  } else {
    // Flipper Zero compatible startup
    Serial.println(F("ir rx"));
    Serial.println(F("Receiving..."));
    Serial.println(F("Press Ctrl+C to stop"));
    Serial.println(F("(Insert Pin 10->GND jumper and restart for debug mode)"));
    Serial.println(F("Commands: DISP:text, DISP:CLR, LED:ack, LED:matrix 45, LED:rainbow 60, LED:off"));

    // Show "----" on display in production mode
    displayController.showDashes();
  }

  // Welcome flash
  delay(1000);
  ledAnimations.flashAck();
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
  // Update LED animations
  ledAnimations.update();

  // Process any incoming serial commands for display and LED
  processSerialCommand();

  // Check if we received IR data
  if (IrReceiver.decode()) {

    // Extract all signal information
    String protocol = getFlipperProtocolName(IrReceiver.getProtocolString());
    uint16_t address = IrReceiver.decodedIRData.address;
    uint16_t command = IrReceiver.decodedIRData.command;
    uint32_t rawValue = IrReceiver.decodedIRData.decodedRawData;
    uint8_t bits = IrReceiver.decodedIRData.numberOfBits;
    bool isRepeat = (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT);

    // Filter out noise: UNKNOWN protocols with zero address and command
    bool isNoise = (protocol == "UNKNOWN" && address == 0 && command == 0);
    
    // Also filter out signals with 0 bits (usually noise)
    if (bits == 0) {
      isNoise = true;
    }

    // Only process valid signals (not noise, not repeats unless in debug mode)
    bool shouldProcess = !isNoise && (!isRepeat || (DEBUG_MODE && SHOW_REPEATS));

    if (shouldProcess) {
      // Flash acknowledgment for valid received IR signals (brief, non-blocking)
      if (!isRepeat) {
        // we let the USB-Serial do all acks / nacks for now...
        //ledAnimations.flashAck();
      }

      if (DEBUG_MODE) {
        // Detailed debug output
        printDebugInfo(protocol, address, command, rawValue, bits, isRepeat);
      } else {
        // Simple Flipper-compatible output (skip repeats)
        if (!isRepeat) {
          printFlipperOutput(protocol, address, command);
        }
      }
    } else if (DEBUG_MODE && isNoise && !isRepeat) {
      // In debug mode, show filtered noise signals with a simple indicator
      Serial.println(F("[NOISE FILTERED]"));
    }

    // Enable receiving of the next IR signal
    IrReceiver.resume();
  }

  delay(50);
}
