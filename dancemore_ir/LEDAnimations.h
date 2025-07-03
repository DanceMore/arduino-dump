/*
 * LEDAnimations.h
 * LED Animation Library for RGB LED Effects
 * Supports various animation modes with timing control
 */

#ifndef LEDANIMATIONS_H
#define LEDANIMATIONS_H

#include <Arduino.h>
#include <avr/pgmspace.h> // Required for PROGMEM

// Animation mode constants
enum AnimationMode {
  ANIM_OFF,
  ANIM_ACK,
  ANIM_NACK,
  ANIM_RED_BLUE,
  ANIM_TRAFFIC,
  ANIM_MATRIX,
  ANIM_RAINBOW,
  ANIM_PULSE_RED,
  ANIM_PULSE_BLUE,
  ANIM_STROBE,
  ANIM_FIRE,
  ANIM_OCEAN,
  ANIM_THINKING
};

// Command structure for LED animations
struct LEDCommand {
  const char* name;
  uint8_t animationType; // Changed to uint8_t for consistency with enum
  bool requiresDuration;
};

class LEDAnimations {
private:
  // Pin assignments
  int redPin, greenPin, bluePin;
  
  // Animation state
  AnimationMode animationMode; // Changed to enum type
  unsigned long animationEndTime;
  unsigned long lastAnimationUpdate;
  int animationStep;
  int animationInterval;
  uint8_t animationIndex;   // New: Integer index for sine wave animations
  uint8_t animationIndex2;  // New: Second integer index for ocean effect
  bool ackFlashState;
  bool debugMode;
  
  // Command table - automatically expandable
  static const LEDCommand commands[];
  static const int numCommands;
  
  // Internal methods
  // Removed hsvToRgb as it's no longer used with the lookup table approach
  void setColor(uint8_t red, uint8_t green, uint8_t blue);
  void getRainbowColor(uint8_t index, uint8_t &r, uint8_t &g, uint8_t &b);

  // PROGMEM tables
  static const uint8_t rainbowTable[][3]; // Declaration for rainbow table
  static const uint8_t RAINBOW_TABLE_SIZE; // Declaration for rainbow table size
  static const uint8_t sineLookup[]; // New: Declaration for sine lookup table
  
public:
  // Constructor
  LEDAnimations(int redPin, int greenPin, int bluePin);
  
  // Public methods
  void begin(bool debugMode = false);
  void update();
  void startAnimation(int animType, int durationSeconds);
  void flashAck();
  void flashNack();
  void off();
  bool isAnimating();
  
  // Command processing
  bool processCommand(const String& command);
  void printHelp();
};

#endif
