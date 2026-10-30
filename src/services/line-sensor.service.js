import BleService from "@/services/ble.services.js";

const STORAGE_KEY_LINE_SENSOR = "airoc_line_sensor_data";

class LineSensorService {
  constructor() {
    this.listeners = new Set();
    this.rxBuffer = "";

    const saved = this._load();
    this.irLevel = saved.irLevel || [302, 128, 94, 112];
    this.irMin = saved.irMin || [191, 85, 63, 80];
    this.irMax = saved.irMax || [768, 284, 218, 198];
    this.irNow = saved.irNow || [768, 284, 218, 198];
    this.threshold = saved.threshold || [302, 128, 94, 112];
    this.hasReceivedLevel = Boolean(saved.hasReceivedLevel);
    this.lastCalibratedAt = saved.lastCalibratedAt || null;
    this.isCalibrating = false;
    this.statusMessage = "";

    this._onBleData = this._onBleData.bind(this);
    if (typeof window !== "undefined" && BleService && typeof BleService.onData === "function") {
      BleService.onData(this._onBleData);
    }
  }

  _load() {
    try {
      if (typeof window !== "undefined" && window.localStorage) {
        const raw = localStorage.getItem(STORAGE_KEY_LINE_SENSOR);
        if (raw) {
          const parsed = JSON.parse(raw);
          if (parsed && typeof parsed === "object") {
            return parsed;
          }
        }
      }
    } catch (e) {
      console.error(e);
    }
    return {};
  }

  _save() {
    try {
      if (typeof window !== "undefined" && window.localStorage) {
        const payload = {
          irLevel: this.irLevel,
          irMin: this.irMin,
          irMax: this.irMax,
          irNow: this.irNow,
          threshold: this.threshold,
          hasReceivedLevel: this.hasReceivedLevel,
          lastCalibratedAt: this.lastCalibratedAt,
        };
        localStorage.setItem(STORAGE_KEY_LINE_SENSOR, JSON.stringify(payload));
      }
    } catch (e) {
      console.error(e);
    }
  }

  _notify() {
    const state = this.getState();
    this.listeners.forEach((listener) => {
      try {
        listener(state);
      } catch (e) {
        console.error(e);
      }
    });
  }

  subscribe(listener) {
    this.listeners.add(listener);
    return () => {
      this.listeners.delete(listener);
    };
  }

  getState() {
    return {
      irLevel: [...this.irLevel],
      irMin: [...this.irMin],
      irMax: [...this.irMax],
      irNow: [...this.irNow],
      threshold: [...this.threshold],
      hasReceivedLevel: this.hasReceivedLevel,
      lastCalibratedAt: this.lastCalibratedAt,
      isCalibrating: this.isCalibrating,
      statusMessage: this.statusMessage,
    };
  }

  getIrLevel() {
    return [...this.irLevel];
  }

  getThreshold() {
    return [...this.threshold];
  }

  _onBleData(chunk) {
    if (typeof chunk !== "string") return;
    this.rxBuffer += chunk;

    let nlIndex = -1;
    while ((nlIndex = this.rxBuffer.indexOf("\n")) !== -1) {
      const line = this.rxBuffer.substring(0, nlIndex).trim();
      this.rxBuffer = this.rxBuffer.substring(nlIndex + 1);
      if (line.length > 0) {
        this.processIncomingLine(line);
      }
    }

    if (this.rxBuffer.length > 2000) {
      this.rxBuffer = this.rxBuffer.slice(-500);
    }
  }

