#include <Leanbot.h>
#include <avr/pgmspace.h>

enum Opcode : uint8_t {
  OP_NONE = 0,
  OP_MAJU,
  OP_MUNDUR,
  OP_KANAN,
  OP_KIRI,
  OP_BELOK_KANAN,
  OP_BELOK_KIRI,
  OP_STOP,
  OP_SPEED,
  OP_TUNGGU,
  OP_CAPIT,
  OP_LEPAS,
  OP_BEEP,
  OP_LED,
  OP_LED_WARNA,
  OP_LED_KEDIP,
  OP_EMERGENCY_STOP,
  OP_FOLLOW_LINE,
  OP_MAJU_LINE,
  OP_MUNDUR_LINE,
  OP_STOP_LINE,
  OP_KALIBRASI,
  OP_CONFIG
};

enum ConfigParam : uint8_t {
  CFG_SPEED = 1,
  CFG_MAJU,
  CFG_MUNDUR,
  CFG_KANAN,
  CFG_KIRI,
  CFG_BELOK_KANAN,
  CFG_BELOK_KIRI,
  CFG_FOLLOW_LINE,
  CFG_MAJU_LINE,
  CFG_MUNDUR_LINE,
  CFG_TUNGGU,
  CFG_BEEP
};

enum RobotState : uint8_t {
  STATE_READY = 0,
  STATE_RECEIVING,
  STATE_EXECUTING,
  STATE_EMERGENCY
};

struct Instruction {
  uint8_t opcode;
  int16_t p1;
  int16_t p2;
};

#define MAX_COMMANDS 48
Instruction commandQueue[MAX_COMMANDS];
uint8_t commandCount = 0;

int16_t currentSpeed = 1000;
volatile bool emergencyFlag = false;
RobotState currentRobotState = STATE_READY;

int16_t defaultSpeed = 5;
int16_t defaultMaju = 10;
int16_t defaultMundur = 10;
int16_t defaultKanan = 90;
int16_t defaultKiri = 90;
int16_t defaultBelokKanan = 90;
int16_t defaultBelokKiri = 90;
int16_t defaultFollowLine = 1;
int16_t defaultMajuLine = 50;
int16_t defaultMundurLine = 50;
int16_t defaultTunggu = 1000;
int16_t defaultBeepFreq = 1500;
int16_t defaultBeepDur = 100;

char tokenBuf[48];
uint8_t tokenLen = 0;
char prefixBuf[8];
uint8_t prefixLen = 0;
char emergencyRxBuf[16];
uint8_t emergencyRxLen = 0;
bool inPacket = false;
unsigned long lastRxTime = 0;

int freeRam();
int16_t toMotorSpeed(int16_t spd);
bool isLineDetected(int line);
void updateRobotState(RobotState newState);
void printOpcodeName(uint8_t op);
void printInstructionInfo(const Instruction& inst);
bool enqueueToken();
void triggerEmergencyStop();
bool checkEmergency();
void delayInterruptible(unsigned long ms);
void moveMm(int mm, int dir, int speed);
void rotateDeg(int deg, int dir, int speed);
void pivotTurnDeg(int deg, int dir, int speed);
void moveToLine(int dir, int maxDistCm, int speed);
void followLineUntilIntersection(int targetIntersections, int speed);
void playBeep(int freq, int duration);
void blinkLed(int count, int intervalMs);
uint8_t parseColorIndex(const char* s);
CRGB getColorFromIndex(uint8_t idx);
bool parseInstruction(char* str, Instruction& inst);
void executeInstruction(const Instruction& inst);
void executeCommandQueue();
void processCodePacket(char* payload);

int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

int16_t toMotorSpeed(int16_t spd) {
  if (spd <= 0) return currentSpeed > 0 ? currentSpeed : 1000;
  if (spd <= 10) return map(spd, 1, 10, 400, 2000);
  if (spd <= 255) return map(spd, 0, 255, 350, 2000);
  return constrain(spd, 350, 2000);
}

bool isLineDetected(int line) {
  uint8_t bits = line & 0b1111;
  if ((bits & 0b0110) == 0b0110) return true;
  uint8_t count = (bits & 1) + ((bits >> 1) & 1) + ((bits >> 2) & 1) + ((bits >> 3) & 1);
  return count >= 3;
}

void updateRobotState(RobotState newState) {
  if (currentRobotState == newState) return;
  currentRobotState = newState;
  Serial.print(F("[STATE] "));
  switch (currentRobotState) {
    case STATE_READY:
      Serial.println(F("READY"));
      break;
    case STATE_RECEIVING:
      Serial.println(F("RECV"));
      break;
    case STATE_EXECUTING:
      Serial.println(F("EXEC"));
      break;
    case STATE_EMERGENCY:
      Serial.println(F("EMG"));
      break;
  }
}

