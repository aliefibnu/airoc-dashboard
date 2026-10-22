#define  LEANBOT_SHOW_VERSION
#include <Leanbot.h>
 
 
void setup() {
  Leanbot.begin();                    // initialize Leanbot
}
 
 
void serial_checkCommand() {
  if (Serial.available() <= 0) return;
 
  int cmd = Serial.read();
 
  static int lastCmd = 0;
  if (lastCmd == cmd) return;
  lastCmd = cmd;
 
  switch (cmd) {
    case 'F':  Leanbot_setSpeed(+4, +4);        break;    // Forward
    case 'B':  Leanbot_setSpeed(-4, -4);        break;    // Backward
    case 'L':  Leanbot_setSpeed(-4, +4);        break;    // Left
    case 'R':  Leanbot_setSpeed(+4, -4);        break;    // Right
 
    case 'G':  Leanbot_setSpeed(+2, +4);        break;    // Forward Left
    case 'I':  Leanbot_setSpeed(+4, +2);        break;    // Forward Right
    case 'H':  Leanbot_setSpeed(-2, -4);        break;    // Back Left
    case 'J':  Leanbot_setSpeed(-4, -2);        break;    // Back Right
 
    case 'S':  Leanbot_setSpeed( 0,  0);        break;    // Stop
 
    case 'q':  Leanbot_SetVelocity(10);         break;
    case '0' ... '9':
               Leanbot_SetVelocity(cmd - '0');  break;
 
    case 'W':  ledOn();                         break;
    case 'w':  ledOff();                        break;
 
    case 'U':  ;                                break;    // back light on
    case 'u':  ;                                break;    // back light off
 
    case 'V':  hornOn();                        break;
    case 'v':  hornOff();                       break;
 
    case 'X':  LbGripper.close();               break;
    case 'x':  LbGripper.open();                break;
  }
}
 
void lbTouch_check() {
  if (LbTouch.onPress(TB1A)) {
    if (LbMotion.isMoving())      Leanbot_setSpeed(0, 0);
    else {
      delay(100);
      if (LbTouch.onPress(TB1B))  Leanbot_setSpeed(+2, +2);
      else                        Leanbot_setSpeed(+0, +2);
    }
    beepSingle();
    delay(50);
    return;
  }
 
  if (LbTouch.onPress(TB1B)) {
    if (LbMotion.isMoving())      Leanbot_setSpeed(0, 0);
    else {
      delay(100);
      if (LbTouch.onPress(TB1A))  Leanbot_setSpeed(+2, +2);
      else                        Leanbot_setSpeed(+2, +0);
    }
    beepSingle();
    delay(50);
    return;
  }
 
  if (LbTouch.read(TB2A) && LbTouch.read(TB2B)) {
    beepSingle();
    LbGripper_Toggle();
  }
}
 
 
void loop() {
  delay(1);
  serial_checkCommand();
  lbTouch_check();
}
 
 
/*******************************************************************************
Leanbot_Beep
*******************************************************************************/
 
#define BEEP_FREQ   1500
 
void hornOn() {
  Leanbot.tone(BEEP_FREQ, 5000);      // play sound at 1500Hz for 5000ms
}
 
void hornOff() {
  Leanbot.noTone();
}
 
void beepSingle() {
  Leanbot.tone(BEEP_FREQ, 50);        // play sound at 1500Hz for 50ms
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
 
 
/*******************************************************************************
Leanbot_Motion
*******************************************************************************/
 
const int V_MAX = 2000;
const int V_MIN = 350;
 
int velocity  = V_MAX / 1;
int motorLDir = 0;
int motorRDir = 0;
 
void Leanbot_run() {
  LbMotion.runLR(velocity * motorLDir / 4, velocity * motorRDir / 4);
}
 
void Leanbot_setSpeed(int speedL, int speedR) {
  motorLDir = speedL;
  motorRDir = speedR;
  Leanbot_run();
}
 
void Leanbot_SetVelocity(int speed) {
  velocity = map(speed, 0, 10, V_MIN, V_MAX);
  Leanbot_run();
}
 
 
/*******************************************************************************
Leanbot_Gripper
*******************************************************************************/
 
void LbGripper_Toggle() {
  static bool GripperIsClose = false;
 
  if (GripperIsClose) {
    LbGripper.open();
    GripperIsClose = false;
  } else {
    LbGripper.close();
    GripperIsClose = true;
  }
}
