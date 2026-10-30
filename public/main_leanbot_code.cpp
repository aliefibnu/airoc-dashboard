#include <Leanbot.h>
#include <avr/pgmspace.h>

enum Opcode : uint8_t {
  OP_NONE = 0,
  OP_MAJU,
  OP_MUNDUR,
  OP_KANAN,
  OP_KIRI,
  OP_BELOK_KANAN,
  OP_BELOK_KANAN_REV,
  OP_BELOK_KIRI,
  OP_BELOK_KIRI_REV,
  OP_STOP,
  OP_SPEED,
  OP_TUNGGU,
  OP_CAPIT,
  OP_LEPAS,
  OP_BUNYI,
  OP_LED_WARNA,
  OP_LED_KEDIP,
  OP_EMERGENCY_STOP,
  OP_END,
  OP_FOLLOW_LINE,
  OP_FOLLOW_LINE_REV,
  OP_MAJU_LINE,
  OP_MUNDUR_LINE,
  OP_STOP_LINE,
  OP_KALIBRASI,
  OP_SET_THRESHOLD,
  OP_MISI_DONE
};

enum RobotState : uint8_t {
  STATE_READY = 0,
  STATE_RECEIVING,
  STATE_EXECUTING,
  STATE_EMERGENCY
};

struct __attribute__((packed)) Instruction {
  uint8_t opcode;
  int16_t p1;
  int16_t p2;
};

#define MAX_BATCHES 10
#define MAX_COMMANDS_PER_BATCH 12

struct Batch {
  Instruction commands[MAX_COMMANDS_PER_BATCH];
  uint8_t count;
};

Batch batchQueue[MAX_BATCHES];
uint8_t currentBatchIdx = 0;
bool isLegacyCode = false;

int16_t currentSpeed = 1000;
volatile bool emergencyFlag = false;
RobotState currentRobotState = STATE_READY;

char tokenBuf[36];
uint8_t tokenLen = 0;
char prefixBuf[16];
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
void clearAllBatches();
bool enqueueBatchToken(uint8_t bIdx);
void triggerEmergencyStop();
bool checkEmergency();
void delayInterruptible(unsigned long ms);
void moveMm(int mm, int dir, int speed);
void rotateDeg(int deg, int dir, int speed);
void pivotTurnDeg(int deg, int turnDir, int moveDir, int speed);
void moveToLine(int dir, int maxDistCm, int speed);
void followLineUntilIntersection(int targetIntersections, int speed, int dir);
void playBunyi(int count);
void blinkLed(int count, int intervalMs);
uint8_t parseColorIndex(const char* s);
CRGB getColorFromIndex(uint8_t idx);
uint8_t parseMissionIndex(const char* s);
void printMissionKey(uint8_t idx);
bool parseInstruction(char* str, Instruction& inst);
void executeInstruction(const Instruction& inst);
void executeAllBatches();
void setLineThreshold(int16_t t0, int16_t t1, int16_t t2, int16_t t3);
void handleSetThresholdString(char* params);

int16_t currentThresholds[4] = {302, 128, 94, 112};

void setLineThreshold(int16_t t0, int16_t t1, int16_t t2, int16_t t3) {
  currentThresholds[0] = t0;
  currentThresholds[1] = t1;
  currentThresholds[2] = t2;
  currentThresholds[3] = t3;
  LbIRLine.setThreshold(t0, t1, t2, t3);
  Serial.print(F("[THRESHOLD_SET:"));
  Serial.print(t0);
  Serial.print(',');
  Serial.print(t1);
  Serial.print(',');
  Serial.print(t2);
  Serial.print(',');
  Serial.print(t3);
  Serial.println(']');
  Leanbot.tone(2000, 80);
}

