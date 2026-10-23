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
  OP_GESER_KANAN,
  OP_GESER_KIRI,
  OP_BUNYI,
  OP_BEEP,
  OP_LED,
  OP_LED_WARNA,
  OP_LED_KEDIP,
  OP_LAPOR_JARAK,
  OP_EMERGENCY_STOP,
  OP_END,
  OP_SERVO,
  OP_CONFIG
};

struct Instruction {
  uint8_t opcode;
  long p1;
  long p2;
};

const int MAX_COMMANDS = 64;
Instruction prodQueue[MAX_COMMANDS];
int prodCount = 0;

int currentSpeed = 1000;
int scanMaxAngle = 90;
volatile bool emergencyFlag = false;

// Default values (synced from Web via CONFIG or individual commands)
int defaultMaju = 10;
int defaultMundur = 10;
int defaultKanan = 90;
int defaultKiri = 90;
int defaultSpeed = 5;
int defaultTunggu = 1000;
int defaultScanDist = 20;
int defaultDekati = 6;
int defaultJauhi = 20;
int defaultCek = 20;
int defaultGeser = 10;
int defaultBunyi = 1;
int defaultBeepFreq = 1500;
int defaultBeepDur = 100;
int defaultServo = 90;

char rxBuf[300];
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
void shiftRight(int cm);
void shiftLeft(int cm);
void playBunyi(int count);
void playBeep(int freq, int duration);
void blinkLed(int count, int intervalMs);
CRGB parseColor(const char* s);
bool isEmergencyString(const char* s);
bool parseInstruction(char* str, Instruction& inst);
void executeInstruction(const Instruction& inst);
void processCommandLine(char* line);

void setup() {
  Leanbot.begin();
  LbRGB.fillColor(CRGB::Black);
  LbRGB.show();
}

void loop() {
  if (LbTouch.onPress(TB1A) || LbTouch.onPress(TB2A)) {
    triggerEmergencyStop();
  }

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (rxLen > 0) {
        rxBuf[rxLen] = '\0';
        processCommandLine(rxBuf);
        rxLen = 0;
      }
    } else {
      if (rxLen < (int)sizeof(rxBuf) - 1) {
        rxBuf[rxLen++] = c;
        rxBuf[rxLen] = '\0';
      }
      lastRxTime = millis();
    }
  }

  if (rxLen > 0 && (millis() - lastRxTime > 100)) {
    rxBuf[rxLen] = '\0';
    processCommandLine(rxBuf);
    rxLen = 0;
  }
}

void triggerEmergencyStop() {
  emergencyFlag = true;
  LbMotion.runLR(0, 0);
  LbMotion.stopAndWait();
  Leanbot.noTone();
  LbRGB.fillColor(CRGB::Red);
  LbRGB.show();
  prodCount = 0;
  Serial.println(F("EMERGENCY_STOP"));
}

bool isEmergencyString(const char* s) {
  while (*s == ' ' || *s == '\t') s++;
  if (strcasecmp(s, "EMERGENCY_STOP") == 0) return true;
  if (strcasecmp(s, "ESTOP") == 0) return true;
  if (strcasecmp(s, "STOP") == 0) return true;
  if (strcasecmp(s, "STP") == 0) return true;
  if (strcasecmp(s, "!") == 0) return true;
  if (strstr(s, "EMERGENCY_STOP") != NULL) return true;
  return false;
}

void checkEmergency() {
  if (LbTouch.onPress(TB1A) || LbTouch.onPress(TB2A)) {
    triggerEmergencyStop();
    return;
  }

  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (rxLen > 0) {
        rxBuf[rxLen] = '\0';
        if (isEmergencyString(rxBuf)) {
          triggerEmergencyStop();
        }
        rxLen = 0;
      }
    } else {
      if (rxLen < (int)sizeof(rxBuf) - 1) {
        rxBuf[rxLen++] = c;
        rxBuf[rxLen] = '\0';
      }
      if (isEmergencyString(rxBuf)) {
        triggerEmergencyStop();
        rxLen = 0;
      }
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
  int spd = currentSpeed;
  if (spd <= 0) spd = 1000;
  LbMotion.runLR(dir * spd, dir * spd);
  LbMotion.setZeroOrigin();
  int step = 2;
  for (int d = step; d <= mm; d += step) {
    LbMotion.waitUntilDistanceMm(dir * d);
    checkEmergency();
    if (emergencyFlag) break;
  }
  if (!emergencyFlag && (mm % step != 0)) {
    LbMotion.waitUntilDistanceMm(dir * mm);
  }
  LbMotion.stopAndWait();
}

