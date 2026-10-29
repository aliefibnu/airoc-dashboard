<template>
  <div class="max-w-5xl mx-auto space-y-6 select-none pb-16">
    <div
      class="flex flex-col md:flex-row justify-between items-start md:items-center gap-4 border-b-4 border-black pb-6"
    >
      <div class="space-y-1">
        <h1
          class="text-lg md:text-2xl font-black uppercase tracking-tight text-black pt-1"
        >
          PRODUCTION INSTRUCTION AREA
        </h1>
      </div>

      <div class="flex flex-wrap items-center gap-3 flex-row">
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

    <div class="sticky top-2 z-40 py-1">
      <button
        type="button"
        @click="emergencyStop"
        class="w-full bg-red-600 hover:bg-red-700 text-white font-black uppercase text-xl md:text-2xl py-4 px-6 rounded-2xl border-4 border-black shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center justify-center gap-3"
      >
        <span class="text-3xl">🛑</span>
        <span>EMERGENCY STOP </span>
      </button>
    </div>

    <div
      v-if="isExecuting"
      class="bg-red-500 text-white border-4 border-black p-4 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] text-center animate-pulse"
    >
      <p class="text-sm md:text-base font-black uppercase tracking-wider">
        🛑 TEKAN [SPACEBAR] PADA KEYBOARD UNTUK EMERGENCY STOP KAPAN SAJA
      </p>
    </div>

    <div class="space-y-4">
      <div
        v-for="bNum in 10"
        :key="bNum"
        class="border-4 border-black rounded-3xl p-5 md:p-6 transition-all shadow-[5px_5px_0px_0px_rgba(0,0,0,1)]"
        :class="
          !isBatchEnabled(bNum - 1)
            ? 'bg-gray-200 border-gray-600 opacity-75'
            : getBatchCount(bNum - 1) > 0
              ? 'bg-yellow-50 border-black'
              : 'bg-white border-black'
        "
      >
        <div
          class="flex flex-wrap items-center justify-between gap-3 border-b-3 border-black pb-3 select-none"
        >
          <div class="flex flex-wrap items-center gap-2 md:gap-3">
            <div
              class="flex items-center gap-2 cursor-pointer group"
              @dblclick="openBatchInRemote(bNum - 1)"
              title="Double click untuk buka batch ini di Remote Debugger"
            >
              <span
                class="text-base md:text-lg font-black uppercase px-3 py-1 rounded-xl border-2 border-black group-hover:scale-105 transition-transform"
                :class="
                  !isBatchEnabled(bNum - 1)
                    ? 'bg-gray-400 text-white line-through'
                    : 'bg-yellow-400 text-black shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] group-hover:bg-yellow-300'
                "
              >
                BATCH {{ bNum }}
              </span>

              <span
                v-if="getBatchName(bNum - 1)"
                class="text-sm md:text-base font-black font-mono uppercase px-3 py-1 rounded-xl border-2 border-black bg-blue-100 text-blue-900 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)]"
              >
                {{ getBatchName(bNum - 1) }}
              </span>

              <span
                class="text-xs font-mono font-black px-2.5 py-1 rounded-lg border-2 border-black bg-white group-hover:bg-yellow-100 transition-colors"
                :class="
                  getBatchCount(bNum - 1) >= 12
                    ? 'text-red-600'
                    : getBatchCount(bNum - 1) > 0
                      ? 'text-green-700'
                      : 'text-gray-400'
                "
              >
                {{ getBatchCount(bNum - 1) }}/12 INSTRUKSI
              </span>
            </div>

            <button
              type="button"
              @click.stop="openRenameModal(bNum - 1)"
              class="text-xs font-black uppercase bg-white hover:bg-yellow-200 border-2 border-black px-2.5 py-1 rounded-lg cursor-pointer shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all flex items-center gap-1"
              title="Ubah nama batch"
            >
              <span>✏️</span>
              <span class="hidden sm:inline">RENAME</span>
            </button>
          </div>

          <div class="flex items-center gap-3">
            <span
              class="text-xs md:text-sm font-black uppercase"
              :class="
                isBatchEnabled(bNum - 1) ? 'text-green-800' : 'text-red-700'
              "
            >
              {{
                isBatchEnabled(bNum - 1)
                  ? "ON (AKAN DIKIRIM)"
                  : "OFF (TIDAK AKAN DIKIRIM)"
              }}
            </span>

            <button
              type="button"
              @click="toggleBatch(bNum - 1)"
              class="relative inline-flex h-8 w-16 items-center rounded-full border-3 border-black transition-colors cursor-pointer select-none"
              :class="isBatchEnabled(bNum - 1) ? 'bg-green-500' : 'bg-red-400'"
              :aria-label="`Toggle Batch ${bNum}`"
            >
              <span
                class="inline-block h-6 w-6 transform rounded-full bg-white border-2 border-black transition-transform shadow-[1px_1px_0px_0px_rgba(0,0,0,1)]"
                :class="
                  isBatchEnabled(bNum - 1) ? 'translate-x-8' : 'translate-x-1'
                "
              />
            </button>
          </div>
        </div>

        <div class="pt-3">
          <div
            v-if="getBatchTokens(bNum - 1).length > 0"
            class="flex flex-wrap gap-2 select-text"
          >
            <span
              v-for="(token, tIdx) in getBatchTokens(bNum - 1)"
              :key="tIdx"
              @dblclick="selectStartFromInstruction(bNum, tIdx + 1)"
              title="Double click untuk set start dari instruksi ini"
              class="bg-white border-2 border-black px-3 py-1.5 rounded-xl font-mono font-black text-xs md:text-sm shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] inline-flex items-center gap-1.5 cursor-pointer hover:bg-yellow-300 hover:scale-105 active:scale-95 transition-all"
              :class="[
                !isBatchEnabled(bNum - 1)
                  ? 'text-gray-400 line-through opacity-60'
                  : selectedStartBatch === bNum && selectedStartInst === tIdx + 1
                    ? 'bg-blue-100 border-blue-600 ring-2 ring-blue-600 text-blue-900 shadow-[3px_3px_0px_0px_rgba(37,99,235,1)]'
                    : 'text-black'
              ]"
            >
              <span class="text-gray-500 text-[10px]">#{{ tIdx + 1 }}</span>
              <span>{{ token }}</span>
            </span>
          </div>

          <p v-else class="text-xs md:text-sm font-bold text-gray-400 italic">
            Batch ini belum memiliki instruksi. Anda dapat menambahkan instruksi
            melalui halaman Remote.
          </p>
        </div>
      </div>
    </div>

    <div class="bg-yellow-100 rounded-3xl space-y-6">
      <div class="grid grid-cols-1 lg:grid-cols-2 gap-6">
        <div
          class="bg-white border-4 border-black p-5 md:p-6 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] flex flex-col justify-between space-y-4"
        >
          <button
            type="button"
            :disabled="isExecuting"
            @click="startFromBeginning"
            class="w-full bg-green-500 hover:bg-green-400 text-black font-black uppercase text-base md:text-lg py-4 px-6 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center justify-center gap-2 disabled:opacity-50 disabled:cursor-not-allowed"
          >
            <span>{{ isExecuting ? "⏳" : "🚀" }}</span>
            <span>{{
              isExecuting ? "SEDANG MENJALANKAN MISI..." : "START DARI AWAL"
            }}</span>
          </button>
        </div>

        <div
          class="bg-white border-4 border-black p-5 md:p-6 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] flex flex-col justify-between space-y-4"
        >
          <div class="space-y-3">
            <div class="grid grid-cols-1 sm:grid-cols-2 gap-3 pt-1">
              <div class="space-y-1">
                <select
                  v-model.number="selectedStartBatch"
                  @change="onBatchSelectChange"
                  class="w-full bg-yellow-50 border-3 border-black rounded-xl p-2.5 text-sm font-black font-mono focus:outline-none focus:bg-white shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] cursor-pointer uppercase"
                >
                  <option v-for="bNum in 10" :key="bNum" :value="bNum">
                    BATCH {{ bNum }}{{ getBatchName(bNum - 1) ? ` - ${getBatchName(bNum - 1)}` : '' }} ({{ getBatchCount(bNum - 1) }} CMD)
                  </option>
                </select>
              </div>

              <div class="space-y-1">
                <select
                  v-model.number="selectedStartInst"
                  :disabled="availableInstructionsForSelectedBatch.length === 0"
                  class="w-full bg-yellow-50 border-3 border-black rounded-xl p-2.5 text-sm font-black font-mono focus:outline-none focus:bg-white shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] cursor-pointer uppercase disabled:bg-gray-200 disabled:cursor-not-allowed"
                >
                  <option
                    v-if="availableInstructionsForSelectedBatch.length === 0"
                    :value="1"
                  >
                    (BATCH KOSONG)
                  </option>
                  <option
                    v-for="instOpt in availableInstructionsForSelectedBatch"
                    :key="instOpt.index"
                    :value="instOpt.index"
                  >
                    #{{ instOpt.index }}: {{ instOpt.token }}
                  </option>
                </select>
              </div>
            </div>
          </div>

          <button
            ref="specificStartButtonRef"
            type="button"
            :disabled="
              isExecuting || availableInstructionsForSelectedBatch.length === 0
            "
            @click="startFromSpecific"
            class="w-full bg-blue-600 hover:bg-blue-500 text-white font-black uppercase text-base md:text-lg py-4 px-6 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center justify-center gap-2 disabled:opacity-50 disabled:cursor-not-allowed"
          >
            <span>{{ isExecuting ? "⏳" : "🎯" }}</span>
            <span>
              {{
                isExecuting
                  ? "SEDANG MENJALANKAN MISI..."
                  : `START: BATCH ${selectedStartBatch}${getBatchName(selectedStartBatch - 1) ? ` (${getBatchName(selectedStartBatch - 1)})` : ''} INST #${selectedStartInst}`
              }}
            </span>
          </button>
        </div>
      </div>
    </div>

    <BatchRenameModal
      :is-open="isRenameModalOpen"
      :batch-index="renamingBatchIndex"
      :initial-name="getBatchName(renamingBatchIndex)"
      @close="isRenameModalOpen = false"
      @save="onSaveBatchName"
    />
  </div>
