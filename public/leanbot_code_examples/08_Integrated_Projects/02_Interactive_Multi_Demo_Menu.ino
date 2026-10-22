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
 
int SPEED_MAX  = 2000;                        // running at a maximum speed of 2000
int speed      = 1000;                        // run at 50% speed
 
 
void setup() {
  Leanbot.begin();                            // initialize Leanbot
  LbIRLine.doManualCalibration(TB1A);         // touch TB1A to measure black/white level, align bar sensor
//LbIRLine.setThreshold(50, 50, 50, 50);      // or measure first and then edit the threshold in the code
 
  Serial.println();
  Serial.println(F("#######################################"));
  Serial.println(F("#            Leanbot demo             #"));
  Serial.println(F("#######################################"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#        *------*      *------*       #"));
  Serial.println(F("#        | TB1A |      | TB1B |       #"));
  Serial.println(F("#        *------*      *------*       #"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#        *------*      *------*       #"));
  Serial.println(F("#        | TB2A |      | TB2B |       #"));
  Serial.println(F("#        *------*      *------*       #"));
  Serial.println(F("#                                     #"));
  Serial.println(F("#######################################"));
  Serial.println();
  Serial.println(F("Touch any key to continue..."));
  Serial.println();
  while (LbTouch.readBits() == 0);
  beepSingle();
}
 
void loop() {
  int demo = selectDemo();
  runDemo(demo);
}
 
 
void printDemo(int demo) {
  switch (demo) {
    case 0:
      Serial.println(F("  [1]  Motor demo"));
      ledIndicator(1);
      break;
 
    case 1:
      Serial.println(F("  [2]  Ping demo"));
      ledIndicator(2);
      break;
 
    case 2:
      Serial.println(F("  [3]  Line following demo"));
      ledIndicator(3);
      break;
  }
}
 
#define DEMO_COUNT  3
 
int selectDemo() {
  Serial.println();
  Serial.println(F("#######################################"));
  Serial.println(F("#              Select demo            #"));
  Serial.println(F("#######################################"));
  Serial.println(F("[TB2B] : Select"));
  Serial.println(F("[TB2A] : Enter"));
  Serial.println();
 
  int demo = -1;
 
  while (1) {
    if (LbTouch.onPress(TB2B)) {
      beepSingle();
      demo = (demo + 1) % DEMO_COUNT;
      printDemo(demo);
    }
 
    if ( (demo >= 0) && (LbTouch.onPress(TB2A)) ) {
      beepDoudle();
      ledOff();
      break;
    }
  }
 
  return demo;
}
 
 
void runDemo(int demo) {
  switch (demo) {
    case 0:  demo_motor(); break;
    case 1:  demo_srf04(); break;
    case 2:  demo_line();  break;
  }
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
    case 0b1100:                         // oo..
    case 0b1000:                         // o...
      LbMotion.runLR(0, +speed);         //        turn left
      break;
 
    case 0b0010:                         // ..o. - if the black line deviates to the left right
    case 0b0111:                         // .ooo
    case 0b0011:                         // ..oo
    case 0b0001:                         // ...o
      LbMotion.runLR(+speed, 0);         //        turn right
      break;
      
    case 0b1111:
      Leanbot_Stop();
      ringTone_stop();
      ledBlink();
      beepDoudle();      
    break;
    
    default:                             // all other cases
      LbMotion.runLR(+speed, +speed);    //       go straight
  }
  return ( lineValue != 0b0000 );
}
 
void demo_line() {
  Serial.println();
  Serial.println(F("#######################################"));
  Serial.println(F("#          Line following demo        #"));
  Serial.println(F("#######################################"));
  Serial.println(F("  [1]  TB1A : Play/Pause melody"));
  Serial.println(F("  [2]  TB2A : Run"));
  Serial.println(F("  [3]  TB2B : Stop/Exit"));
  Serial.println();
 
  int state = 0;
  bool playMelody = false;
  
  while (1) {
    if (LbTouch.onPress(TB1A)) {
      if (playMelody) {
        Serial.println(F("  Play melody"));
      } else {
        Serial.println(F("  Pause melody"));
        ringTone_stop();
      }
      beepSingle();
      playMelody = !playMelody;
    }
    else if (LbTouch.onPress(TB2A)) {
      Serial.println(F("  Run"));
      beepSingle();
      state = 1;
    }
    else if (LbTouch.onPress(TB2B)) {
      if (state != 0) {
        Serial.println(F("  Stop, touch again to exit"));
        beepSingle();
        Leanbot_Stop();
        ringTone_stop();
        state = 0;
        playMelody = false;
      } else {
        Serial.println(F("  Exit demo"));
        beepLong();
        return;
      }
    }
 
    if (state != 0) {
      followLineCarefully();
    }
 
    if (playMelody) {
      ringTone_play();
    }
 
  }
}
 
 
void demo_motor() {
  Serial.println();
  Serial.println(F("#######################################"));
  Serial.println(F("#              Motor demo             #"));
  Serial.println(F("#######################################"));
  Serial.println(F("  [1]  TB1A : Turn left"));
  Serial.println(F("  [2]  TB1B : Turn right"));
  Serial.println(F("  [3]  TB2A : Move forward/backward"));
  Serial.println(F("  [4]  TB2B : Stop/Exit"));
  Serial.println();
 
  int state = 1;  // 0=stop, 1=forward, 2=backward
 
  while (1) { 
    if (LbTouch.onPress(TB1A)) {
      Serial.println(F("  Turn left"));
      beepSingle();
      Leanbot_TurnLeft();
      state = 1;
    }
 
    else if (LbTouch.onPress(TB1B)) {
      Serial.println(F("  Turn right"));
      beepSingle();
      Leanbot_TurnRight();
      state = 1;
    }
 
    else if (LbTouch.onPress(TB2A)) {
      beepSingle();
      if (state == 1) {
        Serial.println(F("  Move forward"));
        Leanbot_MoveForward();
        state = 2;
      } else {
        Serial.println(F("  Move backward"));
        Leanbot_MoveBackward();
        state = 1;
      }
    }
 
    else if (LbTouch.onPress(TB2B)) {
      if (state != 0) {
        Serial.println(F("  Stop, touch again to exit"));
        beepSingle();
        Leanbot_Stop();
        state = 0;
      } else {
        Serial.println(F("  Exit demo"));
        beepLong();
        return;
      }
    }
  }
}
 
 
void demo_srf04() {
  Serial.println();
  Serial.println(F("#######################################"));
  Serial.println(F("#              Ping demo             #"));
  Serial.println(F("#######################################"));
  Serial.println(F("  [1]  TB1A : Colour fading"));
  Serial.println(F("  [2]  TB1B : Avoid object"));
  Serial.println(F("  [3]  TB2A : Follow object"));
  Serial.println(F("  [4]  TB2B : Stop/Exit"));
  Serial.println();
 
  int state  = 0;
  int limit  = 20;
  int offset = 1;
 
  while (1) {
    if (LbTouch.onPress(TB1A)) {
      Serial.println(F("  Colour fading"));
      beepSingle();
      Leanbot_Stop();
      state = 1;
    }
    else if (LbTouch.onPress(TB1B)) {
      Serial.println(F("  Avoid object"));
      beepSingle();
      ledOff();
      state = 2;
    }
    else if (LbTouch.onPress(TB2A)) {
      Serial.println(F("  Follow object"));
      beepSingle();
      ledOff();
      state = 3;
    }
    else if (LbTouch.onPress(TB2B)) {
      if (state != 0) {
        Serial.println(F("  Stop, touch again to exit"));
        beepSingle();
        ledOff();
        Leanbot_Stop();
        state = 0;
      } else {
        Serial.println("  Exit demo");
        beepLong();
        return;
      }
    }
 
    int d = Leanbot.pingCm();
    //Serial.println(d);
 
    switch (state) {
      case 1: {
        d = min(d, 50);
        byte c = map(d, 0, 50, 0, 255);  // note: declare variable inside SwitchCase
        LbRGB.fillColor(CRGB(255 - c, c, 0) );
        LbRGB.show(); 
        break;
      }
 
      case 2:
        if ( (d == 0) || (d >= limit) ) {
          Leanbot_Stop();
        } else {
          Leanbot_Spin();
        }
        break;
 
      case 3:
        if ( (d == 0) || (d > 100) ) {
          Leanbot_Stop();
        } else if (d > (limit + offset)) {
          Leanbot_MoveForward();
        } else if (d < (limit - offset)) {
          Leanbot_MoveBackward();
        }
        break;
    } // end switch
 
    delay(50);   // Wait between pings (about 20 pings/sec)
  } // end loop
 
} // end func
 
 
/*******************************************************************************
Leanbot_Beep
*******************************************************************************/
 
#define BEEP_FREQ   1500
 
void beepSingle() {
  Leanbot.tone(BEEP_FREQ, 50);            // play sound at 1500Hz for 50ms
}
 
void beepDoudle() {
  Leanbot.tone(BEEP_FREQ, 50); 
  delay(100);
  Leanbot.tone(BEEP_FREQ, 50); 
}
 
void beepLong() {
  Leanbot.tone(BEEP_FREQ, 800); 
  delay(800);
}
 
void ringTone_play() {
  if (rtttl::done()) {
    rtttl::begin(LBPIN_BUZZER, mario);
  } else {
    rtttl::play();
  }
}
 
void ringTone_stop() {
  rtttl::stop();
}
 
 
/*******************************************************************************
Leanbot_RGB
*******************************************************************************/
 
void ledOn() {
  LbRGB.fillColor(CRGB::White);       // white setting for all lights
  LbRGB.show();                       // light status update
}
 
void ledOff() {
  LbRGB.fillColor(CRGB::Black);       // black setting for all lights
  LbRGB.show();                       // light status update
}
 
void ledBlink() {
  LbRGB.fillColor(CRGB::Yellow);
  LbRGB.show();                       // light status update
  delay(20);
  LbRGB.fillColor(CRGB::Black);       // black setting for all lights
  LbRGB.show();  
  delay(20);
}
 
void ledIndicator(int number) {
  for (int i = 1; i <= number; i++) {
    LbRGB.led[i] = CRGB::White;
  }
  for (int i = (number + 1); i < 7; i++) {
    LbRGB.led[i] = CRGB::Black;
  }
  LbRGB.show();
}
 
 
/*******************************************************************************
Leanbot_Motion
*******************************************************************************/
 
void Leanbot_Stop() {
  LbMotion.runLR(0, 0);
}
 
void Leanbot_MoveForward() {
  LbMotion.runLR(SPEED_MAX, SPEED_MAX);
}
 
void Leanbot_MoveBackward() {
  LbMotion.runLR(-SPEED_MAX, -SPEED_MAX);
}
 
void Leanbot_TurnLeft() {
  LbMotion.runLR(speed, SPEED_MAX);
}
 
void Leanbot_TurnRight() {
  LbMotion.runLR(SPEED_MAX, speed);
}
 
void Leanbot_Spin() {
  LbMotion.runLR(SPEED_MAX, -SPEED_MAX);
}
