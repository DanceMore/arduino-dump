/*
 * StringConstants.h - Memory-optimized string constants and help system
 * Stores all strings in PROGMEM to save RAM and provides structured access
 */

#ifndef STRING_CONSTANTS_H
#define STRING_CONSTANTS_H

#include <Arduino.h>
#include <avr/pgmspace.h>

// ================== FLASH MEMORY STRING STORAGE ==================

// System messages
const char MSG_IR_RX[] PROGMEM = "ir rx";
const char MSG_RECEIVING[] PROGMEM = "Receiving...";
const char MSG_PRESS_CTRL_C[] PROGMEM = "Press Ctrl+C to stop";
const char MSG_JUMPER_HINT[] PROGMEM = "(Insert Pin 10->GND jumper and restart for debug mode)";
const char MSG_BASIC_COMMANDS[] PROGMEM = "Commands: DISP:text, DISP:CLR, LED:ack, LED:matrix 45, LED:rainbow 60, LED:off";

// Debug startup banner
const char DEBUG_BANNER[] PROGMEM = "=== Arduino IR Receiver + TM1637 Display + Enhanced RGB LED (DEBUG MODE) ===";
const char DEBUG_CONFIG[] PROGMEM = "Configuration:";
const char DEBUG_JUMPER_INSTALLED[] PROGMEM = "  - Debug Jumper: INSTALLED (Pin 10 -> GND)";
const char DEBUG_MODE_ON[] PROGMEM = "  - Debug Mode: ON";
const char DEBUG_SHOW_REPEATS[] PROGMEM = "  - Show Repeats: ";
const char DEBUG_SHOW_RAW[] PROGMEM = "  - Show Raw Data: ";
const char DEBUG_BAUD_RATE[] PROGMEM = "  - Baud Rate: ";
const char DEBUG_DISPLAY_TYPE[] PROGMEM = "  - Display: TM1637 4-Digit 7-Segment";
const char DEBUG_LED_TYPE[] PROGMEM = "  - RGB LED: Common Anode (Pins 9,6,3)";
const char DEBUG_WAITING[] PROGMEM = "Waiting for IR signals and commands...";
const char DEBUG_FORMAT_HEADER[] PROGMEM = "Format: Protocol | Address | Command | Raw Value | Bits | Time";
const char DEBUG_SEPARATOR[] PROGMEM = "---";
const char DEBUG_REMOVE_JUMPER[] PROGMEM = "Remove jumper and restart for production mode";

// Common strings
const char STR_ON[] PROGMEM = "ON";
const char STR_OFF[] PROGMEM = "OFF";
const char STR_UNKNOWN[] PROGMEM = "UNKNOWN";
const char STR_NOISE_FILTERED[] PROGMEM = "[NOISE FILTERED]";
const char STR_REPEAT[] PROGMEM = "REPEAT";
const char STR_STATS[] PROGMEM = "Stats: ";
const char STR_VALID_SIGNALS[] PROGMEM = " valid signals, ";
const char STR_SUCCESS_RATE[] PROGMEM = "% success rate";

// Error messages
const char ERR_INVALID_DURATION[] PROGMEM = "Invalid duration for ";
const char ERR_INVALID_BRIGHTNESS[] PROGMEM = "Invalid brightness (0-7)";
const char ERR_DISPLAY_OFF[] PROGMEM = "Display is OFF - use DISP:ON to enable";

// Command prefixes
const char CMD_DISP[] PROGMEM = "DISP:";
const char CMD_LED[] PROGMEM = "LED:";
const char CMD_CLR[] PROGMEM = "CLR";
const char CMD_BRT[] PROGMEM = "BRT:";

// Animation names (for commands)
const char ANIM_NAME_ACK[] PROGMEM = "ack";
const char ANIM_NAME_NACK[] PROGMEM = "nack";
const char ANIM_NAME_RED_BLUE[] PROGMEM = "red-blue";
const char ANIM_NAME_TRAFFIC[] PROGMEM = "red-green-yellow";
const char ANIM_NAME_MATRIX[] PROGMEM = "matrix";
const char ANIM_NAME_RAINBOW[] PROGMEM = "rainbow";
const char ANIM_NAME_PULSE_RED[] PROGMEM = "pulse-red";
const char ANIM_NAME_PULSE_BLUE[] PROGMEM = "pulse-blue";
const char ANIM_NAME_STROBE[] PROGMEM = "strobe";
const char ANIM_NAME_FIRE[] PROGMEM = "fire";
const char ANIM_NAME_OCEAN[] PROGMEM = "ocean";
const char ANIM_NAME_THINKING[] PROGMEM = "thinking";

