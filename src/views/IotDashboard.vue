<template>
  <div class="max-w-6xl mx-auto space-y-8 select-none pb-16">
    <div
      class="flex flex-col md:flex-row justify-between items-start md:items-center gap-4 border-b-4 border-black pb-6"
    >
      <div class="space-y-1">
        <h1
          class="text-2xl md:text-4xl font-black uppercase tracking-tight text-black"
        >
          IOT VIRTUAL PIN DASHBOARD
        </h1>
      </div>

      <div class="flex flex-wrap items-center gap-3">
        <div
          class="text-sm md:text-base font-black px-5 py-3 rounded-2xl uppercase border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] flex items-center gap-3 transition-all select-none"
          :class="
            isBleConnected
              ? 'bg-green-500 text-white animate-pulse'
              : 'bg-red-500 text-white'
          "
        >
          <span
            class="w-3.5 h-3.5 rounded-full border-2 border-black inline-block"
            :class="isBleConnected ? 'bg-white animate-ping' : 'bg-black'"
          ></span>
          <h3>
            {{
              isBleConnected
                ? connectedDeviceName
                  ? `${connectedDeviceName}`
                  : "BLE CONNECTED"
                : "BLE DISCONNECTED"
            }}
          </h3>
        </div>

        <button
          v-if="!isBleConnected"
          type="button"
          @click="connectBle"
          class="bg-blue-600 text-white font-black text-sm md:text-base py-3 px-5 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-blue-700 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
        >
          <span>📡</span>
          <span>HUBUNGKAN BLE</span>
        </button>

        <button
          v-if="isBleConnected"
          type="button"
          @click="disconnectBle"
          class="bg-red-600 text-white font-black text-sm md:text-base py-3 px-5 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-red-700 active:translate-y-1 active:shadow-none transition-all cursor-pointer"
        >
          PUTUSKAN
        </button>
      </div>
    </div>

    <div
      v-if="statusMessage"
      class="border-4 border-black p-4 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] flex items-center justify-between gap-4"
      :class="
        statusMessage.includes('Gagal') || statusMessage.includes('gagal')
          ? 'bg-red-100 text-red-950'
          : statusMessage.includes('berhasil') ||
              statusMessage.includes('Berhasil')
            ? 'bg-green-100 text-green-950'
            : 'bg-blue-100 text-blue-950'
      "
    >
      <div class="flex items-center gap-3">
        <span class="text-2xl">{{
          statusMessage.includes("Gagal") || statusMessage.includes("gagal")
            ? "⚠️"
            : "ℹ️"
        }}</span>
        <span class="font-black text-sm md:text-base uppercase">{{
          statusMessage
        }}</span>
      </div>
      <button
        @click="statusMessage = ''"
        class="font-black text-lg px-2 hover:bg-black/10 rounded border-2 border-black cursor-pointer"
      >
        ✕
      </button>
    </div>

    <div
      class="bg-yellow-100 border-4 border-black p-5 md:p-6 rounded-3xl shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] flex flex-wrap items-center justify-between gap-4"
    >
      <div class="space-y-1">
        <h2 class="text-xl font-black uppercase tracking-tight text-black">
          MANAJEMEN VIRTUAL PIN
        </h2>
      </div>

      <div class="flex flex-wrap items-center gap-3">
        <button
          type="button"
          @click="openAddModal"
          class="bg-blue-600 text-white font-black text-sm md:text-base py-3 px-5 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] hover:bg-blue-700 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
        >
          <span>TAMBAH VIRTUAL PIN</span>
        </button>

        <button
          v-if="pins.length > 0"
          type="button"
          @click="resetPins"
          class="bg-red-500 text-white font-black text-sm md:text-base py-3 px-5 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] hover:bg-red-600 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
        >
          <span>RESET KE 0 PIN</span>
        </button>
      </div>
    </div>

    <div
      v-if="pins.length === 0"
      class="bg-white border-4 border-dashed border-black p-10 md:p-16 rounded-3xl text-center space-y-4 shadow-[4px_4px_0px_0px_rgba(0,0,0,1)]"
    >
      <div class="text-6xl">📡</div>
      <h3 class="text-2xl font-black uppercase tracking-tight text-black">
        0 IOT VIRTUAL PIN
      </h3>
      <div class="flex flex-wrap justify-center items-center gap-3 pt-2">
        <button
          type="button"
          @click="openAddModal"
          class="bg-blue-600 text-white font-black text-base py-3.5 px-6 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-blue-700 active:translate-y-1 active:shadow-none transition-all cursor-pointer"
        >
          ➕ TAMBAH PIN BARU
        </button>
      </div>
    </div>

    <div v-else class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
      <div
        v-for="pin in pins"
        :key="pin.id"
        class="bg-white border-4 border-black p-5 rounded-3xl shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] flex flex-col justify-between space-y-4 transition-all"
        :class="[
          !pin.enabled
            ? 'opacity-60 bg-gray-100 border-gray-600'
            : pin.done
              ? 'ring-4 ring-green-500'
              : '',
        ]"
      >
        <div class="space-y-3">
          <div
            class="flex items-center justify-between gap-2 border-b-3 border-black pb-3"
          >
            <div class="flex items-center gap-2">
              <h3
                class="font-black text-base uppercase text-black truncate grow"
              >
                {{ pin.name }}
              </h3>
            </div>
          </div>

          <div class="space-y-2">
            <div
              class="border-3 border-black p-3.5 rounded-2xl flex items-center justify-between transition-all"
              :class="[
                !pin.enabled
                  ? 'bg-gray-200 text-gray-600'
                  : pin.done
                    ? 'bg-green-500 text-white shadow-[3px_3px_0px_0px_rgba(0,0,0,1)]'
                    : 'bg-yellow-200 text-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)]',
              ]"
            >
              <div class="flex items-center gap-2">
                <div>
                  <p class="text-lg font-black uppercase">
                    {{
                      !pin.enabled
                        ? "DISABLED"
                        : pin.done
                          ? "SELESAI"
                          : "BELUM SELESAI"
                    }}
                  </p>
                </div>
              </div>

              <button
                type="button"
                @click="togglePinDone(pin.id)"
                :disabled="!pin.enabled"
                class="font-black text-xs uppercase px-2.5 py-1.5 rounded-xl border-2 border-black bg-white text-black hover:bg-yellow-100 cursor-pointer shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all disabled:opacity-40 disabled:cursor-not-allowed"
                title="Klik untuk mengubah status secara manual"
              >
                TOGGLE
              </button>
            </div>

            <div
              class="flex items-center justify-between text-[11px] font-mono text-gray-600 pt-1"
            >
              <span>UPDATE: {{ formatTime(pin.lastUpdated) }}</span>
            </div>
          </div>
        </div>

        <div
          class="flex items-center justify-end gap-2 border-t-2 border-gray-200 pt-3"
        >
          <button
            type="button"
            @click="openEditModal(pin)"
            class="text-xs font-black uppercase bg-yellow-300 hover:bg-yellow-400 text-black border-2 border-black px-3 py-1.5 rounded-xl cursor-pointer shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all flex items-center gap-1"
          >
            <span>EDIT</span>
          </button>

          <button
            type="button"
            @click="deletePin(pin.id)"
            class="text-xs font-black uppercase bg-red-500 hover:bg-red-600 text-white border-2 border-black px-3 py-1.5 rounded-xl cursor-pointer shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all flex items-center gap-1"
          >
            <span>HAPUS</span>
          </button>
        </div>
      </div>
    </div>

    <teleport to="body">
      <div
        v-if="isModalOpen"
        class="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/60 backdrop-blur-xs select-none"
        @click.self="closeModal"
      >
        <div
          class="bg-yellow-50 border-4 border-black rounded-3xl p-6 md:p-8 max-w-md w-full shadow-[8px_8px_0px_0px_rgba(0,0,0,1)] space-y-5"
        >
          <div
            class="flex items-center justify-between border-b-4 border-black pb-3 gap-3"
          >
            <h2 class="text-2xl font-black uppercase tracking-tight text-black">
              {{ isEditing ? "EDIT VIRTUAL PIN" : "TAMBAH VIRTUAL PIN" }}
            </h2>
            <button
              type="button"
              @click="closeModal"
              class="border-2 border-black font-black text-xl px-2.5 py-0.5 rounded-xl bg-red-500 text-white hover:bg-red-600 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer shrink-0"
            >
              ✕
            </button>
          </div>

          <form @submit.prevent="savePin" class="space-y-4">
            <div class="space-y-1">
              <label
                class="text-xs font-black uppercase tracking-wider text-black block"
              >
                PRESET MISI
              </label>
              <select
                v-model="formSelectedPreset"
                @change="onPresetChange"
                required
                class="w-full bg-white border-4 border-black rounded-xl p-3 text-sm font-black font-mono focus:outline-none focus:bg-yellow-100 shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] cursor-pointer uppercase"
              >
                <option value="" disabled>-- PILIH PRESET MISI --</option>
                <option v-for="m in MISSION_LIST" :key="m.key" :value="m.key">
                  {{ m.title }}
                </option>
              </select>
            </div>

            <div class="space-y-1">
              <label
                class="text-xs font-black uppercase tracking-wider text-black block"
              >
                NAMA / LABEL PIN
              </label>
              <input
                v-model="formName"
                type="text"
                required
                placeholder="Contoh: Misi Neutral Ungu"
                class="w-full bg-white border-4 border-black rounded-xl p-3 text-base font-black uppercase focus:outline-none focus:bg-yellow-100 shadow-[3px_3px_0px_0px_rgba(0,0,0,1)]"
              />
            </div>

            <div class="grid grid-cols-2 gap-3 pt-1">
              <div class="space-y-1">
                <label
                  class="text-xs font-black uppercase tracking-wider text-black block"
                >
                  STATUS AWAL
                </label>
                <select
                  v-model="formDone"
                  class="w-full bg-white border-3 border-black rounded-xl p-2.5 text-sm font-black font-mono focus:outline-none focus:bg-yellow-100 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] cursor-pointer uppercase"
                >
                  <option :value="false">BELUM SELESAI</option>
                  <option :value="true">SELESAI</option>
                </select>
              </div>

              <div class="space-y-1">
                <label
                  class="text-xs font-black uppercase tracking-wider text-black block"
                >
                  DIGUNAKAN
                </label>
                <select
                  v-model="formEnabled"
                  class="w-full bg-white border-3 border-black rounded-xl p-2.5 text-sm font-black font-mono focus:outline-none focus:bg-yellow-100 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] cursor-pointer uppercase"
                >
                  <option :value="true">YA</option>
                  <option :value="false">TIDAK</option>
                </select>
              </div>
            </div>

            <div class="flex items-center justify-end gap-3 pt-3">
              <button
                type="button"
                @click="closeModal"
                class="bg-slate-200 text-black font-black uppercase text-sm py-3 px-5 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] hover:bg-slate-300 active:translate-y-0.5 active:shadow-none transition-all cursor-pointer"
              >
                BATAL
              </button>

              <button
                type="submit"
                class="bg-green-500 text-black font-black uppercase text-sm py-3 px-6 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-0.5 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
              >
                <span>💾</span>
                <span>SIMPAN PIN</span>
              </button>
            </div>
          </form>
        </div>
      </div>
    </teleport>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from "vue";
