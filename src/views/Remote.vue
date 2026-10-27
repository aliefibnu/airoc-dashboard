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
      <div class="space-y-3">
        <div class="flex flex-wrap items-center justify-between gap-2">
          <button
            type="button"
            @click="clearAllBatches"
            class="font-black text-xs uppercase bg-red-100 hover:bg-red-200 text-red-900 border-2 border-black px-2.5 py-1 rounded-lg cursor-pointer transition-colors shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none"
          >
            RESET SEMUA BATCH
          </button>
        </div>

        <div class="grid grid-cols-5 sm:grid-cols-10 gap-2">
          <button
            v-for="bNum in 10"
            :key="bNum"
            type="button"
            @click="selectBatch(bNum - 1)"
            class="py-2 px-1 sm:px-2 rounded-xl border-3 border-black text-center font-black transition-all cursor-pointer flex flex-col items-center justify-center gap-0.5 select-none"
            :class="
              activeBatchIndex === bNum - 1
                ? 'bg-yellow-400 text-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] -translate-y-0.5'
                : getBatchCount(bNum - 1) > 0
                  ? 'bg-green-100 text-black border-black shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] hover:bg-green-200'
                  : 'bg-white text-gray-400 border-gray-400 hover:bg-gray-100 hover:text-black'
            "
          >
            <span class="text-xs uppercase tracking-tight">B{{ bNum }}</span>
            <span
              class="text-[10px] font-mono font-bold"
              :class="
                getBatchCount(bNum - 1) >= 12 ? 'text-red-600 font-black' : ''
              "
            >
              {{ getBatchCount(bNum - 1) }}/12
            </span>
          </button>
        </div>
      </div>

      <div class="space-y-2">
        <div class="flex items-center justify-between">
          <button
            v-if="currentBatchString.length > 0"
            type="button"
            @click="clearCurrentBatch"
            class="text-xs font-black uppercase text-red-600 hover:text-red-700 cursor-pointer underline"
          >
            KOSONGKAN BATCH INI
          </button>
        </div>

        <textarea
          v-model="currentBatchString"
          @input="onCurrentBatchInput"
          @change="onCurrentBatchInput"
          rows="3"
          :placeholder="`KETIK ATAU KLIK TOMBOL INSTRUKSI UNTUK BATCH ${activeBatchIndex + 1} (CONTOH: MAJU:10;KANAN:90;CAPIT;END)...`"
          class="w-full font-mono text-base md:text-lg font-black bg-white border-4 border-black p-4 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] focus:outline-none focus:bg-yellow-100 select-text"
        ></textarea>
      </div>

      <div class="flex flex-wrap items-center justify-between gap-4 pt-2">
        <button
          type="button"
          @click="runProdMission"
          class="bg-green-500 text-black font-black uppercase text-base md:text-lg py-3.5 px-6 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
        >
          <h3>JALANKAN MISI</h3>
        </button>

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

const batchList = ref(Array(10).fill(""));
const activeBatchIndex = ref(0);
const currentBatchString = ref("");
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
    batchList.value = state.batches || Array(10).fill("");
    activeBatchIndex.value = state.activeBatchIndex || 0;
    currentBatchString.value = batchList.value[activeBatchIndex.value] || "";
    instructionList.value = state.instructions || [];
  });

  removeDisconnectListener = BleService.onDisconnect(() => {
    checkBle();
  });
});

onUnmounted(() => {
  if (unsubscribeState) unsubscribeState();
  if (removeDisconnectListener) removeDisconnectListener();
});

function refreshState() {
  const state = remoteDebugService.readState();
  batchList.value = state.batches || Array(10).fill("");
  activeBatchIndex.value = state.activeBatchIndex || 0;
  currentBatchString.value = batchList.value[activeBatchIndex.value] || "";
  instructionList.value = state.instructions || [];
}

function selectBatch(index) {
  remoteDebugService.setActiveBatchIndex(index);
}

function getBatchCount(index) {
  const str = batchList.value[index] || "";
  if (!str.trim()) return 0;
  return str
    .split(";")
    .map((t) => t.trim())
    .filter((t) => t.length > 0 && t.toUpperCase() !== "END").length;
}

function clearCurrentBatch() {
  remoteDebugService.clearBatch(activeBatchIndex.value);
}

function clearAllBatches() {
  remoteDebugService.clearAllBatches();
}

function onCurrentBatchInput(e) {
  const val = e.target.value;
  remoteDebugService.setBatchInstructionString(activeBatchIndex.value, val);
}

function getInstructionsByCategory(catId) {
  return INSTRUCTION_ALIASES.filter((item) => item.categoryId === catId);
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
      statusMessage.value = `Berhasil terhubung ke ${connectedDeviceName.value || "Robot"}!`;
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
  statusMessage.value = `Berhasil kirim: ${cmd}`;
}

