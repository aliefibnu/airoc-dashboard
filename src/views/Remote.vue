<template>
  <div class="max-w-6xl mx-auto space-y-8 select-none pb-12">
    <div
      class="flex flex-col md:flex-row justify-between items-start md:items-center gap-4 border-b-4 border-black pb-6"
    >
      <div>
        <h1 class="text-3xl md:text-4xl font-black uppercase tracking-tight">
          REMOTE DEBUGGING CONTROLLER
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
        statusMessage.includes('Gagal')
          ? 'bg-red-100 text-red-950'
          : 'bg-blue-100 text-blue-950'
      "
    >
      <div class="flex items-center gap-3">
        <span class="text-2xl">{{
          statusMessage.includes("Gagal") ? "⚠️" : "ℹ️"
        }}</span>
        <span class="font-black text-sm md:text-base uppercase">{{
          statusMessage
        }}</span>
      </div>
      <button
        @click="statusMessage = ''"
        class="font-black text-lg px-2 hover:bg-black/10 rounded border-2 border-black"
      >
        ✕
      </button>
    </div>

    <div
      class="border-4 border-black bg-yellow-50 p-6 md:p-8 rounded-3xl shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] space-y-4"
    >
      <textarea
        v-model="manualInstructionString"
        rows="3"
        placeholder="KETIK ATAU KONFIRMASI TOMBOL DI BAWAH (CONTOH: SPEED:5;MAJU:10;BELOK_KANAN:90;FOLLOW_LINE:1;CAPIT;BUNYI:2;END)..."
        class="w-full font-mono text-base md:text-lg font-black bg-white border-4 border-black p-4 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] focus:outline-none focus:bg-yellow-100 select-text"
      ></textarea>

      <div class="flex flex-wrap items-center justify-between gap-4 pt-2">
        <div class="flex flex-wrap items-center gap-3">
          <button
            type="button"
            @click="runProdMission"
            class="bg-green-500 text-black font-black uppercase text-base md:text-lg py-3.5 px-6 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
          >
            <span>🚀</span>
            <h3>JALANKAN MISI (CODE)</h3>
          </button>

          <button
            type="button"
            @click="saveManualInstruction"
            class="bg-yellow-400 text-black font-black uppercase text-base md:text-lg py-3.5 px-5 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-yellow-300 active:translate-y-1 active:shadow-none transition-all cursor-pointer"
          >
            <h3>SIMPAN INSTRUKSI</h3>
          </button>
        </div>

        <button
          type="button"
          @click="emergencyStop"
          class="bg-red-600 text-white font-black uppercase text-base md:text-lg py-3.5 px-6 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-red-700 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
        >
          <span>🛑</span>
          <h3>EMERGENCY STOP</h3>
        </button>
      </div>
    </div>

    <div class="space-y-6">
      <div
        v-for="cat in INSTRUCTION_CATEGORIES"
        :key="cat.id"
        class="space-y-4"
      >
        <h3
          class="text-lg font-black uppercase tracking-wider text-black border-l-4 border-black pl-3"
        >
          {{ cat.title }}
        </h3>
        <div
          class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-4"
        >
          <InstructionButton
            v-for="item in getInstructionsByCategory(cat.id)"
            :key="item.alias"
            :alias="item.alias"
            :label="item.label"
            :func="(p) => executeTestInstruction(item.alias, p)"
          />
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from "vue";
import { RouterLink } from "vue-router";
import InstructionButton from "@/components/InstructionButton.vue";
import remoteDebugService from "@/services/remote-debug.service.js";
import BleService from "@/services/ble.services.js";
import instructionDefaultsService from "@/services/instruction-defaults.service.js";
import {
  INSTRUCTION_CATEGORIES,
  INSTRUCTION_ALIASES,
} from "@/data/instruction-aliases.js";

const manualInstructionString = ref("");
const instructionList = ref([]);
const isBleConnected = ref(false);
const connectedDeviceName = ref("");
const statusMessage = ref("");
const lastSentCommand = ref("");

let unsubscribeState = null;
let removeDisconnectListener = null;

onMounted(() => {
  refreshState();
  checkBle();

  unsubscribeState = remoteDebugService.subscribe((state) => {
    manualInstructionString.value = state.instructionString;
    instructionList.value = state.instructions;
  });

  removeDisconnectListener = BleService.onDisconnect(() => {
    checkBle();
  });
});

onUnmounted(() => {
  if (unsubscribeState) unsubscribeState();
  if (removeDisconnectListener) removeDisconnectListener();
});

function getInstructionsByCategory(catId) {
  return INSTRUCTION_ALIASES.filter((item) => item.categoryId === catId);
}

function refreshState() {
  const state = remoteDebugService.readState();
  manualInstructionString.value = state.instructionString;
  instructionList.value = state.instructions;
}

