<template>
  <div class="max-w-4xl mx-auto space-y-8">
    <div
      class="flex flex-col md:flex-row justify-between items-start md:items-center gap-6 border-b-4 border-black pb-8"
    >
      <div>
        <h1 class="text-3xl md:text-4xl font-black uppercase tracking-tight">
          BLE Central Control
        </h1>
      </div>

      <div
        class="text-xl md:text-2xl font-black px-6 py-3 rounded-2xl uppercase border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] flex items-center gap-3 transition-all select-none"
        :class="statusBadgeClasses"
      >
        <span
          class="w-4 h-4 rounded-full border-2 border-black inline-block"
          :class="statusDotClasses"
        ></span>
        <span>STATUS: {{ status }}</span>
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
        class="font-black text-lg px-2 hover:bg-red-200 rounded border-2 border-black"
        title="Tutup pesan"
      >
        ✕
      </button>
    </div>

    <div
      class="border-4 border-black rounded-3xl p-6 md:p-8 shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] transition-all"
      :class="connectionCardBgClass"
    >
      <div v-if="status === 'connected' && connectedInfo" class="space-y-6">
        <div
          class="bg-white border-4 border-black p-5 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)]"
        >
          <div class="flex items-center gap-2">
            <span class="w-3 h-3 rounded-full bg-green-500 animate-ping"></span>
            <span
              class="text-xs font-black uppercase tracking-wider text-gray-500"
            >
              Perangkat Yang Sedang Terhubung:
            </span>
          </div>
          <div
            class="flex flex-col md:flex-row md:items-center justify-between gap-3 mt-2"
          >
            <h3
              class="text-3xl md:text-4xl font-black uppercase text-green-700 tracking-tight"
            >
              {{ connectedInfo.name }}
            </h3>
          </div>
        </div>

        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div
            class="bg-white border-4 border-black p-4 rounded-xl shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] space-y-1"
          >
            <span class="text-xs font-black uppercase text-gray-500"
              >Device ID / Hardware Address</span
            >
            <p
              class="text-sm md:text-base font-mono font-bold text-black break-all select-all"
            >
              {{ connectedInfo.id }}
            </p>
          </div>

          <div
            class="bg-white border-4 border-black p-4 rounded-xl shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] space-y-1"
          >
            <span class="text-xs font-black uppercase text-gray-500"
              >Waktu Terhubung</span
            >
            <p class="text-sm md:text-base font-mono font-bold text-black">
              {{ connectedInfo.connectedAt }}
            </p>
          </div>

          <div
            class="bg-white border-4 border-black p-4 rounded-xl shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] space-y-1"
          >
            <span class="text-xs font-black uppercase text-gray-500"
              >Target GATT Service UUID</span
            >
            <p
              class="text-xs md:text-sm font-mono font-bold text-blue-700 break-all select-all"
            >
              {{ connectedInfo.serviceUUID }}
            </p>
          </div>

          <div
            class="bg-white border-4 border-black p-4 rounded-xl shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] space-y-1"
          >
            <span class="text-xs font-black uppercase text-gray-500"
              >Communication Characteristic UUID</span
            >
            <p
              class="text-xs md:text-sm font-mono font-bold text-purple-700 break-all select-all"
            >
              {{ connectedInfo.characteristicUUID }}
            </p>
          </div>
        </div>
      </div>

      <div
        v-else-if="status === 'connecting'"
        class="bg-white border-4 border-black p-6 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] space-y-4 text-center"
      >
        <div class="inline-block animate-spin text-4xl">⚙️</div>
        <div>
          <p class="text-xs font-black uppercase tracking-wider text-gray-500">
            Sedang Menghubungkan Ke Target:
          </p>
          <h3
            class="text-2xl md:text-3xl font-black uppercase text-blue-600 mt-1"
          >
            {{ targetConnecting?.name || "PERANGKAT BLE" }}
          </h3>
          <p class="text-xs md:text-sm font-mono text-gray-600 mt-1">
            {{ targetConnecting?.id || "Mencari server GATT..." }}
          </p>
        </div>
        <div
          class="bg-blue-50 border-2 border-black p-3 rounded-xl max-w-md mx-auto"
        >
          <p class="text-xs md:text-sm font-bold text-blue-900 animate-pulse">
            Melakukan inisialisasi sesi GATT Server & negosiasi primary
            service...
          </p>
        </div>
      </div>

      <div
        v-else-if="status === 'disconnecting'"
        class="bg-white border-4 border-black p-6 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] space-y-4 text-center"
      >
        <div class="inline-block animate-bounce text-4xl">🔌</div>
        <div>
          <p class="text-xs font-black uppercase tracking-wider text-gray-500">
            Sedang Memutuskan Koneksi Dari:
          </p>
          <h3
            class="text-2xl md:text-3xl font-black uppercase text-orange-600 mt-1"
          >
            {{
              lastDisconnectedDevice?.name ||
              connectedInfo?.name ||
              "PERANGKAT BLE"
            }}
          </h3>
          <p class="text-xs md:text-sm font-mono text-gray-600 mt-1">
            {{ lastDisconnectedDevice?.id || connectedInfo?.id || "" }}
          </p>
        </div>
        <div
          class="bg-orange-50 border-2 border-black p-3 rounded-xl max-w-md mx-auto"
        >
          <p class="text-xs md:text-sm font-bold text-orange-900 animate-pulse">
            Menutup koneksi GATT server dan melepaskan stream...
          </p>
        </div>
      </div>

      <div
        v-else-if="status === 'disconnected'"
        class="bg-white border-4 border-black p-6 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] space-y-4"
      >
        <div class="flex flex-col sm:flex-row items-start gap-4">
          <span class="text-3xl">🔌</span>
          <div class="space-y-1">
            <h3 class="text-2xl font-black uppercase text-red-600">
              Tidak Terhubung Ke Perangkat (Disconnected)
            </h3>
            <p
              v-if="lastDisconnectedDevice"
              class="text-sm font-bold text-gray-700"
            >
              Koneksi ke target
              <span class="font-black text-black underline">{{
                lastDisconnectedDevice.name
              }}</span>
              <span class="font-mono text-xs ml-1"
                >({{ lastDisconnectedDevice.id }})</span
              >
              telah ditutup/terputus.
            </p>
            <p v-else class="text-sm font-bold text-gray-700">
              Saat ini tidak ada perangkat BLE yang tersambung.
            </p>
          </div>
        </div>

        <div class="pt-2 flex flex-wrap gap-3">
          <button
            @click="scanDevices"
            :disabled="scanning"
            class="bg-blue-600 text-white font-black px-6 py-3 rounded-xl border-2 border-black hover:bg-blue-700 transition"
          >
            SCAN ULANG PERANGKAT
          </button>
          <button
            @click="setStatus('waiting')"
            class="bg-yellow-300 text-black font-black px-6 py-3 rounded-xl border-2 border-black hover:bg-yellow-400 transition"
          >
            KEMBALI KE STATUS WAITING
          </button>
        </div>
      </div>

      <div class="bg-yellow-100 border-2 border-black p-3 rounded-xl" v-else>
        <span class="text-xs font-black uppercase text-yellow-900 block"
          >Filter Layanan Target (Primary Service):</span
        >
        <p
          class="text-xs font-mono font-bold text-gray-800 break-all select-all mt-0.5"
        >
          {{ BleService.serviceUUID }}
        </p>
      </div>
    </div>

    <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
      <button
        @click="scanDevices"
        :disabled="
          scanning || status === 'connecting' || status === 'disconnecting'
        "
        class="bg-blue-600 text-white text-2xl font-black py-6 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-blue-700 active:translate-y-1 active:shadow-none transition-all disabled:bg-gray-400 disabled:shadow-none cursor-pointer disabled:cursor-not-allowed"
      >
        {{ scanning ? "SEARCHING..." : "DISCOVER PERIPHERALS" }}
      </button>

      <button
        @click="disconnectAll"
        :disabled="status !== 'connected'"
        class="bg-red-600 text-white text-2xl font-black py-6 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-red-700 active:translate-y-1 active:shadow-none transition-all disabled:bg-gray-400 disabled:shadow-none cursor-pointer disabled:cursor-not-allowed"
      >
        {{
          status === "disconnecting" ? "DISCONNECTING..." : "TERMINATE KONEKSI"
        }}
      </button>
    </div>

    <div class="space-y-6" v-if="status !== 'connected'">
      <div class="flex justify-between items-center">
        <h2 class="text-3xl font-black uppercase tracking-wide">
          Perangkat Terdeteksi
        </h2>
        <span
          v-if="devices.size > 0"
          class="text-sm font-black bg-black text-white px-3 py-1 rounded-full border-2 border-black"
        >
          {{ devices.size }} PERANGKAT
        </span>
      </div>

      <div
        v-if="devices.size === 0"
        class="border-4 border-dashed border-gray-400 p-12 text-center rounded-2xl bg-white/40"
      >
        <h3 class="text-2xl font-bold text-gray-400 uppercase">
          Tidak Ada Data Perangkat
        </h3>
        <p class="text-gray-500 font-bold text-sm mt-2">
          Klik tombol "DISCOVER PERIPHERALS" untuk memindai perangkat BLE di
          sekitar browser Anda.
        </p>
      </div>

      <div v-else class="grid grid-cols-1 gap-4">
        <div
          v-for="[id, device] in devices"
          :key="id"
          class="border-4 border-black p-6 rounded-2xl flex flex-col md:flex-row justify-between items-center bg-white shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] gap-4 transition-all"
          :class="
            connectedInfo?.id === id ? 'bg-green-50 border-green-600' : ''
          "
        >
          <div class="text-center md:text-left space-y-1">
            <div
              class="flex items-center justify-center md:justify-start gap-3 flex-wrap"
            >
              <h3 class="text-2xl font-black uppercase text-black">
                {{ device.name || "UNKNOWN HARDWARE" }}
              </h3>
              <span
                v-if="connectedInfo?.id === id"
                class="bg-green-500 text-white text-xs font-black px-3 py-1 rounded-lg border-2 border-black uppercase"
              >
                TERHUBUNG
              </span>
            </div>
            <p class="text-base font-bold text-gray-600 font-mono break-all">
              {{ id }}
            </p>
          </div>

          <div class="flex items-center gap-3 w-full md:w-auto">
            <button
              v-if="connectedInfo?.id === id"
              @click="disconnectAll"
              :disabled="status === 'disconnecting'"
              class="w-full md:w-auto bg-red-600 text-white text-xl font-black px-8 py-4 rounded-xl border-2 border-black hover:bg-red-700 active:translate-y-1 transition disabled:bg-gray-400 cursor-pointer"
            >
              {{ status === "disconnecting" ? "MEMUTUSKAN..." : "PUTUSKAN" }}
            </button>

            <button
              v-else
              @click="connectDevice(id)"
              :disabled="
                status === 'connecting' ||
                status === 'connected' ||
                status === 'disconnecting'
              "
              class="w-full md:w-auto bg-green-500 text-black text-xl font-black px-8 py-4 rounded-xl border-2 border-black hover:bg-green-400 active:translate-y-1 transition disabled:bg-gray-300 disabled:text-gray-500 disabled:border-gray-400 disabled:cursor-not-allowed cursor-pointer"
            >
              <span
                v-if="status === 'connecting' && targetConnecting?.id === id"
              >
                CONNECTING...
              </span>
              <span v-else> HUBUNGKAN </span>
            </button>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from "vue";