void printOpcodeName(uint8_t op) {
  switch (op) {
    case OP_MAJU: Serial.print(F("MAJU")); break;
    case OP_MUNDUR: Serial.print(F("MUNDUR")); break;
    case OP_KANAN: Serial.print(F("KANAN")); break;
    case OP_KIRI: Serial.print(F("KIRI")); break;
    case OP_BELOK_KANAN: Serial.print(F("BELOK_KANAN")); break;
    case OP_BELOK_KIRI: Serial.print(F("BELOK_KIRI")); break;
    case OP_STOP: Serial.print(F("STOP")); break;
    case OP_SPEED: Serial.print(F("SPEED")); break;
    case OP_TUNGGU: Serial.print(F("TUNGGU")); break;
    case OP_CAPIT: Serial.print(F("CAPIT")); break;
    case OP_LEPAS: Serial.print(F("LEPAS")); break;
    case OP_BEEP: Serial.print(F("BEEP")); break;
    case OP_LED: Serial.print(F("LED")); break;
    case OP_LED_WARNA: Serial.print(F("LED_WARNA")); break;
    case OP_LED_KEDIP: Serial.print(F("LED_KEDIP")); break;
    case OP_EMERGENCY_STOP: Serial.print(F("EMG_STOP")); break;
    case OP_FOLLOW_LINE: Serial.print(F("FOLLOW_LINE")); break;
    case OP_MAJU_LINE: Serial.print(F("MAJU_LINE")); break;
    case OP_MUNDUR_LINE: Serial.print(F("MUNDUR_LINE")); break;
    case OP_STOP_LINE: Serial.print(F("STOP_LINE")); break;
    case OP_KALIBRASI: Serial.print(F("KALIBRASI")); break;
    case OP_CONFIG: Serial.print(F("CONFIG")); break;
    default: Serial.print(op); break;
  }
}

void printInstructionInfo(const Instruction& inst) {
  printOpcodeName(inst.opcode);
  if (inst.p1 != 0 || inst.p2 != 0) {
    Serial.print(':');
    Serial.print(inst.p1);
    if (inst.p2 != 0) {
      Serial.print(',');
      Serial.print(inst.p2);
    }
  }
}

bool enqueueToken() {
  if (tokenLen == 0) return false;
  tokenBuf[tokenLen] = '\0';
  Instruction inst;
  bool ok = parseInstruction(tokenBuf, inst);
  Serial.print(F("[RECV] "));
  Serial.print(tokenBuf);
  if (ok && commandCount < MAX_COMMANDS) {
    commandQueue[commandCount++] = inst;
    Serial.print(F(" (#"));
    Serial.print(commandCount);
    Serial.println(')');
  } else {
    Serial.println(F(" ERR"));
  }
  tokenLen = 0;
  tokenBuf[0] = '\0';
  return ok;
}

void setup() {
  Leanbot.begin();
  LbIRLine.doManualCalibration(TB1A);
  delay(200);
  LbRGB.fillColor(CRGB::Black);
  LbRGB.show();
  Leanbot.tone(1500, 100);
  while (Serial.available() > 0) Serial.read();
  inPacket = false;
  tokenLen = 0;
  prefixLen = 0;
  emergencyRxLen = 0;
  tokenBuf[0] = '\0';
  prefixBuf[0] = '\0';
  emergencyRxBuf[0] = '\0';
  Serial.print(F("FREE_RAM:"));
  Serial.println(freeRam());
  updateRobotState(STATE_READY);
}

void triggerEmergencyStop() {
  emergencyFlag = true;
  updateRobotState(STATE_EMERGENCY);
  LbMotion.runLR(0, 0);
  LbMotion.stopAndWait();
  Leanbot.noTone();
  LbRGB.fillColor(CRGB::Red);
  LbRGB.show();
  commandCount = 0;
  inPacket = false;
  tokenLen = 0;
  prefixLen = 0;
  emergencyRxLen = 0;
  tokenBuf[0] = '\0';
  prefixBuf[0] = '\0';
  emergencyRxBuf[0] = '\0';
  while (Serial.available() > 0) Serial.read();
  Serial.println(F("EMERGENCY_STOP"));
}

