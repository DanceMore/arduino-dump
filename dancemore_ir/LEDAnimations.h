/*
 * LEDAnimations.h
 * LED Animation Library for RGB LED Effects
 * Supports various animation modes with timing control
 */

#ifndef LEDANIMATIONS_H
#define LEDANIMATIONS_H

#include <Arduino.h>

// Animation mode constants
#define ANIM_OFF 0
#define ANIM_ACK 1
#define ANIM_NACK 2
#define ANIM_RED_BLUE 3
#define ANIM_TRAFFIC 4
#define ANIM_MATRIX 5
#define ANIM_RAINBOW 6
#define ANIM_PULSE_RED 7
#define ANIM_PULSE_BLUE 8
#define ANIM_STROBE 9
#define ANIM_FIRE 10
#define ANIM_OCEAN 11
#define ANIM_THINKING 12

// Command structure for LED animations
struct LEDCommand {
  const char* name;
  int animationType;
  bool requiresDuration;
};

class LEDAnimations {
private:
  // Pin assignments
  int redPin, greenPin, bluePin;
  
  // Animation state
  int animationMode;
  unsigned long animationEndTime;
  unsigned long lastAnimationUpdate;
  int animationStep;
  int animationInterval;
  float animationPhase;
  bool ackFlashState;
  bool debugMode;
  
  // Command table - automatically expandable
  static const LEDCommand commands[];
  static const int numCommands;
  
  // Internal methods
  void hsvToRgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b);
  void setColor(uint8_t red, uint8_t green, uint8_t blue);

  void updateAck();
  void updateNack();
  void updateSimpleToggle();
  void updateTraffic();
  void updateSinWave();
  void updateRainbow();
  void updateFire();
  void updateThinking();
  
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