import { useRouter } from "vue-router";
import BleService from "@/services/ble.services";

const router = useRouter();

const status = ref("waiting");
const scanning = ref(false);
const devices = ref(new Map());
const errorMessage = ref("");

const connectedInfo = ref(null);
const targetConnecting = ref(null);
const lastDisconnectedDevice = ref(null);

let removeDisconnectListener = null;

onMounted(() => {
  checkConnectionStatus();

  removeDisconnectListener = BleService.onDisconnect((prevDevice) => {
    lastDisconnectedDevice.value = prevDevice
      ? {
          id: prevDevice.id,
          name: prevDevice.name || "Perangkat BLE",
        }
      : connectedInfo.value;
    connectedInfo.value = null;
    status.value = "disconnected";
  });
});

onUnmounted(() => {
  if (removeDisconnectListener) {
    removeDisconnectListener();
  }
});

function checkConnectionStatus() {
  if (BleService.isConnected()) {
    status.value = "connected";
    const info = BleService.getConnectedDeviceInfo();
    connectedInfo.value = {
      ...info,
      connectedAt:
        connectedInfo.value?.connectedAt || new Date().toLocaleTimeString(),
    };
  } else {
    if (status.value !== "disconnected") {
      status.value = "waiting";
    }
    connectedInfo.value = null;
  }
}

