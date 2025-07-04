/*
 * LEDAnimations.cpp - Memory Optimized Version
 * LED Animation Library Implementation
 */

#include "LEDAnimations.h"

// Define the static member variables declared in the header
// These must match the declarations in LEDAnimations.h

// Command table - stored in PROGMEM to save RAM
const LEDCommand LEDAnimations::commands[] PROGMEM = {
  {"off", ANIM_OFF, false},
  {"ack", ANIM_ACK, false},
  {"nack", ANIM_NACK, false},
  {"red-blue", ANIM_RED_BLUE, true},
  {"red-green-yellow", ANIM_TRAFFIC, true},
  {"matrix", ANIM_MATRIX, true},
  {"rainbow", ANIM_RAINBOW, true},
  {"pulse-red", ANIM_PULSE_RED, true},
  {"pulse-blue", ANIM_PULSE_BLUE, true},
  {"strobe", ANIM_STROBE, true},
  {"fire", ANIM_FIRE, true},
  {"ocean", ANIM_OCEAN, true},
  {"thinking", ANIM_THINKING, true}
};

const int LEDAnimations::numCommands = sizeof(LEDAnimations::commands) / sizeof(LEDAnimations::commands[0]);

// Improved Rainbow color lookup table - 64 entries for smooth looping
// Power of 2 size for efficient bit-masking: index & 63
const uint8_t LEDAnimations::rainbowTable[][3] PROGMEM = {
  // Red to Orange to Yellow (0-10)
  {255,0,0}, {255,16,0}, {255,32,0}, {255,48,0}, {255,64,0}, {255,80,0}, {255,96,0}, {255,112,0}, {255,128,0}, {255,144,0}, {255,160,0},
  // Yellow to Yellow-Green (11-15)
  {255,176,0}, {255,192,0}, {255,208,0}, {255,224,0}, {255,240,0},
  // Yellow-Green to Green (16-21)
  {255,255,0}, {224,255,0}, {192,255,0}, {160,255,0}, {128,255,0}, {96,255,0},
  // Green to Green-Cyan (22-26)
  {64,255,0}, {32,255,0}, {16,255,0}, {0,255,0}, {0,255,32},
  // Green-Cyan to Cyan (27-31)
  {0,255,64}, {0,255,96}, {0,255,128}, {0,255,160}, {0,255,192},
  // Cyan to Cyan-Blue (32-36)
  {0,255,224}, {0,255,255}, {0,224,255}, {0,192,255}, {0,160,255},
  // Cyan-Blue to Blue (37-42)
  {0,128,255}, {0,96,255}, {0,64,255}, {0,32,255}, {0,16,255}, {0,0,255},
  // Blue to Blue-Purple (43-47)
  {16,0,255}, {32,0,255}, {48,0,255}, {64,0,255}, {80,0,255},
  // Blue-Purple to Purple (48-52)
  {96,0,255}, {112,0,255}, {128,0,255}, {144,0,255}, {160,0,255},
  // Purple to Purple-Red (53-57)
  {176,0,255}, {192,0,255}, {208,0,255}, {224,0,255}, {240,0,255},
  // Purple-Red to Red (58-63)
  {255,0,255}, {255,0,224}, {255,0,192}, {255,0,160}, {255,0,128}, {255,0,96},
  {255,0,64}, {255,0,32}
};

const uint8_t LEDAnimations::RAINBOW_TABLE_SIZE = 64;

#define SINE_LOOKUP_SIZE 64

// Mathematically precise sine lookup table - 64 entries (0 to 2π)
// Values represent (sin(angle) + 1.0) / 2.0 * 255
// Each entry is sin(i * 2π / 64) mapped to 0-255 range
const uint8_t LEDAnimations::sineLookup[64] PROGMEM = {
  128, 140, 152, 164, 176, 187, 198, 208, 218, 227, 235, 242, 248, 252, 255,
  255, 252, 248, 242, 235, 227, 218, 208, 198, 187, 176, 164, 152, 140, 128,
  115, 103, 91, 79, 68, 57, 47, 37, 28, 20, 13, 7, 3, 0, 0,
  3, 7, 13, 20, 28, 37, 47, 57, 68, 79, 91, 103, 115, 128, 140,
  152, 164, 176, 187
};

