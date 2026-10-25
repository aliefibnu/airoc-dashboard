#include <Leanbot.h>

enum Opcode {
  OP_NONE = 0,
  OP_MAJU,
  OP_MUNDUR,
  OP_KANAN,
  OP_KIRI,
  OP_STOP,
  OP_SPEED,
  OP_TUNGGU,
  OP_UKUR_JARAK,
  OP_SCAN_KANAN,
  OP_SCAN_KIRI,
  OP_DEKATI_OBJEK,
  OP_JAUHI_OBJEK,
  OP_CEK_OBJEK,
  OP_BATAS_SCAN,
  OP_CAPIT,
  OP_LEPAS,
  OP_BUNYI,
  OP_BEEP,
  OP_LED,
  OP_LED_WARNA,
  OP_LED_KEDIP,
  OP_LAPOR_JARAK,
  OP_EMERGENCY_STOP,
  OP_END,
  OP_SERVO,
  OP_FOLLOW_LINE,
  OP_MAJU_LINE,
  OP_MUNDUR_LINE,
  OP_STOP_LINE,
  OP_KALIBRASI,
  OP_CONFIG
};

struct Instruction {
  uint8_t opcode;
  long p1;
  long p2;
};

const int MAX_COMMANDS = 64;
Instruction commandQueue[MAX_COMMANDS];
int commandCount = 0;

int currentSpeed = 1000;
int scanMaxAngle = 90;
volatile bool emergencyFlag = false;

int defaultMaju = 10;
int defaultMundur = 10;
int defaultKanan = 90;
int defaultKiri = 90;
int defaultSpeed = 5;
int defaultTunggu = 1000;
int defaultScanDist = 20;
int defaultScanAngle = 90;
int defaultDekati = 6;
int defaultJauhi = 20;
int defaultCek = 20;
int defaultMajuLine = 50;
int defaultMundurLine = 50;
int defaultBunyi = 1;
int defaultBeepFreq = 1500;
int defaultBeepDur = 100;
int defaultServo = 90;
int defaultFollowLine = 1;

char rxBuf[512];
int rxLen = 0;
unsigned long lastRxTime = 0;

void triggerEmergencyStop();
void checkEmergency();
void delayInterruptible(unsigned long ms);
void moveMm(int mm, int dir);
void rotateDeg(int deg, int dir);
void scanObject(int dir, int maxDistCm, int maxDeg);
void approachObject(int targetCm);
void retreatObject(int targetCm);
void moveToLine(int dir, int maxDistCm);
void followLineUntilIntersection(int targetIntersections);
void playBunyi(int count);
void playBeep(int freq, int duration);
void blinkLed(int count, int intervalMs);
CRGB parseColor(const char* s);
bool parseInstruction(char* str, Instruction& inst);
void executeInstruction(const Instruction& inst);
void processCodePacket(char* payload);

void setup() {
  Leanbot.begin();
  LbIRLine.doManualCalibration(TB1A);
  while (LbTouch.read(TB1A));
  delay(200);
  LbTouch.readBits();
  LbRGB.fillColor(CRGB::Black);
  LbRGB.show();
  Leanbot.tone(1500, 100);
  while (Serial.available() > 0) {
    Serial.read();
  }
  rxLen = 0;
  rxBuf[0] = '\0';
}