void rotateDeg(int deg, int dir) {
  if (emergencyFlag || deg <= 0) return;
  int spd = currentSpeed;
  if (spd <= 0) spd = 1000;
  if (dir > 0) {
    LbMotion.runLR(+spd, -spd);
  } else {
    LbMotion.runLR(-spd, +spd);
  }
  LbMotion.setZeroOrigin();
  int step = 2;
  for (int r = step; r <= deg; r += step) {
    LbMotion.waitUntilRotationDeg(dir * r);
    checkEmergency();
    if (emergencyFlag) break;
  }
  if (!emergencyFlag && (deg % step != 0)) {
    LbMotion.waitUntilRotationDeg(dir * deg);
  }
  LbMotion.stopAndWait();
}

void scanObject(int dir, int maxDistCm, int maxDeg) {
  if (emergencyFlag) return;
  if (maxDistCm <= 0) maxDistCm = 20;
  if (maxDeg <= 0) maxDeg = scanMaxAngle;

  int d = Leanbot.pingCm();
  if (d > 0 && d <= maxDistCm) {
    Serial.print(F("OBJEK:"));
    Serial.println(d);
    return;
  }

  int spd = currentSpeed;
  if (spd <= 0) spd = 600;
  if (spd > 700) spd = 700;

  if (dir > 0) {
    LbMotion.runLR(+spd, -spd);
  } else {
    LbMotion.runLR(-spd, +spd);
  }
  LbMotion.setZeroOrigin();

  int step = 2; // Step rotasi diperkecil (2 deg) agar scan rapat dan tidak ada benda terlewat
  bool found = false;
  int foundDist = 0;

  for (int angle = step; angle <= maxDeg; angle += step) {
    LbMotion.waitUntilRotationDeg(dir * angle);
    checkEmergency();
    if (emergencyFlag) break;

    d = Leanbot.pingCm();
    if (d > 0 && d <= maxDistCm) {
      found = true;
      foundDist = d;
      break;
    }
  }

  if (!emergencyFlag && !found && (maxDeg % step != 0)) {
    LbMotion.waitUntilRotationDeg(dir * maxDeg);
    checkEmergency();
    if (!emergencyFlag) {
      d = Leanbot.pingCm();
      if (d > 0 && d <= maxDistCm) {
        found = true;
        foundDist = d;
      }
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
  if (targetCm <= 0) targetCm = 6;

  int d = Leanbot.pingCm();
  if (d <= 0 || d <= targetCm || d > 80) {
    Serial.print(F("JARAK_AKHIR:"));
    Serial.println(Leanbot.pingCm());
    return;
  }

  int spd = currentSpeed;
  if (spd <= 0) spd = 600;
  if (spd > 700) spd = 700;

  LbMotion.runLR(+spd, +spd);
  LbMotion.setZeroOrigin();

  int stepMm = 5;
  int currentDistanceTraveled = 0;
  int maxTravelMm = 800; // Batas aman maju maksimal 80 cm
  int missCount = 0;

  while (currentDistanceTraveled < maxTravelMm) {
    currentDistanceTraveled += stepMm;
    LbMotion.waitUntilDistanceMm(currentDistanceTraveled);
    checkEmergency();
    if (emergencyFlag) break;

    d = Leanbot.pingCm();

    if (d > 0 && d <= targetCm) {
      break;
    }

    if (d <= 0 || d > 80) {
      missCount++;
      if (missCount >= 3) {
        break;
      }
    } else {
      missCount = 0;
      // Kurangi kecepatan saat mendekati target agar berhenti presisi tanpa overshoot
      if (d - targetCm <= 3 && spd > 350) {
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
  if (targetCm <= 0) targetCm = 20;

  int d = Leanbot.pingCm();
  if (d <= 0 || d >= targetCm || d > 80) {
    Serial.print(F("JARAK_AKHIR:"));
    Serial.println(Leanbot.pingCm());
    return;
  }

  int spd = currentSpeed;
  if (spd <= 0) spd = 600;
  if (spd > 700) spd = 700;

  LbMotion.runLR(-spd, -spd);
  LbMotion.setZeroOrigin();

  int stepMm = 5;
  int currentDistanceTraveled = 0;
  int maxTravelMm = 800; // Batas aman mundur maksimal 80 cm
  int missCount = 0;

  while (currentDistanceTraveled < maxTravelMm) {
    currentDistanceTraveled += stepMm;
    LbMotion.waitUntilDistanceMm(-currentDistanceTraveled);
    checkEmergency();
    if (emergencyFlag) break;

    d = Leanbot.pingCm();

    if (d >= targetCm && d <= 80) {
      break;
    }

    if (d <= 0 || d > 80) {
      missCount++;
      if (missCount >= 3) {
        break;
      }
    } else {
      missCount = 0;
      // Kurangi kecepatan saat hampir mencapai target jarak
      if (targetCm - d <= 3 && spd > 350) {
        spd = 350;
        LbMotion.runLR(-spd, -spd);
      }
    }
  }

  LbMotion.stopAndWait();
  Serial.print(F("JARAK_AKHIR:"));
  Serial.println(Leanbot.pingCm());
}

void shiftRight(int cm) {
  if (emergencyFlag) return;
  if (cm <= 0) cm = 10;
  rotateDeg(90, +1);
  if (emergencyFlag) return;
  moveMm(cm * 10, +1);
  if (emergencyFlag) return;
  rotateDeg(90, -1);
}

void shiftLeft(int cm) {
  if (emergencyFlag) return;
  if (cm <= 0) cm = 10;
  rotateDeg(90, -1);
  if (emergencyFlag) return;
  moveMm(cm * 10, +1);
  if (emergencyFlag) return;
  rotateDeg(90, +1);
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

  if (strcasecmp(cmd, "MAJU") == 0 || strcasecmp(cmd, "FWD") == 0) {
    inst.opcode = OP_MAJU;
    inst.p1 = defaultMaju;
  } else if (strcasecmp(cmd, "MUNDUR") == 0 || strcasecmp(cmd, "BWD") == 0) {
    inst.opcode = OP_MUNDUR;
    inst.p1 = defaultMundur;
  } else if (strcasecmp(cmd, "KANAN") == 0 || strcasecmp(cmd, "RIGHT") == 0) {
    inst.opcode = OP_KANAN;
    inst.p1 = defaultKanan;
  } else if (strcasecmp(cmd, "KIRI") == 0 || strcasecmp(cmd, "LEFT") == 0) {
    inst.opcode = OP_KIRI;
    inst.p1 = defaultKiri;
  } else if (strcasecmp(cmd, "STOP") == 0 || strcasecmp(cmd, "STP") == 0) {
    inst.opcode = OP_STOP;
  } else if (strcasecmp(cmd, "SPEED") == 0 || strcasecmp(cmd, "SPD") == 0) {
    inst.opcode = OP_SPEED;
    inst.p1 = defaultSpeed;
  } else if (strcasecmp(cmd, "TUNGGU") == 0 || strcasecmp(cmd, "DELAY") == 0 || strcasecmp(cmd, "DLY") == 0) {
    inst.opcode = OP_TUNGGU;
    inst.p1 = defaultTunggu;
  } else if (strcasecmp(cmd, "UKUR_JARAK") == 0) {
    inst.opcode = OP_UKUR_JARAK;
  } else if (strcasecmp(cmd, "SCAN_KANAN") == 0) {
    inst.opcode = OP_SCAN_KANAN;
    inst.p1 = defaultScanDist;
    inst.p2 = scanMaxAngle;
  } else if (strcasecmp(cmd, "SCAN_KIRI") == 0) {
    inst.opcode = OP_SCAN_KIRI;
    inst.p1 = defaultScanDist;
    inst.p2 = scanMaxAngle;
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
    inst.p1 = scanMaxAngle;
  } else if (strcasecmp(cmd, "CAPIT") == 0 || strcasecmp(cmd, "GRIPPER_CLOSE") == 0 || strcasecmp(cmd, "GCL") == 0 || strcasecmp(cmd, "ARM_DOWN") == 0 || strcasecmp(cmd, "ADN") == 0) {
    inst.opcode = OP_CAPIT;
  } else if (strcasecmp(cmd, "LEPAS") == 0 || strcasecmp(cmd, "GRIPPER_OPEN") == 0 || strcasecmp(cmd, "GOP") == 0 || strcasecmp(cmd, "ARM_UP") == 0 || strcasecmp(cmd, "AUP") == 0) {
    inst.opcode = OP_LEPAS;
  } else if (strcasecmp(cmd, "GESER_KANAN") == 0) {
    inst.opcode = OP_GESER_KANAN;
    inst.p1 = defaultGeser;
  } else if (strcasecmp(cmd, "GESER_KIRI") == 0) {
    inst.opcode = OP_GESER_KIRI;
    inst.p1 = defaultGeser;
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
  } else if (strcasecmp(cmd, "EMERGENCY_STOP") == 0 || strcasecmp(cmd, "ESTOP") == 0) {
    inst.opcode = OP_EMERGENCY_STOP;
  } else if (strcasecmp(cmd, "END") == 0) {
    inst.opcode = OP_END;
  } else if (strcasecmp(cmd, "SERVO") == 0 || strcasecmp(cmd, "SRV") == 0) {
    inst.opcode = OP_SERVO;
    inst.p1 = defaultServo;
  } else if (strcasecmp(cmd, "CONFIG") == 0 || strcasecmp(cmd, "SET_DEFAULT") == 0 || strcasecmp(cmd, "CFG") == 0) {
    inst.opcode = OP_CONFIG;
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
        else if (strcasecmp(key, "GESER") == 0 || strcasecmp(key, "GESER_KANAN") == 0 || strcasecmp(key, "GESER_KIRI") == 0) { inst.p1 = OP_GESER_KANAN; inst.p2 = numVal; }
        else if (strcasecmp(key, "BUNYI") == 0) { inst.p1 = OP_BUNYI; inst.p2 = numVal; }
        else if (strcasecmp(key, "BEEP") == 0) { inst.p1 = OP_BEEP; inst.p2 = numVal; }
        else if (strcasecmp(key, "SERVO") == 0) { inst.p1 = OP_SERVO; inst.p2 = numVal; }
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

    case OP_UKUR_JARAK: {
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
      if (d > 0 && d <= inst.p1) {
        Serial.print(F("OBJEK_TERDETEKSI:"));
        Serial.println(d);
      } else {
        Serial.println(F("TIDAK_ADA_OBJEK"));
      }
      break;
    }

    case OP_BATAS_SCAN:
      if (inst.p1 > 0) scanMaxAngle = inst.p1;
      break;

    case OP_CAPIT:
      LbGripper.close();
      delayInterruptible(400);
      break;

    case OP_LEPAS:
      LbGripper.open();
      delayInterruptible(400);
      break;

    case OP_GESER_KANAN:
      shiftRight(inst.p1);
      break;

    case OP_GESER_KIRI:
      shiftLeft(inst.p1);
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

    case OP_LAPOR_JARAK: {
      int d = Leanbot.pingCm();
      Serial.print(F("JARAK:"));
      Serial.println(d);
      break;
    }

    case OP_EMERGENCY_STOP:
      triggerEmergencyStop();
      break;

    case OP_END:
      LbMotion.runLR(0, 0);
      LbMotion.stopAndWait();
      break;

    case OP_SERVO:
      LbGripper.close(inst.p1);
      delayInterruptible(400);
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
        case OP_GESER_KANAN:
        case OP_GESER_KIRI: if (val > 0) defaultGeser = val; break;
        case OP_BUNYI: if (val > 0) defaultBunyi = val; break;
        case OP_BEEP: if (val > 0) defaultBeepFreq = val; break;
        case OP_SERVO: if (val >= 0) defaultServo = val; break;
      }
      break;
    }

    default:
      break;
  }
}

void processCommandLine(char* line) {
  while (*line == ' ' || *line == '\t') line++;
  if (*line == '\0') return;

  if (isEmergencyString(line)) {
    triggerEmergencyStop();
    return;
  }

  bool isProd = false;
  if (strncasecmp(line, "PROD", 4) == 0) {
    char after = line[4];
    if (after == ';' || after == ':' || after == ' ' || after == '\t' || after == '\0') {
      isProd = true;
      line += 4;
      while (*line == ';' || *line == ':' || *line == ' ' || *line == '\t') line++;
    }
  }

  if (isProd) {
    prodCount = 0;
    char* p = line;
    while (*p != '\0') {
      char* seg = p;
      while (*p != '\0' && *p != ';') p++;
      if (*p == ';') {
        *p = '\0';
        p++;
      }
      Instruction inst;
      if (parseInstruction(seg, inst)) {
        if (prodCount < MAX_COMMANDS) {
          prodQueue[prodCount++] = inst;
        }
      }
      while (*p == ' ' || *p == '\t') p++;
    }

    if (prodCount > 0) {
      emergencyFlag = false;
      LbRGB.fillColor(CRGB::Black);
      LbRGB.show();
      Serial.print(F("PROD_START:"));
      Serial.println(prodCount);
      for (int i = 0; i < prodCount; i++) {
        checkEmergency();
        if (emergencyFlag) break;
        executeInstruction(prodQueue[i]);
      }
      if (!emergencyFlag) {
        Serial.println(F("PROD_END"));
      }
    }
  } else {
    char* p = line;
    while (*p != '\0' && *p != ';') p++;
    *p = '\0';
    Instruction inst;
    if (parseInstruction(line, inst)) {
      emergencyFlag = false;
      LbRGB.fillColor(CRGB::Black);
      LbRGB.show();
      Serial.println(F("TEST_START"));
      executeInstruction(inst);
      if (!emergencyFlag) {
        Serial.println(F("TEST_END"));
      }
    }
  }
}
