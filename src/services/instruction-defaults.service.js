import { INSTRUCTION_ALIASES } from "@/data/instruction-aliases.js";
import BleService from "@/services/ble.services.js";

const STORAGE_KEY_DEFAULTS = "airoc_instruction_defaults";

class InstructionDefaultsService {
  constructor() {
    this.listeners = new Set();
    this.defaults = this._loadDefaults();

    // Auto-sync ke robot Leanbot saat Bluetooth berhasil terhubung
    BleService.onConnect(async (device) => {
      console.log(
        `[InstructionDefaults] BLE terhubung ke "${device.name || device.id}", menyinkronkan default value...`,
      );
      try {
        await this.syncToLeanbot();
        console.log("[InstructionDefaults] Auto-sync saat koneksi BLE sukses!");
      } catch (err) {
        console.warn("[InstructionDefaults] Auto-sync saat koneksi BLE gagal:", err);
      }
    });
  }

  _getBaselines() {
    const baselines = {};
    INSTRUCTION_ALIASES.forEach((item) => {
      if (Array.isArray(item.params) && item.params.length > 0) {
        baselines[item.alias] = {};
        item.params.forEach((p) => {
          baselines[item.alias][p.name] =
            p.defaultValue !== undefined ? p.defaultValue : "";
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
        for (const alias in parsed) {
          if (merged[alias]) {
            merged[alias] = { ...merged[alias], ...parsed[alias] };
          } else {
            merged[alias] = parsed[alias];
          }
        }
        return merged;
      }
    } catch (e) {
      console.error("[InstructionDefaults] Gagal memuat dari localStorage:", e);
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
      console.error("[InstructionDefaults] Gagal menyimpan ke localStorage:", e);
    }
    this._notify();

    let syncPromise = Promise.resolve({ success: false, reason: "Auto sync disabled" });
    if (autoSync && BleService.isConnected()) {
      syncPromise = this.syncToLeanbot().catch((err) => {
        console.warn("[InstructionDefaults] Auto-sync gagal saat update default value:", err);
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
    const scanAngle = this.getDefault("BATAS_SCAN", "sudut_deg") ?? 90;
    const scanDist = this.getDefault("SCAN_KANAN", "jarak_cm") ?? 20;
    const majuVal = this.getDefault("MAJU", "cm") ?? 10;
    const mundurVal = this.getDefault("MUNDUR", "cm") ?? 10;
    const kananVal = this.getDefault("KANAN", "deg") ?? 90;
    const kiriVal = this.getDefault("KIRI", "deg") ?? 90;
    const dekatiVal = this.getDefault("DEKATI_OBJEK", "jarak_akhir_cm") ?? 6;
    const jauhiVal = this.getDefault("JAUHI_OBJEK", "jarak_akhir_cm") ?? 20;
    const cekVal = this.getDefault("CEK_OBJEK", "jarak_cm") ?? 20;
    const geserVal = this.getDefault("GESER_KANAN", "cm") ?? 10;
    const tungguVal = this.getDefault("TUNGGU", "ms") ?? 1000;
    const bunyiVal = this.getDefault("BUNYI", "jumlah") ?? 1;

    // Perintah sinkronisasi default value ke Leanbot:
    // 1. SPEED & BATAS_SCAN mengubah variabel konfigurasi global Leanbot
    // 2. CONFIG:<target>,<val> mengubah nilai default instruction pada Leanbot firmware
    const parts = [
      `SPEED:${speedVal}`,
      `BATAS_SCAN:${scanAngle}`,
      `CONFIG:MAJU,${majuVal}`,
      `CONFIG:MUNDUR,${mundurVal}`,
      `CONFIG:KANAN,${kananVal}`,
      `CONFIG:KIRI,${kiriVal}`,
      `CONFIG:SPEED,${speedVal}`,
      `CONFIG:BATAS_SCAN,${scanAngle}`,
      `CONFIG:SCAN_JARAK,${scanDist}`,
      `CONFIG:DEKATI,${dekatiVal}`,
      `CONFIG:JAUHI,${jauhiVal}`,
      `CONFIG:CEK,${cekVal}`,
      `CONFIG:GESER,${geserVal}`,
      `CONFIG:TUNGGU,${tungguVal}`,
      `CONFIG:BUNYI,${bunyiVal}`,
    ];

    return `PROD;${parts.join(";")}`;
  }

  async syncToLeanbot() {
    if (!BleService.isConnected()) {
      return { success: false, reason: "Robot Bluetooth belum terhubung" };
    }
    const cmd = this.generateSyncCommand();
    console.log("[InstructionDefaults] Mengirim perintah sync ke Leanbot:", cmd);
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