// Protocol mappings for Flipper Zero compatibility
const char PROTOCOL_SONY[] PROGMEM = "Sony";
const char PROTOCOL_SIRC[] PROGMEM = "SIRC";
const char PROTOCOL_SAMSUNG32[] PROGMEM = "Samsung32";
const char PROTOCOL_SAMSUNG[] PROGMEM = "Samsung";

// ================== HELP SYSTEM STRUCTURES ==================

// Structure for command help entries
struct CommandHelp {
    const char* command;      // Command string in PROGMEM
    const char* description;  // Description string in PROGMEM
    bool showInBasicHelp;     // Show in basic help mode
};

// Display commands help
const char HELP_DISP_TEXT[] PROGMEM = "DISP:text    - Display text (up to 4 chars)";
const char HELP_DISP_NUM[] PROGMEM = "DISP:1234    - Display number";
const char HELP_DISP_CLR[] PROGMEM = "DISP:CLR     - Clear display";
const char HELP_DISP_BRT[] PROGMEM = "DISP:BRT:7   - Set brightness (0-7)";
const char HELP_DISP_ON[] PROGMEM = "DISP:ON      - Turn display on";
const char HELP_DISP_OFF[] PROGMEM = "DISP:OFF     - Turn display off";

// LED commands help
const char HELP_LED_ACK[] PROGMEM = "LED:ack                    - Quick green acknowledgment flash";
const char HELP_LED_NACK[] PROGMEM = "LED:nack                   - Quick red acknowledgment flash";
const char HELP_LED_POLICE[] PROGMEM = "LED:red-blue 30            - Police style for 30 seconds";
const char HELP_LED_TRAFFIC[] PROGMEM = "LED:red-green-yellow 60    - Traffic light for 60 seconds";
const char HELP_LED_MATRIX[] PROGMEM = "LED:matrix 45              - Green Matrix fade for 45 seconds";
const char HELP_LED_RAINBOW[] PROGMEM = "LED:rainbow 60             - Rainbow hue shift for 60 seconds";
const char HELP_LED_PULSE_RED[] PROGMEM = "LED:pulse-red 30           - Red pulsing for 30 seconds";
const char HELP_LED_PULSE_BLUE[] PROGMEM = "LED:pulse-blue 30          - Blue pulsing for 30 seconds";
const char HELP_LED_STROBE[] PROGMEM = "LED:strobe 15              - White strobe for 15 seconds";
const char HELP_LED_FIRE[] PROGMEM = "LED:fire 40                - Fire flicker for 40 seconds";
const char HELP_LED_OCEAN[] PROGMEM = "LED:ocean 50               - Ocean waves for 50 seconds";
const char HELP_LED_THINKING[] PROGMEM = "LED:thinking 20            - Simon-like thinking";
const char HELP_LED_OFF[] PROGMEM = "LED:off                    - Turn off LED";

// Help system arrays
const CommandHelp DISPLAY_COMMANDS[] PROGMEM = {
    {HELP_DISP_TEXT, nullptr, true},
    {HELP_DISP_NUM, nullptr, true},
    {HELP_DISP_CLR, nullptr, true},
    {HELP_DISP_BRT, nullptr, true},
    {HELP_DISP_ON, nullptr, true},
    {HELP_DISP_OFF, nullptr, true}
};

const CommandHelp LED_COMMANDS[] PROGMEM = {
    {HELP_LED_ACK, nullptr, true},
    {HELP_LED_NACK, nullptr, false},
    {HELP_LED_POLICE, nullptr, true},
    {HELP_LED_TRAFFIC, nullptr, true},
    {HELP_LED_MATRIX, nullptr, true},
    {HELP_LED_RAINBOW, nullptr, true},
    {HELP_LED_PULSE_RED, nullptr, true},
    {HELP_LED_PULSE_BLUE, nullptr, true},
    {HELP_LED_STROBE, nullptr, true},
    {HELP_LED_FIRE, nullptr, true},
    {HELP_LED_OCEAN, nullptr, true},
    {HELP_LED_THINKING, nullptr, true},
    {HELP_LED_OFF, nullptr, true}
};

