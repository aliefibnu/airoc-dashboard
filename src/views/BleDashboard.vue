<template>
  <div class="max-w-5xl mx-auto space-y-8 select-none pb-12">
    <div
      class="flex flex-col md:flex-row justify-between items-start md:items-center gap-6 border-b-4 border-black pb-8"
    >
      <div class="space-y-1">
        <h1 class="text-3xl md:text-4xl font-black uppercase tracking-tight">
          BLE Central Control
        </h1>
        <p class="text-xs md:text-sm font-bold text-gray-700 uppercase">
          Mendukung koneksi simultan hingga 2 robot Leanbot
        </p>
      </div>

      <div
        class="text-lg md:text-xl font-black px-6 py-3 rounded-2xl uppercase border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] flex items-center gap-3 transition-all select-none"
        :class="statusBadgeClasses"
      >
        <span
          class="w-4 h-4 rounded-full border-2 border-black inline-block"
          :class="statusDotClasses"
        ></span>
        <span>{{ statusBadgeText }}</span>
      </div>
    </div>

    <div
      v-if="errorMessage"
      class="border-4 border-black bg-red-100 p-4 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] flex items-center justify-between gap-4"
    >
      <div class="flex items-center gap-3">
        <span class="text-2xl">⚠️</span>
        <span class="font-bold text-red-900 text-sm md:text-base">{{
          errorMessage
        }}</span>
      </div>
      <button
        @click="errorMessage = ''"
        class="font-black text-lg px-2 hover:bg-red-200 rounded border-2 border-black cursor-pointer"
        title="Tutup pesan"
      >
        ✕
      </button>
    </div>

    <div
      v-if="connectedDevices.length > 0"
      class="bg-yellow-100 border-4 border-black p-5 md:p-6 rounded-3xl shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] flex flex-wrap items-center justify-between gap-4"
    >
      <div class="space-y-1">
        <h2 class="text-base md:text-lg font-black uppercase tracking-tight text-black">
          TARGET PENGIRIMAN DATA
        </h2>
        <p class="text-xs font-bold text-gray-700 uppercase">
          Pilih target pengiriman perintah kode atau biarkan broadcast ke semua robot
        </p>
      </div>

      <div class="flex items-center gap-3">
        <select
          v-model="activeTarget"
          @change="onTargetChange"
          class="bg-white border-3 border-black rounded-xl p-2.5 text-sm md:text-base font-black font-mono focus:outline-none focus:bg-yellow-50 shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] cursor-pointer uppercase"
        >
          <option value="all">📢 SEMUA ROBOT (BROADCAST)</option>
          <option
            v-for="(dev, idx) in connectedDevices"
            :key="dev.id"
            :value="dev.id"
          >
            🤖 ROBOT {{ idx + 1 }}: {{ dev.name }}
          </option>
        </select>
      </div>
    </div>

    <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
      <div
        class="border-4 border-black rounded-3xl p-6 shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] flex flex-col justify-between space-y-4 transition-all"
        :class="slot1 ? 'bg-emerald-50' : 'bg-white border-dashed'"
      >
        <div class="space-y-4">
          <div class="flex items-center justify-between border-b-3 border-black pb-3">
            <div class="flex items-center gap-2">
              <span
                class="bg-black text-yellow-300 font-mono font-black text-xs px-2.5 py-1 rounded-lg uppercase"
              >
                SLOT 1
              </span>
              <h3 class="text-xl font-black uppercase tracking-tight text-black">
                {{ slot1 ? slot1.name : "SLOT 1 KOSONG" }}
              </h3>
            </div>

            <span
              v-if="slot1"
              class="bg-green-500 text-white font-black text-xs px-2.5 py-1 rounded-lg border-2 border-black uppercase"
            >
              TERHUBUNG
            </span>
            <span
              v-else
              class="bg-gray-200 text-gray-600 font-black text-xs px-2.5 py-1 rounded-lg border-2 border-gray-400 uppercase"
            >
              STANDBY
            </span>
          </div>

          <div v-if="slot1" class="space-y-2 text-xs font-mono">
            <div class="flex justify-between bg-white border-2 border-black p-2.5 rounded-xl">
              <span class="font-bold text-gray-500">ID / ADDRESS:</span>
              <span class="font-black text-black select-all">{{ slot1.id }}</span>
            </div>
            <div class="flex justify-between bg-white border-2 border-black p-2.5 rounded-xl">
              <span class="font-bold text-gray-500">TERHUBUNG PADA:</span>
              <span class="font-black text-black">{{ slot1.connectedAt }}</span>
            </div>
            <div class="flex justify-between bg-white border-2 border-black p-2.5 rounded-xl">
              <span class="font-bold text-gray-500">SERVICE UUID:</span>
              <span class="font-bold text-blue-700 truncate max-w-[180px]">{{ slot1.serviceUUID }}</span>
            </div>
          </div>

          <div v-else class="py-8 text-center space-y-2">
            <span class="text-4xl">🤖</span>
            <p class="text-sm font-bold text-gray-500 uppercase">
              Slot Robot 1 belum terhubung
            </p>
          </div>
        </div>

        <div class="pt-2 border-t-2 border-black/10">
          <button
            v-if="slot1"
            type="button"
            @click="disconnectSingle(slot1.id)"
            class="w-full bg-red-500 hover:bg-red-600 text-white font-black text-sm uppercase py-3 px-4 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer"
          >
            PUTUSKAN ROBOT 1
          </button>
          <button
            v-else
            type="button"
            @click="scanDevices"
            :disabled="scanning || isConnecting"
            class="w-full bg-blue-600 hover:bg-blue-700 text-white font-black text-sm uppercase py-3 px-4 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer disabled:bg-gray-400 disabled:cursor-not-allowed"
          >
            {{ scanning ? "MEMINDAI..." : "+ HUBUNGKAN ROBOT 1" }}
          </button>
        </div>
      </div>

      <div
        class="border-4 border-black rounded-3xl p-6 shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] flex flex-col justify-between space-y-4 transition-all"
        :class="slot2 ? 'bg-emerald-50' : 'bg-white border-dashed'"
      >
        <div class="space-y-4">
          <div class="flex items-center justify-between border-b-3 border-black pb-3">
            <div class="flex items-center gap-2">
              <span
                class="bg-black text-yellow-300 font-mono font-black text-xs px-2.5 py-1 rounded-lg uppercase"
              >
                SLOT 2
              </span>
              <h3 class="text-xl font-black uppercase tracking-tight text-black">
                {{ slot2 ? slot2.name : "SLOT 2 KOSONG" }}
              </h3>
            </div>

            <span
              v-if="slot2"
              class="bg-green-500 text-white font-black text-xs px-2.5 py-1 rounded-lg border-2 border-black uppercase"
            >
              TERHUBUNG
            </span>
            <span
              v-else
              class="bg-gray-200 text-gray-600 font-black text-xs px-2.5 py-1 rounded-lg border-2 border-gray-400 uppercase"
            >
              STANDBY
            </span>
          </div>

          <div v-if="slot2" class="space-y-2 text-xs font-mono">
            <div class="flex justify-between bg-white border-2 border-black p-2.5 rounded-xl">
              <span class="font-bold text-gray-500">ID / ADDRESS:</span>
              <span class="font-black text-black select-all">{{ slot2.id }}</span>
            </div>
            <div class="flex justify-between bg-white border-2 border-black p-2.5 rounded-xl">
              <span class="font-bold text-gray-500">TERHUBUNG PADA:</span>
              <span class="font-black text-black">{{ slot2.connectedAt }}</span>
            </div>
            <div class="flex justify-between bg-white border-2 border-black p-2.5 rounded-xl">
              <span class="font-bold text-gray-500">SERVICE UUID:</span>
              <span class="font-bold text-blue-700 truncate max-w-[180px]">{{ slot2.serviceUUID }}</span>
            </div>
          </div>

          <div v-else class="py-8 text-center space-y-2">
            <span class="text-4xl">🤖</span>
            <p class="text-sm font-bold text-gray-500 uppercase">
              Slot Robot 2 belum terhubung
            </p>
          </div>
        </div>

        <div class="pt-2 border-t-2 border-black/10">
          <button
            v-if="slot2"
            type="button"
            @click="disconnectSingle(slot2.id)"
            class="w-full bg-red-500 hover:bg-red-600 text-white font-black text-sm uppercase py-3 px-4 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer"
          >
            PUTUSKAN ROBOT 2
          </button>
          <button
            v-else
            type="button"
            @click="scanDevices"
            :disabled="scanning || isConnecting"
            class="w-full bg-blue-600 hover:bg-blue-700 text-white font-black text-sm uppercase py-3 px-4 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer disabled:bg-gray-400 disabled:cursor-not-allowed"
          >
            {{ scanning ? "MEMINDAI..." : "+ HUBUNGKAN ROBOT 2" }}
          </button>
        </div>
      </div>
    </div>

    <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
      <button
        @click="scanDevices"
        :disabled="scanning || connectedDevices.length >= 2 || isConnecting"
        class="bg-blue-600 text-white text-xl md:text-2xl font-black py-5 px-6 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-blue-700 active:translate-y-1 active:shadow-none transition-all disabled:bg-gray-400 disabled:shadow-none cursor-pointer disabled:cursor-not-allowed flex items-center justify-center gap-2"
      >
        <span>📡</span>
        <span>{{
          scanning
            ? "SEARCHING..."
            : connectedDevices.length >= 2
              ? "MAKSIMAL 2 ROBOT PENUH"
              : "CARI & TAMBAH LEANBOT"
        }}</span>
      </button>

      <button
        @click="disconnectAll"
        :disabled="connectedDevices.length === 0"
        class="bg-red-600 text-white text-xl md:text-2xl font-black py-5 px-6 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-red-700 active:translate-y-1 active:shadow-none transition-all disabled:bg-gray-400 disabled:shadow-none cursor-pointer disabled:cursor-not-allowed flex items-center justify-center gap-2"
      >
        <span>🛑</span>
        <span>PUTUSKAN SEMUA ROBOT</span>
      </button>
    </div>

    <div class="space-y-4" v-if="devices.size > 0">
      <div class="flex justify-between items-center">
        <h2 class="text-2xl font-black uppercase tracking-wide">
          Perangkat Ditemukan
        </h2>
        <span
          class="text-sm font-black bg-black text-white px-3 py-1 rounded-full border-2 border-black"
        >
          {{ devices.size }} PERANGKAT
        </span>
      </div>

      <div class="grid grid-cols-1 gap-4">
        <div
          v-for="[id, device] in devices"
          :key="id"
          class="border-4 border-black p-5 rounded-2xl flex flex-col md:flex-row justify-between items-center bg-white shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] gap-4 transition-all"
        >
          <div class="text-center md:text-left space-y-1">
            <div
              class="flex items-center justify-center md:justify-start gap-3 flex-wrap"
            >
              <h3 class="text-xl font-black uppercase text-black">
                {{ device.name || "UNKNOWN HARDWARE" }}
              </h3>
              <span
                v-if="isDeviceConnected(id)"
                class="bg-green-500 text-white text-xs font-black px-2.5 py-0.5 rounded-lg border-2 border-black uppercase"
              >
                TERHUBUNG
              </span>
            </div>
            <p class="text-xs font-bold text-gray-600 font-mono break-all">
              {{ id }}
            </p>
          </div>

          <div class="flex items-center gap-3 w-full md:w-auto">
            <button
              v-if="isDeviceConnected(id)"
              @click="disconnectSingle(id)"
              class="w-full md:w-auto bg-red-600 text-white text-sm font-black px-6 py-3 rounded-xl border-2 border-black hover:bg-red-700 active:translate-y-0.5 transition cursor-pointer"
            >
              PUTUSKAN
            </button>

            <button
              v-else
              @click="connectDevice(id)"
              :disabled="connectedDevices.length >= 2 || isConnecting"
              class="w-full md:w-auto bg-green-500 text-black text-sm font-black px-6 py-3 rounded-xl border-2 border-black hover:bg-green-400 active:translate-y-0.5 transition disabled:bg-gray-300 disabled:text-gray-500 disabled:border-gray-400 disabled:cursor-not-allowed cursor-pointer"
            >
              {{ isConnecting && targetConnectingId === id ? "CONNECTING..." : "HUBUNGKAN" }}
            </button>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from "vue";