function setStatus(newStatus) {
  status.value = newStatus;
  errorMessage.value = "";
}

async function scanDevices() {
  scanning.value = true;
  errorMessage.value = "";

  if (status.value !== "connected") {
    status.value = "waiting";
  }

  devices.value.clear();
  try {
    const list = await BleService.startScanning();
    devices.value = new Map(list);
  } catch (err) {
    console.error("[BLE] Scan error:", err);
    if (
      err.name === "NotFoundError" ||
      err.message?.includes("User cancelled")
    ) {
      console.log("[BLE] Pemindaian dibatalkan pengguna.");
    } else {
      errorMessage.value = err.message || "Gagal memindai perangkat BLE.";
    }
  } finally {
    scanning.value = false;
  }
}

async function connectDevice(id) {
  const target = devices.value.get(id);
  targetConnecting.value = {
    id,
    name: target?.name || "Unknown Hardware",
  };
  status.value = "connecting";
  errorMessage.value = "";

  try {
    await BleService.connectDevice(id);

    const info = BleService.getConnectedDeviceInfo();
    connectedInfo.value = {
      ...info,
      connectedAt: new Date().toLocaleTimeString(),
    };
    status.value = "connected";
    targetConnecting.value = null;
  } catch (err) {
    console.error("[BLE] Connect error:", err);
    status.value = "disconnected";
    lastDisconnectedDevice.value = {
      id,
      name: target?.name || "Hardware Target",
    };
    targetConnecting.value = null;
    errorMessage.value = `Gagal menghubungkan ke ${target?.name || id}: ${err.message}`;
  }
}