function resolveInstructionToken(token) {
  const trimmed = String(token || "").trim();
  if (!trimmed) return "";
  const colonIdx = trimmed.indexOf(":");
  let cmd = trimmed;
  let paramPart = "";
  if (colonIdx !== -1) {
    cmd = trimmed.slice(0, colonIdx).trim().toUpperCase();
    paramPart = trimmed.slice(colonIdx + 1).trim();
  } else {
    cmd = trimmed.toUpperCase();
  }

  if (paramPart.length > 0) {
    return `${cmd}:${paramPart}`;
  }

  const defs = instructionDefaultsService.getAllDefaults();
  const defForCmd = defs[cmd];
  if (defForCmd && typeof defForCmd === "object") {
    const vals = Object.values(defForCmd).filter(
      (v) => v !== undefined && v !== null && v !== "",
    );
    if (vals.length > 0) {
      return `${cmd}:${vals.join(",")}`;
    }
  }

  const action = INSTRUCTION_ALIASES.find(
    (item) =>
      item.alias.toUpperCase() === cmd || item.name.toUpperCase() === cmd,
  );
  if (action && Array.isArray(action.params) && action.params.length > 0) {
    const defaultVals = action.params
      .map((p) => p.defaultValue)
      .filter((v) => v !== undefined && v !== null && v !== "");
    if (defaultVals.length > 0) {
      return `${cmd}:${defaultVals.join(",")}`;
    }
  }

  return cmd;
}

async function executeTestInstruction(name, payload) {
  let cmd = name;
  const clean = remoteDebugService.normalizeParams(payload);
  if (clean.length > 0) {
    cmd = `${name}:${clean.join(",")}`;
  } else {
    cmd = resolveInstructionToken(name);
  }
  await sendRawBle(cmd);
}

let abortBatchExecution = false;

async function runProdMission() {
  if (!BleService.isConnected()) {
    statusMessage.value = "Robot BLE belum terhubung!";
    return;
  }

  const allBatches = remoteDebugService.getAllBatches();
  const preparedBatches = [];

  for (let b = 0; b < 10; b++) {
    const raw = String(allBatches[b] || "").trim();
    if (!raw) continue;
    const tokens = raw
      .split(";")
      .map((t) => t.trim())
      .filter(
        (t) =>
          t.length > 0 &&
          t.toUpperCase() !== "END" &&
          t.toUpperCase() !== "CODE" &&
          t.toUpperCase() !== "PROD",
      )
      .map((t) => resolveInstructionToken(t));

    if (tokens.length > 0) {
      const chunk = tokens.slice(0, 12);
      chunk.push("END");
      preparedBatches.push({
        batchNum: b + 1,
        command: `BATCH:${b + 1};${chunk.join(";")}`,
        count: chunk.length - 1,
      });
    }
  }

  if (preparedBatches.length === 0) {
    statusMessage.value =
      "Semua batch kosong! Silakan tambahkan instruksi terlebih dahulu.";
    return;
  }

  abortBatchExecution = false;

  try {
    statusMessage.value = "Mereset variabel batch di robot...";
    const clearPromise = BleService.waitForAcknowledgment("[BATCHES_CLEARED]", 2000);
    await sendRawBle("CLEAR");
    await clearPromise;
    await new Promise((r) => setTimeout(r, 60));

    for (const pb of preparedBatches) {
      if (abortBatchExecution) {
        statusMessage.value = "Pengunggahan batch dibatalkan!";
        return;
      }

      statusMessage.value = `Mengunggah Batch ${pb.batchNum} (${pb.count} instruksi) ke robot...`;
      const ackPromise = BleService.waitForAcknowledgment(
        `[BATCH_LOADED:${pb.batchNum}`,
        5000,
      );
      await sendRawBle(pb.command);
      const ack = await ackPromise;
      if (!ack.success) {
        throw new Error(`Robot tidak mengonfirmasi Batch ${pb.batchNum}: ${ack.reason || "Timeout"}`);
      }
      await new Promise((r) => setTimeout(r, 60));
    }

    if (abortBatchExecution) {
      statusMessage.value = "Pengunggahan batch dibatalkan!";
      return;
    }

    statusMessage.value = `Semua ${preparedBatches.length} batch berhasil diterima robot! Memulai eksekusi...`;
    const resPromise = BleService.waitForBatchCompletion(300000);
    await sendRawBle("START_MISSION");

    statusMessage.value =
      "Robot sedang menjalankan seluruh misi dari Batch 1...";
    const res = await resPromise;

    if (res.success) {
      statusMessage.value =
        "Seluruh instruksi misi berhasil diselesaikan oleh robot!";
    } else {
      statusMessage.value = `Misi berhenti: ${res.reason}`;
    }
  } catch (err) {
    statusMessage.value = `Gagal menjalankan misi: ${err.message || err}`;
  }
}

async function emergencyStop() {
  abortBatchExecution = true;
  try {
    await sendRawBle("EMERGENCY_STOP");
    statusMessage.value =
      "EMERGENCY STOP DIKIRIM! Robot dihentikan dan semua batch direset.";
  } catch (err) {
    statusMessage.value = `Emergency Stop gagal: ${err.message || err}`;
  }
}
</script>
