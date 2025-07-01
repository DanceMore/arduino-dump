#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

// Create a TM1637Display object
TM1637Display display(CLK, DIO);

// Custom segment patterns for animations
// Each byte represents segments: 0bABCDEFG where bits are segments a-g + dp
const uint8_t spinner[] = {
  0b00000001,  // bottom horizontal (segment a)
  0b00000010,  // bottom right vertical (segment b)  
  0b00000100,  // top right vertical (segment c)
  0b00001000,  // top horizontal (segment d)
  0b00010000,  // top left vertical (segment e)
  0b00100000   // bottom left vertical (segment f)
};

const uint8_t wave[] = {
  0b00001000,  // top
  0b00000100,  // top right
  0b00000010,  // bottom right
  0b00000001,  // bottom
  0b00100000,  // bottom left
  0b00010000   // top left
};

const uint8_t loading_dots[] = {
  0b10000000,  // dot only
  0b00000000   // empty
};

void setup() {
  display.setBrightness(0x0f);
  display.clear();
  
  Serial.begin(115200);
  Serial.println("Starting TM1637 animations");
}

void loop() {
  // Animation 1: Spinning segments across all digits
  spinnerAnimation();
  delay(500);
  
  // Animation 2: Wave effect across digits
  waveAnimation();
  delay(500);
  
  // Animation 3: Loading dots
  loadingDotsAnimation();
  delay(500);
  
  // Animation 4: Segment sweep left to right
  segmentSweepAnimation();
  delay(500);
  
  // Animation 5: Full digit rotation
  digitRotationAnimation();
  delay(500);
}

void spinnerAnimation() {
  // Rotate spinner pattern on each digit simultaneously
  for (int cycle = 0; cycle < 3; cycle++) {
    for (int i = 0; i < 6; i++) {
      uint8_t segments[] = {spinner[i], spinner[i], spinner[i], spinner[i]};
      display.setSegments(segments);
      delay(80);
    }
  }
  display.clear();
}

void waveAnimation() {
  // Wave effect moving from left to right
  for (int cycle = 0; cycle < 4; cycle++) {
    for (int pos = 0; pos < 4; pos++) {
      uint8_t segments[] = {0, 0, 0, 0};
      for (int i = 0; i < 6; i++) {
        segments[0] = segments[1] = segments[2] = segments[3] = 0;
        if (pos == 0) segments[0] = wave[i];
        if (pos == 1) segments[1] = wave[i];
        if (pos == 2) segments[2] = wave[i];
        if (pos == 3) segments[3] = wave[i];
        display.setSegments(segments);
        delay(60);
      }
    }
  }
  display.clear();
}

void loadingDotsAnimation() {
  // Loading dots animation
  for (int cycle = 0; cycle < 5; cycle++) {
    for (int pos = 0; pos < 4; pos++) {
      uint8_t segments[] = {0, 0, 0, 0};
      segments[pos] = loading_dots[0]; // dot
      display.setSegments(segments);
      delay(150);
    }
  }
  display.clear();
}

void segmentSweepAnimation() {
  // Turn on segments from left to right across all digits
  uint8_t segments[] = {0, 0, 0, 0};
  
  // Build up segments one by one
  for (int seg = 0; seg < 7; seg++) {
    uint8_t pattern = 1 << seg;
    for (int digit = 0; digit < 4; digit++) {
      segments[digit] |= pattern;
      display.setSegments(segments);
      delay(50);
    }
  }
  
  delay(300);
  
  // Clear segments one by one
  for (int seg = 0; seg < 7; seg++) {
    uint8_t pattern = ~(1 << seg);
    for (int digit = 0; digit < 4; digit++) {
      segments[digit] &= pattern;
      display.setSegments(segments);
      delay(50);
    }
  }
  display.clear();
}

void digitRotationAnimation() {
  // Rotate through different patterns on each digit
  const uint8_t patterns[] = {
    0b00111111,  // 0
    0b00000110,  // 1  
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
  };
  
  for (int cycle = 0; cycle < 2; cycle++) {
    for (int i = 0; i < 10; i++) {
      uint8_t segments[] = {patterns[i], patterns[i], patterns[i], patterns[i]};
      display.setSegments(segments);
      delay(100);
    }
  }
  display.clear();
}
