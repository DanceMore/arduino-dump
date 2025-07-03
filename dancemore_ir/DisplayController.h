/*
 * DisplayController.h - TM1637 7-Segment Display Controller
 *
 * Handles all TM1637 display operations including:
 * - Text and numeric display
 * - Brightness control
 * - On/off state management
 * - Serial command processing
 * - Character encoding for 7-segment display
 */

#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <Arduino.h>
#include <TM1637Display.h>

#define CHAR_LOOKUP_MIN 32  // ' '
#define CHAR_LOOKUP_MAX 90  // 'Z'
#define CHAR_LOOKUP_LEN (CHAR_LOOKUP_MAX - CHAR_LOOKUP_MIN + 1)  // = 59

static const uint8_t CHAR_LOOKUP[CHAR_LOOKUP_LEN] PROGMEM = {
  // 32-47: space and symbols (16 entries)
  0x00, 0x00, 0x00, 0x00,  // ' ', '!', '"', '#'
  0x00, 0x00, 0x00, 0x00,  // '$', '%', '&', '''
  0x00, 0x00, 0x00, 0x40,  // '(', ')', '*', **'+' (we map this to dash)**
  0x00, 0x40, 0x00, 0x00,  // ',', **'-'**, '.', '/' (only dash & comma mapped)

  // 48-57: '0'–'9'
  0x3F, 0x06, 0x5B, 0x4F,
  0x66, 0x6D, 0x7D, 0x07,
  0x7F, 0x6F,

  // 58-64: symbols before letters
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ':', ';', '<', '=', '>', '?', '@'

  // 65-90: 'A'–'Z'
  0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76,
  0x06, 0x1E, 0x00, 0x38, 0x00, 0x54, 0x3F, 0x73,
  0x00, 0x50, 0x6D, 0x78, 0x3E, 0x00, 0x00, 0x00,
  0x6E, 0x00
};

class DisplayController {
private:
    TM1637Display display;
    uint8_t displayBrightness;
    bool displayEnabled;
    bool debugMode;

    // Character encoding for 7-segment display
    uint8_t encodeChar(char c);

    // Check if a string contains only digits
    bool isNumericString(String str);

    // Display a numeric string exactly as specified (no automatic padding)
    void displayNumericString(String numStr);

public:
    // Constructor
    DisplayController(int clkPin, int dioPin, bool debug = false);

    // Initialize the display
    void begin();

    // Process display commands from serial input
    // Returns true if command was processed, false if not a display command
    bool processCommand(String command);

    // Direct display control methods
    void clear();
    void displayText(String text);
    void displayNumber(int number);
    void setBrightness(int brightness);
    void turnOn();
    void turnOff();
    bool isEnabled();

    // Show startup patterns
    void showStartupPattern();
    void showReady();
    void showDashes();

    // Set debug mode
    void setDebugMode(bool debug);
};

#endif
