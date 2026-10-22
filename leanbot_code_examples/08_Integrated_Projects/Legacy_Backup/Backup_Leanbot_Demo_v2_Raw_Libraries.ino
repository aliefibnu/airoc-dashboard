#include <Leanbot.h>
 
#define FASTLED_INTERNAL
#include <FastLED.h>
CRGB led[7];
 
#include <NewPing.h>
NewPing sonar(LBPIN_SRF04, LBPIN_SRF04, 300);
 
#include <NonBlockingRtttl.h>
const char *mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a5,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a5,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6,p,p,p,p";
 
 
int SPEED_MAX  = 2000;             // running at a maximum speed of 2000
int SPEED_4    = 0.8 * SPEED_MAX;  // run at 80% speed
int SPEED_3    = 0.6 * SPEED_MAX;  // run at 60% speed
int SPEED_2    = 0.4 * SPEED_MAX;  // run at 40% speed
int SPEED_1    = 0.2 * SPEED_MAX;  // run at 20% speed
int SPEED_0    = 0.0 * SPEED_MAX;  // stop
 
 
void setup() {
  LbIRArray.read(IRPair1);
 
  MotorL.run(0);
  MotorR.run(0);
 
  FastLED.addLeds<WS2812B, LBPIN_WS2812B, GRB>(led, 7);
  FastLED.setBrightness(20);
  ledOff();
 
  Serial.begin(115200);
 
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
 
 
void printDemo(int demo) {
  switch (demo) {
    case 0:
      Serial.println(F("  [1]  Motor demo"));
      ledIndicator(1);
      break;
 
    case 1:
      Serial.println(F("  [2]  SRF04 demo"));
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
 
 
void line_following() {
  int line = LbIRArray.readLine(250);    // reads 4 bar sensors with a threshold value of 250
//LbIRArray.printLine(line);             // transfer the results to the computer
 
  switch (line) {
    case 0b0000:
    case 0b0110:
      Leanbot_MoveForward();
      break;
 
 
    case 0b0010:
      MotorL.run(SPEED_MAX);
      MotorR.run(SPEED_3);
      break;
 
    case 0b0011:
      MotorL.run(SPEED_MAX);
      MotorR.run(SPEED_2);
      break;
 
    case 0b0001:
      MotorL.run(SPEED_MAX);
      MotorR.run(SPEED_0);
      break;
 
 
    case 0b0100:
      MotorL.run(SPEED_3);
      MotorR.run(SPEED_MAX);
      break;
 
    case 0b1100:
      MotorL.run(SPEED_2);
      MotorR.run(SPEED_MAX);
      break;
 
    case 0b1000:
      MotorL.run(SPEED_0);
      MotorR.run(SPEED_MAX);
      break;
 
 
    case 0b1111:
      Leanbot_Stop();
      ringTone_stop();
      ledBlink();
      beepDoudle();
      break;
  }
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
      line_following();
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
  Serial.println(F("#              SRF04 demo             #"));
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
 
    int d = sonar.ping_cm();
    //Serial.println(d);
 
    switch (state) {
      case 1: {
        d = min(d, 50);
        byte c = map(d, 0, 50, 0, 255);  // note: declare variable inside SwitchCase
        ledShow( CRGB(255 - c, c, 0) );
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
 
 
void loop() {
  int demo = selectDemo();
  runDemo(demo);
}
 
 
/*******************************************************************************
Leanbot_Beep
*******************************************************************************/
 
#define BEEP_FREQ   500
 
void beepSingle() {
  tone(LBPIN_BUZZER, BEEP_FREQ, 50);
}
 
void beepDoudle() {
  tone(LBPIN_BUZZER, BEEP_FREQ, 50);
  delay(100);
  tone(LBPIN_BUZZER, BEEP_FREQ, 50);
}
 
void beepLong() {
  tone(LBPIN_BUZZER, BEEP_FREQ, 800);
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
 
void ledShow(CRGB colour) {
  for (int i = 0; i < 7; i++) {
    led[i] = colour;
  }
  FastLED.show();
}
 
void ledOn() {
  ledShow(CRGB::Green);
}
 
void ledOff() {
  ledShow(CRGB::Black);
}
 
void ledBlink() {
  ledShow(CRGB::Yellow);
  delay(20);
  ledShow(CRGB::Black);
  delay(20);
}
 
void ledIndicator(int number) {
  for (int i = 1; i <= number; i++) {
    led[i] = CRGB::White;
  }
  for (int i = (number + 1); i < 7; i++) {
    led[i] = CRGB::Black;
  }
  FastLED.show();
}
 
 
/*******************************************************************************
Leanbot_Stepper
*******************************************************************************/
 
void Leanbot_Stop() {
  MotorL.stop();
  MotorR.stop();
}
 
void Leanbot_MoveForward() {
  MotorL.run(SPEED_MAX);
  MotorR.run(SPEED_MAX);
}
 
void Leanbot_MoveBackward() {
  MotorL.run(-SPEED_MAX);
  MotorR.run(-SPEED_MAX);
}
 
void Leanbot_TurnLeft() {
  MotorL.run(SPEED_MAX / 2);
  MotorR.run(SPEED_MAX);
}
 
void Leanbot_TurnRight() {
  MotorL.run(SPEED_MAX);
  MotorR.run(SPEED_MAX / 2);
}
 
void Leanbot_Spin() {
  MotorL.run(+SPEED_MAX);
  MotorR.run(-SPEED_MAX);
}