import BleService from "@/services/ble.services.js";
import iotService from "@/services/iot.service.js";
import { MISSION_LIST } from "@/data/missions.js";

const pins = ref([]);
const logs = ref([]);
const isBleConnected = ref(false);
const connectedDeviceName = ref("");
const statusMessage = ref("");

const isModalOpen = ref(false);
const isEditing = ref(false);
const editingPinId = ref(null);

const formPinNumber = ref(0);
const formName = ref("");
const formKey = ref("neutral_ungu");
const formSelectedPreset = ref("neutral_ungu");
const formDone = ref(false);
const formEnabled = ref(true);

const simKey = ref("neutral_ungu");
const simDone = ref(true);

let unsubscribeIot = null;
let removeDisconnectListener = null;

onMounted(() => {
  checkBle();
  pins.value = iotService.getPins();
  logs.value = iotService.getLogs();

  unsubscribeIot = iotService.subscribe((state) => {
    pins.value = state.pins || [];
    logs.value = state.logs || [];
  });

  removeDisconnectListener = BleService.onDisconnect(() => {
    checkBle();
  });
});

onUnmounted(() => {
  if (unsubscribeIot) unsubscribeIot();
  if (removeDisconnectListener) removeDisconnectListener();
});

function checkBle() {
  isBleConnected.value = BleService.isConnected();
  const info = BleService.getConnectedDeviceInfo();
  connectedDeviceName.value = info ? info.name : "";
}

