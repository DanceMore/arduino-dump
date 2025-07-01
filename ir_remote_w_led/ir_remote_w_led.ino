/*
 * Arduino IR Receiver with Jumper-Controlled Debug Mode + TM1637 Display + Enhanced RGB LED Effects
 * Compatible with Flipper Zero ir rx format + optional debugging + 7-segment display + LED animations
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
 * LED Animation Control via Serial Commands:
 * - LED:red-blue 30         -> Fast police-style red/blue for 30 seconds
 * - LED:red-green-yellow 60 -> Slow traffic light cycle for 60 seconds
 * - LED:ack                 -> Quick green flash for command acknowledgment
 * - LED:matrix 45           -> Green Matrix-style fade effect for 45 seconds
 * - LED:rainbow 60          -> Rainbow hue shift for 60 seconds
 * - LED:pulse-red 30        -> Red pulsing effect for 30 seconds
 * - LED:pulse-blue 30       -> Blue pulsing effect for 30 seconds
 * - LED:strobe 15           -> White strobe effect for 15 seconds
 * - LED:fire 40             -> Fire effect (red/orange flicker) for 40 seconds
 * - LED:ocean 50            -> Ocean wave effect (blue/cyan) for 50 seconds
 * - LED:off                 -> Stop all animations and turn off LED
 *
 * Wiring:
 * IR Receiver Module -> Arduino
 * VCC    -> 3.3V
 * GND    -> GND
 * OUTPUT -> Pin 2
 *
 * TM1637 Display -> Arduino
 * VCC -> 5V
 * GND -> GND
 * DIO -> Pin 4
 * CLK -> Pin 5
 *
 * RGB LED -> Arduino (Common Anode)
 * VCC -> 3.3V
 * RED -> Pin 9 (through 220Ω resistor)
 * GREEN -> Pin 6 (through 220Ω resistor)
 * BLUE -> Pin 3 (through 220Ω resistor)
 *
 * Debug Jumper:
 * Pin 10 -> GND (jumper wire or actual jumper)
 *
 * LIBRARY REQUIRED:
 * Install "TM1637" library by Avishay Orpaz from Library Manager
 */

#include <IRremote.h>
#include <TM1637Display.h>

// TM1637 Display Configuration
const int CLK = 5;
const int DIO = 4;
TM1637Display display(CLK, DIO);

// RGB LED Configuration
const int RED_PIN = 9;
const int GREEN_PIN = 6;
const int BLUE_PIN = 3;

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

// LED Animation variables
int animationMode = 0;                   // 0=off, 1=red-blue, 2=traffic, 3=ack, 4=matrix, 5=rainbow, 6=pulse-red, 7=pulse-blue, 8=strobe, 9=fire, 10=ocean
unsigned long animationEndTime = 0;
unsigned long lastAnimationUpdate = 0;
int animationStep = 0;
int animationInterval = 500;             // Default interval in ms
float animationPhase = 0.0;              // For smooth animations
bool ackFlashState = false;              // For acknowledgment flash

// HSV to RGB conversion for rainbow effect
void hsvToRgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b) {
  float c = v * s;
  float x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
  float m = v - c;

  float r1, g1, b1;
  if (h >= 0 && h < 60) {
    r1 = c; g1 = x; b1 = 0;
  } else if (h >= 60 && h < 120) {
    r1 = x; g1 = c; b1 = 0;
  } else if (h >= 120 && h < 180) {
    r1 = 0; g1 = c; b1 = x;
  } else if (h >= 180 && h < 240) {
    r1 = 0; g1 = x; b1 = c;
  } else if (h >= 240 && h < 300) {
    r1 = x; g1 = 0; b1 = c;
  } else {
    r1 = c; g1 = 0; b1 = x;
  }

  r = (uint8_t)((r1 + m) * 255);
  g = (uint8_t)((g1 + m) * 255);
  b = (uint8_t)((b1 + m) * 255);
}

// Set RGB LED color (Common Anode - inverted logic)
void setColor(uint8_t red, uint8_t green, uint8_t blue) {
  analogWrite(RED_PIN, 255 - red);
  analogWrite(GREEN_PIN, 255 - green);
  analogWrite(BLUE_PIN, 255 - blue);
}

