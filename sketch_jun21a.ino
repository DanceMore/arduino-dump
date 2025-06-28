/*
 * Arduino IR Receiver with Jumper-Controlled Debug Mode + TM1637 Display
 * Compatible with Flipper Zero ir rx format + optional debugging + 7-segment display
 * 
 * Debug Control:
 * - Insert jumper between Pin 10 and GND for DEBUG_MODE=true
 * - Remove jumper for production/Flipper Zero compatibility
 * - No recompilation needed!
 * 
 * Display Control via Serial Commands:
 * - DISP:text    -> Display text (up to 4 chars)
 * - DISP:1234    -> Display number 1234
 * - DISP:CLR     -> Clear display
 * - DISP:BRT:7   -> Set brightness (0-7)
 * - DISP:ON      -> Turn display on
 * - DISP:OFF     -> Turn display off
 * 
 * Wiring:
 * IR Receiver Module -> Arduino
 * VCC    -> 5V
 * GND    -> GND
 * OUTPUT -> Pin 2
 * 
 * TM1637 Display -> Arduino
 * VCC -> 5V
 * GND -> GND
 * DIO -> Pin 4
 * CLK -> Pin 5
 * 
 * Debug Jumper:
 * Pin 10 -> GND (jumper wire or actual jumper)
 */

#include <IRremote.h>
#include <TM1637Display.h>

// TM1637 Display Configuration
const int CLK = 5;
const int DIO = 4;
TM1637Display display(CLK, DIO);

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

// Display variables
uint8_t displayBrightness = 4;           // Default brightness (0-7)
bool displayEnabled = true;

// Check if debug jumper is installed
bool isDebugJumperInstalled() {
  pinMode(DEBUG_JUMPER_PIN, INPUT_PULLUP);
  delay(10); // Small delay for stable reading
  return digitalRead(DEBUG_JUMPER_PIN) == LOW; // LOW = jumper to ground
}

// Process serial commands for display control
void processSerialCommand() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    if (command.startsWith("DISP:")) {
      String param = command.substring(5);
      
      if (param == "CLR") {
        // Clear display
        display.clear();
        if (DEBUG_MODE) Serial.println("Display cleared");
        
      } else if (param == "ON") {
        // Turn display on
        displayEnabled = true;
        display.setBrightness(displayBrightness);
        if (DEBUG_MODE) Serial.println("Display turned ON");
        
      } else if (param == "OFF") {
        // Turn display off
        displayEnabled = false;
        display.setBrightness(0);
        if (DEBUG_MODE) Serial.println("Display turned OFF");
        
      } else if (param.startsWith("BRT:")) {
        // Set brightness
        int brightness = param.substring(4).toInt();
        if (brightness >= 0 && brightness <= 7) {
          displayBrightness = brightness;
          if (displayEnabled) {
            display.setBrightness(displayBrightness);
          }
          if (DEBUG_MODE) {
            Serial.print("Display brightness set to: ");
            Serial.println(brightness);
          }
        } else if (DEBUG_MODE) {
          Serial.println("Invalid brightness (0-7)");
        }
        
      } else {
        // Display text or number
        if (displayEnabled) {
          // Check if it's a number
          if (param.length() <= 4 && param.toInt() != 0 || param == "0") {
            int number = param.toInt();
            display.showNumberDec(number);
          } else {
            // Display as text (up to 4 characters)
            if (param.length() > 4) {
              param = param.substring(0, 4);
            }
            
            // Convert string to display segments
            uint8_t segments[4] = {0, 0, 0, 0};
            for (int i = 0; i < param.length() && i < 4; i++) {
              segments[i] = encodeChar(param.charAt(i));
            }
            display.setSegments(segments);
          }
          
          if (DEBUG_MODE) {
            Serial.print("Displayed: ");
            Serial.println(param);
          }
        } else if (DEBUG_MODE) {
          Serial.println("Display is OFF - use DISP:ON to enable");
        }
      }
    } else if (DEBUG_MODE) {
      Serial.println("Unknown command. Use DISP:text, DISP:CLR, DISP:ON, DISP:OFF, DISP:BRT:n");
    }
  }
}

