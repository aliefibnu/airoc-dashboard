import { INSTRUCTION_ALIASES } from "@/data/instruction-aliases.js";

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
  BUNYI: ["jumlah"],
};

class InstructionDefaultsService {
  constructor() {
    this.listeners = new Set();
    this.defaults = this._loadDefaults();
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

  saveDefaults(newDefaults) {
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
    return { defaults: this.defaults };
  }

  resetToBaselines() {
    const baselines = this._getBaselines();
    return this.saveDefaults(baselines);
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