bool checkEmergency() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c >= 'a' && c <= 'z') c -= 32;
    if ((c >= 'A' && c <= 'Z') || c == '_' || c == ';') {
      if (emergencyRxLen < sizeof(emergencyRxBuf) - 1) {
        emergencyRxBuf[emergencyRxLen++] = c;
        emergencyRxBuf[emergencyRxLen] = '\0';
      } else {
        memmove(emergencyRxBuf, emergencyRxBuf + 1, sizeof(emergencyRxBuf) - 2);
        emergencyRxBuf[sizeof(emergencyRxBuf) - 2] = c;
        emergencyRxBuf[sizeof(emergencyRxBuf) - 1] = '\0';
      }
      if (strstr_P(emergencyRxBuf, PSTR("STOP")) != NULL ||
          strstr_P(emergencyRxBuf, PSTR("EMERGENCY")) != NULL ||
          strstr_P(emergencyRxBuf, PSTR("END")) != NULL) {
        triggerEmergencyStop();
        return true;
      }
    } else if (c == '\n' || c == '\r') {
      emergencyRxLen = 0;
      emergencyRxBuf[0] = '\0';
    }
  }
  return emergencyFlag;
}

void loop() {
  if (checkEmergency()) return;

  while (Serial.available() > 0) {
    char c = Serial.read();
    lastRxTime = millis();

    if (!inPacket) {
      if (prefixLen < sizeof(prefixBuf) - 1) {
        prefixBuf[prefixLen++] = c;
        prefixBuf[prefixLen] = '\0';
      } else {
        memmove(prefixBuf, prefixBuf + 1, sizeof(prefixBuf) - 2);
        prefixBuf[sizeof(prefixBuf) - 2] = c;
        prefixBuf[sizeof(prefixBuf) - 1] = '\0';
      }

      if (strstr_P(prefixBuf, PSTR("STOP")) != NULL || strstr_P(prefixBuf, PSTR("EMG")) != NULL) {
        triggerEmergencyStop();
        return;
      }

      if (strstr_P(prefixBuf, PSTR("CODE;")) != NULL || strstr_P(prefixBuf, PSTR("code;")) != NULL) {
        inPacket = true;
        commandCount = 0;
        tokenLen = 0;
        prefixLen = 0;
        emergencyRxLen = 0;
        prefixBuf[0] = '\0';
        tokenBuf[0] = '\0';
        emergencyRxBuf[0] = '\0';
        updateRobotState(STATE_RECEIVING);
        Serial.println(F("[RECV] CODE;"));
      }
    } else {
      if (c == ';') {
        enqueueToken();
      } else if (c == '\n' || c == '\r') {
        enqueueToken();
        inPacket = false;
        prefixLen = 0;
        prefixBuf[0] = '\0';
        executeCommandQueue();
        return;
      } else {
        if (tokenLen < sizeof(tokenBuf) - 1) {
          tokenBuf[tokenLen++] = c;
          tokenBuf[tokenLen] = '\0';
        }
      }
    }
  }

  if (inPacket && (millis() - lastRxTime > 350)) {
    enqueueToken();
    inPacket = false;
    prefixLen = 0;
    prefixBuf[0] = '\0';
    executeCommandQueue();
  }
}

void delayInterruptible(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (checkEmergency()) return;
    delay(5);
  }
}

void moveMm(int mm, int dir, int speed) {
  if (emergencyFlag || mm <= 0) return;
  int spd = speed > 0 ? toMotorSpeed(speed) : (currentSpeed > 0 ? currentSpeed : 1000);
  LbMotion.runLR(dir * spd, dir * spd);
  int remaining = mm;
  while (remaining > 0 && !emergencyFlag) {
    int step = remaining > 10 ? 10 : remaining;
    LbMotion.waitDistanceMm(dir * step);
    remaining -= step;
    if (checkEmergency()) break;
  }
  LbMotion.stopAndWait();
}

void rotateDeg(int deg, int dir, int speed) {
  if (emergencyFlag || deg <= 0) return;
  int spd = speed > 0 ? toMotorSpeed(speed) : (currentSpeed > 0 ? currentSpeed : 1000);
  if (dir > 0) {
    LbMotion.runLR(+spd, -spd);
  } else {
    LbMotion.runLR(-spd, +spd);
  }
  int remaining = deg;
  while (remaining > 0 && !emergencyFlag) {
    int step = remaining > 10 ? 10 : remaining;
    LbMotion.waitRotationDeg(dir * step);
    remaining -= step;
    if (checkEmergency()) break;
  }
  LbMotion.stopAndWait();
}

void pivotTurnDeg(int deg, int dir, int speed) {
  if (emergencyFlag || deg <= 0) return;
  int spd = speed > 0 ? toMotorSpeed(speed) : (currentSpeed > 0 ? currentSpeed : 1000);
  if (dir > 0) {
    LbMotion.runLR(+spd, 0);
  } else {
    LbMotion.runLR(0, +spd);
  }
  int remaining = deg;
  while (remaining > 0 && !emergencyFlag) {
    int step = remaining > 10 ? 10 : remaining;
    LbMotion.waitRotationDeg(dir * step);
    remaining -= step;
    if (checkEmergency()) break;
  }
  LbMotion.stopAndWait();
}