// Update LED animation
void updateLEDAnimation() {
  unsigned long currentTime = millis();

  // Check if animation should end
  if (animationEndTime > 0 && currentTime > animationEndTime) {
    animationMode = 0;
    setColor(0, 0, 0);
    return;
  }

  // Check if it's time to update animation
  if (currentTime - lastAnimationUpdate < animationInterval) {
    return;
  }

  lastAnimationUpdate = currentTime;

  switch (animationMode) {
    case 1: // red-blue police style
      if (animationStep % 2 == 0) {
        setColor(255, 0, 0); // Red
      } else {
        setColor(0, 0, 255); // Blue
      }
      animationStep++;
      break;

    case 2: // red-green-yellow traffic light
      switch (animationStep % 3) {
        case 0: setColor(255, 0, 0); break;   // Red
        case 1: setColor(0, 255, 0); break;   // Green
        case 2: setColor(255, 255, 0); break; // Yellow
      }
      animationStep++;
      break;

    case 3: // Quick acknowledgment flash
      if (animationStep == 0) {
        setColor(0, 1, 0); // Bright green
        ackFlashState = true;
      } else if (animationStep == 1) {
        setColor(0, 0, 0); // Off
        ackFlashState = false;
        animationMode = 0; // End after one flash
        animationEndTime = 0;
      }
      animationStep++;
      break;

    case 4: // Matrix effect - green fade
      {
        float intensity = (sin(animationPhase) + 1.0) / 2.0; // 0.0 to 1.0
        uint8_t green = (uint8_t)(intensity * 255);
        setColor(0, green, 0);
        animationPhase += 0.1;
        if (animationPhase > 6.28) animationPhase = 0; // Reset at 2π
      }
      break;

    case 5: // Rainbow hue shift
      {
        uint8_t r, g, b;
        float hue = fmod(animationPhase * 10, 360); // Cycle through hues
        hsvToRgb(hue, 1.0, 1.0, r, g, b);
        setColor(r, g, b);
        animationPhase += 0.05;
        if (animationPhase > 36.0) animationPhase = 0; // Reset
      }
      break;

    case 6: // Pulse red
      {
        float intensity = (sin(animationPhase) + 1.0) / 2.0; // 0.0 to 1.0
        uint8_t red = (uint8_t)(intensity * 255);
        setColor(red, 0, 0);
        animationPhase += 0.15;
        if (animationPhase > 6.28) animationPhase = 0;
      }
      break;

    case 7: // Pulse blue
      {
        float intensity = (sin(animationPhase) + 1.0) / 2.0; // 0.0 to 1.0
        uint8_t blue = (uint8_t)(intensity * 255);
        setColor(0, 0, blue);
        animationPhase += 0.15;
        if (animationPhase > 6.28) animationPhase = 0;
      }
      break;

    case 8: // Strobe white
      if (animationStep % 2 == 0) {
        setColor(255, 255, 255); // White
      } else {
        setColor(0, 0, 0); // Off
      }
      animationStep++;
      break;

    case 9: // Fire effect
      {
        // Random flicker between red and orange
        uint8_t red = 200 + random(56);     // 200-255
        uint8_t green = random(100);        // 0-99 for orange tint
        uint8_t blue = 0;
        setColor(red, green, blue);
      }
      break;

    case 10: // Ocean wave effect
      {
        float wave1 = (sin(animationPhase) + 1.0) / 2.0;
        float wave2 = (sin(animationPhase * 1.3 + 1.0) + 1.0) / 2.0;
        uint8_t blue = (uint8_t)(wave1 * 255);
        uint8_t cyan = (uint8_t)(wave2 * 100); // Add some cyan
        setColor(0, cyan, blue);
        animationPhase += 0.08;
        if (animationPhase > 6.28) animationPhase = 0;
      }
      break;

    default: // Off
      setColor(0, 0, 0);
      break;
  }
}

// Start LED animation
void startLEDAnimation(int animType, int durationSeconds) {
  animationMode = animType;
  animationStep = 0;
  animationPhase = 0.0;
  lastAnimationUpdate = 0;

  switch (animType) {
    case 1: // Red-blue police style - fast
      animationInterval = 150;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case 2: // Traffic light - slow
      animationInterval = 800;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case 3: // Quick acknowledgment flash
      animationInterval = 100;
      animationEndTime = millis() + 300; // 300ms total
      break;

    case 4: // Matrix effect
      animationInterval = 50;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case 5: // Rainbow
      animationInterval = 30;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case 6: // Pulse red
    case 7: // Pulse blue
      animationInterval = 30;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case 8: // Strobe
      animationInterval = 100;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case 9: // Fire
      animationInterval = 80;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case 10: // Ocean
      animationInterval = 40;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    default: // Turn off
      animationEndTime = 0;
      setColor(0, 0, 0);
      break;
  }

  if (DEBUG_MODE && animType != 3) { // Don't spam debug for ack flashes
    Serial.print("LED animation started: mode ");
    Serial.print(animType);
    if (durationSeconds > 0) {
      Serial.print(", duration ");
      Serial.print(durationSeconds);
      Serial.println(" seconds");
    } else {
      Serial.println(" (brief)");
    }
  }
}

