/*
 * DisplayController.cpp - TM1637 7-Segment Display Controller Implementation
 */

#include "DisplayController.h"

DisplayController::DisplayController(int clkPin, int dioPin, bool debug) 
    : display(clkPin, dioPin), displayBrightness(4), displayEnabled(true), debugMode(debug) {
}

void DisplayController::begin() {
    display.setBrightness(displayBrightness);
    display.clear();
    showStartupPattern();
}

void DisplayController::showStartupPattern() {
    display.showNumberDec(8888); // All segments on briefly
    delay(500);
    display.clear();
}

void DisplayController::showReady() {
    uint8_t readySegments[] = {0x50, 0x79, 0x5E, 0x6E}; // "REDY"
    display.setSegments(readySegments);
}

void DisplayController::showDashes() {
    uint8_t dashSegments[] = {0x40, 0x40, 0x40, 0x40}; // "----"
    display.setSegments(dashSegments);
}

void DisplayController::setDebugMode(bool debug) {
    debugMode = debug;
}

bool DisplayController::processCommand(String command) {
    // Convert to uppercase for display commands
    command.toUpperCase();

    if (!command.startsWith("DISP:")) {
        return false; // Not a display command
    }

    String param = command.substring(5);

    if (param == "CLR") {
        clear();
        if (debugMode) Serial.println("Display cleared");

    } else if (param == "ON") {
        turnOn();
        if (debugMode) Serial.println("Display turned ON");

    } else if (param == "OFF") {
        turnOff();
        if (debugMode) Serial.println("Display turned OFF");

    } else if (param.startsWith("BRT:")) {
        int brightness = param.substring(4).toInt();
        if (brightness >= 0 && brightness <= 7) {
            setBrightness(brightness);
            if (debugMode) {
                Serial.print("Display brightness set to: ");
                Serial.println(brightness);
            }
        } else if (debugMode) {
            Serial.println("Invalid brightness (0-7)");
        }

    } else {
        // Display text or number
        if (displayEnabled) {
            displayText(param);
            if (debugMode) {
                Serial.print("Displayed: ");
                Serial.println(param);
            }
        } else if (debugMode) {
            Serial.println("Display is OFF - use DISP:ON to enable");
        }
    }

    return true; // Command was processed
}

void DisplayController::clear() {
    display.clear();
}

void DisplayController::displayText(String text) {
    if (!displayEnabled) return;

    // Check if it's a numeric string (including those with leading zeros)
    if (isNumericString(text) && text.length() <= 4) {
        displayNumericString(text);
    } else {
        // Display as text (up to 4 characters)
        if (text.length() > 4) {
            text = text.substring(0, 4);
        }

        // Convert string to display segments
        uint8_t segments[4] = {0, 0, 0, 0};
        for (int i = 0; i < text.length() && i < 4; i++) {
            segments[i] = encodeChar(text.charAt(i));
        }
        display.setSegments(segments);
    }
}

void DisplayController::displayNumber(int number) {
    if (!displayEnabled) return;
    display.showNumberDec(number);
}

void DisplayController::setBrightness(int brightness) {
    if (brightness >= 0 && brightness <= 7) {
        displayBrightness = brightness;
        if (displayEnabled) {
            display.setBrightness(displayBrightness);
        }
    }
}

void DisplayController::turnOn() {
    displayEnabled = true;
    display.setBrightness(displayBrightness);
}

void DisplayController::turnOff() {
    displayEnabled = false;
    display.setBrightness(0);
}

bool DisplayController::isEnabled() {
    return displayEnabled;
}

bool DisplayController::isNumericString(String str) {
    if (str.length() == 0) return false;
    for (int i = 0; i < str.length(); i++) {
        if (!isdigit(str.charAt(i))) {
            return false;
        }
    }
    return true;
}

void DisplayController::displayNumericString(String numStr) {
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

uint8_t DisplayController::encodeChar(char c) {
    if (c < 32 || c > 90) return 0x00;
    return pgm_read_byte(&CHAR_LOOKUP[c - 32]);
}