void moveToLine(int dir, int maxDistCm, int speed) {
  if (emergencyFlag) return;
  int spd = speed > 0 ? toMotorSpeed(speed) : (currentSpeed > 0 ? currentSpeed : 500);
  int maxMm = maxDistCm > 0 ? maxDistCm * 10 : 500;
  int traveledMm = 0;
  bool found = false;

  if (isLineDetected(LbIRLine.read())) {
    LbRGB.fillColor(CRGB::Green);
    LbRGB.show();
    playBeep(1500, 100);
    return;
  }

  LbMotion.runLR(dir * spd, dir * spd);

  while (!emergencyFlag && traveledMm < maxMm) {
    LbMotion.waitDistanceMm(dir * 2);
    traveledMm += 2;
    if (checkEmergency()) break;

    int line = LbIRLine.read();
    LbIRLine.displayOnRGB(line, CRGB::Blue);

    if (isLineDetected(line)) {
      found = true;
      break;
    }
  }

  LbMotion.stopAndWait();

  if (!emergencyFlag) {
    LbRGB.fillColor(found ? CRGB::Green : CRGB::Red);
    LbRGB.show();
    playBeep(1500, 100);
  }
}

void followLineUntilIntersection(int targetIntersections, int speed) {
  if (emergencyFlag) return;
  if (targetIntersections <= 0) {
    LbMotion.runLR(0, 0);
    LbMotion.stopAndWait();
    return;
  }

  int spd = speed > 0 ? toMotorSpeed(speed) : (currentSpeed > 0 ? currentSpeed : 1000);
  if (spd < 350) spd = 350;
  if (spd > 1500) spd = 1500;

  int intersectionsCount = 0;
  unsigned long lastSeenLine = millis();

  int initLine = LbIRLine.read();
  if ((initLine == 0b1111) || ((initLine & 0b1001) == 0b1001)) {
    LbMotion.runLR(spd, spd);
    LbMotion.waitDistanceMm(25);
  }

  while (!emergencyFlag) {
    if (checkEmergency()) break;

    int lineValue = LbIRLine.read();
    LbIRLine.displayOnRGB(CRGB::DarkCyan);

    bool isIntersection = (lineValue == 0b1111) || ((lineValue & 0b1001) == 0b1001);

    if (isIntersection) {
      intersectionsCount++;
      if (intersectionsCount >= targetIntersections) break;
      LbMotion.runLR(spd, spd);
      LbMotion.waitDistanceMm(25);
      continue;
    }

    if (lineValue != 0b0000) lastSeenLine = millis();

    switch (lineValue) {
      case 0b0100:
      case 0b1110:
        LbMotion.runLR(0, +spd);
        break;

      case 0b1100:
      case 0b1000:
        LbMotion.runLR(-spd, +spd);
        break;

      case 0b0010:
      case 0b0111:
        LbMotion.runLR(+spd, 0);
        break;

      case 0b0011:
      case 0b0001:
        LbMotion.runLR(+spd, -spd);
        break;

      case 0b0000:
        if (millis() - lastSeenLine > 1500) {
          LbMotion.runLR(0, 0);
          LbMotion.stopAndWait();
          LbRGB.fillColor(CRGB::Red);
          LbRGB.show();
          playBeep(1500, 100);
          return;
        }
        break;

      default:
        LbMotion.runLR(+spd, +spd);
        break;
    }

    delay(2);
  }

  LbMotion.runLR(0, 0);
  LbMotion.stopAndWait();
  if (!emergencyFlag) {
    LbRGB.fillColor(CRGB::Green);
    LbRGB.show();
    playBeep(1500, 100);
  }
}

void playBeep(int freq, int duration) {
  if (emergencyFlag) return;
  if (freq <= 0) freq = 1500;
  if (duration <= 0) duration = 100;
  Leanbot.tone(freq, duration);
  delayInterruptible(duration);
}

void blinkLed(int count, int intervalMs) {
  if (emergencyFlag) return;
  if (count <= 0) count = 3;
  if (intervalMs <= 0) intervalMs = 200;
  for (int i = 0; i < count; i++) {
    LbRGB.fillColor(CRGB::Yellow);
    LbRGB.show();
    delayInterruptible(intervalMs);
    if (emergencyFlag) break;
    LbRGB.fillColor(CRGB::Black);
    LbRGB.show();
    delayInterruptible(intervalMs);
    if (emergencyFlag) break;
  }
}