import BleService from "@/services/ble.services.js";

const scanning = ref(false);
const isConnecting = ref(false);
const targetConnectingId = ref(null);
const devices = ref(new Map());
const errorMessage = ref("");
const connectedDevices = ref([]);
const activeTarget = ref("all");

let removeDisconnectListener = null;
let removeConnectListener = null;

const slot1 = computed(() => {
  return connectedDevices.value[0] || null;
});

const slot2 = computed(() => {
  return connectedDevices.value[1] || null;
});

const statusBadgeClasses = computed(() => {
  const count = connectedDevices.value.length;
  if (count === 2) return "bg-green-500 text-white animate-pulse";
  if (count === 1) return "bg-blue-600 text-white";
  return "bg-red-500 text-white";
});

const statusDotClasses = computed(() => {
  const count = connectedDevices.value.length;
  if (count > 0) return "bg-white animate-ping";
  return "bg-black";
});

const statusBadgeText = computed(() => {
  const count = connectedDevices.value.length;
  if (count === 2) return "STATUS: 2 / 2 ROBOT TERHUBUNG";
  if (count === 1) return "STATUS: 1 / 2 ROBOT TERHUBUNG";
  return "STATUS: DISCONNECTED (0/2)";
});

onMounted(() => {
  refreshState();

  removeDisconnectListener = BleService.onDisconnect(() => {
    refreshState();
  });

  removeConnectListener = BleService.onConnect(() => {
    refreshState();
  });
});

