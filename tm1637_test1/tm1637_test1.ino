#include <TM1637Display.h>
const int CLK = 5;
const int DIO = 4;
TM1637Display display(CLK, DIO);

void setup() {
  display.setBrightness(4);
  display.showNumberDec(1234);
  delay(2000);
  display.clear();
  delay(2000);
  display.setBrightness(1);
  display.showNumberDec(5678);
}

void loop() {}