uint8_t parseColorIndex(const char* s) {
  if (strcasecmp_P(s, PSTR("RED")) == 0 || strcasecmp_P(s, PSTR("MERAH")) == 0) return 1;
  if (strcasecmp_P(s, PSTR("GREEN")) == 0 || strcasecmp_P(s, PSTR("HIJAU")) == 0) return 2;
  if (strcasecmp_P(s, PSTR("BLUE")) == 0 || strcasecmp_P(s, PSTR("BIRU")) == 0) return 3;
  if (strcasecmp_P(s, PSTR("YELLOW")) == 0 || strcasecmp_P(s, PSTR("KUNING")) == 0) return 4;
  if (strcasecmp_P(s, PSTR("CYAN")) == 0) return 5;
  if (strcasecmp_P(s, PSTR("MAGENTA")) == 0) return 6;
  if (strcasecmp_P(s, PSTR("WHITE")) == 0 || strcasecmp_P(s, PSTR("PUTIH")) == 0) return 7;
  if (strcasecmp_P(s, PSTR("ORANGE")) == 0 || strcasecmp_P(s, PSTR("JINGGA")) == 0) return 8;
  if (strcasecmp_P(s, PSTR("PURPLE")) == 0 || strcasecmp_P(s, PSTR("UNGU")) == 0) return 9;
  if (strcasecmp_P(s, PSTR("BLACK")) == 0 || strcasecmp_P(s, PSTR("HITAM")) == 0 || strcasecmp_P(s, PSTR("OFF")) == 0) return 0;
  if (isdigit(s[0])) return (uint8_t)atoi(s);
  return 7;
}

CRGB getColorFromIndex(uint8_t idx) {
  switch (idx) {
    case 0: return CRGB::Black;
    case 1: return CRGB::Red;
    case 2: return CRGB::Green;
    case 3: return CRGB::Blue;
    case 4: return CRGB::Yellow;
    case 5: return CRGB::Cyan;
    case 6: return CRGB::Magenta;
    case 7: return CRGB::White;
    case 8: return CRGB::Orange;
    case 9: return CRGB::Purple;
    default: return CRGB::White;
  }
}