async function disconnectAll() {
  if (!BleService.isConnected() && status.value !== "connected") {
    status.value = "disconnected";
    return;
  }

  status.value = "disconnecting";
  if (connectedInfo.value) {
    lastDisconnectedDevice.value = { ...connectedInfo.value };
  }

  try {
    await BleService.disconnectDevice();
  } catch (err) {
    console.error("[BLE] Disconnect error:", err);
  } finally {
    connectedInfo.value = null;
    status.value = "disconnected";
  }
}

function goToIoT() {
  router.push("/iot-dashboard");
}

const statusBadgeClasses = computed(() => {
  switch (status.value) {
    case "connected":
      return "bg-green-500 text-white animate-pulse";
    case "connecting":
      return "bg-cyan-400 text-black animate-pulse";
    case "disconnecting":
      return "bg-orange-500 text-white animate-pulse";
    case "disconnected":
      return "bg-red-500 text-white";
    case "waiting":
    default:
      return "bg-amber-400 text-black";
  }
});

const statusDotClasses = computed(() => {
  switch (status.value) {
    case "connected":
      return "bg-white animate-ping";
    case "connecting":
      return "bg-blue-900 animate-ping";
    case "disconnecting":
      return "bg-white animate-ping";
    case "disconnected":
      return "bg-black";
    case "waiting":
    default:
      return "bg-black animate-ping";
  }
});

const connectionCardBgClass = computed(() => {
  switch (status.value) {
    case "connected":
      return "bg-emerald-50";
    case "connecting":
      return "bg-cyan-50";
    case "disconnecting":
      return "bg-orange-50";
    case "disconnected":
      return "bg-red-50";
    case "waiting":
    default:
      return "bg-white";
  }
});

const statusSubBadgeClasses = computed(() => {
  switch (status.value) {
    case "connected":
      return "bg-green-400 text-black";
    case "connecting":
      return "bg-cyan-300 text-black";
    case "disconnecting":
      return "bg-orange-400 text-white";
    case "disconnected":
      return "bg-red-400 text-white";
    case "waiting":
    default:
      return "bg-yellow-300 text-black";
  }
});

const statusDescription = computed(() => {
  switch (status.value) {
    case "connected":
      return "Terhubung & Aktif";
    case "connecting":
      return "Proses Menghubungkan";
    case "disconnecting":
      return "Proses Memutuskan";
    case "disconnected":
      return "Koneksi Terputus";
    case "waiting":
    default:
      return "Menunggu Aksi (Standby)";
  }
});
</script>
