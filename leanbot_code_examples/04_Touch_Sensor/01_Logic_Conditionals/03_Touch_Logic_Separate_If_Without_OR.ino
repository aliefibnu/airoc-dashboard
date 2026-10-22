#include <Leanbot.h>
 
void setup() {
  Leanbot.begin();                    // initialize Leanbot
}
 
void loop() {
  int value1A = LbTouch.read(TB1A);
  int value1B = LbTouch.read(TB1B);
 
  if (value1A == HIGH) {
    LbMotion.runLR(500, 0);
  }
 
  if (value1B == HIGH) {
    LbMotion.runLR(500, 0);
  }
}