const int DISPLAY_COMMANDS_COUNT = sizeof(DISPLAY_COMMANDS) / sizeof(CommandHelp);
const int LED_COMMANDS_COUNT = sizeof(LED_COMMANDS) / sizeof(CommandHelp);

// ================== ANIMATION CONFIGURATION ==================

// Animation configuration structure
struct AnimationConfig {
    uint8_t mode;           // Animation mode constant
    uint16_t interval;      // Update interval in milliseconds
    const char* name;       // Name string in PROGMEM
    uint8_t nameLength;     // Length of name string for parsing
};

// Animation configurations in PROGMEM
const AnimationConfig ANIMATIONS[] PROGMEM = {
    {ANIM_ACK, 100, ANIM_NAME_ACK, 3},
    {ANIM_NACK, 100, ANIM_NAME_NACK, 4},
    {ANIM_RED_BLUE, 150, ANIM_NAME_RED_BLUE, 8},
    {ANIM_TRAFFIC, 800, ANIM_NAME_TRAFFIC, 16},
    {ANIM_MATRIX, 50, ANIM_NAME_MATRIX, 6},
    {ANIM_RAINBOW, 30, ANIM_NAME_RAINBOW, 7},
    {ANIM_PULSE_RED, 30, ANIM_NAME_PULSE_RED, 9},
    {ANIM_PULSE_BLUE, 30, ANIM_NAME_PULSE_BLUE, 10},
    {ANIM_STROBE, 100, ANIM_NAME_STROBE, 6},
    {ANIM_FIRE, 80, ANIM_NAME_FIRE, 4},
    {ANIM_OCEAN, 40, ANIM_NAME_OCEAN, 5},
    {ANIM_THINKING, 200, ANIM_NAME_THINKING, 8}
};

const int ANIMATIONS_COUNT = sizeof(ANIMATIONS) / sizeof(AnimationConfig);

// ================== UTILITY FUNCTIONS ==================

class StringManager {
private:
    static char buffer[64]; // Shared buffer for PROGMEM string operations
    
public:
    // Read a PROGMEM string into the buffer
    static char* readProgmemString(const char* progmemStr) {
        strcpy_P(buffer, progmemStr);
        return buffer;
    }
    
    // Compare a RAM string with a PROGMEM string
    static bool compareWithProgmem(const char* ramStr, const char* progmemStr) {
        return strcmp_P(ramStr, progmemStr) == 0;
    }
    
    // Get animation config by name
    static const AnimationConfig* getAnimationConfig(const char* name) {
        for (int i = 0; i < ANIMATIONS_COUNT; i++) {
            AnimationConfig config;
            memcpy_P(&config, &ANIMATIONS[i], sizeof(AnimationConfig));
            if (compareWithProgmem(name, config.name)) {
                return &ANIMATIONS[i];
            }
        }
        return nullptr;
    }
    
    // Print help for display commands
    static void printDisplayHelp(bool debugMode = false) {
        if (debugMode) {
            Serial.println(readProgmemString(PSTR("Display Commands:")));
        }
        
        for (int i = 0; i < DISPLAY_COMMANDS_COUNT; i++) {
            CommandHelp cmd;
            memcpy_P(&cmd, &DISPLAY_COMMANDS[i], sizeof(CommandHelp));
            
            if (debugMode || cmd.showInBasicHelp) {
                Serial.print(F("  "));
                Serial.println(readProgmemString(cmd.command));
            }
        }
    }
    
    // Print help for LED commands
    static void printLEDHelp(bool debugMode = false) {
        if (debugMode) {
            Serial.println(readProgmemString(PSTR("Enhanced LED Animation Commands:")));
        }
        
        for (int i = 0; i < LED_COMMANDS_COUNT; i++) {
            CommandHelp cmd;
            memcpy_P(&cmd, &LED_COMMANDS[i], sizeof(CommandHelp));
            
            if (debugMode || cmd.showInBasicHelp) {
                Serial.print(F("  "));
                Serial.println(readProgmemString(cmd.command));
            }
        }
    }
    