// Quick acknowledgment flash
void flashAck() {
  startLEDAnimation(3, 0); // Mode 3, no duration needed
}

// Check if debug jumper is installed
bool isDebugJumperInstalled() {
  pinMode(DEBUG_JUMPER_PIN, INPUT_PULLUP);
  delay(10); // Small delay for stable reading
  return digitalRead(DEBUG_JUMPER_PIN) == LOW; // LOW = jumper to ground
}

// Check if a string contains only digits
bool isNumericString(String str) {
  if (str.length() == 0) return false;
  for (int i = 0; i < str.length(); i++) {
    if (!isdigit(str.charAt(i))) {
      return false;
    }
  }
  return true;
}

// Display a numeric string exactly as specified (no automatic padding)
void displayNumericString(String numStr) {
  uint8_t segments[4] = {0, 0, 0, 0}; // All blank by default

  // Right-align the number (pad with blanks, not zeros)
  int startPos = 4 - numStr.length();
  if (startPos < 0) startPos = 0;

  // Leading positions remain blank (0x00)
  // Fill only the actual digits from the string
  for (int i = 0; i < numStr.length() && i < 4; i++) {
    segments[startPos + i] = encodeChar(numStr.charAt(i));
  }

  display.setSegments(segments);
}

// Process serial commands for display and LED control
void processSerialCommand() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    // Handle LED commands (case sensitive for lowercase commands)
    if (command.startsWith("LED:")) {
      String param = command.substring(4);

      if (param == "off") {
        startLEDAnimation(0, 0);

      } else if (param == "ack") {
        flashAck();

      } else if (param.startsWith("red-blue ")) {
        int duration = param.substring(9).toInt();
        if (duration > 0) {
          startLEDAnimation(1, duration);
        } else if (DEBUG_MODE) {
          Serial.println("Invalid duration for red-blue animation");
        }

      } else if (param.startsWith("red-green-yellow ")) {
        int duration = param.substring(17).toInt();
        if (duration > 0) {
          startLEDAnimation(2, duration);
        } else if (DEBUG_MODE) {
          Serial.println("Invalid duration for red-green-yellow animation");
        }

      } else if (param.startsWith("matrix ")) {
        int duration = param.substring(7).toInt();
        if (duration > 0) {
          startLEDAnimation(4, duration);
        } else if (DEBUG_MODE) {
          Serial.println("Invalid duration for matrix animation");
        }

      } else if (param.startsWith("rainbow ")) {
        int duration = param.substring(8).toInt();
        if (duration > 0) {
          startLEDAnimation(5, duration);
        } else if (DEBUG_MODE) {
          Serial.println("Invalid duration for rainbow animation");
        }

      } else if (param.startsWith("pulse-red ")) {
        int duration = param.substring(10).toInt();
        if (duration > 0) {
          startLEDAnimation(6, duration);
        } else if (DEBUG_MODE) {
          Serial.println("Invalid duration for pulse-red animation");
        }

      } else if (param.startsWith("pulse-blue ")) {
        int duration = param.substring(11).toInt();
        if (duration > 0) {
          startLEDAnimation(7, duration);
        } else if (DEBUG_MODE) {
          Serial.println("Invalid duration for pulse-blue animation");
        }

      } else if (param.startsWith("strobe ")) {
        int duration = param.substring(7).toInt();
        if (duration > 0) {
          startLEDAnimation(8, duration);
        } else if (DEBUG_MODE) {
          Serial.println("Invalid duration for strobe animation");
        }

      } else if (param.startsWith("fire ")) {
        int duration = param.substring(5).toInt();
        if (duration > 0) {
          startLEDAnimation(9, duration);
        } else if (DEBUG_MODE) {
          Serial.println("Invalid duration for fire animation");
        }

      } else if (param.startsWith("ocean ")) {
        int duration = param.substring(6).toInt();
        if (duration > 0) {
          startLEDAnimation(10, duration);
        } else if (DEBUG_MODE) {
          Serial.println("Invalid duration for ocean animation");
        }

      } else if (DEBUG_MODE) {
        Serial.println("LED commands:");
        Serial.println("  LED:ack - Quick green flash");
        Serial.println("  LED:red-blue 30 - Police style");
        Serial.println("  LED:red-green-yellow 60 - Traffic light");
        Serial.println("  LED:matrix 45 - Green Matrix fade");
        Serial.println("  LED:rainbow 60 - Rainbow hue shift");
        Serial.println("  LED:pulse-red 30 - Red pulsing");
        Serial.println("  LED:pulse-blue 30 - Blue pulsing");
        Serial.println("  LED:strobe 15 - White strobe");
        Serial.println("  LED:fire 40 - Fire flicker");
        Serial.println("  LED:ocean 50 - Ocean waves");
        Serial.println("  LED:off - Turn off");
      }
      return; // Don't process as display command
    }

    // Convert to uppercase for display commands
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

          // Check if it's a numeric string (including those with leading zeros)
          if (isNumericString(param) && param.length() <= 4) {
            // Handle as numeric string to preserve leading zeros
            displayNumericString(param);
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
      Serial.println("Commands: DISP:text, DISP:CLR, DISP:ON, DISP:OFF, DISP:BRT:n");
      Serial.println("LED: LED:ack, LED:matrix 45, LED:rainbow 60, LED:fire 40, LED:off");
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

  // Initialize RGB LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setColor(0, 0, 0); // Start with LED off

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
    Serial.println("=== Arduino IR Receiver + TM1637 Display + Enhanced RGB LED (DEBUG MODE) ===");
    Serial.println("Configuration:");
    Serial.print("  - Debug Jumper: "); Serial.println("INSTALLED (Pin 10 -> GND)");
    Serial.print("  - Debug Mode: "); Serial.println("ON");
    Serial.print("  - Show Repeats: "); Serial.println(SHOW_REPEATS ? "ON" : "OFF");
    Serial.print("  - Show Raw Data: "); Serial.println(SHOW_RAW_DATA ? "ON" : "OFF");
    Serial.print("  - Baud Rate: "); Serial.println(BAUD_RATE);
    Serial.print("  - Display: "); Serial.println("TM1637 4-Digit 7-Segment");
    Serial.print("  - RGB LED: "); Serial.println("Common Anode (Pins 9,6,3)");
    Serial.println();
    Serial.println("Display Commands:");
    Serial.println("  DISP:text    - Display text (up to 4 chars)");
    Serial.println("  DISP:1234    - Display number");
    Serial.println("  DISP:CLR     - Clear display");
    Serial.println("  DISP:BRT:7   - Set brightness (0-7)");
    Serial.println("  DISP:ON      - Turn display on");
    Serial.println("  DISP:OFF     - Turn display off");
    Serial.println();
    Serial.println("Enhanced LED Animation Commands:");
    Serial.println("  LED:ack                    - Quick green acknowledgment flash");
    Serial.println("  LED:red-blue 30            - Police style for 30 seconds");
    Serial.println("  LED:red-green-yellow 60    - Traffic light for 60 seconds");
    Serial.println("  LED:matrix 45              - Green Matrix fade for 45 seconds");
    Serial.println("  LED:rainbow 60             - Rainbow hue shift for 60 seconds");
    Serial.println("  LED:pulse-red 30           - Red pulsing for 30 seconds");
    Serial.println("  LED:pulse-blue 30          - Blue pulsing for 30 seconds");
    Serial.println("  LED:strobe 15              - White strobe for 15 seconds");
    Serial.println("  LED:fire 40                - Fire flicker for 40 seconds");
    Serial.println("  LED:ocean 50               - Ocean waves for 50 seconds");
    Serial.println("  LED:off                    - Turn off LED");
    Serial.println();
    Serial.println("Remove jumper and restart for production mode");
    Serial.println("Waiting for IR signals and commands...");
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
    Serial.println("Commands: DISP:text, DISP:CLR, LED:ack, LED:matrix 45, LED:rainbow 60, LED:off");

    // Show "----" on display in production mode
    uint8_t dashSegments[] = {0x40, 0x40, 0x40, 0x40}; // "----"
    display.setSegments(dashSegments);
  }

  // Welcome flash
  delay(1000);
  flashAck();
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
  updateLEDAnimation();

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
        flashAck();
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
      Serial.println("[NOISE FILTERED]");
    }

    // Enable receiving of the next IR signal
    IrReceiver.resume();
  }

  delay(50);
}
