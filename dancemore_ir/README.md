Arduino IR Receiver with Jumper-Controlled Debug Mode + TM1637 Display + Enhanced RGB LED Effects
Compatible with Flipper Zero ir rx format + optional debugging + 7-segment display + LED animations

Debug Control:
- Insert jumper between Pin 10 and GND for DEBUG_MODE=true
- Remove jumper for production/Flipper Zero compatibility
- No recompilation needed!

Display Control via Serial Commands:
- DISP:text    -> Display text (up to 4 chars)
- DISP:1234    -> Display number 1234
- DISP:CLR     -> Clear display
- DISP:BRT:7   -> Set brightness (0-7)
- DISP:ON      -> Turn display on
- DISP:OFF     -> Turn display off

LED Animation Control via Serial Commands:
- LED:red-blue 30         -> Fast police-style red/blue for 30 seconds
- LED:red-green-yellow 60 -> Slow traffic light cycle for 60 seconds
- LED:ack                 -> Quick green flash for command acknowledgment
- LED:matrix 45           -> Green Matrix-style fade effect for 45 seconds
- LED:rainbow 60          -> Rainbow hue shift for 60 seconds
- LED:pulse-red 30        -> Red pulsing effect for 30 seconds
- LED:pulse-blue 30       -> Blue pulsing effect for 30 seconds
- LED:strobe 15           -> White strobe effect for 15 seconds
- LED:fire 40             -> Fire effect (red/orange flicker) for 40 seconds
- LED:ocean 50            -> Ocean wave effect (blue/cyan) for 50 seconds
- LED:thinking 20         -> Simon-like thinking sequence for 20 seconds
- LED:off                 -> Stop all animations and turn off LED

Wiring:
IR Receiver Module -> Arduino
VCC    -> 3.3V
GND    -> GND
OUTPUT -> Pin 2

TM1637 Display -> Arduino
VCC -> 5V
GND -> GND
DIO -> Pin 4
CLK -> Pin 5

RGB LED -> Arduino (Common Anode)
VCC -> 3.3V
RED -> Pin 9 (through 220Ω resistor)
GREEN -> Pin 6 (through 220Ω resistor)
BLUE -> Pin 3 (through 220Ω resistor)

Debug Jumper:
Pin 10 -> GND (jumper wire or actual jumper)

LIBRARIES REQUIRED:
- Install "TM1637" library by Avishay Orpaz from Library Manager
- IRremote library