// Animation timing lookup table in PROGMEM
struct AnimationTiming {
  uint8_t animType;
  uint16_t interval;
  uint16_t briefDuration; // For ack/nack
};

const AnimationTiming timingTable[] PROGMEM = {
  {ANIM_ACK, 100, 300},
  {ANIM_NACK, 100, 300},
  {ANIM_RED_BLUE, 150, 0},
  {ANIM_TRAFFIC, 800, 0},
  {ANIM_MATRIX, 50, 0},
  {ANIM_RAINBOW, 50, 0},
  {ANIM_PULSE_RED, 30, 0},
  {ANIM_PULSE_BLUE, 30, 0},
  {ANIM_STROBE, 100, 0},
  {ANIM_FIRE, 80, 0},
  {ANIM_OCEAN, 40, 0},
  {ANIM_THINKING, 200, 0}
};

const int numTimings = sizeof(timingTable) / sizeof(timingTable[0]);


LEDAnimations::LEDAnimations(int redPin, int greenPin, int bluePin) {
  this->redPin = redPin;
  this->greenPin = greenPin;
  this->bluePin = bluePin;
  
  // Initialize state
  animationMode = ANIM_OFF;
  animationEndTime = 0;
  lastAnimationUpdate = 0;
  animationStep = 0;
  animationInterval = 500;
  animationIndex = 0;   // Initialize new integer index for sine waves
  animationIndex2 = 0;  // Initialize second integer index for ocean effect
  ackFlashState = false;
  debugMode = false;
}


void LEDAnimations::begin(bool debugMode) {
  this->debugMode = debugMode;

  // Initialize RGB LED pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  setColor(0, 0, 0); // Start with LED off
}


// Get rainbow color by index (0-59) - replaces HSV conversion
void LEDAnimations::getRainbowColor(uint8_t index, uint8_t &r, uint8_t &g, uint8_t &b) {
  index = index % RAINBOW_TABLE_SIZE;
  r = pgm_read_byte(&rainbowTable[index][0]);
  g = pgm_read_byte(&rainbowTable[index][1]);
  b = pgm_read_byte(&rainbowTable[index][2]);
}

// Set RGB LED color (Common Anode - inverted logic)
void LEDAnimations::setColor(uint8_t red, uint8_t green, uint8_t blue) {
  analogWrite(redPin, 255 - red);
  analogWrite(greenPin, 255 - green);
  analogWrite(bluePin, 255 - blue);
}

