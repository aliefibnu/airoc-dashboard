/*
  Steps to perform black/white sampling with tbX touch button:
 
  1. Initially, all 4 RGB LEDs blink red indicating Leanbot is not aligned
  2. Lift and place Leanbot so that all 4 IR Line sensors are looking at the white area
  3. Touch the tbX button (1st time) to read the white level value
  4. All 4 RGB Leds turn to flashing yellow, indicating that the white level alignment is complete
  5. Lift and place Leanbot so that all 4 IR line sensors look at the black area
  6. Touch the tbX button (2nd time) to read the black level value
  7. Leanbot goes into test mode:
     RGB led blinks green if sensor looks at black area
  8. (Optional) Lift Leanbot to different positions
     and observe 4 RGB leds to see if 4 sensors have correctly distinguished black/white
  9. After enough testing to see that 4 sensors have correctly distinguished black/white
     then touch the tbX button (3rd, last time) to finish the alignment
 
  If there is an error during alignment (wrong operation or RGB led is displayed incorrectly)
    Then press the Reset button to start over
*/
 
#include <Leanbot.h>
 
#include <NonBlockingRtttl.h>
const char *mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a5,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a5,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6,p,p,p,p";
#define BEEP_FREQ   1500
 
int speed = 1000;
 
void setup() {
  Leanbot.begin();                                   // initialize Leanbot
  LbIRLine.doManualCalibration(TB1A);                // touch TB1A to measure black/white level, align bar sensor
//LbIRLine.setThreshold(287,  209,  472,  281);      // or measure first and then edit the threshold in the code
}
 
void loop() {
  if (! check_touch()) {
    return;
  }
 
  if (! check_distance()) {
    return;
  }
 
  if (! followLineCarefully()) {
    return;
  }
  play_melody();
}
 
 
void ledOn() {
  LbRGB.fillColor(CRGB::White);       // white setting for all lights
  LbRGB.show();                       // light status update
}
 
void ledOff() {
  LbRGB.fillColor(CRGB::Black);       // black setting for all lights
  LbRGB.show();                       // light status update
}
 
void ledBlink() {
  LbRGB.fillColor(CRGB::Yellow);      // yellow setting for all lights
  LbRGB.show();                       // light status update
  delay(20);
  LbRGB.fillColor(CRGB::Black);
  LbRGB.show();
  delay(20);
}
 
 
bool check_touch() {
  static bool retValue = false;
 
  if (LbTouch.onPress(TB2A)) {
    LbMotion.runLR(0,0);
    Leanbot.tone(BEEP_FREQ, 1000);
    ledOff();
 
    retValue = false;
  } else if (LbTouch.onPress(TB2B)) {
    Leanbot.tone(BEEP_FREQ, 50);
    retValue = true;
  }
 
  return retValue;
}
 
bool check_distance() {
  bool retValue;
  int d = Leanbot.pingCm();         // Measure the distance to the obstacle
  if ((d > 0) && (d < 20)) {        // If the distance to the obstacle is greater than 0cm and less than 20cm
    Serial.print("d = ");
    Serial.println(d);
 
    Leanbot_Stop();
    retValue = false;
  } else {
    retValue = true;
  }
  return retValue;
}
 
 
boolean followLineCarefully() {          // Follow the black line, if you don't see the black line, read it several more times to be sure
  for (int i = 0; i < 10; i++) {         // go up to 10*1mm = 10mm when the black line is not visible
    if ( followLineBasic() ) {
      return true;
    }
    LbMotion.waitDistanceMm(1);          // go 1mm and then read the sensor again
  }
  return false;
}
 
boolean followLineBasic() {              //
  int lineValue = LbIRLine.read();       // Read the value of 4 bar sensors
  LbIRLine.displayOnRGB(CRGB::DarkCyan); // Display the results on RGB Led for observation
 
  switch (lineValue) {                   // check the position of the black line relative to Leanbot
    case 0b0100:                         // .o.. - if the black line deviates to the left
    case 0b1110:                         // ooo.
      LbMotion.runLR(0, +speed);         //        turn left
      break;
 
    case 0b1100:                         // oo..
    case 0b1000:                         // o...
      LbMotion.runLR(-speed, +speed);    //        spin left
      break;
 
    case 0b0010:                         // ..o. - if the black line deviates to the right
    case 0b0111:                         // .ooo
      LbMotion.runLR(+speed, 0);         //        turn right
      break;
 
    case 0b0011:                         // ..oo
    case 0b0001:                         // ...o
      LbMotion.runLR(+speed, -speed);    //        spin right
      break;
 
    case 0b1111:
      Leanbot_Stop();
    break;
 
    default:                             // all other cases
      LbMotion.runLR(+speed, +speed);    //        go straight
  }
  return ( lineValue != 0b0000 );
}
 
void Leanbot_Stop() {
  LbMotion.runLR(0,0);
  Leanbot.tone(BEEP_FREQ, 50);
  delay(100);
  Leanbot.tone(BEEP_FREQ, 50);
  ledBlink();
}
 
void play_melody() {
  if (rtttl::done()) {
    rtttl::begin(LBPIN_BUZZER, mario);
  } else {
    rtttl::play();
  }
}
