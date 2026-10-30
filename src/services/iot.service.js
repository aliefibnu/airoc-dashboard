import BleService from "@/services/ble.services.js";
import { MISSION_LIST } from "@/data/missions.js";

const STORAGE_KEY_PINS = "airoc_iot_virtual_pins";
const STORAGE_KEY_LOGS = "airoc_iot_logs";

class IotService {
  constructor() {
    this.listeners = new Set();
    this.pins = this._loadPins();
    this.logs = this._loadLogs();
    this.rxBuffer = "";

    this._onBleData = this._onBleData.bind(this);
    if (typeof window !== "undefined" && BleService && typeof BleService.onData === "function") {
      BleService.onData(this._onBleData);
    }
  }

  _loadPins() {
    try {
      const raw = localStorage.getItem(STORAGE_KEY_PINS);
      if (raw) {
        const parsed = JSON.parse(raw);
        if (Array.isArray(parsed)) {
          return parsed;
        }
      }
    } catch (e) {
      console.error(e);
    }
    return [];
  }

  _savePins() {
    try {
      localStorage.setItem(STORAGE_KEY_PINS, JSON.stringify(this.pins));
    } catch (e) {
      console.error(e);
    }
  }

  _loadLogs() {
    try {
      const raw = localStorage.getItem(STORAGE_KEY_LOGS);
      if (raw) {
        const parsed = JSON.parse(raw);
        if (Array.isArray(parsed)) {
          return parsed.slice(0, 50);
        }
      }
    } catch (e) {
      console.error(e);
    }
    return [];
  }

  _saveLogs() {
    try {
      localStorage.setItem(STORAGE_KEY_LOGS, JSON.stringify(this.logs.slice(0, 50)));
    } catch (e) {
      console.error(e);
    }
  }