bool parseInstruction(char* str, Instruction& inst) {
  while (*str == ' ' || *str == '\t') str++;
  if (*str == '\0') return false;

  char* end = str + strlen(str) - 1;
  while (end > str && (*end == ' ' || *end == '\t' || *end == ';' || *end == '\r' || *end == '\n')) {
    *end = '\0';
    end--;
  }

  char* colon = strchr(str, ':');
  char* cmd = str;
  char* params = NULL;
  if (colon != NULL) {
    *colon = '\0';
    params = colon + 1;
    while (*params == ' ' || *params == '\t') params++;
  }

  char* endCmd = cmd + strlen(cmd) - 1;
  while (endCmd > cmd && (*endCmd == ' ' || *endCmd == '\t')) {
    *endCmd = '\0';
    endCmd--;
  }

  inst.opcode = OP_NONE;
  inst.p1 = 0;
  inst.p2 = 0;

  if (strcasecmp_P(cmd, PSTR("MAJU")) == 0) {
    inst.opcode = OP_MAJU;
    inst.p1 = defaultMaju;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("MUNDUR")) == 0) {
    inst.opcode = OP_MUNDUR;
    inst.p1 = defaultMundur;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("KANAN")) == 0 || strcasecmp_P(cmd, PSTR("ROTATE_KANAN")) == 0) {
    inst.opcode = OP_KANAN;
    inst.p1 = defaultKanan;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("KIRI")) == 0 || strcasecmp_P(cmd, PSTR("ROTATE_KIRI")) == 0) {
    inst.opcode = OP_KIRI;
    inst.p1 = defaultKiri;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("BELOK_KANAN")) == 0 || strcasecmp_P(cmd, PSTR("PIVOT_KANAN")) == 0) {
    inst.opcode = OP_BELOK_KANAN;
    inst.p1 = defaultBelokKanan;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("BELOK_KIRI")) == 0 || strcasecmp_P(cmd, PSTR("PIVOT_KIRI")) == 0) {
    inst.opcode = OP_BELOK_KIRI;
    inst.p1 = defaultBelokKiri;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("BELOK")) == 0) {
    inst.opcode = OP_BELOK_KANAN;
    inst.p1 = defaultBelokKanan;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("STOP")) == 0 || strcasecmp_P(cmd, PSTR("END")) == 0) {
    inst.opcode = OP_STOP;
  } else if (strcasecmp_P(cmd, PSTR("SPEED")) == 0) {
    inst.opcode = OP_SPEED;
    inst.p1 = defaultSpeed;
  } else if (strcasecmp_P(cmd, PSTR("TUNGGU")) == 0) {
    inst.opcode = OP_TUNGGU;
    inst.p1 = defaultTunggu;
  } else if (strcasecmp_P(cmd, PSTR("CAPIT")) == 0) {
    inst.opcode = OP_CAPIT;
  } else if (strcasecmp_P(cmd, PSTR("LEPAS")) == 0) {
    inst.opcode = OP_LEPAS;
  } else if (strcasecmp_P(cmd, PSTR("BEEP")) == 0) {
    inst.opcode = OP_BEEP;
    inst.p1 = defaultBeepFreq;
    inst.p2 = defaultBeepDur;
  } else if (strcasecmp_P(cmd, PSTR("LED")) == 0) {
    inst.opcode = OP_LED;
    inst.p1 = 1;
  } else if (strcasecmp_P(cmd, PSTR("LED_WARNA")) == 0) {
    inst.opcode = OP_LED_WARNA;
    inst.p1 = 7;
  } else if (strcasecmp_P(cmd, PSTR("LED_KEDIP")) == 0) {
    inst.opcode = OP_LED_KEDIP;
    inst.p1 = 3;
    inst.p2 = 200;
  } else if (strcasecmp_P(cmd, PSTR("EMERGENCY_STOP")) == 0 || strcasecmp_P(cmd, PSTR("EMERGENCY")) == 0) {
    inst.opcode = OP_EMERGENCY_STOP;
  } else if (strcasecmp_P(cmd, PSTR("FOLLOW_LINE")) == 0) {
    inst.opcode = OP_FOLLOW_LINE;
    inst.p1 = defaultFollowLine;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("MAJU_LINE")) == 0) {
    inst.opcode = OP_MAJU_LINE;
    inst.p1 = defaultMajuLine;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("MUNDUR_LINE")) == 0) {
    inst.opcode = OP_MUNDUR_LINE;
    inst.p1 = defaultMundurLine;
    inst.p2 = 0;
  } else if (strcasecmp_P(cmd, PSTR("STOP_LINE")) == 0) {
    inst.opcode = OP_STOP_LINE;
  } else if (strcasecmp_P(cmd, PSTR("KALIBRASI")) == 0 || strcasecmp_P(cmd, PSTR("CALIBRATE")) == 0) {
    inst.opcode = OP_KALIBRASI;
  } else if (strcasecmp_P(cmd, PSTR("CONFIG")) == 0) {
    inst.opcode = OP_CONFIG;
  } else {
    return false;
  }

  if (params != NULL && strlen(params) > 0) {
    if (strcasecmp_P(cmd, PSTR("BELOK")) == 0) {
      if (strncasecmp_P(params, PSTR("KANAN"), 5) == 0) {
        inst.opcode = OP_BELOK_KANAN;
        params += 5;
        if (*params == ',') params++;
      } else if (strncasecmp_P(params, PSTR("KIRI"), 4) == 0) {
        inst.opcode = OP_BELOK_KIRI;
        params += 4;
        if (*params == ',') params++;
      }
      while (*params == ' ' || *params == '\t') params++;
    }

    if (inst.opcode == OP_LED) {
      if (strcasecmp_P(params, PSTR("ON")) == 0 || strcmp_P(params, PSTR("1")) == 0) {
        inst.p1 = 1;
      } else if (strcasecmp_P(params, PSTR("OFF")) == 0 || strcmp_P(params, PSTR("0")) == 0 || strcasecmp_P(params, PSTR("CLEAR")) == 0) {
        inst.p1 = 0;
      } else if (strcasecmp_P(params, PSTR("SMILE")) == 0 || strcasecmp_P(params, PSTR("SMILEY")) == 0) {
        inst.p1 = 2;
      } else if (strcasecmp_P(params, PSTR("FROWN")) == 0 || strcasecmp_P(params, PSTR("FROWNING")) == 0) {
        inst.p1 = 3;
      } else if (strcasecmp_P(params, PSTR("LEFT")) == 0 || strcasecmp_P(params, PSTR("KIRI")) == 0) {
        inst.p1 = 4;
      } else if (strcasecmp_P(params, PSTR("RIGHT")) == 0 || strcasecmp_P(params, PSTR("KANAN")) == 0) {
        inst.p1 = 5;
      } else {
        inst.p1 = 10;
        inst.p2 = parseColorIndex(params);
      }
    } else if (inst.opcode == OP_LED_WARNA) {
      inst.p1 = parseColorIndex(params);
    } else if (inst.opcode == OP_FOLLOW_LINE) {
      char* comma = strchr(params, ',');
      if (comma != NULL) {
        *comma = '\0';
        char* p1Str = params;
        char* p2Str = comma + 1;
        while (*p2Str == ' ' || *p2Str == '\t') p2Str++;
        if (strcasecmp_P(p1Str, PSTR("OFF")) == 0 || strcmp_P(p1Str, PSTR("0")) == 0) inst.p1 = 0;
        else if (strcasecmp_P(p1Str, PSTR("ON")) == 0) inst.p1 = 9999;
        else inst.p1 = atoi(p1Str);
        inst.p2 = atoi(p2Str);
      } else {
        if (strcasecmp_P(params, PSTR("OFF")) == 0 || strcmp_P(params, PSTR("0")) == 0) inst.p1 = 0;
        else if (strcasecmp_P(params, PSTR("ON")) == 0) inst.p1 = 9999;
        else inst.p1 = atoi(params);
      }
    } else if (inst.opcode == OP_CONFIG) {
      char* comma = strchr(params, ',');
      if (comma != NULL) {
        *comma = '\0';
        char* key = params;
        char* val = comma + 1;
        while (*val == ' ' || *val == '\t') val++;
        int16_t numVal = atoi(val);
        if (strcasecmp_P(key, PSTR("SPEED")) == 0) { inst.p1 = CFG_SPEED; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("MAJU")) == 0) { inst.p1 = CFG_MAJU; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("MUNDUR")) == 0) { inst.p1 = CFG_MUNDUR; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("KANAN")) == 0) { inst.p1 = CFG_KANAN; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("KIRI")) == 0) { inst.p1 = CFG_KIRI; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("BELOK_KANAN")) == 0) { inst.p1 = CFG_BELOK_KANAN; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("BELOK_KIRI")) == 0) { inst.p1 = CFG_BELOK_KIRI; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("FOLLOW_LINE")) == 0) { inst.p1 = CFG_FOLLOW_LINE; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("MAJU_LINE")) == 0) { inst.p1 = CFG_MAJU_LINE; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("MUNDUR_LINE")) == 0) { inst.p1 = CFG_MUNDUR_LINE; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("TUNGGU")) == 0) { inst.p1 = CFG_TUNGGU; inst.p2 = numVal; }
        else if (strcasecmp_P(key, PSTR("BEEP")) == 0) { inst.p1 = CFG_BEEP; inst.p2 = numVal; }
      }
    } else {
      char* comma = strchr(params, ',');
      if (comma != NULL) {
        *comma = '\0';
        char* p1Str = params;
        char* p2Str = comma + 1;
        while (*p2Str == ' ' || *p2Str == '\t') p2Str++;
        inst.p1 = atoi(p1Str);
        inst.p2 = atoi(p2Str);
      } else {
        inst.p1 = atoi(params);
      }
    }
  }

  return true;
}

