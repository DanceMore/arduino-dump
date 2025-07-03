/*
 * LEDAnimations.cpp - Memory Optimized Version
 * LED Animation Library Implementation
 */

#include "LEDAnimations.h"

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
  animationPhase = 0.0;
  ackFlashState = false;
  debugMode = false;
}

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

void LEDAnimations::begin(bool debugMode) {
  this->debugMode = debugMode;

  // Initialize RGB LED pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  setColor(0, 0, 0); // Start with LED off
}

// Rainbow color lookup table - 60 entries covering full spectrum
// This replaces the floating-point HSV calculation entirely
const uint8_t rainbowTable[][3] PROGMEM = {
  {255,0,0}, {255,17,0}, {255,34,0}, {255,51,0}, {255,68,0}, {255,85,0}, {255,102,0}, {255,119,0}, {255,136,0}, {255,153,0},
  {255,170,0}, {255,187,0}, {255,204,0}, {255,221,0}, {255,238,0}, {255,255,0}, {238,255,0}, {221,255,0}, {204,255,0}, {187,255,0},
  {170,255,0}, {153,255,0}, {136,255,0}, {119,255,0}, {102,255,0}, {85,255,0}, {68,255,0}, {51,255,0}, {34,255,0}, {17,255,0},
  {0,255,0}, {0,255,17}, {0,255,34}, {0,255,51}, {0,255,68}, {0,255,85}, {0,255,102}, {0,255,119}, {0,255,136}, {0,255,153},
  {0,255,170}, {0,255,187}, {0,255,204}, {0,255,221}, {0,255,238}, {0,255,255}, {0,238,255}, {0,221,255}, {0,204,255}, {0,187,255},
  {0,170,255}, {0,153,255}, {0,136,255}, {0,119,255}, {0,102,255}, {0,85,255}, {0,68,255}, {0,51,255}, {0,34,255}, {0,17,255}
};

const uint8_t RAINBOW_TABLE_SIZE = sizeof(rainbowTable) / sizeof(rainbowTable[0]);

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
        case 0: setColor(255, 0, 0); break;   // Red
        case 1: setColor(0, 255, 0); break;   // Green
        case 2: setColor(255, 255, 0); break; // Yellow
      }
      animationStep++;
      break;

    case ANIM_MATRIX: // Matrix effect - green fade
      {
        float intensity = (sin(animationPhase) + 1.0) / 2.0; // 0.0 to 1.0
        uint8_t green = (uint8_t)(intensity * 255);
        setColor(0, green, 0);
        animationPhase += 0.1;
        if (animationPhase > 6.28) animationPhase = 0; // Reset at 2π
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

    case ANIM_PULSE_RED: // Pulse red
      {
        float intensity = (sin(animationPhase) + 1.0) / 2.0; // 0.0 to 1.0
        uint8_t red = (uint8_t)(intensity * 255);
        setColor(red, 0, 0);
        animationPhase += 0.15;
        if (animationPhase > 6.28) animationPhase = 0;
      }
      break;

    case ANIM_PULSE_BLUE: // Pulse blue
      {
        float intensity = (sin(animationPhase) + 1.0) / 2.0; // 0.0 to 1.0
        uint8_t blue = (uint8_t)(intensity * 255);
        setColor(0, 0, blue);
        animationPhase += 0.15;
        if (animationPhase > 6.28) animationPhase = 0;
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

    case ANIM_OCEAN: // Ocean wave effect
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

    case ANIM_THINKING: // Thinking - Simon-like sequence
      {
        // Cycle through: green, red, yellow, blue with smooth fade
        int colorIndex = (animationStep / 3) % 4; // Each color lasts 3 steps
        int fadeStep = animationStep % 3;         // 0=fade in, 1=hold, 2=fade out
        
        float intensity = (fadeStep == 0) ? 0.6 : (fadeStep == 1) ? 1.0 : 0.4;
        uint8_t brightness = (uint8_t)(intensity * 180); // Not full brightness for subtlety
        
        switch (colorIndex) {
          case 0: setColor(0, brightness, 0); break;           // Green
          case 1: setColor(brightness, 0, 0); break;           // Red  
          case 2: setColor(brightness, brightness, 0); break;  // Yellow
          case 3: setColor(0, 0, brightness); break;           // Blue
        }
        animationStep++;
      }
      break;

    default: // Off
      setColor(0, 0, 0);
      break;
  }
}

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
  {ANIM_RAINBOW, 30, 0},
  {ANIM_PULSE_RED, 30, 0},
  {ANIM_PULSE_BLUE, 30, 0},
  {ANIM_STROBE, 100, 0},
  {ANIM_FIRE, 80, 0},
  {ANIM_OCEAN, 40, 0},
  {ANIM_THINKING, 200, 0}
};

const int numTimings = sizeof(timingTable) / sizeof(timingTable[0]);

// Start LED animation - optimized with lookup table
void LEDAnimations::startAnimation(int animType, int durationSeconds) {
  animationMode = animType;
  animationStep = 0;
  animationPhase = 0.0;
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