async function connectBle() {
  try {
    statusMessage.value = "Membuka pencarian Bluetooth...";
    await BleService.startScanning();
    const peripherals = BleService.discoveredPeripherals;
    if (peripherals && peripherals.size > 0) {
      const firstId = Array.from(peripherals.keys())[0];
      await BleService.connectDevice(firstId);
      checkBle();
      statusMessage.value = `Berhasil terhubung ke ${connectedDeviceName.value}`;
    }
  } catch (err) {
    statusMessage.value = `Gagal menghubungkan BLE: ${err.message || err}`;
  }
}

async function disconnectBle() {
  try {
    await BleService.disconnectDevice();
    checkBle();
    statusMessage.value = "Koneksi BLE diputuskan.";
  } catch (err) {
    statusMessage.value = `Gagal memutuskan: ${err.message || err}`;
  }
}

function openAddModal() {
  isEditing.value = false;
  editingPinId.value = null;
  formPinNumber.value = iotService.getNextPinNumber();
  const defaultPreset = MISSION_LIST[0];
  formSelectedPreset.value = defaultPreset.key;
  formKey.value = defaultPreset.key;
  formName.value = defaultPreset.title;
  formDone.value = false;
  formEnabled.value = true;
  isModalOpen.value = true;
}

function openEditModal(pin) {
  isEditing.value = true;
  editingPinId.value = pin.id;
  formPinNumber.value = pin.pinNumber;
  formName.value = pin.name;
  formKey.value = pin.key;
  const match = MISSION_LIST.find((m) => m.key === pin.key);
  formSelectedPreset.value = match ? match.key : MISSION_LIST[0].key;
  formDone.value = pin.done;
  formEnabled.value = pin.enabled !== false;
  isModalOpen.value = true;
}