void loop() {
  checkEmergency();

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (rxLen < (int)sizeof(rxBuf) - 1) {
      rxBuf[rxLen++] = c;
      rxBuf[rxLen] = '\0';
    }
    lastRxTime = millis();
  }

  if (rxLen > 0) {
    char* codePtr = strstr(rxBuf, "CODE;");
    if (codePtr != NULL) {
      if (codePtr > rxBuf) {
        memmove(rxBuf, codePtr, strlen(codePtr) + 1);
        rxLen = strlen(rxBuf);
      }
      char lastChar = rxBuf[rxLen - 1];
      bool hasNewline = (lastChar == '\n' || lastChar == '\r');
      bool hasTimeout = (millis() - lastRxTime > 80);

      if (hasNewline || hasTimeout) {
        while (rxLen > 0 && (rxBuf[rxLen - 1] == '\r' || rxBuf[rxLen - 1] == '\n' || rxBuf[rxLen - 1] == ' ' || rxBuf[rxLen - 1] == '\t')) {
          rxBuf[--rxLen] = '\0';
        }
        processCodePacket(rxBuf + 5);
        rxLen = 0;
        rxBuf[0] = '\0';
      }
    } else {
      if (rxLen > 10) {
        char temp[5];
        int keep = (rxLen >= 4) ? 4 : rxLen;
        memcpy(temp, rxBuf + rxLen - keep, keep);
        temp[keep] = '\0';
        memcpy(rxBuf, temp, keep + 1);
        rxLen = keep;
      }
      if (millis() - lastRxTime > 200) {
        rxLen = 0;
        rxBuf[0] = '\0';
      }
    }
  }
}

void triggerEmergencyStop() {
  emergencyFlag = true;
  LbMotion.runLR(0, 0);
  LbMotion.stopAndWait();
  Leanbot.noTone();
  LbRGB.fillColor(CRGB::Red);
  LbRGB.show();
  commandCount = 0;
  Serial.println(F("EMERGENCY_STOP"));
}

void checkEmergency() {
  if (LbTouch.onPress(TB2A) || LbTouch.onPress(TB2B)) {
    triggerEmergencyStop();
    return;
  }

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (rxLen < (int)sizeof(rxBuf) - 1) {
      rxBuf[rxLen++] = c;
      rxBuf[rxLen] = '\0';
    }
    lastRxTime = millis();
    if (strstr(rxBuf, "EMERGENCY_STOP") != NULL || strstr(rxBuf, "STOP") != NULL) {
      triggerEmergencyStop();
      rxLen = 0;
      rxBuf[0] = '\0';
      return;
    }
  }
}

void delayInterruptible(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    checkEmergency();
    if (emergencyFlag) break;
    delay(5);
  }
}

void moveMm(int mm, int dir) {
  if (emergencyFlag || mm <= 0) return;
  int spd = currentSpeed > 0 ? currentSpeed : 1000;
  LbMotion.runLR(dir * spd, dir * spd);
  int remaining = mm;
  while (remaining > 0 && !emergencyFlag) {
    int step = remaining > 50 ? 50 : remaining;
    LbMotion.waitDistanceMm(dir * step);
    remaining -= step;
    checkEmergency();
  }
  LbMotion.stopAndWait();
}

void rotateDeg(int deg, int dir) {
  if (emergencyFlag || deg <= 0) return;
  int spd = currentSpeed > 0 ? currentSpeed : 1000;
  if (dir > 0) {
    LbMotion.runLR(+spd, -spd);
  } else {
    LbMotion.runLR(-spd, +spd);
  }
  int remaining = deg;
  while (remaining > 0 && !emergencyFlag) {
    int step = remaining > 45 ? 45 : remaining;
    LbMotion.waitRotationDeg(dir * step);
    remaining -= step;
    checkEmergency();
  }
  LbMotion.stopAndWait();
}

void scanObject(int dir, int maxDistCm, int maxDeg) {
  if (emergencyFlag) return;
  if (maxDistCm <= 0) maxDistCm = defaultScanDist;
  if (maxDeg <= 0) maxDeg = defaultScanAngle;

  int d = Leanbot.pingCm();
  if (d > 0 && d <= maxDistCm) {
    Serial.print(F("OBJEK:"));
    Serial.println(d);
    return;
  }

  int spd = 600;
  if (dir > 0) {
    LbMotion.runLR(+spd, -spd);
  } else {
    LbMotion.runLR(-spd, +spd);
  }

  int step = 3;
  bool found = false;
  int foundDist = 0;
  int traveledDeg = 0;

  while (traveledDeg < maxDeg && !emergencyFlag) {
    int curStep = (maxDeg - traveledDeg > step) ? step : (maxDeg - traveledDeg);
    LbMotion.waitRotationDeg(dir * curStep);
    traveledDeg += curStep;
    checkEmergency();
    if (emergencyFlag) break;

    d = Leanbot.pingCm();
    if (d > 0 && d <= maxDistCm) {
      found = true;
      foundDist = d;
      break;
    }
  }

  LbMotion.stopAndWait();
  if (emergencyFlag) return;

  if (found) {
    Serial.print(F("OBJEK:"));
    Serial.println(foundDist);
  } else {
    Serial.println(F("OBJEK:TIDAK_ADA"));
  }
}