  processIncomingLine(rawLine) {
    if (!rawLine || typeof rawLine !== "string") return;
    const trimmed = rawLine.trim();

    if (trimmed.includes("[CALIBRATION_START]")) {
      this.isCalibrating = true;
      this.statusMessage = "Mode kalibrasi sensor dimulai...";
      this._notify();
      return;
    }

    if (trimmed.includes("[CALIBRATION_DONE]")) {
      this.isCalibrating = false;
      this.statusMessage = "Kalibrasi sensor manual selesai!";
      this._notify();
      return;
    }

    const threshSetMatch = trimmed.match(/\[THRESHOLD_SET:\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\]/i);
    if (threshSetMatch) {
      const t0 = parseInt(threshSetMatch[1], 10);
      const t1 = parseInt(threshSetMatch[2], 10);
      const t2 = parseInt(threshSetMatch[3], 10);
      const t3 = parseInt(threshSetMatch[4], 10);
      this.threshold = [t0, t1, t2, t3];
      this.statusMessage = `Threshold dikonfirmasi robot: (${t0}, ${t1}, ${t2}, ${t3})`;
      this._save();
      this._notify();
      return;
    }

    const irRegex = /\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)\s*=\s*(irLevel|irNow|irMax|irMin)/i;
    const match = trimmed.match(irRegex);
    if (match) {
      const vals = [
        parseInt(match[1], 10),
        parseInt(match[2], 10),
        parseInt(match[3], 10),
        parseInt(match[4], 10),
      ];
      const kind = match[5].toLowerCase();

      if (kind === "irlevel") {
        this.irLevel = vals;
        this.hasReceivedLevel = true;
        this.lastCalibratedAt = new Date().toLocaleTimeString();
        this.statusMessage = `IR Level diterima: (${vals.join(", ")})`;
        this._save();
        this._notify();
      } else if (kind === "irnow") {
        this.irNow = vals;
        this._notify();
      } else if (kind === "irmax") {
        this.irMax = vals;
        this._save();
        this._notify();
      } else if (kind === "irmin") {
        this.irMin = vals;
        this._save();
        this._notify();
      }
      return;
    }

    const altLevelMatch = trimmed.match(/^IR_LEVEL\s*:\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)/i);
    if (altLevelMatch) {
      this.irLevel = [
        parseInt(altLevelMatch[1], 10),
        parseInt(altLevelMatch[2], 10),
        parseInt(altLevelMatch[3], 10),
        parseInt(altLevelMatch[4], 10),
      ];
      this.hasReceivedLevel = true;
      this.lastCalibratedAt = new Date().toLocaleTimeString();
      this.statusMessage = `IR Level diterima: (${this.irLevel.join(", ")})`;
      this._save();
      this._notify();
    }
  }

  async triggerCalibration(targetDeviceId = "all") {
    this.isCalibrating = true;
    this.statusMessage = "Mengirim instruksi KALIBRASI ke Leanbot...";
    this._notify();
    try {
      await BleService.sendCommand("KALIBRASI", targetDeviceId);
    } catch (err) {
      this.isCalibrating = false;
      this.statusMessage = `Gagal kirim instruksi kalibrasi: ${err.message || err}`;
      this._notify();
      throw err;
    }
  }

  async setThreshold(customValues = null, targetDeviceId = "all") {
    let vals = Array.isArray(customValues) && customValues.length === 4
      ? customValues.map((v) => Number(v) || 0)
      : [...this.irLevel];

    const cmd = `SET_THRESHOLD:${vals.join(",")}`;
    this.statusMessage = `Mengirim threshold (${vals.join(", ")}) ke Leanbot...`;
    this._notify();

    try {
      await BleService.sendCommand(cmd, targetDeviceId);
      this.threshold = [...vals];
      this._save();
      this.statusMessage = `Berhasil menerapkan threshold: LbIRLine.setThreshold(${vals.join(", ")})`;
      this._notify();
      return vals;
    } catch (err) {
      this.statusMessage = `Gagal mengirim threshold: ${err.message || err}`;
      this._notify();
      throw err;
    }
  }

  formatCppCode(customValues = null) {
    const vals = Array.isArray(customValues) && customValues.length === 4
      ? customValues
      : this.threshold;
    return `LbIRLine.setThreshold(${vals.join(", ")});`;
  }
}

const lineSensorService = new LineSensorService();
export default lineSensorService;