// Update LED animation
void LEDAnimations::update() {
  unsigned long currentTime = millis();

  // Check if animation should end
  if (animationEndTime > 0 && currentTime > animationEndTime) {
    animationMode = ANIM_OFF;
    setColor(0, 0, 0);
    return;
  }

  // Check if it's time to update animation
  if (currentTime - lastAnimationUpdate < animationInterval) {
    return;
  }

  lastAnimationUpdate = currentTime;

  switch (animationMode) {
    case ANIM_ACK: // Quick acknowledgment flash
      if (animationStep == 0) {
        setColor(0, 64, 0); // Bright green
        ackFlashState = true;
      } else if (animationStep == 1) {
        setColor(0, 0, 0); // Off
        ackFlashState = false;
        animationMode = ANIM_OFF; // End after one flash
        animationEndTime = 0;
      }
      animationStep++;
      break;

    case ANIM_NACK: // Quick negative acknowledgment flash
      if (animationStep == 0) {
        setColor(64, 0, 0); // Bright red
        ackFlashState = true;
      } else if (animationStep == 1) {
        setColor(0, 0, 0); // Off
        ackFlashState = false;
        animationMode = ANIM_OFF; // End after one flash
        animationEndTime = 0;
      }
      animationStep++;
      break;

    case ANIM_RED_BLUE: // red-blue police style
      if (animationStep % 2 == 0) {
        setColor(255, 0, 0); // Red
      } else {
        setColor(0, 0, 255); // Blue
      }
      animationStep++;
      break;

    case ANIM_TRAFFIC: // red-green-yellow traffic light
      switch (animationStep % 3) {
        case 0: setColor(255, 0, 0); break;  // Red
        case 1: setColor(0, 255, 0); break;  // Green
        case 2: setColor(255, 255, 0); break; // Yellow
      }
      animationStep++;
      break;

    case ANIM_MATRIX: // Matrix effect - green fade using sine lookup
      {
        // Read intensity from PROGMEM sine table
        uint8_t green = pgm_read_byte(&sineLookup[animationIndex]);
        setColor(0, green, 0);
        // Increment index for next step, wrap around table size
        animationIndex = (animationIndex + 1) % SINE_LOOKUP_SIZE;
      }
      break;

    case ANIM_RAINBOW: // Rainbow using lookup table
      {
        uint8_t r, g, b;
        uint8_t colorIndex = animationStep % RAINBOW_TABLE_SIZE;
        getRainbowColor(colorIndex, r, g, b);
        setColor(r, g, b);
        animationStep++;
      }
      break;

    case ANIM_PULSE_RED: // Pulse red using sine lookup
      {
        // Read intensity from PROGMEM sine table
        uint8_t red = pgm_read_byte(&sineLookup[animationIndex]);
        setColor(red, 0, 0);
        // Increment index for next step (faster pulse), wrap around table size
        animationIndex = (animationIndex + 2) % SINE_LOOKUP_SIZE; 
      }
      break;

    case ANIM_PULSE_BLUE: // Pulse blue using sine lookup
      {
        // Read intensity from PROGMEM sine table
        uint8_t blue = pgm_read_byte(&sineLookup[animationIndex]);
        setColor(0, 0, blue);
        // Increment index for next step (faster pulse), wrap around table size
        animationIndex = (animationIndex + 2) % SINE_LOOKUP_SIZE;
      }
      break;

    case ANIM_STROBE: // Strobe white
      if (animationStep % 2 == 0) {
        setColor(255, 255, 255); // White
      } else {
        setColor(0, 0, 0); // Off
      }
      animationStep++;
      break;

    case ANIM_FIRE: // Fire effect
      {
        // Random flicker between red and orange
        uint8_t red = 200 + random(56);     // 200-255
        uint8_t green = random(100);        // 0-99 for orange tint
        uint8_t blue = 0;
        setColor(red, green, blue);
      }
      break;

    case ANIM_OCEAN: // Ocean wave effect using sine lookup
      {
        // Read wave intensities from PROGMEM sine table using two different indices
        uint8_t blue_intensity = pgm_read_byte(&sineLookup[animationIndex]);
        uint8_t cyan_intensity = pgm_read_byte(&sineLookup[animationIndex2]);

        // Scale cyan intensity to 0-100 range as per original logic
        uint8_t blue = blue_intensity;
        uint8_t cyan = (uint8_t)(cyan_intensity * 100 / 255); 
        
        setColor(0, cyan, blue);

        // Increment indices for different wave speeds, wrap around table size
        animationIndex = (animationIndex + 1) % SINE_LOOKUP_SIZE;
        animationIndex2 = (animationIndex2 + 2) % SINE_LOOKUP_SIZE; // Faster wave for cyan
      }
      break;

    case ANIM_THINKING: // Thinking - Simon-like sequence
      {
        // Cycle through: green, red, yellow, blue with smooth fade
        int colorIndex = (animationStep / 3) % 4; // Each color lasts 3 steps
        int fadeStep = animationStep % 3;         // 0=fade in, 1=hold, 2=fade out
        
        // This still uses float for intensity calculation. If further memory optimization
        // is needed, this could also be replaced with a small lookup table or
        // fixed-point arithmetic if the exact values are critical.
        float intensity_float = (fadeStep == 0) ? 0.6 : (fadeStep == 1) ? 1.0 : 0.4;
        uint8_t brightness = (uint8_t)(intensity_float * 180); // Not full brightness for subtlety
        
        switch (colorIndex) {
          case 0: setColor(0, brightness, 0); break;          // Green
          case 1: setColor(brightness, 0, 0); break;          // Red 
          case 2: setColor(brightness, brightness, 0); break; // Yellow
          case 3: setColor(0, 0, brightness); break;          // Blue
        }
        animationStep++;
      }
      break;

    default: // Off
      setColor(0, 0, 0);
      break;
  }
}

