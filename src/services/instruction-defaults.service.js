import { INSTRUCTION_ALIASES } from "@/data/instruction-aliases.js";
import BleService from "@/services/ble.services.js";

const STORAGE_KEY_DEFAULTS = "airoc_instruction_defaults";

const CRUCIAL_DEFAULT_KEYS = {
  SPEED: ["level"],
  MAJU: ["cm"],
  MUNDUR: ["cm"],
  KANAN: ["deg"],
  KIRI: ["deg"],
  BELOK_KANAN: ["deg"],
  BELOK_KIRI: ["deg"],
  FOLLOW_LINE: ["jumlah_simpangan"],
  MAJU_LINE: ["max_cm"],
  MUNDUR_LINE: ["max_cm"],
  TUNGGU: ["ms"],
  BEEP: ["frekuensi"],
};

class InstructionDefaultsService {
  constructor() {
    this.listeners = new Set();
    this.defaults = this._loadDefaults();

    BleService.onConnect(async (device) => {
      try {
        await this.syncToLeanbot();
      } catch (err) {
        console.warn(err);
      }
    });
  }

  _getBaselines() {
    const baselines = {};
    INSTRUCTION_ALIASES.forEach((item) => {
      const allowedParams = CRUCIAL_DEFAULT_KEYS[item.alias];
      if (allowedParams && Array.isArray(item.params)) {
        baselines[item.alias] = {};
        item.params.forEach((p) => {
          if (allowedParams.includes(p.name)) {
            baselines[item.alias][p.name] =
              p.defaultValue !== undefined ? p.defaultValue : "";
          }
        });
      }
    });
    return baselines;
  }

  _loadDefaults() {
    const baselines = this._getBaselines();
    try {
      const raw = localStorage.getItem(STORAGE_KEY_DEFAULTS);
      if (raw) {
        const parsed = JSON.parse(raw);
        const merged = { ...baselines };
        for (const alias in baselines) {
          if (parsed[alias]) {
            merged[alias] = { ...merged[alias], ...parsed[alias] };
          }
        }
        return merged;
      }
    } catch (e) {
      console.error(e);
    }
    return baselines;
  }

  getAllDefaults() {
    return { ...this.defaults };
  }

  getDefault(alias, paramName) {
    if (!this.defaults[alias]) {
      const baseline = this._getBaselines()[alias];
      if (!baseline) return "";
      return paramName !== undefined ? baseline[paramName] : baseline;
    }
    if (paramName !== undefined) {
      const val = this.defaults[alias][paramName];
      return val !== undefined ? val : "";
    }
    return this.defaults[alias];
  }

  saveDefaults(newDefaults, autoSync = true) {
    this.defaults = { ...this.defaults, ...newDefaults };
    try {
      localStorage.setItem(
        STORAGE_KEY_DEFAULTS,
        JSON.stringify(this.defaults),
      );
    } catch (e) {
      console.error(e);
    }
    this._notify();

    let syncPromise = Promise.resolve({ success: false, reason: "Auto sync disabled" });
    if (autoSync && BleService.isConnected()) {
      syncPromise = this.syncToLeanbot().catch((err) => {
        return { success: false, reason: err.message || err };
      });
    }
    return { defaults: this.defaults, syncPromise };
  }

  resetToBaselines(autoSync = true) {
    const baselines = this._getBaselines();
    return this.saveDefaults(baselines, autoSync);
  }

  generateSyncCommand() {
    const speedVal = this.getDefault("SPEED", "level") ?? 5;
    const majuVal = this.getDefault("MAJU", "cm") ?? 10;
    const mundurVal = this.getDefault("MUNDUR", "cm") ?? 10;
    const kananVal = this.getDefault("KANAN", "deg") ?? 90;
    const kiriVal = this.getDefault("KIRI", "deg") ?? 90;
    const belokKananVal = this.getDefault("BELOK_KANAN", "deg") ?? 90;
    const belokKiriVal = this.getDefault("BELOK_KIRI", "deg") ?? 90;
    const followLineVal = this.getDefault("FOLLOW_LINE", "jumlah_simpangan") ?? 1;
    const majuLineVal = this.getDefault("MAJU_LINE", "max_cm") ?? 50;
    const mundurLineVal = this.getDefault("MUNDUR_LINE", "max_cm") ?? 50;
    const tungguVal = this.getDefault("TUNGGU", "ms") ?? 1000;
    const beepVal = this.getDefault("BEEP", "frekuensi") ?? 1500;

    const parts = [
      `SPEED:${speedVal}`,
      `CONFIG:MAJU,${majuVal}`,
      `CONFIG:MUNDUR,${mundurVal}`,
      `CONFIG:KANAN,${kananVal}`,
      `CONFIG:KIRI,${kiriVal}`,
      `CONFIG:BELOK_KANAN,${belokKananVal}`,
      `CONFIG:BELOK_KIRI,${belokKiriVal}`,
      `CONFIG:FOLLOW_LINE,${followLineVal}`,
      `CONFIG:MAJU_LINE,${majuLineVal}`,
      `CONFIG:MUNDUR_LINE,${mundurLineVal}`,
      `CONFIG:TUNGGU,${tungguVal}`,
      `CONFIG:BEEP,${beepVal}`,
    ];

    return `CODE;${parts.join(";")}`;
  }

  async syncToLeanbot() {
    if (!BleService.isConnected()) {
      return { success: false, reason: "Robot Bluetooth belum terhubung" };
    }
    const cmd = this.generateSyncCommand();
    await BleService.sendCommand(cmd);
    return { success: true, command: cmd };
  }

  subscribe(listener) {
    this.listeners.add(listener);
    return () => this.listeners.delete(listener);
  }

  _notify() {
    this.listeners.forEach((fn) => {
      try {
        fn(this.defaults);
      } catch (e) {
        console.error(e);
      }
    });
  }
}

export default new InstructionDefaultsService();
