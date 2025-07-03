/*
 * LEDAnimations.cpp
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

// Command table - add new commands here and they'll automatically work
const LEDCommand LEDAnimations::commands[] = {
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

// HSV to RGB conversion for rainbow effect
void LEDAnimations::hsvToRgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b) {
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

    case ANIM_RAINBOW: // Rainbow hue shift
      {
        uint8_t r, g, b;
        float hue = fmod(animationPhase * 10, 360); // Cycle through hues
        hsvToRgb(hue, 1.0, 1.0, r, g, b);
        setColor(r, g, b);
        animationPhase += 0.05;
        if (animationPhase > 36.0) animationPhase = 0; // Reset
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

// Start LED animation
void LEDAnimations::startAnimation(int animType, int durationSeconds) {
  animationMode = animType;
  animationStep = 0;
  animationPhase = 0.0;
  lastAnimationUpdate = 0;

  switch (animType) {
    case ANIM_ACK: // Quick acknowledgment flash
      animationInterval = 100;
      animationEndTime = millis() + 300; // 300ms total
      break;

    case ANIM_NACK: // Quick negative acknowledgment flash
      animationInterval = 100;
      animationEndTime = millis() + 300; // 300ms total
      break;

    case ANIM_RED_BLUE: // Red-blue police style - fast
      animationInterval = 150;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case ANIM_TRAFFIC: // Traffic light - slow
      animationInterval = 800;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case ANIM_MATRIX: // Matrix effect
      animationInterval = 50;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case ANIM_RAINBOW: // Rainbow
      animationInterval = 30;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case ANIM_PULSE_RED: // Pulse red
    case ANIM_PULSE_BLUE: // Pulse blue
      animationInterval = 30;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case ANIM_STROBE: // Strobe
      animationInterval = 100;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case ANIM_FIRE: // Fire
      animationInterval = 80;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case ANIM_OCEAN: // Ocean
      animationInterval = 40;
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    case ANIM_THINKING: // Thinking - Simon sequence
      animationInterval = 200; // 200ms per step = 600ms per color
      animationEndTime = millis() + (durationSeconds * 1000UL);
      break;

    default: // Turn off
      animationEndTime = 0;
      setColor(0, 0, 0);
      break;
  }

  if (debugMode && animType != ANIM_ACK && animType != ANIM_NACK) { // Don't spam debug for ack/nack flashes
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
void LEDAnimations::flashAck() {
  startAnimation(ANIM_ACK, 0); // Mode 1, no duration needed
}

// Quick negative acknowledgment flash
void LEDAnimations::flashNack() {
  startAnimation(ANIM_NACK, 0); // Mode 2, no duration needed
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


// Process LED command - replaces the giant if/else block
bool LEDAnimations::processCommand(const String& command) {
  if (!command.startsWith("LED:")) {
    return false; // Not an LED command
  }

  String param = command.substring(4);

  // Check each command in the table
  for (int i = 0; i < numCommands; i++) {
    const LEDCommand& cmd = commands[i];

    if (cmd.requiresDuration) {
      // Command requires duration parameter
      String cmdName = String(cmd.name) + " ";
      if (param.startsWith(cmdName)) {
        int duration = param.substring(cmdName.length()).toInt();
        if (duration > 0) {
          startAnimation(cmd.animationType, duration);
          return true;
        } else if (debugMode) {
          Serial.print("Invalid duration for ");
          Serial.print(cmd.name);
          Serial.println(" animation");
        }
        return true; // Command was recognized even if invalid
      }
    } else {
      // Simple command without duration
      if (param == cmd.name) {
        if (cmd.animationType == ANIM_OFF) {
          off();
        } else {
          startAnimation(cmd.animationType, 0);
        }
        return true;
      }
    }
  }

  // Command not found - show help if in debug mode
  if (debugMode) {
    printHelp();
  }

  return true; // We handled the LED: prefix even if command was invalid
}

// Print help - automatically generated from command table
void LEDAnimations::printHelp() {
  Serial.println("LED commands:");
  for (int i = 0; i < numCommands; i++) {
    const LEDCommand& cmd = commands[i];
    Serial.print("  LED:");
    Serial.print(cmd.name);
    if (cmd.requiresDuration) {
      Serial.println(" <duration>");
    } else {
      Serial.println();
    }
  }
}
