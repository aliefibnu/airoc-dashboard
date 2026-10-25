const BLE_SERVICE = "0000ffe0-0000-1000-8000-00805f9b34fb";
const BLE_CHARACTERISTIC = "0000ffe1-0000-1000-8000-00805f9b34fb";

class BleService {
  constructor() {
    this.connectedDevice = null;
    this.connectedServer = null;
    this.discoveredPeripherals = new Map();
    this.serviceUUID = BLE_SERVICE;
    this.characteristicUUID = BLE_CHARACTERISTIC;
    this.disconnectListeners = new Set();
    this.connectListeners = new Set();
    this._onGattDisconnected = this._onGattDisconnected.bind(this);
  }

  _onGattDisconnected(event) {
    console.log("[BLE] GATT Server terputus secara eksternal:", event);
    const prevDevice = this.connectedDevice;
    this.connectedDevice = null;
    this.connectedServer = null;
    this.disconnectListeners.forEach((listener) => {
      try {
        listener(prevDevice);
      } catch (e) {
        console.error("[BLE] Error pada disconnect listener:", e);
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

  initNoble() {}

  async startScanning() {
    try {
      console.log("[BLE] Membuka jendela pemindaian browser...");

      const device = await navigator.bluetooth.requestDevice({
        filters: [{ services: [BLE_SERVICE] }],
      });

      this.discoveredPeripherals.clear();
      this.discoveredPeripherals.set(device.id, device);
      console.log(`[BLE] Perangkat dipilih: ${device.name} [ID: ${device.id}]`);

      return this.discoveredPeripherals;
    } catch (error) {
      console.error("[BLE] Pemindaian dibatalkan atau gagal:", error);
      throw error;
    }
  }

  stopScanning() {
    console.log("[BLE] Pemindaian selesai via browser.");
  }

  async connectDevice(peripheralId) {
    const device = this.discoveredPeripherals.get(peripheralId);
    if (!device)
      throw new Error("Perangkat tidak ditemukan. Silakan scan ulang.");

    console.log(`[BLE] Menghubungkan ke ${device.name}...`);

    if (device.gatt && device.gatt.connected) {
      await device.gatt.disconnect();
    }

    try {
      device.removeEventListener("gattserverdisconnected", this._onGattDisconnected);
      device.addEventListener("gattserverdisconnected", this._onGattDisconnected);
    } catch (e) {
      console.warn("[BLE] Gagal memasang event listener gattserverdisconnected:", e);
    }

    this.connectedServer = await device.gatt.connect();
    this.connectedDevice = device;

    this.connectListeners.forEach((listener) => {
      try {
        listener(device);
      } catch (e) {
        console.error("[BLE] Error pada connect listener:", e);
      }
    });

    return device;
  }

  async disconnectDevice() {
    if (this.connectedDevice && this.connectedDevice.gatt) {
      try {
        if (this.connectedDevice.gatt.connected) {
          this.connectedDevice.gatt.disconnect();
        }
      } catch (e) {
        console.warn("[BLE] Error saat disconnect:", e);
      }
      console.log("[BLE] Koneksi terputus.");
    }
    this.connectedDevice = null;
    this.connectedServer = null;
  }

  isConnected() {
    return !!(
      this.connectedServer &&
      this.connectedServer.connected &&
      this.connectedDevice
    );
  }

  getConnectedDeviceInfo() {
    if (this.isConnected()) {
      return {
        id: this.connectedDevice.id,
        name: this.connectedDevice.name || "UNKNOWN HARDWARE",
        serviceUUID: this.serviceUUID,
        characteristicUUID: this.characteristicUUID,
      };
    }
    return null;
  }

  async readCharacteristic(serviceUUID, characteristicUUID) {
    if (!this.connectedServer)
      throw new Error("Tidak ada perangkat yang terhubung.");

    const service = await this.connectedServer.getPrimaryService(serviceUUID);
    const characteristic = await service.getCharacteristic(characteristicUUID);
    const value = await characteristic.readValue();
    return value;
  }

  async writeCharacteristic(
    serviceUUID,
    characteristicUUID,
    dataBuffer,
    withoutResponse = false,
  ) {
    if (!this.connectedServer)
      throw new Error("Tidak ada perangkat yang terhubung.");

    const service = await this.connectedServer.getPrimaryService(serviceUUID);
    const characteristic = await service.getCharacteristic(characteristicUUID);

    if (withoutResponse) {
      await characteristic.writeValueWithoutResponse(dataBuffer);
    } else {
      await characteristic.writeValueWithResponse(dataBuffer);
    }
    console.log("[BLE] Data berhasil dikirim.");
  }

  async sendCommand(commandStr) {
    if (!this.isConnected()) {
      throw new Error("Perangkat Bluetooth belum terhubung!");
    }
    const trimmed = commandStr.trim();
    const fullCmd = trimmed.toUpperCase().startsWith("CODE;") ? trimmed : `CODE;${trimmed}`;
    const encoder = new TextEncoder();
    const data = encoder.encode(fullCmd);
    const CHUNK_SIZE = 20;
    for (let i = 0; i < data.length; i += CHUNK_SIZE) {
      const chunk = data.slice(i, i + CHUNK_SIZE);
      try {
        await this.writeCharacteristic(
          this.serviceUUID,
          this.characteristicUUID,
          chunk,
          true,
        );
      } catch {
        await this.writeCharacteristic(
          this.serviceUUID,
          this.characteristicUUID,
          chunk,
          false,
        );
      }
      if (data.length > CHUNK_SIZE) {
        await new Promise((resolve) => setTimeout(resolve, 25));
      }
    }
  }
}

export default new BleService();