void executeInstruction(const Instruction& inst) {
  if (emergencyFlag && inst.opcode != OP_EMERGENCY_STOP) return;

  switch (inst.opcode) {
    case OP_MAJU:
      moveMm(inst.p1 * 10, +1, inst.p2);
      break;

    case OP_MUNDUR:
      moveMm(inst.p1 * 10, -1, inst.p2);
      break;

    case OP_KANAN:
      rotateDeg(inst.p1, +1, inst.p2);
      break;

    case OP_KIRI:
      rotateDeg(inst.p1, -1, inst.p2);
      break;

    case OP_BELOK_KANAN:
      pivotTurnDeg(inst.p1, +1, inst.p2);
      break;

    case OP_BELOK_KIRI:
      pivotTurnDeg(inst.p1, -1, inst.p2);
      break;

    case OP_STOP:
    case OP_STOP_LINE:
      LbMotion.runLR(0, 0);
      LbMotion.stopAndWait();
      break;

    case OP_SPEED:
      if (inst.p1 <= 0) {
        currentSpeed = 0;
      } else if (inst.p1 <= 10) {
        currentSpeed = map(inst.p1, 1, 10, 400, 2000);
      } else if (inst.p1 <= 255) {
        currentSpeed = map(inst.p1, 0, 255, 350, 2000);
      } else {
        currentSpeed = constrain(inst.p1, 350, 2000);
      }
      break;

    case OP_TUNGGU:
      delayInterruptible(inst.p1);
      break;

    case OP_CAPIT:
      LbGripper.close();
      delayInterruptible(400);
      break;

    case OP_LEPAS:
      LbGripper.open();
      delayInterruptible(400);
      break;

    case OP_BEEP:
      playBeep(inst.p1, inst.p2);
      break;

    case OP_LED:
      if (inst.p1 == 1) {
        LbRGB.fillColor(CRGB::White);
        LbRGB.show();
      } else if (inst.p1 == 0) {
        LbRGB.clear();
        LbRGB.show();
      } else if (inst.p1 == 2) {
        LbRGB.clear();
        LbRGB.fillColor(CRGB::Green, BITMAP(ledA, ledB, ledC, ledD));
        LbRGB.show();
      } else if (inst.p1 == 3) {
        LbRGB.clear();
        LbRGB.fillColor(CRGB::Yellow, BITMAP(ledA, ledF, ledE, ledD));
        LbRGB.show();
      } else if (inst.p1 == 4) {
        LbRGB.clear();
        LbRGB.fillColor(CRGB::Red, BITMAP(ledA, ledO, ledE, ledD, ledC));
        LbRGB.show();
      } else if (inst.p1 == 5) {
        LbRGB.clear();
        LbRGB.fillColor(CRGB::Green, BITMAP(ledD, ledO, ledF, ledA, ledB));
        LbRGB.show();
      } else if (inst.p1 == 10) {
        LbRGB.fillColor(getColorFromIndex(inst.p2));
        LbRGB.show();
      }
      break;

    case OP_LED_WARNA:
      LbRGB.fillColor(getColorFromIndex(inst.p1));
      LbRGB.show();
      break;

    case OP_LED_KEDIP:
      blinkLed(inst.p1, inst.p2);
      break;

    case OP_EMERGENCY_STOP:
      triggerEmergencyStop();
      break;

    case OP_FOLLOW_LINE:
      if (inst.p1 > 0) {
        followLineUntilIntersection(inst.p1, inst.p2);
      } else {
        LbMotion.runLR(0, 0);
        LbMotion.stopAndWait();
      }
      break;

    case OP_MAJU_LINE:
      moveToLine(+1, inst.p1, inst.p2);
      break;

    case OP_MUNDUR_LINE:
      moveToLine(-1, inst.p1, inst.p2);
      break;

    case OP_KALIBRASI:
      LbMotion.runLR(0, 0);
      LbMotion.stopAndWait();
      LbIRLine.doManualCalibration(TB1A);
      while (LbTouch.read(TB1A));
      delay(200);
      LbTouch.readBits();
      LbRGB.fillColor(CRGB::Black);
      LbRGB.show();
      playBeep(1500, 200);
      break;

    case OP_CONFIG: {
      int16_t val = inst.p2;
      switch (inst.p1) {
        case CFG_SPEED:
          if (val > 0) {
            defaultSpeed = val;
            currentSpeed = toMotorSpeed(val);
          }
          break;
        case CFG_MAJU: if (val > 0) defaultMaju = val; break;
        case CFG_MUNDUR: if (val > 0) defaultMundur = val; break;
        case CFG_KANAN: if (val > 0) defaultKanan = val; break;
        case CFG_KIRI: if (val > 0) defaultKiri = val; break;
        case CFG_BELOK_KANAN: if (val > 0) defaultBelokKanan = val; break;
        case CFG_BELOK_KIRI: if (val > 0) defaultBelokKiri = val; break;
        case CFG_FOLLOW_LINE: if (val >= 0) defaultFollowLine = val; break;
        case CFG_MAJU_LINE: if (val > 0) defaultMajuLine = val; break;
        case CFG_MUNDUR_LINE: if (val > 0) defaultMundurLine = val; break;
        case CFG_TUNGGU: if (val > 0) defaultTunggu = val; break;
        case CFG_BEEP: if (val > 0) defaultBeepFreq = val; break;
      }
      break;
    }

    default:
      break;
  }
}