function closeModal() {
  isModalOpen.value = false;
}

function onPresetChange() {
  if (!formSelectedPreset.value) return;
  const match = MISSION_LIST.find((m) => m.key === formSelectedPreset.value);
  if (match) {
    formKey.value = match.key;
    formName.value = match.title;
  }
}

function savePin() {
  if (!formSelectedPreset.value) {
    statusMessage.value = "Preset misi wajib dipilih!";
    return;
  }
  const match = MISSION_LIST.find((m) => m.key === formSelectedPreset.value);
  const keyToUse = match ? match.key : formKey.value;
  const nameToUse =
    formName.value.trim() ||
    (match ? match.title : `Virtual Pin V${formPinNumber.value}`);

  if (isEditing.value && editingPinId.value) {
    iotService.updatePin(editingPinId.value, {
      pinNumber: formPinNumber.value,
      name: nameToUse,
      key: keyToUse,
      done: formDone.value,
      enabled: formEnabled.value,
    });
    statusMessage.value = `Virtual Pin V${formPinNumber.value} berhasil diperbarui.`;
  } else {
    iotService.addPin({
      pinNumber: formPinNumber.value,
      name: nameToUse,
      key: keyToUse,
      done: formDone.value,
      enabled: formEnabled.value,
    });
    statusMessage.value = `Virtual Pin V${formPinNumber.value} berhasil ditambahkan.`;
  }
  closeModal();
}

function deletePin(pinId) {
  const pin = pins.value.find((p) => p.id === pinId);
  if (pin && confirm(`Hapus virtual pin ${pin.pinLabel} (${pin.name})?`)) {
    iotService.removePin(pinId);
    statusMessage.value = `Virtual pin ${pin.pinLabel} berhasil dihapus.`;
  }
}

function togglePinEnabled(pinId) {
  const updatedState = iotService.togglePinEnabled(pinId);
  const pin = pins.value.find((p) => p.id === pinId);
  const label = pin ? pin.pinLabel : "Pin";
  statusMessage.value = `${label} sekarang ${updatedState ? "AKTIF" : "DISABLED"}.`;
}

function togglePinDone(pinId) {
  const newDone = iotService.togglePinDone(pinId);
  const pin = pins.value.find((p) => p.id === pinId);
  const label = pin ? pin.pinLabel : "Pin";
  statusMessage.value = `${label} status diubah ke ${newDone ? "TRUE" : "FALSE"}.`;
}

function resetPins() {
  if (confirm("Reset semua virtual pin kembali ke 0 pin (default)?")) {
    iotService.clearPins();
    statusMessage.value = "Semua virtual pin direset ke 0 pin.";
  }
}

function clearLogs() {
  iotService.clearLogs();
  statusMessage.value = "Log pesan IOT dibersihkan.";
}

function triggerSimulation() {
  const raw = iotService.simulateMessage(simKey.value, simDone.value);
  statusMessage.value = `Simulasi terkirim: ${raw}`;
}

async function sendBleInstruction() {
  if (!BleService.isConnected()) {
    statusMessage.value = "Robot BLE belum terhubung!";
    return;
  }
  const cmd = `MISI_DONE:${simKey.value},${simDone.value ? 1 : 0}`;
  try {
    statusMessage.value = `Mengirim perintah ${cmd} ke robot...`;
    await BleService.sendCommand(cmd);
    statusMessage.value = `Perintah ${cmd} berhasil dikirim ke robot! Menunggu robot membalas...`;
  } catch (err) {
    statusMessage.value = `Gagal mengirim ke robot: ${err.message || err}`;
  }
}

function formatTime(isoStr) {
  if (!isoStr) return "-";
  try {
    const d = new Date(isoStr);
    return d.toLocaleTimeString();
  } catch {
    return isoStr;
  }
}
</script>