</template>

<script setup>
import { ref, computed, nextTick, onMounted, onUnmounted } from "vue";
import { RouterLink, useRouter } from "vue-router";
import BatchRenameModal from "@/components/BatchRenameModal.vue";
import remoteDebugService from "@/services/remote-debug.service.js";
import BleService from "@/services/ble.services.js";

const router = useRouter();
const specificStartButtonRef = ref(null);

const batchList = ref(Array(10).fill(""));
const batchesEnabled = ref(Array(10).fill(true));
const batchNames = ref(Array(10).fill(""));
const isRenameModalOpen = ref(false);
const renamingBatchIndex = ref(0);
const isBleConnected = ref(false);
const connectedDeviceName = ref("");
const statusMessage = ref("");
const isExecuting = ref(false);

const selectedStartBatch = ref(1);
const selectedStartInst = ref(1);

let unsubscribeState = null;
let removeDisconnectListener = null;
let abortExecution = false;

function handleKeyDown(event) {
  if (event.code === "Space" || event.key === " ") {
    if (isExecuting.value) {
      event.preventDefault();
      emergencyStop();
    }
  }
}

onMounted(() => {
  refreshState();
  checkBle();

  window.addEventListener("keydown", handleKeyDown);

  unsubscribeState = remoteDebugService.subscribe((state) => {
    batchList.value = state.batches || Array(10).fill("");
    batchesEnabled.value = state.batchesEnabled || Array(10).fill(true);
    batchNames.value = state.batchNames || Array(10).fill("");
  });

  removeDisconnectListener = BleService.onDisconnect(() => {
    checkBle();
  });
});