// Start LED animation - optimized with lookup table
void LEDAnimations::startAnimation(int animType, int durationSeconds) {
  animationMode = (AnimationMode)animType; // Cast to enum type
  animationStep = 0;
  animationIndex = 0;   // Reset sine lookup index
  animationIndex2 = 0;  // Reset second sine lookup index
  lastAnimationUpdate = 0;

  // Find timing in table
  for (int i = 0; i < numTimings; i++) {
    if (pgm_read_byte(&timingTable[i].animType) == animType) {
      animationInterval = pgm_read_word(&timingTable[i].interval);
      uint16_t briefDuration = pgm_read_word(&timingTable[i].briefDuration);
      
      if (briefDuration > 0) {
        animationEndTime = millis() + briefDuration;
      } else if (durationSeconds > 0) {
        animationEndTime = millis() + (durationSeconds * 1000UL);
      } else {
        animationEndTime = 0;
      }
      break;
    }
  }

  if (animType == ANIM_OFF) {
    animationEndTime = 0;
    setColor(0, 0, 0);
  }

  if (debugMode && animType != ANIM_ACK && animType != ANIM_NACK) {
    Serial.print(F("LED animation started: mode "));
    Serial.print(animType);
    if (durationSeconds > 0) {
      Serial.print(F(", duration "));
      Serial.print(durationSeconds);
      Serial.println(F(" seconds"));
    } else {
      Serial.println(F(" (brief)"));
    }
  }
}

// Quick acknowledgment flash
void LEDAnimations::flashAck() {
  startAnimation(ANIM_ACK, 0);
}

// Quick negative acknowledgment flash
void LEDAnimations::flashNack() {
  startAnimation(ANIM_NACK, 0);
}

// Turn off LED
void LEDAnimations::off() {
  animationMode = ANIM_OFF;
  animationEndTime = 0;
  setColor(0, 0, 0);
}

// Check if currently animating
bool LEDAnimations::isAnimating() {
  return (animationMode != ANIM_OFF);
}

// Process LED command - optimized for flash memory
bool LEDAnimations::processCommand(const String& command) {
  if (!command.startsWith(F("LED:"))) {
    return false;
  }

  String param = command.substring(4);

  // Check each command in the table
  for (int i = 0; i < numCommands; i++) {
    char cmdName[20];
    strcpy_P(cmdName, (char*)pgm_read_ptr(&(commands[i].name)));

    bool requiresDuration = pgm_read_byte(&(commands[i].requiresDuration));
    uint8_t animationType = pgm_read_byte(&(commands[i].animationType));

    if (requiresDuration) {
      String cmdNameStr = String(cmdName) + " ";
      if (param.startsWith(cmdNameStr)) {
        int duration = param.substring(cmdNameStr.length()).toInt();
        if (duration > 0) {
          startAnimation(animationType, duration);
          return true;
        } else if (debugMode) {
          Serial.print(F("Invalid duration for "));
          Serial.print(cmdName);
          Serial.println(F(" animation"));
        }
        return true;
      }
    } else {
      if (param == cmdName) {
        if (animationType == ANIM_OFF) {
          off();
        } else {
          startAnimation(animationType, 0);
        }
        return true;
      }
    }
  }

  if (debugMode) {
    printHelp();
  }
  return true;
}

// Print help - optimized for flash memory
void LEDAnimations::printHelp() {
  Serial.println(F("LED commands:"));
  for (int i = 0; i < numCommands; i++) {
    char cmdName[20];
    strcpy_P(cmdName, (char*)pgm_read_ptr(&(commands[i].name)));
    bool requiresDuration = pgm_read_byte(&(commands[i].requiresDuration));

    Serial.print(F("  LED:"));
    Serial.print(cmdName);
    if (requiresDuration) {
      Serial.println(F(" <duration>"));
    } else {
      Serial.println();
    }
  }
}
