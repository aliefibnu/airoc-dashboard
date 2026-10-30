<template>
  <div class="max-w-6xl mx-auto space-y-8 select-none pb-12">
    <div
      class="flex flex-col md:flex-row justify-between items-start md:items-center gap-4 border-b-4 border-black pb-6"
    >
      <div class="space-y-1">
        <div class="flex items-center gap-3">
          <span class="text-3xl">💻</span>
          <h1 class="text-2xl md:text-4xl font-black uppercase tracking-tight">
            KODE C++ LEANBOT
          </h1>
        </div>
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
              connectedDevicesCount > 1
                ? `${connectedDevicesCount} ROBOT CONNECTED`
                : isBleConnected
                  ? connectedDeviceName
                    ? `${connectedDeviceName}`
                    : "BLE CONNECTED"
                  : "BLE DISCONNECTED"
            }}
          </h3>
        </div>
      </div>
    </div>

    <div
      v-if="statusMessage"
      class="border-4 border-black p-4 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] flex items-center justify-between gap-4"
      :class="
        statusMessage.includes('Gagal') || statusMessage.includes('gagal')
          ? 'bg-red-100 text-red-950'
          : 'bg-green-100 text-green-950'
      "
    >
      <div class="flex items-center gap-3">
        <span class="text-2xl">{{
          statusMessage.includes("Gagal") || statusMessage.includes("gagal")
            ? "⚠️"
            : "✅"
        }}</span>
        <span class="font-black text-sm md:text-base uppercase">{{
          statusMessage
        }}</span>
      </div>
      <button
        type="button"
        @click="statusMessage = ''"
        class="font-black text-lg px-2 hover:bg-black/10 rounded border-2 border-black cursor-pointer"
      >
        ✕
      </button>
    </div>

    <div class="space-y-4">
      <div
        class="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4"
      >
        <div class="flex flex-wrap items-center gap-3 w-full sm:w-auto">
          <button
            type="button"
            @click="copyCode"
            class="grow sm:grow-0 font-black text-sm uppercase px-5 py-3 rounded-xl border-3 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer flex items-center justify-center gap-2"
            :class="
              copied
                ? 'bg-green-400 text-black'
                : 'bg-yellow-300 hover:bg-yellow-400 text-black'
            "
          >
            <span>{{ copied ? "✅" : "📋" }}</span>
            <span>{{ copied ? "BERHASIL DISALIN!" : "SALIN KODE" }}</span>
          </button>

          <button
            type="button"
            @click="downloadCppFile"
            class="grow sm:grow-0 font-black text-sm uppercase bg-blue-500 hover:bg-blue-600 text-white px-5 py-3 rounded-xl border-3 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer flex items-center justify-center gap-2"
          >
            <span>💾</span>
            <span>UNDUH .CPP</span>
          </button>

          <button
            type="button"
            @click="downloadInoFile"
            class="grow sm:grow-0 font-black text-sm uppercase bg-purple-600 hover:bg-purple-700 text-white px-5 py-3 rounded-xl border-3 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer flex items-center justify-center gap-2"
          >
            <span>⚡</span>
            <span>UNDUH .INO</span>
          </button>
        </div>
      </div>

      <div class="relative">
        <pre
          class="bg-slate-900 text-green-400 font-mono text-xs md:text-sm p-6 rounded-3xl border-4 border-black shadow-[8px_8px_0px_0px_rgba(0,0,0,1)] select-text overflow-x-auto max-h-[650px] leading-relaxed"
        ><code>{{ generatedCode }}</code></pre>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from "vue";
import BleService from "@/services/ble.services.js";
import remoteDebugService from "@/services/remote-debug.service.js";
import instructionDefaultsService from "@/services/instruction-defaults.service.js";
import codeGeneratorService from "@/services/code-generator.service.js";

const isBleConnected = ref(false);
const connectedDeviceName = ref("");
const connectedDevicesList = ref([]);
const connectedDevicesCount = computed(() => connectedDevicesList.value.length);
const statusMessage = ref("");
const copied = ref(false);
const showBatchesList = ref(false);

const batches = ref(Array(10).fill(""));
const batchesEnabled = ref(Array(10).fill(true));
const defaultMotorSpeed = ref(1000);

let unsubscribeState = null;
let unsubscribeDefaults = null;
let removeDisconnectListener = null;
let removeConnectListener = null;

onMounted(() => {
  checkBle();
  syncFromStores();

  unsubscribeState = remoteDebugService.subscribe((state) => {
    batches.value = [...state.batches];
    batchesEnabled.value = [...state.batchesEnabled];
  });

  unsubscribeDefaults = instructionDefaultsService.subscribe(() => {
    defaultMotorSpeed.value =
      codeGeneratorService.getDefaultSpeedFromSettings();
  });

  removeDisconnectListener = BleService.onDisconnect(() => {
    checkBle();
  });

  removeConnectListener = BleService.onConnect(() => {
    checkBle();
  });
});

onUnmounted(() => {
  if (unsubscribeState) unsubscribeState();
  if (unsubscribeDefaults) unsubscribeDefaults();
  if (removeDisconnectListener) removeDisconnectListener();
  if (removeConnectListener) removeConnectListener();
});

function checkBle() {
  isBleConnected.value = BleService.isConnected();
  connectedDevicesList.value = BleService.getConnectedDevices();
  const info = BleService.getConnectedDeviceInfo();
  connectedDeviceName.value = info ? info.name : "";
}

function syncFromStores() {
  const state = remoteDebugService.readState();
  batches.value = [...state.batches];
  batchesEnabled.value = [...state.batchesEnabled];
  defaultMotorSpeed.value = codeGeneratorService.getDefaultSpeedFromSettings();
}

const enabledBatchesCount = computed(() => {
  return batchesEnabled.value.filter((b) => b !== false).length;
});

const generatedCode = computed(() => {
  return codeGeneratorService.generateCppCode({
    batches: batches.value,
    batchesEnabled: batchesEnabled.value,
    defaultSpeed: defaultMotorSpeed.value,
  });
});

const codeLineCount = computed(() => {
  return generatedCode.value.split("\n").length;
});

async function copyCode() {
  try {
    await navigator.clipboard.writeText(generatedCode.value);
    copied.value = true;
    statusMessage.value = "Kode C++ berhasil disalin ke clipboard!";
    setTimeout(() => {
      copied.value = false;
    }, 2500);
  } catch (err) {
    statusMessage.value = `Gagal menyalin kode: ${err.message || err}`;
  }
}

function downloadCppFile() {
  const blob = new Blob([generatedCode.value], { type: "text/x-c++src" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = "main_leanbot_code_standalone.cpp";
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
  statusMessage.value =
    "File main_leanbot_code_standalone.cpp berhasil diunduh!";
}

function downloadInoFile() {
  const blob = new Blob([generatedCode.value], { type: "text/x-c++src" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = "leanbot_standalone_mission.ino";
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
  statusMessage.value = "File leanbot_standalone_mission.ino berhasil diunduh!";
}
</script>