onUnmounted(() => {
  window.removeEventListener("keydown", handleKeyDown);
  if (unsubscribeState) unsubscribeState();
  if (removeDisconnectListener) removeDisconnectListener();
});

function refreshState() {
  const state = remoteDebugService.readState();
  batchList.value = state.batches || Array(10).fill("");
  batchesEnabled.value = state.batchesEnabled || Array(10).fill(true);
  batchNames.value = state.batchNames || Array(10).fill("");
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

function isBatchEnabled(index) {
  return batchesEnabled.value[index] !== false;
}

function getBatchName(index) {
  return batchNames.value[index] || "";
}

function openRenameModal(index) {
  renamingBatchIndex.value = index;
  isRenameModalOpen.value = true;
}

function onSaveBatchName(newName) {
  remoteDebugService.setBatchName(renamingBatchIndex.value, newName);
}

function toggleBatch(index) {
  remoteDebugService.toggleBatchEnabled(index);
}

function getBatchTokens(index) {
  const str = batchList.value[index] || "";
  if (!str.trim()) return [];
  return str
    .split(";")
    .map((t) => t.trim())
    .filter(
      (t) =>
        t.length > 0 &&
        t.toUpperCase() !== "END" &&
        t.toUpperCase() !== "CODE" &&
        t.toUpperCase() !== "PROD",
    )
    .map((t) => remoteDebugService.resolveInstructionToken(t));
}

function getBatchCount(index) {
  return getBatchTokens(index).length;
}

const availableInstructionsForSelectedBatch = computed(() => {
  const tokens = getBatchTokens(selectedStartBatch.value - 1);
  return tokens.map((token, idx) => ({
    index: idx + 1,
    token,
  }));
});

function openBatchInRemote(index) {
  remoteDebugService.setActiveBatchIndex(index);
  router.push("/remote");
}

function selectStartFromInstruction(batchNum, instNum) {
  selectedStartBatch.value = batchNum;
  selectedStartInst.value = instNum;
  nextTick(() => {
    if (specificStartButtonRef.value) {
      specificStartButtonRef.value.scrollIntoView({
        behavior: "smooth",
        block: "center",
      });
    }
  });
}

function onBatchSelectChange() {
  selectedStartInst.value = 1;
}

async function sendRawBle(cmd) {
  if (!BleService.isConnected()) {
    statusMessage.value = `Gagal kirim "${cmd}": Robot BLE belum terhubung!`;
    throw new Error("Robot BLE belum terhubung");
  }
  await BleService.sendCommand(cmd);
}

async function executeMission(startBatchNum = 1, startInstNum = 1) {
  if (!BleService.isConnected()) {
    statusMessage.value =
      "Robot BLE belum terhubung! Silakan hubungkan robot terlebih dahulu.";
    return;
  }

  const preparedBatches = remoteDebugService.getPreparedBatches(
    startBatchNum,
    startInstNum,
  );

  if (preparedBatches.length === 0) {
    statusMessage.value =
      "Tidak ada instruksi yang dapat dijalankan dari titik mulai yang dipilih! Silakan periksa batch dan instruksi.";
    return;
  }

  abortExecution = false;
  isExecuting.value = true;

  try {
    statusMessage.value = "Mereset antrean batch di memori robot...";
    const clearPromise = BleService.waitForAcknowledgment(
      "[BATCHES_CLEARED]",
      2000,
    );
    await sendRawBle("CLEAR");
    await clearPromise;
    await new Promise((r) => setTimeout(r, 60));

    for (const pb of preparedBatches) {
      if (abortExecution) {
        statusMessage.value = "Pengunggahan batch dibatalkan!";
        isExecuting.value = false;
        return;
      }

      const bName = getBatchName(pb.batchIndex);
      const bLabel = bName
        ? `Batch ${pb.batchNum} (${bName})`
        : `Batch ${pb.batchNum}`;
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

    if (abortExecution) {
      statusMessage.value = "Pengunggahan batch dibatalkan!";
      isExecuting.value = false;
      return;
    }

    const startBatchName = getBatchName(startBatchNum - 1);
    const startBatchLabel = startBatchName
      ? `Batch ${startBatchNum} (${startBatchName})`
      : `Batch ${startBatchNum}`;

    const startInfo =
      startBatchNum > 1 || startInstNum > 1
        ? `dari ${startBatchLabel} Instruksi #${startInstNum}`
        : "dari awal";
    statusMessage.value = `Semua batch berhasil diterima robot! Memulai eksekusi ${startInfo}... (Tekan SPACE untuk STOP)`;
    const resPromise = BleService.waitForBatchCompletion(300000);
    await sendRawBle("START_MISSION");

    const res = await resPromise;

    if (res.success) {
      remoteDebugService.setActiveBatchIndex(0);
      statusMessage.value =
        "Seluruh instruksi misi berhasil diselesaikan oleh robot! Semua variabel batch di robot telah bersih.";
    } else {
      statusMessage.value = `Misi berhenti: ${res.reason}`;
    }
  } catch (err) {
    statusMessage.value = `Gagal menjalankan misi: ${err.message || err}`;
  } finally {
    isExecuting.value = false;
  }
}

function startFromBeginning() {
  executeMission(1, 1);
}

function startFromSpecific() {
  executeMission(selectedStartBatch.value, selectedStartInst.value);
}

async function emergencyStop() {
  abortExecution = true;
  isExecuting.value = false;
  try {
    await sendRawBle("EMERGENCY_STOP");
    statusMessage.value =
      "EMERGENCY STOP DIKIRIM! Robot dihentikan seketika dan semua variabel batch direset.";
  } catch (err) {
    statusMessage.value = `Emergency Stop gagal: ${err.message || err}`;
  }
}
</script>