    // Print startup messages
    static void printStartupMessage(bool debugMode, bool showRepeats, bool showRawData, unsigned long baudRate) {
        if (debugMode) {
            Serial.println(readProgmemString(DEBUG_BANNER));
            Serial.println(readProgmemString(DEBUG_CONFIG));
            Serial.println(readProgmemString(DEBUG_JUMPER_INSTALLED));
            Serial.println(readProgmemString(DEBUG_MODE_ON));
            
            Serial.print(readProgmemString(DEBUG_SHOW_REPEATS));
            Serial.println(showRepeats ? readProgmemString(STR_ON) : readProgmemString(STR_OFF));
            
            Serial.print(readProgmemString(DEBUG_SHOW_RAW));
            Serial.println(showRawData ? readProgmemString(STR_ON) : readProgmemString(STR_OFF));
            
            Serial.print(readProgmemString(DEBUG_BAUD_RATE));
            Serial.println(baudRate);
            
            Serial.println(readProgmemString(DEBUG_DISPLAY_TYPE));
            Serial.println(readProgmemString(DEBUG_LED_TYPE));
            Serial.println();
            
            printDisplayHelp(true);
            Serial.println();
            printLEDHelp(true);
            Serial.println();
            
            Serial.println(readProgmemString(DEBUG_REMOVE_JUMPER));
            Serial.println(readProgmemString(DEBUG_WAITING));
            Serial.println(readProgmemString(DEBUG_FORMAT_HEADER));
            Serial.println(readProgmemString(DEBUG_SEPARATOR));
            
        } else {
            Serial.println(readProgmemString(MSG_IR_RX));
            Serial.println(readProgmemString(MSG_RECEIVING));
            Serial.println(readProgmemString(MSG_PRESS_CTRL_C));
            Serial.println(readProgmemString(MSG_JUMPER_HINT));
            Serial.println(readProgmemString(MSG_BASIC_COMMANDS));
        }
    }
    
    // Protocol name mapping for Flipper Zero compatibility
    static const char* getFlipperProtocolName(const char* arduinoProtocol) {
        if (compareWithProgmem(arduinoProtocol, PROTOCOL_SONY)) {
            return readProgmemString(PROTOCOL_SIRC);
        }
        if (compareWithProgmem(arduinoProtocol, PROTOCOL_SAMSUNG32) || 
            compareWithProgmem(arduinoProtocol, PROTOCOL_SAMSUNG)) {
            return readProgmemString(PROTOCOL_SAMSUNG32);
        }
        return arduinoProtocol; // Return as-is for most protocols
    }
    
    // Print error messages
    static void printInvalidDuration(const char* animationName) {
        Serial.print(readProgmemString(ERR_INVALID_DURATION));
        Serial.print(animationName);
        Serial.println(readProgmemString(PSTR(" animation")));
    }
    
    // Print statistics
    static void printStats(unsigned long valid, unsigned long total) {
        Serial.print(readProgmemString(PSTR("  [")));
        Serial.print(readProgmemString(STR_STATS));
        Serial.print(valid);
        Serial.print(F("/"));
        Serial.print(total);
        Serial.print(readProgmemString(STR_VALID_SIGNALS));
        Serial.print((valid * 100) / total);
        Serial.print(readProgmemString(STR_SUCCESS_RATE));
        Serial.println(F("]"));
    }
};

// Static buffer definition
char StringManager::buffer[64];

// ================== MEMORY USAGE REPORTING ==================

#ifdef DEBUG_MEMORY_USAGE
void printMemoryUsage() {
    Serial.print(F("Free RAM: "));
    Serial.print(freeRam());
    Serial.println(F(" bytes"));
    
    Serial.print(F("PROGMEM strings: ~"));
    Serial.print(F("2048")); // Approximate calculation
    Serial.println(F(" bytes"));
}

int freeRam() {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif

#endif // STRING_CONSTANTS_H

/*
 * Usage Example in main Arduino file:
 * 
 * #include "StringConstants.h"
 * 
 * void setup() {
 *     Serial.begin(115200);
 *     
 *     // Instead of multiple Serial.println() calls:
 *     StringManager::printStartupMessage(DEBUG_MODE, SHOW_REPEATS, SHOW_RAW_DATA, BAUD_RATE);
 *     
 *     // Instead of string comparisons:
 *     if (StringManager::compareWithProgmem(command.c_str(), CMD_DISP)) {
 *         // Handle display command
 *     }
 *     
 *     // Instead of hardcoded animation names:
 *     const AnimationConfig* config = StringManager::getAnimationConfig("matrix");
 *     if (config) {
 *         // Start animation with config->mode, config->interval
 *     }
 * }
 * 
 * MEMORY SAVINGS:
 * - Moves ~2KB of strings from RAM to PROGMEM
 * - Reduces string fragmentation
 * - Provides structured access to all text
 * - Enables easy localization if needed
 * - Reduces code duplication
 */