void executeCommandQueue() {
  if (commandCount == 0) return;
  emergencyFlag = false;
  updateRobotState(STATE_EXECUTING);
  LbRGB.fillColor(CRGB::Black);
  LbRGB.show();
  Serial.print(F("CODE_START:"));
  Serial.print(commandCount);
  Serial.print(F(",RAM:"));
  Serial.println(freeRam());
  for (uint8_t i = 0; i < commandCount; i++) {
    if (checkEmergency()) break;
    if (emergencyFlag) break;
    Serial.print(F("[EXEC] "));
    Serial.print(i + 1);
    Serial.print('/');
    Serial.print(commandCount);
    Serial.print(' ');
    printInstructionInfo(commandQueue[i]);
    Serial.println();
    executeInstruction(commandQueue[i]);
    if (i < commandCount - 1) {
      delay(10);
    }
  }
  commandCount = 0;
  if (!emergencyFlag) {
    updateRobotState(STATE_READY);
  }
}

void processCodePacket(char* payload) {
  while (*payload == ' ' || *payload == '\t' || *payload == ';') payload++;
  if (*payload == '\0') return;

  Serial.print(F("[RECV] "));
  Serial.println(payload);
  updateRobotState(STATE_RECEIVING);
  commandCount = 0;
  char* p = payload;

  while (*p != '\0') {
    while (*p == ' ' || *p == '\t' || *p == ';') p++;
    if (*p == '\0') break;

    char* seg = p;
    while (*p != '\0' && *p != ';') p++;
    if (*p == ';') {
      *p = '\0';
      p++;
    }

    Instruction inst;
    if (parseInstruction(seg, inst)) {
      if (commandCount < MAX_COMMANDS) {
        commandQueue[commandCount++] = inst;
      }
    }
  }

  executeCommandQueue();
}