void approachObject(int targetCm) {
  if (emergencyFlag) return;
  if (targetCm <= 0) targetCm = defaultDekati;

  int d = Leanbot.pingCm();
  if (d <= 0 || d <= targetCm || d > 80) {
    Serial.print(F("JARAK_AKHIR:"));
    Serial.println(Leanbot.pingCm());
    return;
  }

  int spd = 600;
  LbMotion.runLR(+spd, +spd);

  int stepMm = 10;
  int traveledMm = 0;
  int maxTravelMm = 800;
  int missCount = 0;

  while (traveledMm < maxTravelMm && !emergencyFlag) {
    LbMotion.waitDistanceMm(stepMm);
    traveledMm += stepMm;
    checkEmergency();
    if (emergencyFlag) break;

    d = Leanbot.pingCm();
    if (d > 0 && d <= targetCm) {
      break;
    }
    if (d <= 0 || d > 80) {
      missCount++;
      if (missCount >= 4) {
        break;
      }
    } else {
      missCount = 0;
      if (d - targetCm <= 4 && spd > 350) {
        spd = 350;
        LbMotion.runLR(+spd, +spd);
      }
    }
  }

  LbMotion.stopAndWait();
  Serial.print(F("JARAK_AKHIR:"));
  Serial.println(Leanbot.pingCm());
}

void retreatObject(int targetCm) {
  if (emergencyFlag) return;
  if (targetCm <= 0) targetCm = defaultJauhi;

  int d = Leanbot.pingCm();
  if (d <= 0 || d >= targetCm || d > 80) {
    Serial.print(F("JARAK_AKHIR:"));
    Serial.println(Leanbot.pingCm());
    return;
  }

  int spd = 600;
  LbMotion.runLR(-spd, -spd);

  int stepMm = 10;
  int traveledMm = 0;
  int maxTravelMm = 800;
  int missCount = 0;

  while (traveledMm < maxTravelMm && !emergencyFlag) {
    LbMotion.waitDistanceMm(-stepMm);
    traveledMm += stepMm;
    checkEmergency();
    if (emergencyFlag) break;

    d = Leanbot.pingCm();
    if (d >= targetCm && d <= 80) {
      break;
    }
    if (d <= 0 || d > 80) {
      missCount++;
      if (missCount >= 4) {
        break;
      }
    } else {
      missCount = 0;
      if (targetCm - d <= 4 && spd > 350) {
        spd = 350;
        LbMotion.runLR(-spd, -spd);
      }
    }
  }

  LbMotion.stopAndWait();
  Serial.print(F("JARAK_AKHIR:"));
  Serial.println(Leanbot.pingCm());
}

void moveToLine(int dir, int maxDistCm) {
  if (emergencyFlag) return;
  int spd = 500;
  int maxMm = maxDistCm > 0 ? maxDistCm * 10 : 500;
  int stepMm = 2;
  int traveledMm = 0;
  bool found = false;

  int initialLine = LbIRLine.read();
  if ((initialLine & 0b1111) == 0b0110) {
    LbRGB.fillColor(CRGB::Green);
    LbRGB.show();
    playBunyi(1);
    return;
  }

  LbMotion.runLR(dir * spd, dir * spd);

  while (!emergencyFlag && traveledMm < maxMm) {
    LbMotion.waitDistanceMm(dir * stepMm);
    traveledMm += stepMm;
    checkEmergency();
    if (emergencyFlag) break;

    int line = LbIRLine.read();
    LbIRLine.displayOnRGB(line, CRGB::Blue);

    if ((line & 0b1111) == 0b0110) {
      found = true;
      break;
    }
  }

  LbMotion.stopAndWait();

  if (!emergencyFlag) {
    if (found) {
      LbRGB.fillColor(CRGB::Green);
      LbRGB.show();
      playBunyi(1);
    } else {
      LbRGB.fillColor(CRGB::Red);
      LbRGB.show();
      playBunyi(1);
    }
  }
}

