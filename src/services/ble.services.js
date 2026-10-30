const BLE_SERVICE = "0000ffe0-0000-1000-8000-00805f9b34fb";
const BLE_CHARACTERISTIC = "0000ffe1-0000-1000-8000-00805f9b34fb";
const MAX_CONNECTED_DEVICES = 2;

class BleService {
  constructor() {
    this.connectedDevices = new Map();
    this.discoveredPeripherals = new Map();
    this.serviceUUID = BLE_SERVICE;
    this.characteristicUUID = BLE_CHARACTERISTIC;
    this.disconnectListeners = new Set();
    this.connectListeners = new Set();
    this.dataListeners = new Set();
    this.activeTargetId = "all";
    this._onGattDisconnected = this._onGattDisconnected.bind(this);
  }

  get connectedServer() {
    const list = this.getConnectedDevices();
    return list.length > 0 ? list[0].server : null;
  }

  get connectedDevice() {
    const list = this.getConnectedDevices();
    return list.length > 0 ? list[0].device : null;
  }

  _onGattDisconnected(event) {
    const device = event.target;
    if (!device) return;
    const prev = this.connectedDevices.get(device.id);
    this.connectedDevices.delete(device.id);
    this.disconnectListeners.forEach((listener) => {
      try {
        listener(prev ? prev.device : device);
      } catch (e) {
        console.error(e);
      }
    });
  }

  onDisconnect(listener) {
    this.disconnectListeners.add(listener);
    return () => {
      this.disconnectListeners.delete(listener);
    };
  }

  onConnect(listener) {
    this.connectListeners.add(listener);
    return () => {
      this.connectListeners.delete(listener);
    };
  }

  onData(listener) {
    this.dataListeners.add(listener);
    return () => {
      this.dataListeners.delete(listener);
    };
  }

  _notifyDataListeners(data, device = null) {
    this.dataListeners.forEach((fn) => {
      try {
        fn(data, device);
      } catch (e) {
        console.error(e);
      }
    });
  }

  getConnectedDevices() {
    return Array.from(this.connectedDevices.values()).map((entry) => ({
      id: entry.device.id,
      name: entry.device.name || "LEANBOT",
      connectedAt: entry.connectedAt,
      serviceUUID: this.serviceUUID,
      characteristicUUID: this.characteristicUUID,
      server: entry.server,
      characteristic: entry.characteristic,
      device: entry.device,
    }));
  }

  isConnected(deviceId = null) {
    if (deviceId) {
      const entry = this.connectedDevices.get(deviceId);
      return !!(entry && entry.server && entry.server.connected);
    }
    return this.getConnectedDevices().some(
      (entry) => entry.server && entry.server.connected,
    );
  }

  getConnectedDeviceInfo(deviceId = null) {
    const list = this.getConnectedDevices();
    if (list.length === 0) return null;
    if (deviceId) {
      const found = list.find((d) => d.id === deviceId);
      return found || null;
    }
    if (list.length === 1) {
      return {
        id: list[0].id,
        name: list[0].name,
        connectedAt: list[0].connectedAt,
        serviceUUID: this.serviceUUID,
        characteristicUUID: this.characteristicUUID,
        count: 1,
        devices: list,
      };
    }
    return {
      id: list.map((d) => d.id).join(","),
      name: list.map((d) => d.name).join(" & "),
      connectedAt: list[0].connectedAt,
      serviceUUID: this.serviceUUID,
      characteristicUUID: this.characteristicUUID,
      count: list.length,
      devices: list,
    };
  }

  setActiveTarget(targetId = "all") {
    this.activeTargetId = targetId;
  }

  getActiveTarget() {
    return this.activeTargetId;
  }

  async startScanning() {
    try {
      const device = await navigator.bluetooth.requestDevice({
        filters: [{ services: [BLE_SERVICE] }],
      });

      this.discoveredPeripherals.set(device.id, device);
      return this.discoveredPeripherals;
    } catch (error) {
      console.error(error);
      throw error;
    }
  }

  stopScanning() {
    this.discoveredPeripherals.clear();
  }

  async connectDevice(peripheralId) {
    if (this.connectedDevices.size >= MAX_CONNECTED_DEVICES) {
      throw new Error(
        `Maksimal ${MAX_CONNECTED_DEVICES} Leanbot yang dapat terhubung sekaligus. Putuskan salah satu robot terlebih dahulu.`,
      );
    }

    const device = this.discoveredPeripherals.get(peripheralId);
    if (!device) {
      throw new Error("Perangkat tidak ditemukan. Silakan scan ulang.");
    }

    if (this.connectedDevices.has(device.id)) {
      throw new Error(`Perangkat ${device.name || device.id} sudah terhubung.`);
    }

    if (device.gatt && device.gatt.connected) {
      await device.gatt.disconnect();
    }

    try {
      device.removeEventListener(
        "gattserverdisconnected",
        this._onGattDisconnected,
      );
      device.addEventListener(
        "gattserverdisconnected",
        this._onGattDisconnected,
      );
    } catch (e) {
      console.warn(e);
    }

    const server = await device.gatt.connect();

    let characteristic = null;
    try {
      const service = await server.getPrimaryService(this.serviceUUID);
      characteristic = await service.getCharacteristic(
        this.characteristicUUID,
      );
      if (
        characteristic.properties.notify ||
        characteristic.properties.indicate
      ) {
        await characteristic.startNotifications();
        characteristic.addEventListener(
          "characteristicvaluechanged",
          (event) => {
            const decoder = new TextDecoder();
            const str = decoder.decode(event.target.value);
            this._notifyDataListeners(str, device);
          },
        );
      }
    } catch (e) {
      console.warn(e);
    }

    const entry = {
      device,
      server,
      characteristic,
      connectedAt: new Date().toLocaleTimeString(),
    };
    this.connectedDevices.set(device.id, entry);

    this.connectListeners.forEach((listener) => {
      try {
        listener(device);
      } catch (e) {
        console.error(e);
      }
    });

    return device;
  }