onUnmounted(() => {
  if (removeDisconnectListener) removeDisconnectListener();
  if (removeConnectListener) removeConnectListener();
});

function refreshState() {
  connectedDevices.value = BleService.getConnectedDevices();
  activeTarget.value = BleService.getActiveTarget();
}

function onTargetChange() {
  BleService.setActiveTarget(activeTarget.value);
}

function isDeviceConnected(id) {
  return connectedDevices.value.some((d) => d.id === id);
}

async function scanDevices() {
  if (connectedDevices.value.length >= 2) {
    errorMessage.value = "Maksimal 2 Leanbot sudah terhubung!";
    return;
  }
  scanning.value = true;
  errorMessage.value = "";
  try {
    const list = await BleService.startScanning();
    devices.value = new Map(list);
  } catch (err) {
    if (err.name !== "NotFoundError" && !err.message?.includes("User cancelled")) {
      errorMessage.value = err.message || "Gagal memindai perangkat BLE.";
    }
  } finally {
    scanning.value = false;
  }
}

async function connectDevice(id) {
  if (connectedDevices.value.length >= 2) {
    errorMessage.value = "Maksimal 2 Leanbot sudah terhubung!";
    return;
  }
  isConnecting.value = true;
  targetConnectingId.value = id;
  errorMessage.value = "";

  try {
    await BleService.connectDevice(id);
    refreshState();
  } catch (err) {
    errorMessage.value = `Gagal menghubungkan: ${err.message || err}`;
  } finally {
    isConnecting.value = false;
    targetConnectingId.value = null;
  }
}

async function disconnectSingle(id) {
  try {
    await BleService.disconnectDevice(id);
    refreshState();
  } catch (err) {
    errorMessage.value = `Gagal memutuskan koneksi: ${err.message || err}`;
  }
}

async function disconnectAll() {
  try {
    await BleService.disconnectAll();
    refreshState();
  } catch (err) {
    errorMessage.value = `Gagal memutuskan semua: ${err.message || err}`;
  }
}
</script>
