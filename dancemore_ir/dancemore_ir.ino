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

// Buffer for serial commands - avoid String class
char commandBuffer[32];
uint8_t commandIndex = 0;

// Check if debug jumper is installed
bool isDebugJumperInstalled() {
  pinMode(DEBUG_JUMPER_PIN, INPUT_PULLUP);
  delay(2); // Minimal delay for stable reading
  return digitalRead(DEBUG_JUMPER_PIN) == LOW; // LOW = jumper to ground
}

// Process serial commands for display and LED control - optimized version
void processSerialCommand() {
  while (Serial.available()) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (commandIndex > 0) {
        commandBuffer[commandIndex] = '\0'; // Null terminate
        
        // Try display controller first
        if (displayController.processCommand(commandBuffer)) {
          commandIndex = 0;
          return;
        }
        
        // Try LED controller
        if (ledAnimations.processCommand(commandBuffer)) {
          commandIndex = 0;
          return;
        }
        
        // Unknown command
        if (DEBUG_MODE) {
          Serial.println(F("Commands: DISP:text, DISP:CLR, DISP:ON, DISP:OFF, DISP:BRT:n"));
          ledAnimations.printHelp();
        }
        commandIndex = 0;
      }
    } else if (commandIndex < sizeof(commandBuffer) - 1) {
      commandBuffer[commandIndex++] = c;
    } else {
      // Buffer overflow - reset
      commandIndex = 0;
    }
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  
  // Initialize RGB LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // DIAGNOSTIC: Check jumper status
  pinMode(DEBUG_JUMPER_PIN, INPUT_PULLUP);
  delay(10); // Minimal delay for stable reading

  bool jumperDetected = (digitalRead(DEBUG_JUMPER_PIN) == LOW);
  DEBUG_MODE = jumperDetected;

  // Initialize display controller
  displayController.setDebugMode(DEBUG_MODE);
  displayController.begin();

  // Initialize LED animations
  ledAnimations.begin(DEBUG_MODE);

  // Initialize IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // Reduced startup messages and delays
  if (DEBUG_MODE) {
    // Debug mode startup - use F() macro for string literals
    Serial.println(F("=== Arduino IR Receiver + TM1637 Display + Enhanced RGB LED (DEBUG MODE) ==="));
    Serial.println(F("Configuration:"));
    Serial.println(F("  - Debug Jumper: INSTALLED (Pin 10 -> GND)"));
    Serial.println(F("  - Debug Mode: ON"));
    Serial.print(F("  - Show Repeats: ")); Serial.println(SHOW_REPEATS ? F("ON") : F("OFF"));
    Serial.print(F("  - Show Raw Data: ")); Serial.println(SHOW_RAW_DATA ? F("ON") : F("OFF"));
    Serial.print(F("  - Baud Rate: ")); Serial.println(BAUD_RATE);
    Serial.println(F("  - Display: TM1637 4-Digit 7-Segment"));
    Serial.println(F("  - RGB LED: Common Anode (Pins 9,6,3)"));
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
    // Flipper Zero compatible startup - minimal output
    Serial.println(F("ir rx"));
    Serial.println(F("Receiving..."));
    Serial.println(F("Press Ctrl+C to stop"));
    
    // Show "----" on display in production mode
    displayController.showDashes();
  }

  // Reduced welcome flash delay
  delay(200);
  ledAnimations.flashAck();
}

// Convert protocol names to match Flipper Zero format - handle FlashStringHelper
String getFlipperProtocolName(String arduinoProtocol) {
  if (arduinoProtocol == "Sony") return "SIRC";
  if (arduinoProtocol == "Samsung32") return "Samsung32";
  if (arduinoProtocol == "Samsung") return "Samsung32";
  // Most others match directly
  return arduinoProtocol;
}

// Format hex values with proper padding - optimized version
void printHex(uint32_t value, uint8_t minDigits = 2) {
  char buffer[9]; // Max 8 hex digits + null terminator
  
  // Handle special case of 0
  if (value == 0) {
    for (uint8_t i = 0; i < minDigits; i++) {
      Serial.print('0');
    }
    return;
  }
  
  // Convert to hex string
  uint8_t digits = 0;
  uint32_t temp = value;
  
  // Count digits needed
  while (temp > 0) {
    temp >>= 4;
    digits++;
  }
  
  // Print leading zeros if needed
  while (digits < minDigits) {
    Serial.print('0');
    digits++;
  }
  
  // Print the hex value
  Serial.print(value, HEX);
}

// Print detailed debug information - optimized
void printDebugInfo(String protocol, uint16_t address, uint16_t command,
                   uint32_t rawValue, uint8_t bits, bool isRepeat) {

  totalSignals++;
  unsigned long currentTime = millis();

  if (!isRepeat) {
    validSignals++;
  }

  // Main data line
  Serial.print(protocol);
  Serial.print(F(" | A:0x"));
  printHex(address);
  Serial.print(F(" | C:0x"));
  printHex(command);

  if (SHOW_RAW_DATA) {
    Serial.print(F(" | Raw:0x"));
    printHex(rawValue, 8);
  }

  Serial.print(F(" | "));
  Serial.print(bits);
  Serial.print(F(" bits"));

  if (isRepeat) {
    Serial.print(F(" | REPEAT"));
  }

  // Timing info
  if (lastSignalTime > 0) {
    Serial.print(F(" | +"));
    Serial.print(currentTime - lastSignalTime);
    Serial.print(F("ms"));
  }

  Serial.println();

  // Diagnostic warnings
  if (bits == 0) {
    Serial.println(F("  ^ WARNING: 0 bits received - possible noise or interference"));
  }
  if (protocol == "UNKNOWN") {
    Serial.println(F("  ^ UNKNOWN protocol - may need raw timing analysis"));
  }
  if (address == 0 && command == 0 && protocol != "UNKNOWN") {
    Serial.println(F("  ^ Both address and command are 0 - unusual for this protocol"));
  }

  lastSignalTime = currentTime;

  // Periodic statistics - less frequent to reduce overhead
  if (totalSignals % 100 == 0) {
    Serial.print(F("  [Stats: "));
    Serial.print(validSignals);
    Serial.print('/');
    Serial.print(totalSignals);
    Serial.print(F(" valid signals, "));
    Serial.print((validSignals * 100) / totalSignals);
    Serial.println(F("% success rate]"));
  }
}

// Print simple Flipper-compatible output - optimized
void printFlipperOutput(String protocol, uint16_t address, uint16_t command) {
  Serial.print(protocol);
  Serial.print(F(", A:0x"));
  printHex(address);
  Serial.print(F(", C:0x"));
  printHex(command);
  Serial.println();
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

  // Reduced delay for better responsiveness
  delay(10);
}
