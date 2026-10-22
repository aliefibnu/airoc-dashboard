#include <Leanbot.h>
 
#define BEEP_FREQ   1500
 
byte LbSmiley     = BITMAP(ledA, ledB, ledC, ledD);
byte LbFrowning   = BITMAP(ledA, ledF, ledE, ledD);
byte LbLeftArrow  = BITMAP(ledA, ledO, ledE, ledD, ledC);
byte LbRightArrow = BITMAP(ledD, ledO, ledF, ledA, ledB);
 
 
void setup() {
  Leanbot.begin();                     // initialize Leanbot
  LbRGB.clear();
  LbRGB.fillColor(CRGB::Green, LbSmiley);
  LbRGB.show();
}
 
 
void loop() {
  if (LbTouch.read(TB1A) == HIGH) {    // touch TB1A
    LbMotion.stopAndWait();
    delay(1000);
    beepSingle();
    task1A();
  }
 
  if (LbTouch.read(TB1B) == HIGH) {    // touch TB1B
    LbMotion.stopAndWait();
    delay(1000);
    beepSingle();
    task1B();
  }
  LbDelay(10);                         // time delay between each touch button test
}
 
 
void task1A() { // FollowObject
  LbRGB.clear();
  while(1) {
    int d  = Leanbot.pingCm();         // Measure the distance to the obstacle
    byte c = map(d, 0, 50, 0, 255);
    LbRGB.fillColor( CRGB(255 - c, c, 0) );
    LbRGB.show(); 
    Serial.println(d);                 // Transfer the results to the computer
 
    int limit  = 15;
    int offset = 1;
 
    if ( (d == 0) || (d > 50) ) {
      LbMotion.stopAndWait();
    } else if (d > (limit + offset)) {
      LbMotion.runLR(+1500, +1500);
    } else if (d < (limit - offset)) {
      LbMotion.runLR(-1500, -1500);
    } else {
      LbMotion.stopAndWait();
    }
  
    delay(50);                         // Time delay between each loop
  }
}
 
 
void task1B() {  // Touch_RobotControl
  while(1) {
    if (LbTouch.read(TB1A) == HIGH) {  // Touch the left T1 button then turn left
      LbRGB.clear();
      LbRGB.fillColor(CRGB::Red, LbLeftArrow);
      LbRGB.show();
      LbMotion.runLR(+500, +1000);
    }
 
    if (LbTouch.read(TB1B) == HIGH) {  // Press the right T1 button then turn right
      LbRGB.clear();
      LbRGB.fillColor(CRGB::Green, LbRightArrow);
      LbRGB.show();
      LbRGB.show();
      LbMotion.runLR(+1000, +500);
    }
 
    if (LbTouch.read(TB2A) == HIGH) {  // Touch the left T2 button to move forward
      LbRGB.clear();
      LbRGB.fillColor(CRGB::Yellow, LbFrowning);
      LbRGB.show();
      LbMotion.runLR(+1000, +1000);
    }
 
    if (LbTouch.read(TB2B) == HIGH) {  // press the right button T2 then stop
      LbRGB.clear();
      LbRGB.fillColor(CRGB::Green, LbSmiley);
      LbRGB.show();
      LbMotion.runLR(0, 0);            // LbMotion.stopAndWait();
    }
  }
}
 
 
void beepSingle() {
  Leanbot.tone(BEEP_FREQ, 50);         // play sound at 1500Hz for 50ms
}
