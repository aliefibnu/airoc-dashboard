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
                ? connectedDevicesList.length === 2
                  ? `2 ROBOT: ${connectedDevicesList.map((d) => d.name).join(' & ')}`
                  : `1 ROBOT: ${connectedDeviceName || 'BLE CONNECTED'}`
                : "BLE DISCONNECTED"
            }}
          </h3>
        </div>

        <select
          v-if="connectedDevicesList.length > 1"
          v-model="activeBleTarget"
          @change="onBleTargetChange"
          class="bg-white border-3 border-black rounded-2xl px-3 py-3 text-xs md:text-sm font-black uppercase font-mono shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] cursor-pointer"
        >
          <option value="all">📢 SEMUA ROBOT (BROADCAST)</option>
          <option
            v-for="(dev, idx) in connectedDevicesList"
            :key="dev.id"
            :value="dev.id"
          >
            🤖 R{{ idx + 1 }}: {{ dev.name }}
          </option>
        </select>

        <button
          v-if="connectedDevicesList.length < 2"
          type="button"
          @click="connectBle"
          class="bg-blue-600 text-white font-black text-sm md:text-base py-3 px-5 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-blue-700 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
        >
          <span>📡</span>
          <span>{{ connectedDevicesList.length === 1 ? '+ ROBOT 2' : 'HUBUNGKAN BLE' }}</span>
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
          <div class="flex flex-wrap items-center gap-2">
            <button
              type="button"
              @click="exportBatchesJson"
              class="font-black text-xs uppercase bg-blue-100 hover:bg-blue-200 text-blue-900 border-2 border-black px-3 py-1.5 rounded-lg cursor-pointer transition-colors shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none flex items-center gap-1.5"
            >
              <span>EKSPOR INSTRUKSI</span>
            </button>

            <button
              type="button"
              @click="openImportModal"
              class="font-black text-xs uppercase bg-purple-100 hover:bg-purple-200 text-purple-900 border-2 border-black px-3 py-1.5 rounded-lg cursor-pointer transition-colors shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none flex items-center gap-1.5"
            >
              <span>IMPOR INSTRUKSI</span>
            </button>
          </div>

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
            class="py-2 px-1 sm:px-2 rounded-xl border-3 text-center font-black transition-all cursor-pointer flex flex-col items-center justify-center gap-0.5 select-none relative"
            :class="[
              !isBatchEnabled(bNum - 1)
                ? 'opacity-60 bg-gray-200 text-gray-500 border-gray-500'
                : activeBatchIndex === bNum - 1
                  ? 'bg-yellow-400 text-black border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] -translate-y-0.5'
                  : getBatchCount(bNum - 1) > 0
                    ? 'bg-green-100 text-black border-black shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] hover:bg-green-200'
                    : 'bg-white text-gray-400 border-gray-400 hover:bg-gray-100 hover:text-black',
            ]"
            :title="
              getBatchName(bNum - 1)
                ? `Batch ${bNum}: ${getBatchName(bNum - 1)}`
                : `Batch ${bNum}`
            "
          >
            <div class="flex items-center gap-1">
              <span
                class="text-xs uppercase tracking-tight"
                :class="
                  !isBatchEnabled(bNum - 1) ? 'line-through text-red-600' : ''
                "
                >B{{ bNum }}</span
              >
              <span
                v-if="!isBatchEnabled(bNum - 1)"
                class="text-[9px] font-black text-red-600"
                >✕</span
              >
            </div>
            <span
              v-if="getBatchName(bNum - 1)"
              class="text-[8px] font-mono font-black max-w-full truncate px-1 rounded bg-black/10 text-black leading-tight"
            >
              {{ getBatchName(bNum - 1) }}
            </span>
            <span
              class="text-[10px] font-mono font-bold"
              :class="
                !isBatchEnabled(bNum - 1)
                  ? 'text-gray-500'
                  : getBatchCount(bNum - 1) >= 12
                    ? 'text-red-600 font-black'
                    : ''
              "
            >
              {{ getBatchCount(bNum - 1) }}/12
            </span>
          </button>
        </div>
      </div>

      <div class="space-y-2">
        <div class="flex flex-wrap items-center justify-between gap-2">
          <div class="flex flex-wrap items-center gap-2">
            <span
              class="font-black text-xs uppercase px-2.5 py-1.5 rounded-xl border-2 border-black bg-yellow-400 text-black shadow-[2px_2px_0px_0px_rgba(0,0,0,1)]"
            >
              BATCH {{ activeBatchIndex + 1 }}
            </span>

            <span
              v-if="getBatchName(activeBatchIndex)"
              class="font-black font-mono text-xs uppercase px-2.5 py-1.5 rounded-xl border-2 border-black bg-blue-100 text-blue-900 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)]"
            >
              {{ getBatchName(activeBatchIndex) }}
            </span>

            <button
              type="button"
              @click="openRenameModal(activeBatchIndex)"
              class="font-black text-xs uppercase bg-white hover:bg-yellow-200 text-black border-2 border-black px-2.5 py-1.5 rounded-xl cursor-pointer shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all flex items-center gap-1.5"
              title="Ubah nama batch"
            >
              <span>✏️</span>
              <span>RENAME</span>
            </button>
          </div>

          <div class="flex flex-wrap items-center gap-2">
            <button
              type="button"
              @click="toggleActiveBatchEnabled"
              class="font-black text-xs uppercase px-3 py-1.5 rounded-xl border-2 border-black flex items-center gap-1.5 cursor-pointer shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all"
              :class="
                isCurrentBatchEnabled
                  ? 'bg-green-400 text-black hover:bg-green-300'
                  : 'bg-red-500 text-white hover:bg-red-600'
              "
            >
              <span>{{
                isCurrentBatchEnabled
                  ? "✅ BATCH DIAKTIFKAN"
                  : "🚫 BATCH DINONAKTIFKAN"
              }}</span>
              <span class="text-[10px] opacity-90 font-bold"
                >({{
                  isCurrentBatchEnabled ? "AKAN DIKIRIM" : "TIDAK DIKIRIM"
                }})</span
              >
            </button>

            <button
              v-if="currentBatchString.length > 0"
              type="button"
              @click="clearCurrentBatch"
              class="text-xs font-black uppercase text-red-600 hover:text-red-700 cursor-pointer underline px-1"
            >
              KOSONGKAN
            </button>
          </div>
        </div>

        <textarea
          v-model="currentBatchString"
          @input="onCurrentBatchInput"
          @change="onCurrentBatchInput"
          rows="3"
          :placeholder="`KETIK ATAU KLIK TOMBOL INSTRUKSI UNTUK ${activeBatchDisplayName} (CONTOH: MAJU:10;KANAN:90;CAPIT;END)...`"
          class="w-full font-mono text-base md:text-lg font-black bg-white border-4 border-black p-4 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] focus:outline-none focus:bg-yellow-100 select-text"
          :class="!isCurrentBatchEnabled ? 'bg-amber-50' : ''"
        ></textarea>
      </div>

      <div class="flex flex-wrap items-center justify-between gap-4 pt-2">
        <div class="flex flex-wrap items-center gap-3">
          <button
            type="button"
            @click="runProdMission"
            class="bg-green-500 text-black font-black uppercase text-sm md:text-base py-3.5 px-5 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
          >
            <h3>TEST PRODUCTION INSTRUCTION</h3>
          </button>

          <button
            type="button"
            @click="goToProductionArea"
            class="bg-yellow-400 text-black font-black uppercase text-sm md:text-base py-3.5 px-5 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-yellow-300 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
          >
            <h3>START PRODUCTION INSTRUCTION</h3>
          </button>
        </div>

        <button
          type="button"
          @click="emergencyStop"
          class="bg-red-600 text-white font-black uppercase text-sm md:text-base py-3.5 px-5 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-red-700 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
        >
          <span>🛑</span>
          <h3>EMERGENCY STOP</h3>
        </button>
      </div>
    </div>

    <div class="space-y-6">
      <div
        v-if="!isCurrentBatchEnabled"
        class="border-4 border-black p-4 rounded-2xl bg-amber-200 text-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 select-none"
      >
        <div class="flex items-center gap-3">
          <span class="text-2xl">🚫</span>
          <div>
            <p class="font-black text-sm uppercase">
              BATCH {{ activeBatchIndex + 1 }} SEDANG DINONAKTIFKAN
            </p>
            <p class="font-bold text-xs uppercase opacity-90">
              Semua tombol instruksi dinonaktifkan. Anda tetap dapat mengedit instruksi langsung melalui textarea di atas.
            </p>
          </div>
        </div>
        <button
          type="button"
          @click="toggleActiveBatchEnabled"
          class="font-black text-xs uppercase bg-green-500 hover:bg-green-400 text-black border-2 border-black px-3.5 py-2 rounded-xl cursor-pointer shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all shrink-0"
        >
          AKTIFKAN BATCH
        </button>
      </div>

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
            :disabled="!isCurrentBatchEnabled"
            :func="(p) => executeTestInstruction(item.alias, p)"
          />
        </div>
      </div>
    </div>
    <teleport to="body">
      <div
        v-if="isImportModalOpen"
        class="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/60 backdrop-blur-xs select-none"
        @click.self="closeImportModal"
      >
        <div
          class="bg-yellow-50 border-4 border-black rounded-3xl p-6 md:p-8 max-w-xl w-full shadow-[8px_8px_0px_0px_rgba(0,0,0,1)] space-y-5"
        >
          <div
            class="flex items-center justify-between border-b-4 border-black pb-3 gap-3"
          >
            <div class="flex items-center gap-2">
              <span class="text-2xl">📥</span>
              <h2
                class="text-2xl font-black uppercase tracking-tight text-black"
              >
                IMPOR INSTRUKSI (BATCH 1-10)
              </h2>
            </div>
            <button
              type="button"
              @click="closeImportModal"
              class="border-2 border-black font-black text-xl px-2.5 py-0.5 rounded-xl bg-red-500 text-white hover:bg-red-600 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer shrink-0"
            >
              ✕
            </button>
          </div>

          <div class="space-y-2">
            <div class="flex items-center justify-between">
              <button
                type="button"
                @click="pasteFromClipboard"
                class="text-xs font-black uppercase bg-blue-600 hover:bg-blue-700 text-white border-2 border-black px-3 py-1 rounded-lg cursor-pointer shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all"
              >
                TEMPEL DARI CLIPBOARD
              </button>
            </div>

            <textarea
              v-model="importJsonText"
              rows="6"
              placeholder='Contoh: {"batches": ["MAJU:10;END", "FOLLOW_LINE:1,5,MAJU;END"]}'
              class="w-full bg-white border-4 border-black rounded-xl p-3 font-mono text-sm font-bold focus:outline-none focus:bg-yellow-100 shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] select-text"
            ></textarea>

            <p
              v-if="importErrorMessage"
              class="text-xs font-black text-red-600 uppercase"
            >
              ⚠️ {{ importErrorMessage }}
            </p>
          </div>

          <div class="flex items-center justify-end gap-3 pt-2">
            <button
              type="button"
              @click="closeImportModal"
              class="bg-slate-200 text-black font-black uppercase text-sm py-3 px-5 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] hover:bg-slate-300 active:translate-y-0.5 active:shadow-none transition-all cursor-pointer"
            >
              BATAL
            </button>

            <button
              type="button"
              @click="confirmImport"
              class="bg-green-500 text-black font-black uppercase text-sm py-3 px-6 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-0.5 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
            >
              <span>KONFIRMASI IMPOR</span>
            </button>
          </div>
        </div>
      </div>
    </teleport>

    <BatchRenameModal
      :is-open="isRenameModalOpen"
      :batch-index="renamingBatchIndex"
      :initial-name="getBatchName(renamingBatchIndex)"
      @close="isRenameModalOpen = false"
      @save="onSaveBatchName"
    />

    <CalibrationResultModal
      :is-open="isCalibrationModalOpen"
      @close="isCalibrationModalOpen = false"
      @confirm="handleCalibrationConfirm"
    />
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from "vue";
import { RouterLink, useRouter } from "vue-router";
import InstructionButton from "@/components/InstructionButton.vue";
import BatchRenameModal from "@/components/BatchRenameModal.vue";
import CalibrationResultModal from "@/components/CalibrationResultModal.vue";
import remoteDebugService from "@/services/remote-debug.service.js";
import BleService from "@/services/ble.services.js";
import instructionDefaultsService from "@/services/instruction-defaults.service.js";
import {
  INSTRUCTION_CATEGORIES,
  INSTRUCTION_ALIASES,
} from "@/data/instruction-aliases.js";