function checkBle() {
  isBleConnected.value = BleService.isConnected();
  const info = BleService.getConnectedDeviceInfo();
  connectedDeviceName.value = info ? info.name : "";
}

async function connectBle() {
  try {
    statusMessage.value = "Membuka jendela pemindaian Bluetooth...";
    const peripherals = await BleService.startScanning();
    const firstId = peripherals.keys().next().value;
    if (firstId) {
      statusMessage.value = "Menghubungkan ke robot...";
      await BleService.connectDevice(firstId);
      checkBle();
      statusMessage.value = `Berhasil terhubung ke ${connectedDeviceName.value || "Robot"}! Default value otomatis disinkronkan.`;
    }
  } catch (err) {
    statusMessage.value = `Gagal menghubungkan BLE: ${err.message || err}`;
  }
}

async function disconnectBle() {
  try {
    await BleService.disconnectDevice();
    checkBle();
    statusMessage.value = "Koneksi Bluetooth diputuskan.";
  } catch (err) {
    statusMessage.value = `Error disconnect: ${err.message || err}`;
  }
}

async function sendRawBle(cmd) {
  if (!BleService.isConnected()) {
    statusMessage.value = `Gagal kirim "${cmd}": Robot BLE belum terhubung!`;
    throw new Error("Robot BLE belum terhubung");
  }
  await BleService.sendCommand(cmd);
  lastSentCommand.value = cmd;
  statusMessage.value = `Berhasil kirim instruksi: ${cmd}`;
}

async function executeTestInstruction(name, payload) {
  let cmd = name;
  const clean = remoteDebugService.normalizeParams(payload);
  if (clean.length > 0) {
    cmd = `${name}:${clean.join(",")}`;
  }
  await sendRawBle(cmd);
}

let abortBatchExecution = false;

function parseInstructionBatches(rawStr, batchSize = 12) {
  let cleaned = rawStr.trim();
  if (cleaned.toUpperCase().startsWith("CODE;")) {
    cleaned = cleaned.slice(5).trim();
  }
  const tokens = cleaned
    .split(";")
    .map((t) => t.trim())
    .filter((t) => t.length > 0 && t.toUpperCase() !== "CODE" && t.toUpperCase() !== "PROD");

  if (tokens.length === 0) return [];

  const batches = [];
  for (let i = 0; i < tokens.length; i += batchSize) {
    const chunk = tokens.slice(i, i + batchSize);
    if (!chunk.some((c) => c.toUpperCase() === "END" || c.toUpperCase() === "STOP")) {
      chunk.push("END");
    }
    batches.push(`CODE;${chunk.join(";")}`);
  }
  return batches;
}

async function runProdMission() {
  const str = manualInstructionString.value.trim();
  if (!str) {
    statusMessage.value =
      "String instruksi kosong! Silakan tambahkan instruksi terlebih dahulu.";
    return;
  }
  if (!BleService.isConnected()) {
    statusMessage.value = "Robot BLE belum terhubung!";
    return;
  }

  const batches = parseInstructionBatches(str, 12);
  if (batches.length === 0) {
    statusMessage.value = "Tidak ada instruksi yang valid untuk dikirim.";
    return;
  }

  abortBatchExecution = false;

  try {
    for (let i = 0; i < batches.length; i++) {
      if (abortBatchExecution) {
        statusMessage.value = "Eksekusi batch dibatalkan!";
        break;
      }

      statusMessage.value = batches.length > 1
        ? `Mengirim Batch ${i + 1}/${batches.length}...`
        : "Mengirim instruksi misi...";

      await sendRawBle(batches[i]);

      if (i < batches.length - 1) {
        statusMessage.value = `Batch ${i + 1}/${batches.length} terkirim. Menunggu robot menyelesaikan batch...`;
        await BleService.waitForBatchCompletion(15000);
      }
    }

    if (!abortBatchExecution) {
      statusMessage.value = batches.length > 1
        ? `Semua ${batches.length} batch misi berhasil dikirim dan selesai!`
        : "Instruksi misi berhasil dikirim ke robot!";
    }
  } catch (err) {
    statusMessage.value = `Gagal menjalankan misi: ${err.message || err}`;
  }
}

async function emergencyStop() {
  abortBatchExecution = true;
  try {
    await sendRawBle("EMERGENCY_STOP");
  } catch (err) {
    statusMessage.value = `Emergency Stop gagal: ${err.message || err}`;
  }
}

function saveManualInstruction() {
  remoteDebugService.setInstructionString(manualInstructionString.value);
  refreshState();
  statusMessage.value = "INSTRUKSI BERHASIL DISIMPAN KE PENYIMPANAN LOKAL";
  setTimeout(() => {
    if (statusMessage.value.includes("BERHASIL DISIMPAN")) {
      statusMessage.value = "";
    }
  }, 2500);
}
</script>
