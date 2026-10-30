import remoteDebugService from "@/services/remote-debug.service.js";
import instructionDefaultsService from "@/services/instruction-defaults.service.js";

class CodeGeneratorService {
  escapeCppString(str) {
    if (!str) return "";
    return str
      .replace(/\\/g, "\\\\")
      .replace(/"/g, '\\"')
      .replace(/\r/g, "")
      .replace(/\n/g, "");
  }

  getDefaultSpeedFromSettings() {
    try {
      const rawLevel = instructionDefaultsService.getDefault("SPEED", "level");
      const numLevel = Number(rawLevel);
      if (numLevel > 10) {
        return numLevel;
      } else if (numLevel > 0) {
        return Math.round(400 + ((numLevel - 1) / 9) * (2000 - 400));
      }
    } catch (e) {
      return 1000;
    }
    return 1000;
  }

  generateCppCode(options = {}) {
    let state = { batches: Array(10).fill(""), batchesEnabled: Array(10).fill(true) };
    try {
      if (typeof window !== "undefined" && window.localStorage) {
        state = remoteDebugService.readState();
      }
    } catch (e) {
      state = { batches: Array(10).fill(""), batchesEnabled: Array(10).fill(true) };
    }
    const batches = Array.isArray(options.batches) ? options.batches : state.batches;
    const batchesEnabled = Array.isArray(options.batchesEnabled) ? options.batchesEnabled : state.batchesEnabled;
    const defaultSpeed = options.defaultSpeed || this.getDefaultSpeedFromSettings();

    const batchDeclarations = [];
    for (let i = 0; i < 10; i++) {
      const isEnabled = batchesEnabled[i] !== false;
      const rawContent = batches[i] || "";
      const content = isEnabled ? rawContent : "";
      const escaped = this.escapeCppString(content.trim());
      batchDeclarations.push(`const char EMBEDDED_BATCH_${i + 1}[] PROGMEM = "${escaped}";`);
    }

    const batchArrayItems = [];
    for (let i = 0; i < 10; i++) {
      batchArrayItems.push(`  EMBEDDED_BATCH_${i + 1}`);
    }

    return `#include <Leanbot.h>
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
  OP_BUNYI,
  OP_LED_WARNA,
  OP_LED_KEDIP,
  OP_EMERGENCY_STOP,
  OP_END,
  OP_FOLLOW_LINE,
  OP_MAJU_LINE,
  OP_MUNDUR_LINE,
  OP_STOP_LINE,
  OP_KALIBRASI,
  OP_MISI_DONE
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
  int8_t p3;
};

#define MAX_BATCHES 10
#define MAX_COMMANDS_PER_BATCH 12

struct Batch {
  Instruction commands[MAX_COMMANDS_PER_BATCH];
  uint8_t count;
};

Batch batchQueue[MAX_BATCHES];
int16_t currentSpeed = ${defaultSpeed};
RobotState currentRobotState = STATE_READY;
bool missionStarted = false;

${batchDeclarations.join("\n")}

const char* const EMBEDDED_BATCHES[MAX_BATCHES] PROGMEM = {
${batchArrayItems.join(",\n")}
};

int freeRam();
int16_t toMotorSpeed(int16_t spd);
bool isLineDetected(int line);
void updateRobotState(RobotState newState);
void printOpcodeName(uint8_t op);
void printInstructionInfo(const Instruction& inst);
void clearAllBatches();
void flushBluetoothSerial();
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
const char* getMissionKey(uint8_t idx);
bool parseInstruction(char* str, Instruction& inst);
void loadBatchFromProgmem(uint8_t bIdx, const char* pProgmemStr);
void loadAllEmbeddedBatches();
void executeInstruction(const Instruction& inst);
void executeAllBatches();

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
    case OP_BUNYI: Serial.print(F("BUNYI")); break;
    case OP_LED_WARNA: Serial.print(F("LED_WARNA")); break;
    case OP_LED_KEDIP: Serial.print(F("LED_KEDIP")); break;
    case OP_EMERGENCY_STOP: Serial.print(F("EMG_STOP")); break;
    case OP_END: Serial.print(F("END")); break;
    case OP_FOLLOW_LINE: Serial.print(F("FOLLOW_LINE")); break;
    case OP_MAJU_LINE: Serial.print(F("MAJU_LINE")); break;
    case OP_MUNDUR_LINE: Serial.print(F("MUNDUR_LINE")); break;
    case OP_STOP_LINE: Serial.print(F("STOP_LINE")); break;
    case OP_KALIBRASI: Serial.print(F("KALIBRASI")); break;
    case OP_MISI_DONE: Serial.print(F("MISI_DONE")); break;
    default: Serial.print(op); break;
  }
}

void printInstructionInfo(const Instruction& inst) {
  if (inst.opcode == OP_MISI_DONE) {
    Serial.print(F("MISI_DONE:"));
    Serial.print(getMissionKey(inst.p1));
    Serial.print(',');
    Serial.print(inst.p2 != 0 ? 1 : 0);
    return;
  }
  printOpcodeName(inst.opcode);
  if (inst.p1 != 0 || inst.p2 != 0 || inst.p3 != 0) {
    Serial.print(':');
    Serial.print(inst.p1);
    if (inst.p2 != 0 || inst.p3 < 0) {
      Serial.print(',');
      Serial.print(inst.p2);
      if (inst.p3 < 0) {
        Serial.print(F(",MUNDUR"));
      }
    }
  }
}

void clearAllBatches() {
  for (uint8_t b = 0; b < MAX_BATCHES; b++) {
    batchQueue[b].count = 0;
  }
}

void flushBluetoothSerial() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

void delayInterruptible(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    flushBluetoothSerial();
    delay(5);
  }
}

void moveMm(int mm, int dir, int speed) {
  if (mm <= 0) return;
  int spd = speed > 0 ? toMotorSpeed(speed) : (currentSpeed > 0 ? currentSpeed : 1000);
  LbMotion.runLR(dir * spd, dir * spd);
  int remaining = mm;
  while (remaining > 0) {
    flushBluetoothSerial();
    int step = remaining > 10 ? 10 : remaining;
    LbMotion.waitDistanceMm(dir * step);
    remaining -= step;
  }
  LbMotion.stopAndWait();
}

void rotateDeg(int deg, int dir, int speed) {
  if (deg <= 0) return;
  int spd = speed > 0 ? toMotorSpeed(speed) : (currentSpeed > 0 ? currentSpeed : 1000);
  if (dir > 0) {
    LbMotion.runLR(+spd, -spd);
  } else {
    LbMotion.runLR(-spd, +spd);
  }
  int remaining = deg;
  while (remaining > 0) {
    flushBluetoothSerial();
    int step = remaining > 10 ? 10 : remaining;
    LbMotion.waitRotationDeg(dir * step);
    remaining -= step;
  }
  LbMotion.stopAndWait();
}

void pivotTurnDeg(int deg, int turnDir, int moveDir, int speed) {
  if (deg <= 0) return;
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
  while (remaining > 0) {
    flushBluetoothSerial();
    int step = remaining > 10 ? 10 : remaining;
    LbMotion.waitRotationDeg(rotDir * step);
    remaining -= step;
  }
  LbMotion.stopAndWait();
}

void moveToLine(int dir, int maxDistCm, int speed) {
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

  while (traveledMm < maxMm) {
    flushBluetoothSerial();
    LbMotion.waitDistanceMm(dir * 2);
    traveledMm += 2;

    int line = LbIRLine.read();
    LbIRLine.displayOnRGB(line, CRGB::Blue);

    if (isLineDetected(line)) {
      found = true;
      break;
    }
  }

  LbMotion.stopAndWait();
  LbRGB.fillColor(found ? CRGB::Green : CRGB::Red);
  LbRGB.show();
  playBunyi(1);
}

void followLineUntilIntersection(int targetIntersections, int speed, int dir) {
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

  while (true) {
    flushBluetoothSerial();

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
  LbRGB.fillColor(CRGB::Green);
  LbRGB.show();
  playBunyi(1);
}

void playBunyi(int count) {
  if (count <= 0) count = 1;
  for (int i = 0; i < count; i++) {
    Leanbot.tone(1500, 100);
    delayInterruptible(150);
  }
}

void blinkLed(int count, int intervalMs) {
  if (count <= 0) count = 3;
  if (intervalMs <= 0) intervalMs = 200;
  for (int i = 0; i < count; i++) {
    LbRGB.fillColor(CRGB::Yellow);
    LbRGB.show();
    delayInterruptible(intervalMs);
    LbRGB.fillColor(CRGB::Black);
    LbRGB.show();
    delayInterruptible(intervalMs);
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

const char* getMissionKey(uint8_t idx) {
  switch (idx) {
    case 1: return "neutral_ungu";
    case 2: return "neutral_hijau";
    case 3: return "neutral_biru";
    case 4: return "analysis_normal";
    case 5: return "analysis_mystery";
    case 6: return "containment_merah";
    case 7: return "containment_kuning";
    case 8: return "crl_leanbot_a";
    case 9: return "crl_leanbot_b";
    default: return "neutral_ungu";
  }
}

bool parseInstruction(char* str, Instruction& inst) {
  while (*str == ' ' || *str == '\\t') str++;
  if (*str == '\\0') return false;

  char* end = str + strlen(str) - 1;
  while (end > str && (*end == ' ' || *end == '\\t' || *end == ';' || *end == '\\r' || *end == '\\n')) {
    *end = '\\0';
    end--;
  }

  char* colon = strchr(str, ':');
  char* cmd = str;
  char* params = NULL;
  if (colon != NULL) {
    *colon = '\\0';
    params = colon + 1;
    while (*params == ' ' || *params == '\\t') params++;
  }

  char* endCmd = cmd + strlen(cmd) - 1;
  while (endCmd > cmd && (*endCmd == ' ' || *endCmd == '\\t')) {
    *endCmd = '\\0';
    endCmd--;
  }

  inst.opcode = OP_NONE;
  inst.p1 = 0;
  inst.p2 = 0;
  inst.p3 = 1;

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
    inst.p3 = 1;
  } else if (strcasecmp_P(cmd, PSTR("BELOK_KANAN_MUNDUR")) == 0 || strcasecmp_P(cmd, PSTR("PIVOT_KANAN_MUNDUR")) == 0) {
    inst.opcode = OP_BELOK_KANAN;
    inst.p3 = -1;
  } else if (strcasecmp_P(cmd, PSTR("BELOK_KIRI")) == 0 || strcasecmp_P(cmd, PSTR("PIVOT_KIRI")) == 0) {
    inst.opcode = OP_BELOK_KIRI;
    inst.p3 = 1;
  } else if (strcasecmp_P(cmd, PSTR("BELOK_KIRI_MUNDUR")) == 0 || strcasecmp_P(cmd, PSTR("PIVOT_KIRI_MUNDUR")) == 0) {
    inst.opcode = OP_BELOK_KIRI;
    inst.p3 = -1;
  } else if (strcasecmp_P(cmd, PSTR("BELOK")) == 0) {
    inst.opcode = OP_BELOK_KANAN;
    inst.p3 = 1;
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
    inst.p3 = 1;
  } else if (strcasecmp_P(cmd, PSTR("FOLLOW_LINE_MUNDUR")) == 0) {
    inst.opcode = OP_FOLLOW_LINE;
    inst.p3 = -1;
  } else if (strcasecmp_P(cmd, PSTR("MAJU_LINE")) == 0) {
    inst.opcode = OP_MAJU_LINE;
  } else if (strcasecmp_P(cmd, PSTR("MUNDUR_LINE")) == 0) {
    inst.opcode = OP_MUNDUR_LINE;
  } else if (strcasecmp_P(cmd, PSTR("STOP_LINE")) == 0) {
    inst.opcode = OP_STOP_LINE;
  } else if (strcasecmp_P(cmd, PSTR("KALIBRASI")) == 0 || strcasecmp_P(cmd, PSTR("CALIBRATE")) == 0) {
    inst.opcode = OP_KALIBRASI;
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
    }

    if (inst.opcode == OP_LED_WARNA) {
      char* comma = strchr(params, ',');
      if (comma != NULL) *comma = '\\0';
      inst.p1 = parseColorIndex(params);
      return true;
    }

    if (inst.opcode == OP_MISI_DONE) {
      char* comma = strchr(params, ',');
      if (comma != NULL) {
        *comma = '\\0';
        char* statusStr = comma + 1;
        while (*statusStr == ' ' || *statusStr == '\\t') statusStr++;
        inst.p1 = parseMissionIndex(params);
        if (strcasecmp_P(statusStr, PSTR("true")) == 0 || strcmp_P(statusStr, PSTR("1")) == 0 || strcasecmp_P(statusStr, PSTR("done")) == 0) {
          inst.p2 = 1;
        } else {
          inst.p2 = 0;
        }
      } else {
        inst.p1 = parseMissionIndex(params);
        inst.p2 = 1;
      }
      return true;
    }

    char* p1Str = params;
    char* comma1 = strchr(p1Str, ',');
    if (comma1 != NULL) {
      *comma1 = '\\0';
      char* p2Str = comma1 + 1;
      while (*p2Str == ' ' || *p2Str == '\\t') p2Str++;

      char* comma2 = strchr(p2Str, ',');
      if (comma2 != NULL) {
        *comma2 = '\\0';
        char* p3Str = comma2 + 1;
        while (*p3Str == ' ' || *p3Str == '\\t') p3Str++;

        inst.p1 = atoi(p1Str);
        inst.p2 = atoi(p2Str);
        if (strcasecmp_P(p3Str, PSTR("MUNDUR")) == 0 || atoi(p3Str) < 0) {
          inst.p3 = -1;
        } else {
          inst.p3 = 1;
        }
      } else {
        inst.p1 = atoi(p1Str);
        if (strcasecmp_P(p2Str, PSTR("MUNDUR")) == 0) {
          inst.p3 = -1;
          inst.p2 = 0;
        } else {
          inst.p2 = atoi(p2Str);
        }
      }
    } else {
      inst.p1 = atoi(p1Str);
    }
  }

  return true;
}

void loadBatchFromProgmem(uint8_t bIdx, const char* pProgmemStr) {
  if (bIdx >= MAX_BATCHES || pProgmemStr == NULL) return;
  char buf[256];
  strncpy_P(buf, pProgmemStr, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\\0';
  batchQueue[bIdx].count = 0;
  if (buf[0] == '\\0') return;
  char* token = strtok(buf, ";\\r\\n");
  while (token != NULL) {
    while (*token == ' ' || *token == '\\t') token++;
    if (*token != '\\0' && strncasecmp_P(token, PSTR("BATCH:"), 6) != 0 && strcasecmp_P(token, PSTR("END")) != 0) {
      Instruction inst;
      if (parseInstruction(token, inst) && inst.opcode != OP_NONE) {
        if (batchQueue[bIdx].count < MAX_COMMANDS_PER_BATCH) {
          batchQueue[bIdx].commands[batchQueue[bIdx].count++] = inst;
        }
      }
    }
    token = strtok(NULL, ";\\r\\n");
  }
}

void loadAllEmbeddedBatches() {
  clearAllBatches();
  for (uint8_t i = 0; i < MAX_BATCHES; i++) {
    const char* p = (const char*)pgm_read_ptr(&(EMBEDDED_BATCHES[i]));
    if (p != NULL) {
      loadBatchFromProgmem(i, p);
    }
  }
}

void executeInstruction(const Instruction& inst) {
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
      pivotTurnDeg(inst.p1, +1, inst.p3, inst.p2);
      break;

    case OP_BELOK_KIRI:
      pivotTurnDeg(inst.p1, -1, inst.p3, inst.p2);
      break;

    case OP_STOP:
      LbMotion.runLR(0, 0);
      LbMotion.stopAndWait();
      break;

    case OP_SPEED:
      if (inst.p1 > 0) currentSpeed = toMotorSpeed(inst.p1);
      break;

    case OP_TUNGGU:
      delayInterruptible(inst.p1 > 0 ? inst.p1 : 100);
      break;

    case OP_CAPIT:
      LbGripper.close();
      delayInterruptible(300);
      break;

    case OP_LEPAS:
      LbGripper.open();
      delayInterruptible(300);
      break;

    case OP_BUNYI:
      playBunyi(inst.p1 > 0 ? inst.p1 : 1);
      break;

    case OP_LED_WARNA:
      LbRGB.fillColor(getColorFromIndex((uint8_t)inst.p1));
      LbRGB.show();
      break;

    case OP_LED_KEDIP:
      blinkLed(inst.p1 > 0 ? inst.p1 : 3, inst.p2 > 0 ? inst.p2 : 200);
      break;

    case OP_FOLLOW_LINE:
      followLineUntilIntersection(inst.p1 > 0 ? inst.p1 : 1, inst.p2, inst.p3);
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
      playBunyi(2);
      break;

    case OP_MISI_DONE:
      Serial.print(F("IOT : {\\"key\\":\\""));
      Serial.print(getMissionKey(inst.p1));
      Serial.print(F("\\",\\"done\\":"));
      Serial.print(inst.p2 != 0 ? F("true") : F("false"));
      Serial.println(F("}"));
      break;

    default:
      break;
  }
}

void executeAllBatches() {
  updateRobotState(STATE_EXECUTING);
  LbRGB.fillColor(CRGB::Black);
  LbRGB.show();
  Serial.print(F("MISSION_START,RAM:"));
  Serial.println(freeRam());

  for (uint8_t b = 0; b < MAX_BATCHES; b++) {
    flushBluetoothSerial();
    if (batchQueue[b].count == 0) {
      continue;
    }

    Serial.print(F("[START_BATCH] "));
    Serial.print(b + 1);
    Serial.print(F(",CMDS:"));
    Serial.println(batchQueue[b].count);

    for (uint8_t i = 0; i < batchQueue[b].count; i++) {
      flushBluetoothSerial();
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
    }

    LbMotion.runLR(0, 0);
    LbMotion.stopAndWait();
    Serial.print(F("[BATCH_DONE] "));
    Serial.println(b + 1);
    delayInterruptible(100);
  }

  updateRobotState(STATE_READY);
  LbRGB.fillColor(CRGB::Green);
  LbRGB.show();
  playBunyi(2);
  Serial.println(F("ALL_BATCHES_DONE"));
}

void setup() {
  Leanbot.begin();
  LbIRLine.doManualCalibration(TB1A);
  delay(200);
  LbRGB.fillColor(CRGB::Black);
  LbRGB.show();
  Leanbot.tone(1500, 100);
  loadAllEmbeddedBatches();
  flushBluetoothSerial();
  Serial.print(F("FREE_RAM:"));
  Serial.println(freeRam());
  updateRobotState(STATE_READY);
}

void loop() {
  flushBluetoothSerial();

  if (!missionStarted) {
    delay(3000);
    missionStarted = true;
    executeAllBatches();
  }
}
`;
  }
}

const codeGeneratorService = new CodeGeneratorService();
export default codeGeneratorService;