const batchList = ref(Array(10).fill(""));
const batchesEnabled = ref(Array(10).fill(true));
const batchNames = ref(Array(10).fill(""));
const activeBatchIndex = ref(0);
const currentBatchString = ref("");
const instructionList = ref([]);
const isBleConnected = ref(false);
const connectedDeviceName = ref("");
const connectedDevicesList = ref([]);
const activeBleTarget = ref("all");
const statusMessage = ref("");
const lastSentCommand = ref("");
const isImportModalOpen = ref(false);
const importJsonText = ref("");
const importErrorMessage = ref("");
const isRenameModalOpen = ref(false);
const renamingBatchIndex = ref(0);
const isCalibrationModalOpen = ref(false);

const router = useRouter();

function goToProductionArea() {
  router.push("/production");
}

let unsubscribeState = null;
let removeDisconnectListener = null;
let removeConnectListener = null;

const isCurrentBatchEnabled = computed(() => {
  return batchesEnabled.value[activeBatchIndex.value] !== false;
});

const activeBatchDisplayName = computed(() => {
  const name = getBatchName(activeBatchIndex.value);
  return name
    ? `BATCH ${activeBatchIndex.value + 1} (${name})`
    : `BATCH ${activeBatchIndex.value + 1}`;
});

onMounted(() => {
  refreshState();
  checkBle();

  unsubscribeState = remoteDebugService.subscribe((state) => {
    batchList.value = state.batches || Array(10).fill("");
    batchesEnabled.value = state.batchesEnabled || Array(10).fill(true);
    batchNames.value = state.batchNames || Array(10).fill("");
    activeBatchIndex.value = state.activeBatchIndex || 0;
    currentBatchString.value = batchList.value[activeBatchIndex.value] || "";
    instructionList.value = state.instructions || [];
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
  if (removeDisconnectListener) removeDisconnectListener();
  if (removeConnectListener) removeConnectListener();
});

function refreshState() {
  const state = remoteDebugService.readState();
  batchList.value = state.batches || Array(10).fill("");
  batchesEnabled.value = state.batchesEnabled || Array(10).fill(true);
  batchNames.value = state.batchNames || Array(10).fill("");
  activeBatchIndex.value = state.activeBatchIndex || 0;
  currentBatchString.value = batchList.value[activeBatchIndex.value] || "";
  instructionList.value = state.instructions || [];
}

function selectBatch(index) {
  remoteDebugService.setActiveBatchIndex(index);
}

function isBatchEnabled(index) {
  return batchesEnabled.value[index] !== false;
}

function getBatchName(index) {
  return batchNames.value[index] || "";
}

function openRenameModal(index) {
  renamingBatchIndex.value =
    typeof index === "number" ? index : activeBatchIndex.value;
  isRenameModalOpen.value = true;
}

function onSaveBatchName(newName) {
  remoteDebugService.setBatchName(renamingBatchIndex.value, newName);
}

function toggleActiveBatchEnabled() {
  remoteDebugService.toggleBatchEnabled(activeBatchIndex.value);
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

function handleAppendInstruction(token) {
  if (!isCurrentBatchEnabled.value) {
    statusMessage.value = `Batch ${activeBatchIndex.value + 1} sedang dinonaktifkan!`;
    return;
  }
  const current = currentBatchString.value.trim();
  let nextVal = "";
  if (current.length === 0) {
    nextVal = `${token};`;
  } else if (current.endsWith(";")) {
    nextVal = `${current}${token};`;
  } else {
    nextVal = `${current};${token};`;
  }
  currentBatchString.value = nextVal;
  remoteDebugService.setBatchInstructionString(activeBatchIndex.value, nextVal);
  statusMessage.value = `Instruksi "${token}" berhasil ditambahkan ke Batch ${activeBatchIndex.value + 1}`;
}

function clearAllBatches() {
  remoteDebugService.clearAllBatches();
}

async function exportBatchesJson() {
  try {
    const jsonStr = remoteDebugService.exportToJson();
    await navigator.clipboard.writeText(jsonStr);
    statusMessage.value =
      "Seluruh instruksi (Batch 1-10) berhasil disalin ke clipboard!";
  } catch (err) {
    statusMessage.value = `Gagal menyalin ke clipboard: ${err.message || err}`;
  }
}

function openImportModal() {
  importJsonText.value = "";
  importErrorMessage.value = "";
  isImportModalOpen.value = true;
}

function closeImportModal() {
  isImportModalOpen.value = false;
  importJsonText.value = "";
  importErrorMessage.value = "";
}

async function pasteFromClipboard() {
  try {
    const text = await navigator.clipboard.readText();
    if (text) {
      importJsonText.value = text;
      importErrorMessage.value = "";
    }
  } catch (err) {
    importErrorMessage.value =
      "Tidak dapat membaca clipboard otomatis. Silakan tempel manual.";
  }
}

function confirmImport() {
  if (!importJsonText.value.trim()) {
    importErrorMessage.value =
      "Silakan masukkan atau tempel JSON instruksi terlebih dahulu.";
    return;
  }
  try {
    remoteDebugService.importFromJson(importJsonText.value);
    refreshState();
    closeImportModal();
    statusMessage.value =
      "Berhasil mengimpor instruksi! Seluruh batch sebelumnya telah ditimpa.";
  } catch (err) {
    importErrorMessage.value = err.message || "Gagal mengimpor JSON.";
  }
}

function onCurrentBatchInput(e) {
  const val = e && e.target ? e.target.value : currentBatchString.value;
  remoteDebugService.setBatchInstructionString(activeBatchIndex.value, val);
}

function getInstructionsByCategory(catId) {
  return INSTRUCTION_ALIASES.filter((item) => item.categoryId === catId);
}

function checkBle() {
  isBleConnected.value = BleService.isConnected();
  connectedDevicesList.value = BleService.getConnectedDevices();
  const info = BleService.getConnectedDeviceInfo();
  connectedDeviceName.value = info ? info.name : "";
  activeBleTarget.value = BleService.getActiveTarget();
}

function onBleTargetChange() {
  BleService.setActiveTarget(activeBleTarget.value);
}

async function connectBle() {
  if (connectedDevicesList.value.length >= 2) {
    statusMessage.value = "Maksimal 2 Leanbot sudah terhubung!";
    return;
  }
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
    await BleService.disconnectAll();
    checkBle();
    statusMessage.value = "Semua koneksi Bluetooth diputuskan.";
  } catch (err) {
    statusMessage.value = `Error disconnect: ${err.message || err}`;
  }
}

async function sendRawBle(cmd) {
  if (!BleService.isConnected()) {
    statusMessage.value = `Gagal kirim "${cmd}": Robot BLE belum terhubung!`;
    throw new Error("Robot BLE belum terhubung");
  }
  await BleService.sendCommand(cmd, activeBleTarget.value);
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
  if (name && name.toUpperCase() === "KALIBRASI") {
    isCalibrationModalOpen.value = true;
  }
}

function handleCalibrationConfirm(vals) {
  isCalibrationModalOpen.value = false;
  const token = `SET_THRESHOLD:${vals.join(",")}`;
  handleAppendInstruction(token);
}

let abortBatchExecution = false;

async function runProdMission() {
  if (!BleService.isConnected()) {
    statusMessage.value = "Robot BLE belum terhubung!";
    return;
  }

  const preparedBatches = remoteDebugService.getPreparedBatches();

  if (preparedBatches.length === 0) {
    statusMessage.value =
      "Semua batch kosong atau dinonaktifkan! Silakan aktifkan dan tambahkan instruksi terlebih dahulu.";
    return;
  }

  abortBatchExecution = false;

  try {
    statusMessage.value = "Mereset variabel batch di robot...";
    const clearPromise = BleService.waitForAcknowledgment(
      "[BATCHES_CLEARED]",
      2000,
    );
    await sendRawBle("CLEAR");
    await clearPromise;
    await new Promise((r) => setTimeout(r, 60));

    for (const pb of preparedBatches) {
      if (abortBatchExecution) {
        statusMessage.value = "Pengunggahan batch dibatalkan!";
        return;
      }

      const bName = getBatchName(pb.batchIndex);
      const bLabel = bName ? `Batch ${pb.batchNum} (${bName})` : `Batch ${pb.batchNum}`;
      statusMessage.value = `Mengunggah ${bLabel} (${pb.count} instruksi) ke robot...`;
      const ackPromise = BleService.waitForAcknowledgment(
        `[BATCH_LOADED:${pb.batchNum}`,
        5000,
      );
      await sendRawBle(pb.command);
      const ack = await ackPromise;
      if (!ack.success) {
        throw new Error(
          `Robot tidak mengonfirmasi Batch ${pb.batchNum}: ${ack.reason || "Timeout"}`,
        );
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
      remoteDebugService.setActiveBatchIndex(0);
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
    await BleService.sendCommand("EMERGENCY_STOP", "all");
    lastSentCommand.value = "EMERGENCY_STOP";
    statusMessage.value =
      "EMERGENCY STOP DIKIRIM KE SEMUA ROBOT! Robot dihentikan dan semua batch direset.";
  } catch (err) {
    statusMessage.value = `Emergency Stop gagal: ${err.message || err}`;
  }
}
</script>