void handleSetThresholdString(char* params) {
  int16_t t[4] = {currentThresholds[0], currentThresholds[1], currentThresholds[2], currentThresholds[3]};
  uint8_t idx = 0;
  char* tok = strtok(params, ", \t");
  while (tok != NULL && idx < 4) {
    t[idx++] = atoi(tok);
    tok = strtok(NULL, ", \t");
  }
  if (idx == 1) {
    t[1] = t[0];
    t[2] = t[0];
    t[3] = t[0];
  } else if (idx == 2) {
    t[2] = t[1];
    t[3] = t[1];
  } else if (idx == 3) {
    t[3] = t[2];
  }
  setLineThreshold(t[0], t[1], t[2], t[3]);
}

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
    case OP_BELOK_KANAN:
    case OP_BELOK_KANAN_REV: Serial.print(F("BELOK_KANAN")); break;
    case OP_BELOK_KIRI:
    case OP_BELOK_KIRI_REV: Serial.print(F("BELOK_KIRI")); break;
    case OP_STOP: Serial.print(F("STOP")); break;
    case OP_SPEED: Serial.print(F("SPEED")); break;
    case OP_TUNGGU: Serial.print(F("TUNGGU")); break;
    case OP_CAPIT: Serial.print(F("CAPIT")); break;
    case OP_LEPAS: Serial.print(F("LEPAS")); break;
    case OP_BUNYI: Serial.print(F("BUNYI")); break;
    case OP_LED_WARNA: Serial.print(F("LED_WARNA")); break;
    case OP_LED_KEDIP: Serial.print(F("LED_KEDIP")); break;
    case OP_EMERGENCY_STOP: Serial.print(F("EMG_STOP")); break;
    case OP_END: Serial.print(F("END")); break;
    case OP_FOLLOW_LINE:
    case OP_FOLLOW_LINE_REV: Serial.print(F("FOLLOW_LINE")); break;
    case OP_MAJU_LINE: Serial.print(F("MAJU_LINE")); break;
    case OP_MUNDUR_LINE: Serial.print(F("MUNDUR_LINE")); break;
    case OP_STOP_LINE: Serial.print(F("STOP_LINE")); break;
    case OP_KALIBRASI: Serial.print(F("KALIBRASI")); break;
    case OP_SET_THRESHOLD: Serial.print(F("SET_THRESHOLD")); break;
    case OP_MISI_DONE: Serial.print(F("MISI_DONE")); break;
    default: Serial.print(op); break;
  }
}