void followLineUntilIntersection(int targetIntersections) {
  if (emergencyFlag) return;
  if (targetIntersections <= 0) {
    LbMotion.runLR(0, 0);
    LbMotion.stopAndWait();
    return;
  }

  int spd = currentSpeed > 0 ? currentSpeed : 1000;
  if (spd < 400) spd = 400;
  if (spd > 1200) spd = 1200;

  int intersectionsCount = 0;
  unsigned long lastSeenLine = millis();

  int initLine = LbIRLine.read();
  if ((initLine == 0b1111) || ((initLine & 0b1001) == 0b1001)) {
    LbMotion.runLR(spd, spd);
    LbMotion.waitDistanceMm(25);
  }

  while (!emergencyFlag) {
    checkEmergency();
    if (emergencyFlag) break;

    int lineValue = LbIRLine.read();
    LbIRLine.displayOnRGB(CRGB::DarkCyan);

    bool isIntersection = (lineValue == 0b1111) || ((lineValue & 0b1001) == 0b1001);

    if (isIntersection) {
      intersectionsCount++;
      if (intersectionsCount >= targetIntersections) {
        break;
      }
      LbMotion.runLR(spd, spd);
      LbMotion.waitDistanceMm(25);
      continue;
    }

    if (lineValue != 0b0000) {
      lastSeenLine = millis();
    }

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

CRGB parseColor(const char* s) {
  if (strcasecmp(s, "RED") == 0 || strcasecmp(s, "MERAH") == 0) return CRGB::Red;
  if (strcasecmp(s, "GREEN") == 0 || strcasecmp(s, "HIJAU") == 0) return CRGB::Green;
  if (strcasecmp(s, "BLUE") == 0 || strcasecmp(s, "BIRU") == 0) return CRGB::Blue;
  if (strcasecmp(s, "WHITE") == 0 || strcasecmp(s, "PUTIH") == 0) return CRGB::White;
  if (strcasecmp(s, "YELLOW") == 0 || strcasecmp(s, "KUNING") == 0) return CRGB::Yellow;
  if (strcasecmp(s, "ORANGE") == 0 || strcasecmp(s, "JINGGA") == 0) return CRGB::Orange;
  if (strcasecmp(s, "PURPLE") == 0 || strcasecmp(s, "UNGU") == 0) return CRGB::Purple;
  if (strcasecmp(s, "CYAN") == 0) return CRGB::Cyan;
  if (strcasecmp(s, "MAGENTA") == 0) return CRGB::Magenta;
  if (strcasecmp(s, "BLACK") == 0 || strcasecmp(s, "HITAM") == 0 || strcasecmp(s, "OFF") == 0) return CRGB::Black;
  if (isdigit(s[0])) {
    int idx = atoi(s);
    switch (idx) {
      case 0: return CRGB::Black;
      case 1: return CRGB::Red;
      case 2: return CRGB::Green;
      case 3: return CRGB::Blue;
      case 4: return CRGB::Yellow;
      case 5: return CRGB::Cyan;
      case 6: return CRGB::Magenta;
      default: return CRGB::White;
    }
  }
  return CRGB::White;
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

  if (strcasecmp(cmd, "MAJU") == 0) {
    inst.opcode = OP_MAJU;
    inst.p1 = defaultMaju;
  } else if (strcasecmp(cmd, "MUNDUR") == 0) {
    inst.opcode = OP_MUNDUR;
    inst.p1 = defaultMundur;
  } else if (strcasecmp(cmd, "KANAN") == 0) {
    inst.opcode = OP_KANAN;
    inst.p1 = defaultKanan;
  } else if (strcasecmp(cmd, "KIRI") == 0) {
    inst.opcode = OP_KIRI;
    inst.p1 = defaultKiri;
  } else if (strcasecmp(cmd, "STOP") == 0) {
    inst.opcode = OP_STOP;
  } else if (strcasecmp(cmd, "SPEED") == 0) {
    inst.opcode = OP_SPEED;
    inst.p1 = defaultSpeed;
  } else if (strcasecmp(cmd, "TUNGGU") == 0) {
    inst.opcode = OP_TUNGGU;
    inst.p1 = defaultTunggu;
  } else if (strcasecmp(cmd, "UKUR_JARAK") == 0) {
    inst.opcode = OP_UKUR_JARAK;
  } else if (strcasecmp(cmd, "SCAN_KANAN") == 0) {
    inst.opcode = OP_SCAN_KANAN;
    inst.p1 = defaultScanDist;
    inst.p2 = defaultScanAngle;
  } else if (strcasecmp(cmd, "SCAN_KIRI") == 0) {
    inst.opcode = OP_SCAN_KIRI;
    inst.p1 = defaultScanDist;
    inst.p2 = defaultScanAngle;
  } else if (strcasecmp(cmd, "DEKATI_OBJEK") == 0) {
    inst.opcode = OP_DEKATI_OBJEK;
    inst.p1 = defaultDekati;
  } else if (strcasecmp(cmd, "JAUHI_OBJEK") == 0) {
    inst.opcode = OP_JAUHI_OBJEK;
    inst.p1 = defaultJauhi;
  } else if (strcasecmp(cmd, "CEK_OBJEK") == 0) {
    inst.opcode = OP_CEK_OBJEK;
    inst.p1 = defaultCek;
  } else if (strcasecmp(cmd, "BATAS_SCAN") == 0) {
    inst.opcode = OP_BATAS_SCAN;
    inst.p1 = defaultScanAngle;
  } else if (strcasecmp(cmd, "CAPIT") == 0) {
    inst.opcode = OP_CAPIT;
  } else if (strcasecmp(cmd, "LEPAS") == 0) {
    inst.opcode = OP_LEPAS;
  } else if (strcasecmp(cmd, "BUNYI") == 0) {
    inst.opcode = OP_BUNYI;
    inst.p1 = defaultBunyi;
  } else if (strcasecmp(cmd, "BEEP") == 0) {
    inst.opcode = OP_BEEP;
    inst.p1 = defaultBeepFreq;
    inst.p2 = defaultBeepDur;
  } else if (strcasecmp(cmd, "LED") == 0) {
    inst.opcode = OP_LED;
    inst.p1 = 1;
  } else if (strcasecmp(cmd, "LED_WARNA") == 0) {
    inst.opcode = OP_LED_WARNA;
    CRGB defC = CRGB::White;
    inst.p1 = ((uint32_t)defC.r << 16) | ((uint32_t)defC.g << 8) | (uint32_t)defC.b;
  } else if (strcasecmp(cmd, "LED_KEDIP") == 0) {
    inst.opcode = OP_LED_KEDIP;
    inst.p1 = 3;
    inst.p2 = 200;
  } else if (strcasecmp(cmd, "LAPOR_JARAK") == 0) {
    inst.opcode = OP_LAPOR_JARAK;
  } else if (strcasecmp(cmd, "EMERGENCY_STOP") == 0) {
    inst.opcode = OP_EMERGENCY_STOP;
  } else if (strcasecmp(cmd, "END") == 0) {
    inst.opcode = OP_END;
  } else if (strcasecmp(cmd, "SERVO") == 0) {
    inst.opcode = OP_SERVO;
    inst.p1 = defaultServo;
  } else if (strcasecmp(cmd, "FOLLOW_LINE") == 0) {
    inst.opcode = OP_FOLLOW_LINE;
    inst.p1 = defaultFollowLine;
  } else if (strcasecmp(cmd, "MAJU_LINE") == 0) {
    inst.opcode = OP_MAJU_LINE;
    inst.p1 = defaultMajuLine;
  } else if (strcasecmp(cmd, "MUNDUR_LINE") == 0) {
    inst.opcode = OP_MUNDUR_LINE;
    inst.p1 = defaultMundurLine;
  } else if (strcasecmp(cmd, "STOP_LINE") == 0) {
    inst.opcode = OP_STOP_LINE;
  } else if (strcasecmp(cmd, "KALIBRASI") == 0 || strcasecmp(cmd, "CALIBRATE") == 0 || strcasecmp(cmd, "CALIBRASI_LINE") == 0) {
    inst.opcode = OP_KALIBRASI;
  } else if (strcasecmp(cmd, "CONFIG") == 0) {
    inst.opcode = OP_CONFIG;
  } else if (strcasecmp(cmd, "PROD") == 0 || strcasecmp(cmd, "CODE") == 0) {
    return false;
  } else {
    return false;
  }

  if (params != NULL && strlen(params) > 0) {
    if (inst.opcode == OP_LED) {
      if (strcasecmp(params, "ON") == 0 || strcmp(params, "1") == 0) {
        inst.p1 = 1;
      } else if (strcasecmp(params, "OFF") == 0 || strcmp(params, "0") == 0 || strcasecmp(params, "CLEAR") == 0) {
        inst.p1 = 0;
      } else if (strcasecmp(params, "SMILE") == 0 || strcasecmp(params, "SMILEY") == 0) {
        inst.p1 = 2;
      } else if (strcasecmp(params, "FROWN") == 0 || strcasecmp(params, "FROWNING") == 0) {
        inst.p1 = 3;
      } else if (strcasecmp(params, "LEFT") == 0 || strcasecmp(params, "KIRI") == 0) {
        inst.p1 = 4;
      } else if (strcasecmp(params, "RIGHT") == 0 || strcasecmp(params, "KANAN") == 0) {
        inst.p1 = 5;
      } else {
        CRGB c = parseColor(params);
        inst.p1 = 10;
        inst.p2 = ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | (uint32_t)c.b;
      }
    } else if (inst.opcode == OP_LED_WARNA) {
      CRGB c = parseColor(params);
      inst.p1 = ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | (uint32_t)c.b;
    } else if (inst.opcode == OP_FOLLOW_LINE) {
      if (strcasecmp(params, "OFF") == 0 || strcmp(params, "0") == 0) {
        inst.p1 = 0;
      } else if (strcasecmp(params, "ON") == 0) {
        inst.p1 = 9999;
      } else {
        inst.p1 = atol(params);
      }
    } else if (inst.opcode == OP_CONFIG) {
      char* comma = strchr(params, ',');
      if (comma != NULL) {
        *comma = '\0';
        char* key = params;
        char* val = comma + 1;
        while (*val == ' ' || *val == '\t') val++;
        long numVal = atol(val);
        if (strcasecmp(key, "MAJU") == 0) { inst.p1 = OP_MAJU; inst.p2 = numVal; }
        else if (strcasecmp(key, "MUNDUR") == 0) { inst.p1 = OP_MUNDUR; inst.p2 = numVal; }
        else if (strcasecmp(key, "KANAN") == 0) { inst.p1 = OP_KANAN; inst.p2 = numVal; }
        else if (strcasecmp(key, "KIRI") == 0) { inst.p1 = OP_KIRI; inst.p2 = numVal; }
        else if (strcasecmp(key, "SPEED") == 0) { inst.p1 = OP_SPEED; inst.p2 = numVal; }
        else if (strcasecmp(key, "TUNGGU") == 0) { inst.p1 = OP_TUNGGU; inst.p2 = numVal; }
        else if (strcasecmp(key, "BATAS_SCAN") == 0 || strcasecmp(key, "SCAN_SUDUT") == 0) { inst.p1 = OP_BATAS_SCAN; inst.p2 = numVal; }
        else if (strcasecmp(key, "SCAN_JARAK") == 0 || strcasecmp(key, "SCAN") == 0) { inst.p1 = OP_SCAN_KANAN; inst.p2 = numVal; }
        else if (strcasecmp(key, "DEKATI") == 0 || strcasecmp(key, "DEKATI_OBJEK") == 0) { inst.p1 = OP_DEKATI_OBJEK; inst.p2 = numVal; }
        else if (strcasecmp(key, "JAUHI") == 0 || strcasecmp(key, "JAUHI_OBJEK") == 0) { inst.p1 = OP_JAUHI_OBJEK; inst.p2 = numVal; }
        else if (strcasecmp(key, "CEK") == 0 || strcasecmp(key, "CEK_OBJEK") == 0) { inst.p1 = OP_CEK_OBJEK; inst.p2 = numVal; }
        else if (strcasecmp(key, "BUNYI") == 0) { inst.p1 = OP_BUNYI; inst.p2 = numVal; }
        else if (strcasecmp(key, "BEEP") == 0) { inst.p1 = OP_BEEP; inst.p2 = numVal; }
        else if (strcasecmp(key, "SERVO") == 0) { inst.p1 = OP_SERVO; inst.p2 = numVal; }
        else if (strcasecmp(key, "FOLLOW_LINE") == 0 || strcasecmp(key, "LINE_FOLLOWER") == 0) { inst.p1 = OP_FOLLOW_LINE; inst.p2 = numVal; }
        else if (strcasecmp(key, "MAJU_LINE") == 0) { inst.p1 = OP_MAJU_LINE; inst.p2 = numVal; }
        else if (strcasecmp(key, "MUNDUR_LINE") == 0) { inst.p1 = OP_MUNDUR_LINE; inst.p2 = numVal; }
      }
    } else {
      char* comma = strchr(params, ',');
      if (comma != NULL) {
        *comma = '\0';
        char* p1Str = params;
        char* p2Str = comma + 1;
        while (*p2Str == ' ' || *p2Str == '\t') p2Str++;
        inst.p1 = atol(p1Str);
        inst.p2 = atol(p2Str);
      } else {
        inst.p1 = atol(params);
      }
    }
  }

  return true;
}

void executeInstruction(const Instruction& inst) {
  if (emergencyFlag && inst.opcode != OP_EMERGENCY_STOP) return;

  switch (inst.opcode) {
    case OP_MAJU:
      moveMm(inst.p1 * 10, +1);
      break;

    case OP_MUNDUR:
      moveMm(inst.p1 * 10, -1);
      break;

    case OP_KANAN:
      rotateDeg(inst.p1, +1);
      break;

    case OP_KIRI:
      rotateDeg(inst.p1, -1);
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

    case OP_UKUR_JARAK:
    case OP_LAPOR_JARAK: {
      int d = Leanbot.pingCm();
      Serial.print(F("JARAK:"));
      Serial.println(d);
      break;
    }

    case OP_SCAN_KANAN:
      scanObject(+1, inst.p1, inst.p2);
      break;

    case OP_SCAN_KIRI:
      scanObject(-1, inst.p1, inst.p2);
      break;

    case OP_DEKATI_OBJEK:
      approachObject(inst.p1);
      break;

    case OP_JAUHI_OBJEK:
      retreatObject(inst.p1);
      break;

    case OP_CEK_OBJEK: {
      int d = Leanbot.pingCm();
      int limit = inst.p1 > 0 ? inst.p1 : defaultCek;
      if (d > 0 && d <= limit) {
        LbRGB.fillColor(CRGB::Green);
        LbRGB.show();
        playBunyi(3);
      } else {
        LbRGB.fillColor(CRGB::Red);
        LbRGB.show();
        playBunyi(1);
      }
      break;
    }

    case OP_BATAS_SCAN:
      if (inst.p1 > 0) {
        scanMaxAngle = inst.p1;
        defaultScanAngle = inst.p1;
      }
      break;

    case OP_CAPIT:
      LbGripper.close();
      delayInterruptible(400);
      break;

    case OP_LEPAS:
      LbGripper.open();
      delayInterruptible(400);
      break;

    case OP_SERVO:
      LbGripper.close(inst.p1);
      delayInterruptible(400);
      break;

    case OP_BUNYI:
      playBunyi(inst.p1);
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
        CRGB c = CRGB((inst.p2 >> 16) & 0xFF, (inst.p2 >> 8) & 0xFF, inst.p2 & 0xFF);
        LbRGB.fillColor(c);
        LbRGB.show();
      }
      break;

    case OP_LED_WARNA: {
      CRGB c = CRGB((inst.p1 >> 16) & 0xFF, (inst.p1 >> 8) & 0xFF, inst.p1 & 0xFF);
      LbRGB.fillColor(c);
      LbRGB.show();
      break;
    }

    case OP_LED_KEDIP:
      blinkLed(inst.p1, inst.p2);
      break;

    case OP_EMERGENCY_STOP:
      triggerEmergencyStop();
      break;

    case OP_END:
      LbMotion.runLR(0, 0);
      LbMotion.stopAndWait();
      break;

    case OP_FOLLOW_LINE:
      if (inst.p1 > 0) {
        followLineUntilIntersection(inst.p1);
      } else {
        LbMotion.runLR(0, 0);
        LbMotion.stopAndWait();
      }
      break;

    case OP_MAJU_LINE:
      moveToLine(+1, inst.p1);
      break;

    case OP_MUNDUR_LINE:
      moveToLine(-1, inst.p1);
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
      playBunyi(2);
      break;

    case OP_CONFIG: {
      long val = inst.p2;
      switch (inst.p1) {
        case OP_MAJU: if (val > 0) defaultMaju = val; break;
        case OP_MUNDUR: if (val > 0) defaultMundur = val; break;
        case OP_KANAN: if (val > 0) defaultKanan = val; break;
        case OP_KIRI: if (val > 0) defaultKiri = val; break;
        case OP_SPEED:
          if (val > 0) {
            defaultSpeed = val;
            if (val <= 10) currentSpeed = map(val, 1, 10, 400, 2000);
            else if (val <= 255) currentSpeed = map(val, 0, 255, 350, 2000);
            else currentSpeed = constrain(val, 350, 2000);
          }
          break;
        case OP_BATAS_SCAN:
          if (val > 0) {
            defaultScanAngle = val;
            scanMaxAngle = val;
          }
          break;
        case OP_TUNGGU: if (val > 0) defaultTunggu = val; break;
        case OP_SCAN_KANAN:
        case OP_SCAN_KIRI: if (val > 0) defaultScanDist = val; break;
        case OP_DEKATI_OBJEK: if (val > 0) defaultDekati = val; break;
        case OP_JAUHI_OBJEK: if (val > 0) defaultJauhi = val; break;
        case OP_CEK_OBJEK: if (val > 0) defaultCek = val; break;
        case OP_BUNYI: if (val > 0) defaultBunyi = val; break;
        case OP_BEEP: if (val > 0) defaultBeepFreq = val; break;
        case OP_SERVO: if (val >= 0) defaultServo = val; break;
        case OP_FOLLOW_LINE: if (val >= 0) defaultFollowLine = val; break;
        case OP_MAJU_LINE: if (val > 0) defaultMajuLine = val; break;
        case OP_MUNDUR_LINE: if (val > 0) defaultMundurLine = val; break;
      }
      break;
    }

    default:
      break;
  }
}

void processCodePacket(char* payload) {
  while (*payload == ' ' || *payload == '\t' || *payload == ';') payload++;
  if (*payload == '\0') return;

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

  if (commandCount > 0) {
    emergencyFlag = false;
    LbRGB.fillColor(CRGB::Black);
    LbRGB.show();
    Serial.print(F("CODE_START:"));
    Serial.println(commandCount);
    for (int i = 0; i < commandCount; i++) {
      checkEmergency();
      if (emergencyFlag) break;
      executeInstruction(commandQueue[i]);
    }
    if (!emergencyFlag) {
      Serial.println(F("CODE_END"));
    }
  }
}