  _notify() {
    const state = {
      pins: this.getPins(),
      logs: this.getLogs(),
    };
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

  getPins() {
    return JSON.parse(JSON.stringify(this.pins));
  }

  getLogs() {
    return JSON.parse(JSON.stringify(this.logs));
  }

  getNextPinNumber() {
    const existing = new Set(this.pins.map((p) => Number(p.pinNumber)).filter((n) => !isNaN(n)));
    let num = 0;
    while (existing.has(num)) {
      num++;
    }
    return num;
  }

  addPin({ pinNumber, name, key, done = false, enabled = true } = {}) {
    const assignedNumber = pinNumber !== undefined && pinNumber !== null && !isNaN(Number(pinNumber))
      ? Number(pinNumber)
      : this.getNextPinNumber();

    const cleanKey = String(key || "neutral_ungu").trim();
    const missionInfo = MISSION_LIST.find((m) => m.key === cleanKey);
    const assignedName = name && String(name).trim().length > 0
      ? String(name).trim()
      : (missionInfo ? missionInfo.title : `Virtual Pin V${assignedNumber}`);

    const newPin = {
      id: `pin_${Date.now()}_${Math.random().toString(36).slice(2, 7)}`,
      pinNumber: assignedNumber,
      pinLabel: `V${assignedNumber}`,
      name: assignedName,
      key: cleanKey,
      done: Boolean(done),
      enabled: enabled !== false,
      lastUpdated: null,
      updateCount: 0,
    };

    this.pins.push(newPin);
    this._savePins();
    this._notify();
    return newPin;
  }

  removePin(pinId) {
    const prevLen = this.pins.length;
    this.pins = this.pins.filter((p) => p.id !== pinId);
    if (this.pins.length !== prevLen) {
      this._savePins();
      this._notify();
      return true;
    }
    return false;
  }

  updatePin(pinId, updates = {}) {
    const pin = this.pins.find((p) => p.id === pinId);
    if (!pin) return null;

    if (updates.name !== undefined) pin.name = String(updates.name).trim();
    if (updates.key !== undefined) pin.key = String(updates.key).trim();
    if (updates.pinNumber !== undefined && !isNaN(Number(updates.pinNumber))) {
      pin.pinNumber = Number(updates.pinNumber);
      pin.pinLabel = `V${pin.pinNumber}`;
    }
    if (updates.enabled !== undefined) pin.enabled = Boolean(updates.enabled);
    if (updates.done !== undefined) {
      pin.done = Boolean(updates.done);
      pin.lastUpdated = new Date().toISOString();
      pin.updateCount = (pin.updateCount || 0) + 1;
    }

    this._savePins();
    this._notify();
    return { ...pin };
  }

  togglePinEnabled(pinId) {
    const pin = this.pins.find((p) => p.id === pinId);
    if (!pin) return false;
    pin.enabled = !pin.enabled;
    this._savePins();
    this._notify();
    return pin.enabled;
  }

  setPinEnabled(pinId, enabled) {
    const pin = this.pins.find((p) => p.id === pinId);
    if (!pin) return false;
    pin.enabled = Boolean(enabled);
    this._savePins();
    this._notify();
    return pin.enabled;
  }

  togglePinDone(pinId) {
    const pin = this.pins.find((p) => p.id === pinId);
    if (!pin) return false;
    pin.done = !pin.done;
    pin.lastUpdated = new Date().toISOString();
    pin.updateCount = (pin.updateCount || 0) + 1;
    this._savePins();
    this._notify();
    return pin.done;
  }

  clearPins() {
    this.pins = [];
    this._savePins();
    this._notify();
  }

  addPresetMissions() {
    MISSION_LIST.forEach((mission, idx) => {
      const alreadyHas = this.pins.some((p) => p.key === mission.key);
      if (!alreadyHas) {
        this.addPin({
          pinNumber: idx,
          name: mission.title,
          key: mission.key,
          done: false,
          enabled: true,
        });
      }
    });
  }

  clearLogs() {
    this.logs = [];
    this._saveLogs();
    this._notify();
  }

  _parseIotJson(str) {
    if (!str) return null;
    const startIdx = str.indexOf("{");
    const endIdx = str.lastIndexOf("}");
    if (startIdx === -1 || endIdx === -1 || endIdx <= startIdx) {
      return null;
    }

    const jsonStr = str.substring(startIdx, endIdx + 1);

    try {
      const parsed = JSON.parse(jsonStr);
      if (parsed && typeof parsed === "object") {
        const key = parsed.key !== undefined ? String(parsed.key) : parsed.k;
        const done = parsed.done !== undefined ? Boolean(parsed.done) : Boolean(parsed.status);
        if (key) {
          return { key, done };
        }
      }
    } catch {
      const keyMatch = jsonStr.match(/key\s*:\s*["']?([a-zA-Z0-9_\-]+)["']?/i);
      const doneMatch = jsonStr.match(/done\s*:\s*(true|false|1|0)/i);
      if (keyMatch) {
        const key = keyMatch[1];
        let done = false;
        if (doneMatch) {
          const rawDone = doneMatch[1].toLowerCase();
          done = rawDone === "true" || rawDone === "1";
        }
        return { key, done };
      }
    }
    return null;
  }

  _onBleData(chunk, device = null) {
    if (typeof chunk !== "string") return;
    this.rxBuffer += chunk;

    let nlIndex = -1;
    while ((nlIndex = this.rxBuffer.indexOf("\n")) !== -1) {
      const line = this.rxBuffer.substring(0, nlIndex).trim();
      this.rxBuffer = this.rxBuffer.substring(nlIndex + 1);
      if (line.length > 0) {
        this.processIncomingMessage(line, device);
      }
    }

    if (this.rxBuffer.length > 2000) {
      this.rxBuffer = this.rxBuffer.slice(-500);
    }
  }

  processIncomingMessage(rawLine, device = null) {
    if (!rawLine || typeof rawLine !== "string") return;
    const trimmed = rawLine.trim();

    const iotPrefixMatch = trimmed.match(/^IOT\s*:\s*(.*)$/i) || trimmed.includes("IOT :");
    if (!iotPrefixMatch) return;

    const parsed = this._parseIotJson(trimmed);
    const logItem = {
      id: `log_${Date.now()}_${Math.random().toString(36).slice(2, 6)}`,
      timestamp: new Date().toISOString(),
      formattedTime: new Date().toLocaleTimeString(),
      raw: trimmed,
      parsed: parsed || null,
      matchedPins: [],
      success: !!parsed,
      deviceName: device ? (device.name || "LEANBOT") : null,
    };

    if (parsed && parsed.key) {
      const targetKey = parsed.key.trim().toLowerCase();
      let updatedCount = 0;

      this.pins.forEach((pin) => {
        if (pin.key.trim().toLowerCase() === targetKey) {
          logItem.matchedPins.push(pin.pinLabel);
          if (pin.enabled !== false) {
            pin.done = parsed.done;
            pin.lastUpdated = new Date().toISOString();
            pin.updateCount = (pin.updateCount || 0) + 1;
            updatedCount++;
          }
        }
      });

      if (updatedCount > 0) {
        this._savePins();
      }
    }

    this.logs.unshift(logItem);
    if (this.logs.length > 50) {
      this.logs = this.logs.slice(0, 50);
    }
    this._saveLogs();
    this._notify();
  }

  simulateMessage(key, done = true) {
    const raw = `IOT : {"key":"${key}","done":${Boolean(done)}}`;
    this.processIncomingMessage(raw);
    return raw;
  }
}

const iotService = new IotService();
export default iotService;