// Encode characters for 7-segment display
uint8_t encodeChar(char c) {
  switch (c) {
    case '0': return 0x3F;
    case '1': return 0x06;
    case '2': return 0x5B;
    case '3': return 0x4F;
    case '4': return 0x66;
    case '5': return 0x6D;
    case '6': return 0x7D;
    case '7': return 0x07;
    case '8': return 0x7F;
    case '9': return 0x6F;
    case 'A': return 0x77;
    case 'B': return 0x7C;
    case 'C': return 0x39;
    case 'D': return 0x5E;
    case 'E': return 0x79;
    case 'F': return 0x71;
    case 'G': return 0x3D;
    case 'H': return 0x76;
    case 'I': return 0x06;
    case 'J': return 0x1E;
    case 'L': return 0x38;
    case 'N': return 0x54;
    case 'O': return 0x3F;
    case 'P': return 0x73;
    case 'R': return 0x50;
    case 'S': return 0x6D;
    case 'T': return 0x78;
    case 'U': return 0x3E;
    case 'Y': return 0x6E;
    case '-': return 0x40;
    case '_': return 0x08;
    case ' ': return 0x00;
    default:  return 0x00; // Blank for unsupported characters
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  delay(2000); // sleep so serial port can wake up...

  // Initialize TM1637 Display
  display.setBrightness(displayBrightness);
  display.clear();
  
  // Show startup pattern
  display.showNumberDec(8888); // All segments on briefly
  delay(500);
  display.clear();

  // DIAGNOSTIC: Check jumper status with detailed info
  pinMode(DEBUG_JUMPER_PIN, INPUT_PULLUP);
  delay(100); // Longer delay for stable reading

  bool jumperDetected = (digitalRead(DEBUG_JUMPER_PIN) == LOW);
  DEBUG_MODE = jumperDetected;
  
  // Initialize IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
  if (DEBUG_MODE) {
    // Debug mode startup
    Serial.println("=== Arduino IR Receiver + TM1637 Display (DEBUG MODE) ===");
    Serial.println("Configuration:");
    Serial.print("  - Debug Jumper: "); Serial.println("INSTALLED (Pin 10 -> GND)");
    Serial.print("  - Debug Mode: "); Serial.println("ON");
    Serial.print("  - Show Repeats: "); Serial.println(SHOW_REPEATS ? "ON" : "OFF");
    Serial.print("  - Show Raw Data: "); Serial.println(SHOW_RAW_DATA ? "ON" : "OFF");
    Serial.print("  - Baud Rate: "); Serial.println(BAUD_RATE);
    Serial.print("  - Display: "); Serial.println("TM1637 4-Digit 7-Segment");
    Serial.println();
    Serial.println("Display Commands:");
    Serial.println("  DISP:text    - Display text (up to 4 chars)");
    Serial.println("  DISP:1234    - Display number");
    Serial.println("  DISP:CLR     - Clear display");
    Serial.println("  DISP:BRT:7   - Set brightness (0-7)");
    Serial.println("  DISP:ON      - Turn display on");
    Serial.println("  DISP:OFF     - Turn display off");
    Serial.println();
    Serial.println("Remove jumper and restart for production mode");
    Serial.println("Waiting for IR signals and display commands...");
    Serial.println("Format: Protocol | Address | Command | Raw Value | Bits | Time");
    Serial.println("---");
    
    // Show "REDY" on display in debug mode
    uint8_t readySegments[] = {0x50, 0x79, 0x5E, 0x6E}; // "REDY"
    display.setSegments(readySegments);
    
  } else {
    // Flipper Zero compatible startup
    Serial.println("ir rx");
    Serial.println("Receiving...");
    Serial.println("Press Ctrl+C to stop");
    Serial.println("(Insert Pin 10->GND jumper and restart for debug mode)");
    Serial.println("Display commands: DISP:text, DISP:CLR, DISP:ON, DISP:OFF, DISP:BRT:n");
    
    // Show "----" on display in production mode
    uint8_t dashSegments[] = {0x40, 0x40, 0x40, 0x40}; // "----"
    display.setSegments(dashSegments);
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
  // Process any incoming serial commands for display
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
 * DISPLAY COMMAND EXAMPLES:
 *
 * Basic Usage:
 * DISP:HELLO    -> Shows "HELL" (first 4 chars)
 * DISP:1234     -> Shows "1234"
 * DISP:42       -> Shows "42" (right-aligned)
 * DISP:A1B2     -> Shows "A1B2"
 * DISP:CLR      -> Clears display
 *
 * Brightness Control:
 * DISP:BRT:0    -> Dimmest
 * DISP:BRT:7    -> Brightest
 * DISP:BRT:4    -> Default
 *
 * Power Control:
 * DISP:OFF      -> Turn display off
 * DISP:ON       -> Turn display on
 *
 * WIRING REFERENCE:
 * TM1637 Display:
 *   VCC -> Arduino 5V
 *   GND -> Arduino GND
 *   DIO -> Arduino Pin 4
 *   CLK -> Arduino Pin 5
 *
 * IR Receiver:
 *   VCC -> Arduino 5V
 *   GND -> Arduino GND
 *   OUT -> Arduino Pin 2
 *
 * Debug Jumper:
 *   Pin 10 -> GND (for debug mode)
 *
 * LIBRARY REQUIRED:
 * Install "TM1637" library by Avishay Orpaz from Library Manager
 */
