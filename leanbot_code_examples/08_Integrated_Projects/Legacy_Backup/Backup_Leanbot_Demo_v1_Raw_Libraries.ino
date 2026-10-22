#include <Leanbot.h>
 
int SPEED_MAX  = 2000;             // running at a maximum speed of 2000
int SPEED_4    = 0.8 * SPEED_MAX;  // run at 80% speed
int SPEED_3    = 0.6 * SPEED_MAX;  // run at 60% speed
int SPEED_2    = 0.4 * SPEED_MAX;  // run at 40% speed
int SPEED_1    = 0.2 * SPEED_MAX;  // run at 20% speed
int SPEED_0    = 0.0 * SPEED_MAX;  // stop
 
#define FASTLED_INTERNAL
#include <FastLED.h>
CRGB led[7];             // Declare array of RGB LED
 
#include <NewPing.h>
NewPing sonar(LBPIN_SRF04, LBPIN_SRF04, 300);   // Connection settings to the distance sensor
 
#include <NonBlockingRtttl.h>
const char *mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a5,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a5,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6,p,p,p,p";
 
 
void setup() {
  LbIRArray.read(IRPair1);
 
  MotorL.run(0);
  MotorR.run(0);
 
  FastLED.addLeds<WS2812B, LBPIN_WS2812B, GRB>(led, 7);
  FastLED.setBrightness(20);
 
  Serial.begin(115200);
}
 
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
 
 
bool check_touch() {
  static bool retValue = false;
 
  if (LbTouch.onPress(TB2A)) {
    MotorL.stop();
    MotorR.stop();
 
    tone(LBPIN_BUZZER, 1500, 1000);
    ledOff();
 
    retValue = false;
  } else if (LbTouch.onPress(TB2B)) {
    tone(LBPIN_BUZZER, 1500, 50);
    ledOn();
    retValue = true;
  }
 
  return retValue;
}
 
bool check_distance() {
  bool retValue;
  int d = sonar.ping_cm();          // Measure the distance to the obstacle
  if (d < 20) {                     // If the distance to the obstacle is less than 20cm
    MotorL.run(SPEED_0);
    MotorR.run(SPEED_0);
 
    tone(LBPIN_BUZZER, 1500, 50);
    delay(100);
    tone(LBPIN_BUZZER, 1500, 50);
 
    ledBlink();
 
    retValue = false;
  } else {
    ledOn();
    retValue = true;
  }
  return retValue;
}
 
 
bool check_line() {
  bool retValue = true;
 
  int line = LbIRArray.readLine(250);    // reads 4 bar sensors with a threshold value of 250
  LbIRArray.printLine(line);             // transfer the results to the computer
 
  switch (line) {                        // check the location of Leanbot
    case 0b0000:
    case 0b0110:
      MotorL.run(SPEED_MAX);
      MotorR.run(SPEED_MAX);
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
      MotorL.run(SPEED_0);
      MotorR.run(SPEED_0);
 
      tone(LBPIN_BUZZER, 1500, 50);
      delay(100);
      tone(LBPIN_BUZZER, 1500, 50);
 
      ledBlink();
 
      retValue = false;
      break;
  }
 
  return retValue;
}
 
void play_melody() {
  if (rtttl::done()) {
    rtttl::begin(LBPIN_BUZZER, mario);
  } else {
    rtttl::play();
  }
}
 
 
void loop() {
  if (! check_touch()) {
    return;
  }
 
  if (! check_distance()) {
    return;
  }
 
  if (! check_line()) {
    return;
  }
 
  play_melody();
}