void printInstructionInfo(const Instruction& inst) {
  if (inst.opcode == OP_MISI_DONE) {
    Serial.print(F("MISI_DONE:"));
    printMissionKey(inst.p1);
    Serial.print(',');
    Serial.print(inst.p2 != 0 ? 1 : 0);
    return;
  }
  if (inst.opcode == OP_BELOK_KANAN_REV) {
    Serial.print(F("BELOK_KANAN:"));
    Serial.print(inst.p1);
    if (inst.p2 != 0) {
      Serial.print(',');
      Serial.print(inst.p2);
    }
    Serial.print(F(",MUNDUR"));
    return;
  }
  if (inst.opcode == OP_BELOK_KIRI_REV) {
    Serial.print(F("BELOK_KIRI:"));
    Serial.print(inst.p1);
    if (inst.p2 != 0) {
      Serial.print(',');
      Serial.print(inst.p2);
    }
    Serial.print(F(",MUNDUR"));
    return;
  }
  if (inst.opcode == OP_FOLLOW_LINE_REV) {
    Serial.print(F("FOLLOW_LINE:"));
    Serial.print(inst.p1);
    if (inst.p2 != 0) {
      Serial.print(',');
      Serial.print(inst.p2);
    }
    Serial.print(F(",MUNDUR"));
    return;
  }
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

void clearAllBatches() {
  for (uint8_t b = 0; b < MAX_BATCHES; b++) {
    batchQueue[b].count = 0;
  }
}

bool enqueueBatchToken(uint8_t bIdx) {
  if (tokenLen == 0) return false;
  tokenBuf[tokenLen] = '\0';
  if (strncasecmp_P(tokenBuf, PSTR("BATCH:"), 6) == 0) {
    int bNum = atoi(tokenBuf + 6);
    if (bNum >= 1 && bNum <= MAX_BATCHES) {
      currentBatchIdx = bNum - 1;
      batchQueue[currentBatchIdx].count = 0;
      isLegacyCode = false;
      Serial.print(F("[RECV] BATCH:"));
      Serial.println(currentBatchIdx + 1);
    }
    tokenLen = 0;
    tokenBuf[0] = '\0';
    return true;
  }
  if (strncasecmp_P(tokenBuf, PSTR("SET_THRESHOLD:"), 14) == 0 || strncasecmp_P(tokenBuf, PSTR("THRESHOLD:"), 10) == 0 || strncasecmp_P(tokenBuf, PSTR("SET_TREESHOLD:"), 14) == 0) {
    char* colon = strchr(tokenBuf, ':');
    if (colon != NULL) {
      handleSetThresholdString(colon + 1);
    }
    tokenLen = 0;
    tokenBuf[0] = '\0';
    return true;
  }
  Instruction inst;
  bool ok = parseInstruction(tokenBuf, inst);
  if (ok && inst.opcode != OP_NONE) {
    if (inst.opcode == OP_END) {
    } else if (bIdx < MAX_BATCHES && batchQueue[bIdx].count < MAX_COMMANDS_PER_BATCH) {
      batchQueue[bIdx].commands[batchQueue[bIdx].count++] = inst;
      Serial.print(F("[LOAD] B"));
      Serial.print(bIdx + 1);
      Serial.print(F(" #"));
      Serial.print(batchQueue[bIdx].count);
      Serial.print(' ');
      Serial.println(tokenBuf);
    }
  }
  tokenLen = 0;
  tokenBuf[0] = '\0';
  return ok;
}

void setup() {
  Leanbot.begin();
  setLineThreshold(currentThresholds[0], currentThresholds[1], currentThresholds[2], currentThresholds[3]);
  delay(200);
  LbRGB.fillColor(CRGB::Black);
  LbRGB.show();
  Leanbot.tone(1500, 100);
  clearAllBatches();
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
  clearAllBatches();
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
    if ((c >= 'A' && c <= 'Z') || c == '_') {
      if (emergencyRxLen < sizeof(emergencyRxBuf) - 1) {
        emergencyRxBuf[emergencyRxLen++] = c;
        emergencyRxBuf[emergencyRxLen] = '\0';
      } else {
        memmove(emergencyRxBuf, emergencyRxBuf + 1, sizeof(emergencyRxBuf) - 2);
        emergencyRxBuf[sizeof(emergencyRxBuf) - 2] = c;
        emergencyRxBuf[sizeof(emergencyRxBuf) - 1] = '\0';
      }
      if (strstr_P(emergencyRxBuf, PSTR("EMERGENCY")) != NULL) {
        triggerEmergencyStop();
        return true;
      }
    } else if (c == '\n' || c == '\r' || c == ';') {
      emergencyRxLen = 0;
      emergencyRxBuf[0] = '\0';
    }
  }
  return emergencyFlag;
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    lastRxTime = millis();

    if (!inPacket) {
      if (prefixLen == 0 && (c == '\r' || c == '\n' || c == ' ' || c == '\t')) {
        continue;
      }

      if (prefixLen < sizeof(prefixBuf) - 1) {
        prefixBuf[prefixLen++] = c;
        prefixBuf[prefixLen] = '\0';
      } else {
        memmove(prefixBuf, prefixBuf + 1, sizeof(prefixBuf) - 2);
        prefixBuf[sizeof(prefixBuf) - 2] = c;
        prefixBuf[sizeof(prefixBuf) - 1] = '\0';
      }

      if (strstr_P(prefixBuf, PSTR("EMERGENCY")) != NULL) {
        triggerEmergencyStop();
        return;
      }

      if (strstr_P(prefixBuf, PSTR("CLEAR")) != NULL) {
        clearAllBatches();
        prefixLen = 0;
        prefixBuf[0] = '\0';
        Serial.println(F("[BATCHES_CLEARED]"));
        return;
      }

      if (strstr_P(prefixBuf, PSTR("START")) != NULL) {
        prefixLen = 0;
        prefixBuf[0] = '\0';
        executeAllBatches();
        return;
      }

      if (strstr_P(prefixBuf, PSTR("KALIBRASI")) != NULL || strstr_P(prefixBuf, PSTR("CALIBRATE")) != NULL) {
        prefixLen = 0;
        prefixBuf[0] = '\0';
        Instruction calInst;
        calInst.opcode = OP_KALIBRASI;
        calInst.p1 = 0;
        calInst.p2 = 0;
        executeInstruction(calInst);
        return;
      }

      char* threshPtr = strstr_P(prefixBuf, PSTR("SET_THRESHOLD:"));
      if (threshPtr == NULL) threshPtr = strstr_P(prefixBuf, PSTR("THRESHOLD:"));
      if (threshPtr == NULL) threshPtr = strstr_P(prefixBuf, PSTR("SET_TREESHOLD:"));
      if (threshPtr != NULL) {
        char* semicolon = strchr(threshPtr, ';');
        if (semicolon != NULL || c == '\n' || c == '\r') {
          if (semicolon != NULL) *semicolon = '\0';
          char* colon = strchr(threshPtr, ':');
          if (colon != NULL) {
            handleSetThresholdString(colon + 1);
          }
          prefixLen = 0;
          prefixBuf[0] = '\0';
          return;
        }
      }

      char* batchPtr = strstr_P(prefixBuf, PSTR("BATCH:"));
      if (batchPtr != NULL) {
        char* semicolon = strchr(batchPtr, ';');
        if (semicolon != NULL) {
          *semicolon = '\0';
          int bNum = atoi(batchPtr + 6);
          if (bNum >= 1 && bNum <= MAX_BATCHES) {
            currentBatchIdx = bNum - 1;
          } else {
            currentBatchIdx = 0;
          }
          batchQueue[currentBatchIdx].count = 0;
          inPacket = true;
          isLegacyCode = false;
          emergencyFlag = false;
          tokenLen = 0;
          prefixLen = 0;
          emergencyRxLen = 0;
          prefixBuf[0] = '\0';
          tokenBuf[0] = '\0';
          emergencyRxBuf[0] = '\0';
          updateRobotState(STATE_RECEIVING);
          Serial.print(F("[RECV] BATCH:"));
          Serial.println(currentBatchIdx + 1);
        }
        continue;
      }

      if (strstr_P(prefixBuf, PSTR("CODE;")) != NULL || strstr_P(prefixBuf, PSTR("code;")) != NULL) {
        currentBatchIdx = 0;
        batchQueue[0].count = 0;
        inPacket = true;
        isLegacyCode = true;
        emergencyFlag = false;
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
        enqueueBatchToken(currentBatchIdx);
      } else if (c == '\n' || c == '\r') {
        enqueueBatchToken(currentBatchIdx);
        inPacket = false;
        prefixLen = 0;
        prefixBuf[0] = '\0';
        Serial.print(F("[BATCH_LOADED:"));
        Serial.print(currentBatchIdx + 1);
        Serial.print(F(",COUNT:"));
        Serial.print(batchQueue[currentBatchIdx].count);
        Serial.println(']');
        if (isLegacyCode) {
          executeAllBatches();
        }
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
    enqueueBatchToken(currentBatchIdx);
    inPacket = false;
    prefixLen = 0;
    prefixBuf[0] = '\0';
    Serial.print(F("[BATCH_LOADED:"));
    Serial.print(currentBatchIdx + 1);
    Serial.print(F(",COUNT:"));
    Serial.print(batchQueue[currentBatchIdx].count);
    Serial.println(']');
    if (isLegacyCode) {
      executeAllBatches();
    }
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

void pivotTurnDeg(int deg, int turnDir, int moveDir, int speed) {
  if (emergencyFlag || deg <= 0) return;
  int spd = speed > 0 ? toMotorSpeed(speed) : (currentSpeed > 0 ? currentSpeed : 1000);
  int rotDir = 0;
  if (turnDir > 0) {
    if (moveDir >= 0) {
      LbMotion.runLR(+spd, 0);
      rotDir = +1;
    } else {
      LbMotion.runLR(-spd, 0);
      rotDir = -1;
    }
  } else {
    if (moveDir >= 0) {
      LbMotion.runLR(0, +spd);
      rotDir = -1;
    } else {
      LbMotion.runLR(0, -spd);
      rotDir = +1;
    }
  }
  int remaining = deg;
  while (remaining > 0 && !emergencyFlag) {
    int step = remaining > 10 ? 10 : remaining;
    LbMotion.waitRotationDeg(rotDir * step);
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
    playBunyi(1);
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
    playBunyi(1);
  }
}

void followLineUntilIntersection(int targetIntersections, int speed, int dir) {
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
    if (dir >= 0) {
      LbMotion.runLR(+spd, +spd);
      LbMotion.waitDistanceMm(+25);
    } else {
      LbMotion.runLR(-spd, -spd);
      LbMotion.waitDistanceMm(-25);
    }
  }

  while (!emergencyFlag) {
    if (checkEmergency()) break;

    int lineValue = LbIRLine.read();
    LbIRLine.displayOnRGB(CRGB::DarkCyan);

    bool isIntersection = (lineValue == 0b1111) || ((lineValue & 0b1001) == 0b1001);

    if (isIntersection) {
      intersectionsCount++;
      if (intersectionsCount >= targetIntersections) break;
      if (dir >= 0) {
        LbMotion.runLR(+spd, +spd);
        LbMotion.waitDistanceMm(+25);
      } else {
        LbMotion.runLR(-spd, -spd);
        LbMotion.waitDistanceMm(-25);
      }
      continue;
    }

    if (lineValue != 0b0000) lastSeenLine = millis();

    if (dir >= 0) {
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
            playBunyi(1);
            return;
          }
          break;

        default:
          LbMotion.runLR(+spd, +spd);
          break;
      }
    } else {
      switch (lineValue) {
        case 0b0010:
        case 0b0111:
          LbMotion.runLR(0, -spd);
          break;

        case 0b0011:
        case 0b0001:
          LbMotion.runLR(+spd, -spd);
          break;

        case 0b0100:
        case 0b1110:
          LbMotion.runLR(-spd, 0);
          break;

        case 0b1100:
        case 0b1000:
          LbMotion.runLR(-spd, +spd);
          break;

        case 0b0000:
          if (millis() - lastSeenLine > 1500) {
            LbMotion.runLR(0, 0);
            LbMotion.stopAndWait();
            LbRGB.fillColor(CRGB::Red);
            LbRGB.show();
            playBunyi(1);
            return;
          }
          break;

        default:
          LbMotion.runLR(-spd, -spd);
          break;
      }
    }

    delay(2);
  }

  LbMotion.runLR(0, 0);
  LbMotion.stopAndWait();
  if (!emergencyFlag) {
    LbRGB.fillColor(CRGB::Green);
    LbRGB.show();
    playBunyi(1);
  }
}