  async disconnectDevice(deviceId = null) {
    if (deviceId) {
      const entry = this.connectedDevices.get(deviceId);
      if (entry && entry.server && entry.server.connected) {
        try {
          entry.server.disconnect();
        } catch (e) {
          console.warn(e);
        }
      }
      this.connectedDevices.delete(deviceId);
      this.disconnectListeners.forEach((listener) => {
        try {
          listener(entry ? entry.device : { id: deviceId });
        } catch (e) {
          console.error(e);
        }
      });
      return;
    }

    const entries = Array.from(this.connectedDevices.values());
    for (const entry of entries) {
      if (entry.server && entry.server.connected) {
        try {
          entry.server.disconnect();
        } catch (e) {
          console.warn(e);
        }
      }
    }
    this.connectedDevices.clear();
    this.disconnectListeners.forEach((listener) => {
      try {
        listener(null);
      } catch (e) {
        console.error(e);
      }
    });
  }

  async disconnectAll() {
    await this.disconnectDevice(null);
  }

  async _sendToCharacteristic(characteristic, fullCmd) {
    if (!characteristic) return;
    const encoder = new TextEncoder();
    const data = encoder.encode(fullCmd);
    const CHUNK_SIZE = 20;

    for (let i = 0; i < data.length; i += CHUNK_SIZE) {
      const chunk = data.slice(i, i + CHUNK_SIZE);
      try {
        await characteristic.writeValueWithoutResponse(chunk);
      } catch {
        await characteristic.writeValueWithResponse(chunk);
      }
      if (data.length > CHUNK_SIZE) {
        await new Promise((resolve) => setTimeout(resolve, 25));
      }
    }
  }

  async sendCommand(commandStr, targetDeviceId = null) {
    if (!this.isConnected()) {
      throw new Error("Perangkat Bluetooth belum terhubung!");
    }

    const trimmed = commandStr.trim();
    let rawCmd = trimmed;
    const upper = trimmed.toUpperCase();
    if (
      !upper.startsWith("CODE;") &&
      !upper.startsWith("BATCH:") &&
      !upper.startsWith("CLEAR") &&
      !upper.startsWith("START") &&
      !upper.startsWith("EMERGENCY")
    ) {
      rawCmd = `CODE;${trimmed}`;
    }
    const fullCmd = rawCmd.endsWith("\n") ? rawCmd : `${rawCmd}\n`;

    const target = targetDeviceId || this.activeTargetId || "all";
    const devices = this.getConnectedDevices();

    if (target !== "all") {
      const entry = devices.find((d) => d.id === target);
      if (entry && entry.characteristic) {
        await this._sendToCharacteristic(entry.characteristic, fullCmd);
        return;
      }
    }

    const sendPromises = devices
      .filter((d) => d.characteristic)
      .map((d) => this._sendToCharacteristic(d.characteristic, fullCmd));
    await Promise.all(sendPromises);
  }

  waitForAcknowledgment(expectedPrefix, timeoutMs = 5000) {
    return new Promise((resolve) => {
      let timer = null;
      let unsubscribe = null;
      let accumulated = "";

      const cleanup = () => {
        if (timer) clearTimeout(timer);
        if (unsubscribe) unsubscribe();
      };

      unsubscribe = this.onData((str) => {
        accumulated += str;
        if (accumulated.includes(expectedPrefix)) {
          cleanup();
          resolve({ success: true, text: accumulated });
        } else if (accumulated.includes("EMERGENCY_STOP")) {
          cleanup();
          resolve({ success: false, reason: "Emergency Stop terdeteksi" });
        }
      });

      timer = setTimeout(() => {
        cleanup();
        resolve({ success: false, reason: "Timeout menunggu konfirmasi robot" });
      }, timeoutMs);
    });
  }

  waitForBatchCompletion(timeoutMs = 300000) {
    return new Promise((resolve) => {
      let timer = null;
      let unsubscribe = null;
      let accumulated = "";

      const finish = (result) => {
        if (timer) clearTimeout(timer);
        if (unsubscribe) unsubscribe();
        resolve(result);
      };

      unsubscribe = this.onData((str) => {
        accumulated += str;
        if (
          accumulated.includes("[MISSION_DONE]") ||
          accumulated.includes("BATCH_READY")
        ) {
          finish({ success: true, text: accumulated });
        } else if (
          accumulated.includes("EMERGENCY_STOP") ||
          accumulated.includes("[STATE] EMG")
        ) {
          finish({ success: false, reason: "Emergency Stop terdeteksi" });
        }
      });

      timer = setTimeout(() => {
        finish({
          success: false,
          reason: "Batas waktu tunggu misi habis (Timeout)",
        });
      }, timeoutMs);
    });
  }
}

export default new BleService();
