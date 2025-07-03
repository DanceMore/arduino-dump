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