void playBunyi(int count) {
  if (emergencyFlag) return;
  if (count <= 0) count = 1;
  for (int i = 0; i < count; i++) {
    Leanbot.tone(1500, 100);
    delayInterruptible(150);
    if (emergencyFlag) break;
  }
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

uint8_t parseMissionIndex(const char* s) {
  if (strcasecmp_P(s, PSTR("neutral_ungu")) == 0 || strcasecmp_P(s, PSTR("ungu")) == 0 || strcmp_P(s, PSTR("1")) == 0) return 1;
  if (strcasecmp_P(s, PSTR("neutral_hijau")) == 0 || strcasecmp_P(s, PSTR("hijau")) == 0 || strcmp_P(s, PSTR("2")) == 0) return 2;
  if (strcasecmp_P(s, PSTR("neutral_biru")) == 0 || strcasecmp_P(s, PSTR("biru")) == 0 || strcmp_P(s, PSTR("3")) == 0) return 3;
  if (strcasecmp_P(s, PSTR("analysis_normal")) == 0 || strcasecmp_P(s, PSTR("normal")) == 0 || strcmp_P(s, PSTR("4")) == 0) return 4;
  if (strcasecmp_P(s, PSTR("analysis_mystery")) == 0 || strcasecmp_P(s, PSTR("mystery")) == 0 || strcmp_P(s, PSTR("5")) == 0) return 5;
  if (strcasecmp_P(s, PSTR("containment_merah")) == 0 || strcasecmp_P(s, PSTR("merah")) == 0 || strcmp_P(s, PSTR("6")) == 0) return 6;
  if (strcasecmp_P(s, PSTR("containment_kuning")) == 0 || strcasecmp_P(s, PSTR("kuning")) == 0 || strcmp_P(s, PSTR("7")) == 0) return 7;
  if (strcasecmp_P(s, PSTR("crl_leanbot_a")) == 0 || strcasecmp_P(s, PSTR("leanbot_a")) == 0 || strcmp_P(s, PSTR("8")) == 0) return 8;
  if (strcasecmp_P(s, PSTR("crl_leanbot_b")) == 0 || strcasecmp_P(s, PSTR("leanbot_b")) == 0 || strcmp_P(s, PSTR("9")) == 0) return 9;
  if (isdigit(s[0])) return (uint8_t)atoi(s);
  return 1;
}

void printMissionKey(uint8_t idx) {
  switch (idx) {
    case 1: Serial.print(F("neutral_ungu")); break;
    case 2: Serial.print(F("neutral_hijau")); break;
    case 3: Serial.print(F("neutral_biru")); break;
    case 4: Serial.print(F("analysis_normal")); break;
    case 5: Serial.print(F("analysis_mystery")); break;
    case 6: Serial.print(F("containment_merah")); break;
    case 7: Serial.print(F("containment_kuning")); break;
    case 8: Serial.print(F("crl_leanbot_a")); break;
    case 9: Serial.print(F("crl_leanbot_b")); break;
    default: Serial.print(F("neutral_ungu")); break;
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
  } else if (strcasecmp_P(cmd, PSTR("MUNDUR")) == 0) {
    inst.opcode = OP_MUNDUR;
  } else if (strcasecmp_P(cmd, PSTR("KANAN")) == 0 || strcasecmp_P(cmd, PSTR("ROTATE_KANAN")) == 0) {
    inst.opcode = OP_KANAN;
  } else if (strcasecmp_P(cmd, PSTR("KIRI")) == 0 || strcasecmp_P(cmd, PSTR("ROTATE_KIRI")) == 0) {
    inst.opcode = OP_KIRI;
  } else if (strcasecmp_P(cmd, PSTR("BELOK_KANAN")) == 0 || strcasecmp_P(cmd, PSTR("PIVOT_KANAN")) == 0) {
    inst.opcode = OP_BELOK_KANAN;
  } else if (strcasecmp_P(cmd, PSTR("BELOK_KANAN_MUNDUR")) == 0 || strcasecmp_P(cmd, PSTR("PIVOT_KANAN_MUNDUR")) == 0) {
    inst.opcode = OP_BELOK_KANAN_REV;
  } else if (strcasecmp_P(cmd, PSTR("BELOK_KIRI")) == 0 || strcasecmp_P(cmd, PSTR("PIVOT_KIRI")) == 0) {
    inst.opcode = OP_BELOK_KIRI;
  } else if (strcasecmp_P(cmd, PSTR("BELOK_KIRI_MUNDUR")) == 0 || strcasecmp_P(cmd, PSTR("PIVOT_KIRI_MUNDUR")) == 0) {
    inst.opcode = OP_BELOK_KIRI_REV;
  } else if (strcasecmp_P(cmd, PSTR("BELOK")) == 0) {
    inst.opcode = OP_BELOK_KANAN;
  } else if (strcasecmp_P(cmd, PSTR("STOP")) == 0) {
    inst.opcode = OP_STOP;
  } else if (strcasecmp_P(cmd, PSTR("SPEED")) == 0) {
    inst.opcode = OP_SPEED;
  } else if (strcasecmp_P(cmd, PSTR("TUNGGU")) == 0) {
    inst.opcode = OP_TUNGGU;
  } else if (strcasecmp_P(cmd, PSTR("CAPIT")) == 0) {
    inst.opcode = OP_CAPIT;
  } else if (strcasecmp_P(cmd, PSTR("LEPAS")) == 0) {
    inst.opcode = OP_LEPAS;
  } else if (strcasecmp_P(cmd, PSTR("BUNYI")) == 0) {
    inst.opcode = OP_BUNYI;
  } else if (strcasecmp_P(cmd, PSTR("LED_WARNA")) == 0) {
    inst.opcode = OP_LED_WARNA;
  } else if (strcasecmp_P(cmd, PSTR("LED_KEDIP")) == 0) {
    inst.opcode = OP_LED_KEDIP;
  } else if (strcasecmp_P(cmd, PSTR("EMERGENCY_STOP")) == 0 || strcasecmp_P(cmd, PSTR("EMERGENCY")) == 0) {
    inst.opcode = OP_EMERGENCY_STOP;
  } else if (strcasecmp_P(cmd, PSTR("END")) == 0) {
    inst.opcode = OP_END;
  } else if (strcasecmp_P(cmd, PSTR("FOLLOW_LINE")) == 0) {
    inst.opcode = OP_FOLLOW_LINE;
  } else if (strcasecmp_P(cmd, PSTR("FOLLOW_LINE_MUNDUR")) == 0) {
    inst.opcode = OP_FOLLOW_LINE_REV;
  } else if (strcasecmp_P(cmd, PSTR("MAJU_LINE")) == 0) {
    inst.opcode = OP_MAJU_LINE;
  } else if (strcasecmp_P(cmd, PSTR("MUNDUR_LINE")) == 0) {
    inst.opcode = OP_MUNDUR_LINE;
  } else if (strcasecmp_P(cmd, PSTR("STOP_LINE")) == 0) {
    inst.opcode = OP_STOP_LINE;
  } else if (strcasecmp_P(cmd, PSTR("KALIBRASI")) == 0 || strcasecmp_P(cmd, PSTR("CALIBRATE")) == 0) {
    inst.opcode = OP_KALIBRASI;
  } else if (strcasecmp_P(cmd, PSTR("SET_THRESHOLD")) == 0 || strcasecmp_P(cmd, PSTR("SET_TREESHOLD")) == 0 || strcasecmp_P(cmd, PSTR("THRESHOLD")) == 0) {
    inst.opcode = OP_SET_THRESHOLD;
  } else if (strcasecmp_P(cmd, PSTR("MISI_DONE")) == 0 || strcasecmp_P(cmd, PSTR("TANDAI_DONE")) == 0 || strcasecmp_P(cmd, PSTR("TANDAI_MISI_DONE")) == 0) {
    inst.opcode = OP_MISI_DONE;
    inst.p1 = 1;
    inst.p2 = 1;
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

    if (inst.opcode == OP_SET_THRESHOLD) {
      handleSetThresholdString(params);
      inst.opcode = OP_NONE;
      return true;
    } else if (inst.opcode == OP_MISI_DONE) {
      char* comma = strchr(params, ',');
      if (comma != NULL) {
        *comma = '\0';
        char* p1Str = params;
        char* p2Str = comma + 1;
        while (*p1Str == ' ' || *p1Str == '\t') p1Str++;
        while (*p2Str == ' ' || *p2Str == '\t') p2Str++;
        inst.p1 = parseMissionIndex(p1Str);
        if (strcasecmp_P(p2Str, PSTR("TRUE")) == 0 || strcmp_P(p2Str, PSTR("1")) == 0) {
          inst.p2 = 1;
        } else if (strcasecmp_P(p2Str, PSTR("FALSE")) == 0 || strcmp_P(p2Str, PSTR("0")) == 0) {
          inst.p2 = 0;
        } else {
          inst.p2 = atoi(p2Str);
        }
      } else {
        char* p1Str = params;
        while (*p1Str == ' ' || *p1Str == '\t') p1Str++;
        inst.p1 = parseMissionIndex(p1Str);
        inst.p2 = 1;
      }
    } else if (inst.opcode == OP_LED_WARNA) {
      inst.p1 = parseColorIndex(params);
    } else if (inst.opcode == OP_BELOK_KANAN || inst.opcode == OP_BELOK_KANAN_REV || inst.opcode == OP_BELOK_KIRI || inst.opcode == OP_BELOK_KIRI_REV) {
      char* c1 = strchr(params, ',');
      if (c1 != NULL) {
        *c1 = '\0';
        inst.p1 = atoi(params);
        char* p2 = c1 + 1;
        while (*p2 == ' ' || *p2 == '\t') p2++;
        char* c2 = strchr(p2, ',');
        if (c2 != NULL) {
          *c2 = '\0';
          inst.p2 = atoi(p2);
          char* p3 = c2 + 1;
          while (*p3 == ' ' || *p3 == '\t') p3++;
          if (strcasecmp_P(p3, PSTR("MUNDUR")) == 0 || atoi(p3) == -1) {
            inst.opcode = (inst.opcode == OP_BELOK_KANAN || inst.opcode == OP_BELOK_KANAN_REV) ? OP_BELOK_KANAN_REV : OP_BELOK_KIRI_REV;
          }
        } else {
          if (strcasecmp_P(p2, PSTR("MUNDUR")) == 0 || atoi(p2) == -1) {
            inst.opcode = (inst.opcode == OP_BELOK_KANAN || inst.opcode == OP_BELOK_KANAN_REV) ? OP_BELOK_KANAN_REV : OP_BELOK_KIRI_REV;
            inst.p2 = 0;
          } else {
            inst.p2 = atoi(p2);
          }
        }
      } else {
        inst.p1 = atoi(params);
      }
    } else if (inst.opcode == OP_FOLLOW_LINE || inst.opcode == OP_FOLLOW_LINE_REV) {
      char* c1 = strchr(params, ',');
      if (c1 != NULL) {
        *c1 = '\0';
        char* p1Str = params;
        while (*p1Str == ' ' || *p1Str == '\t') p1Str++;
        if (strcasecmp_P(p1Str, PSTR("OFF")) == 0 || strcmp_P(p1Str, PSTR("0")) == 0) inst.p1 = 0;
        else if (strcasecmp_P(p1Str, PSTR("ON")) == 0) inst.p1 = 9999;
        else inst.p1 = atoi(p1Str);

        char* p2 = c1 + 1;
        while (*p2 == ' ' || *p2 == '\t') p2++;
        char* c2 = strchr(p2, ',');
        if (c2 != NULL) {
          *c2 = '\0';
          inst.p2 = atoi(p2);
          char* p3 = c2 + 1;
          while (*p3 == ' ' || *p3 == '\t') p3++;
          if (strcasecmp_P(p3, PSTR("MUNDUR")) == 0 || atoi(p3) == -1) {
            inst.opcode = OP_FOLLOW_LINE_REV;
          }
        } else {
          if (strcasecmp_P(p2, PSTR("MUNDUR")) == 0 || atoi(p2) == -1) {
            inst.opcode = OP_FOLLOW_LINE_REV;
            inst.p2 = 0;
          } else {
            inst.p2 = atoi(p2);
          }
        }
      } else {
        char* p1Str = params;
        while (*p1Str == ' ' || *p1Str == '\t') p1Str++;
        if (strcasecmp_P(p1Str, PSTR("OFF")) == 0 || strcmp_P(p1Str, PSTR("0")) == 0) inst.p1 = 0;
        else if (strcasecmp_P(p1Str, PSTR("ON")) == 0) inst.p1 = 9999;
        else inst.p1 = atoi(p1Str);
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
      pivotTurnDeg(inst.p1, +1, +1, inst.p2);
      break;

    case OP_BELOK_KANAN_REV:
      pivotTurnDeg(inst.p1, +1, -1, inst.p2);
      break;

    case OP_BELOK_KIRI:
      pivotTurnDeg(inst.p1, -1, +1, inst.p2);
      break;

    case OP_BELOK_KIRI_REV:
      pivotTurnDeg(inst.p1, -1, -1, inst.p2);
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

    case OP_BUNYI:
      playBunyi(inst.p1);
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

    case OP_END:
      LbMotion.runLR(0, 0);
      LbMotion.stopAndWait();
      Serial.println(F("[BATCH_END]"));
      break;

    case OP_FOLLOW_LINE:
      if (inst.p1 > 0) {
        followLineUntilIntersection(inst.p1, inst.p2, +1);
      } else {
        LbMotion.runLR(0, 0);
        LbMotion.stopAndWait();
      }
      break;

    case OP_FOLLOW_LINE_REV:
      if (inst.p1 > 0) {
        followLineUntilIntersection(inst.p1, inst.p2, -1);
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
      Serial.println(F("[CALIBRATION_START]"));
      LbIRLine.doManualCalibration(TB1A);
      while (LbTouch.read(TB1A));
      delay(200);
      LbTouch.readBits();
      LbRGB.fillColor(CRGB::Black);
      LbRGB.show();
      playBunyi(2);
      Serial.println(F("[CALIBRATION_DONE]"));
      break;

    case OP_SET_THRESHOLD:
      break;

    case OP_MISI_DONE:
      Serial.print(F("IOT : {\"key\":\""));
      printMissionKey(inst.p1);
      Serial.print(F("\",\"done\":"));
      Serial.print(inst.p2 != 0 ? F("true") : F("false"));
      Serial.println(F("}"));
      break;

    default:
      break;
  }
}

void executeAllBatches() {
  emergencyFlag = false;
  updateRobotState(STATE_EXECUTING);
  LbRGB.fillColor(CRGB::Black);
  LbRGB.show();
  Serial.print(F("MISSION_START,RAM:"));
  Serial.println(freeRam());

  for (uint8_t b = 0; b < MAX_BATCHES; b++) {
    if (emergencyFlag || checkEmergency()) break;
    if (batchQueue[b].count == 0) {
      continue;
    }

    Serial.print(F("[START_BATCH] "));
    Serial.print(b + 1);
    Serial.print(F(",CMDS:"));
    Serial.println(batchQueue[b].count);

    for (uint8_t i = 0; i < batchQueue[b].count; i++) {
      if (emergencyFlag || checkEmergency()) break;

      Serial.print(F("[EXEC] B"));
      Serial.print(b + 1);
      Serial.print(' ');
      Serial.print(i + 1);
      Serial.print('/');
      Serial.print(batchQueue[b].count);
      Serial.print(' ');
      printInstructionInfo(batchQueue[b].commands[i]);
      Serial.println();

      executeInstruction(batchQueue[b].commands[i]);

      if (i < batchQueue[b].count - 1) {
        delay(10);
      }
    }

    if (emergencyFlag) break;

    Serial.print(F("[BATCH_DONE] "));
    Serial.println(b + 1);
  }

  clearAllBatches();
  currentBatchIdx = 0;

  if (!emergencyFlag) {
    updateRobotState(STATE_READY);
    Serial.println(F("[BATCHES_CLEARED]"));
    Serial.println(F("[MISSION_DONE]"));
  }
}
