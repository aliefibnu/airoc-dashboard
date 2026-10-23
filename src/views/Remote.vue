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
      <div
        class="flex flex-col md:flex-row md:items-center justify-between gap-2 border-b-4 border-black pb-3"
      >
        <h2 class="text-xl md:text-2xl font-black uppercase tracking-tight">
          PROD INSTRUCTION STRING
        </h2>

        <button
          type="button"
          @click="openSettingsModal"
          class="bg-white text-black font-black uppercase text-xs md:text-sm py-2 px-3.5 rounded-xl border-2 border-black shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] hover:bg-gray-100 active:translate-y-0.5 active:shadow-none transition-all cursor-pointer flex items-center gap-1.5 self-start md:self-auto"
        >
          <span>⚙️</span>
          <span>ATUR DEFAULT VALUE</span>
        </button>
      </div>

      <textarea
        v-model="manualInstructionString"
        rows="3"
        placeholder="KETIK ATAU KONFIRMASI TOMBOL DI BAWAH (CONTOH: SPEED:5;SCAN_KANAN:10,90;DEKATI_OBJEK:6;CAPIT;MUNDUR:8;BUNYI:2;STOP)..."
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
            <h3>JALANKAN MISI (PROD)</h3>
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

    <!-- DAFTAR TOMBOL INSTRUKSI (DATA-DRIVEN DARI INSTRUCTION_ALIASES) -->
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

    <!-- MODAL PENGATURAN DEFAULT VALUE -->
    <teleport to="body">
      <div
        v-if="isSettingsOpen"
        class="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/60 backdrop-blur-xs select-none"
        @click.self="closeSettingsModal"
      >
        <div
          class="bg-yellow-50 border-4 border-black rounded-3xl p-6 md:p-8 max-w-4xl w-full max-h-[90vh] flex flex-col shadow-[8px_8px_0px_0px_rgba(0,0,0,1)] space-y-6"
        >
          <div
            class="flex items-center justify-between border-b-4 border-black pb-4 gap-3 shrink-0"
          >
            <div>
              <h2
                class="text-2xl md:text-3xl font-black uppercase tracking-tight text-black"
              >
                PENGATURAN DEFAULT VALUE INSTRUKSI
              </h2>
              <p class="text-xs md:text-sm font-bold text-gray-700 mt-1">
                Disimpan di LocalStorage & otomatis disinkronkan ke Leanbot via
                Bluetooth.
              </p>
            </div>
            <button
              type="button"
              @click="closeSettingsModal"
              class="border-2 border-black font-black text-xl px-2.5 py-0.5 rounded-xl bg-red-500 text-white hover:bg-red-600 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer shrink-0"
            >
              ✕
            </button>
          </div>

          <div class="overflow-y-auto pr-2 space-y-6 flex-1 select-text">
            <div
              class="p-3.5 rounded-xl border-2 border-black flex flex-col sm:flex-row sm:items-center justify-between gap-2 text-xs font-black uppercase"
              :class="
                isBleConnected
                  ? 'bg-green-100 text-green-950'
                  : 'bg-amber-100 text-amber-950'
              "
            >
              <span class="flex items-center gap-2">
                <span
                  class="w-3 h-3 rounded-full border border-black"
                  :class="
                    isBleConnected ? 'bg-green-500 animate-ping' : 'bg-gray-400'
                  "
                ></span>
                {{
                  isBleConnected
                    ? `Bluetooth Terhubung (${connectedDeviceName}): Perubahan akan langsung disinkronkan ke robot.`
                    : "Bluetooth Belum Terhubung: Nilai tersimpan di browser & disinkronkan otomatis saat connect."
                }}
              </span>

              <button
                v-if="isBleConnected"
                type="button"
                @click="manualSyncDefaults"
                class="bg-blue-600 text-white px-3 py-1 rounded-lg border border-black hover:bg-blue-700 active:translate-y-0.5 self-start sm:self-auto cursor-pointer"
              >
                SYNC SEKARANG
              </button>
            </div>

            <div
              v-for="cat in paramCategories"
              :key="cat.id"
              class="space-y-3"
            >
              <h4
                class="text-sm font-black uppercase tracking-wider text-black border-l-4 border-black pl-2"
              >
                {{ cat.title }}
              </h4>
              <div class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 gap-3">
                <div
                  v-for="item in cat.items"
                  :key="item.alias"
                  class="bg-white border-2 border-black p-3.5 rounded-xl shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] space-y-2.5"
                >
                  <div class="flex items-center justify-between border-b pb-1.5">
                    <span class="font-black text-sm uppercase truncate">{{
                      item.label
                    }}</span>
                    <span
                      class="text-[10px] font-mono font-bold bg-gray-100 border border-black px-1.5 py-0.5 rounded ml-2"
                    >
                      {{ item.alias }}
                    </span>
                  </div>

                  <div
                    v-for="p in item.params"
                    :key="p.name"
                    class="space-y-1"
                  >
                    <label
                      class="text-[11px] font-black uppercase text-gray-700 block"
                    >
                      {{ p.label || p.name }}
                    </label>
                    <input
                      v-model="editDefaultsForm[item.alias][p.name]"
                      :type="p.type || 'text'"
                      :inputmode="
                        p.inputmode ||
                        (p.type === 'number' ? 'numeric' : 'text')
                      "
                      :min="p.min"
                      :max="p.max"
                      :step="p.step"
                      class="w-full bg-yellow-50 border-2 border-black rounded-lg p-2 text-sm font-black font-mono focus:outline-none focus:bg-white shadow-[2px_2px_0px_0px_rgba(0,0,0,1)]"
                    />
                  </div>
                </div>
              </div>
            </div>
          </div>

          <div
            class="pt-4 border-t-4 border-black flex flex-wrap items-center justify-between gap-3 shrink-0"
          >
            <button
              type="button"
              @click="resetDefaultsToStandard"
              class="bg-gray-200 text-black font-black uppercase text-xs md:text-sm py-3 px-4 rounded-xl border-2 border-black hover:bg-gray-300 active:translate-y-0.5 transition-all cursor-pointer"
            >
              🔄 RESET KE STANDAR
            </button>

            <div class="flex items-center gap-3">
              <button
                type="button"
                @click="closeSettingsModal"
                class="bg-white text-black font-black uppercase text-xs md:text-sm py-3 px-4 rounded-xl border-2 border-black hover:bg-gray-100 active:translate-y-0.5 transition-all cursor-pointer"
              >
                BATAL
              </button>

              <button
                type="button"
                @click="saveCustomDefaults"
                class="bg-green-500 text-black font-black uppercase text-sm md:text-base py-3 px-6 rounded-xl border-4 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-0.5 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
              >
                <span>💾</span>
                <span>SIMPAN & SYNC KE LEANBOT</span>
              </button>
            </div>
          </div>
        </div>
      </div>
    </teleport>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted, onUnmounted } from "vue";
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

const isSettingsOpen = ref(false);
const editDefaultsForm = reactive({});

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

const paramCategories = computed(() => {
  return INSTRUCTION_CATEGORIES.map((cat) => {
    const items = INSTRUCTION_ALIASES.filter(
      (item) =>
        item.categoryId === cat.id &&
        Array.isArray(item.params) &&
        item.params.length > 0,
    );
    return {
      id: cat.id,
      title: cat.title,
      items,
    };
  }).filter((cat) => cat.items.length > 0);
});

function initEditDefaultsForm() {
  const currentDefaults = instructionDefaultsService.getAllDefaults();
  INSTRUCTION_ALIASES.forEach((item) => {
    if (Array.isArray(item.params) && item.params.length > 0) {
      if (!editDefaultsForm[item.alias]) {
        editDefaultsForm[item.alias] = {};
      }
      item.params.forEach((p) => {
        const val = currentDefaults[item.alias]?.[p.name];
        editDefaultsForm[item.alias][p.name] =
          val !== undefined ? val : p.defaultValue !== undefined ? p.defaultValue : "";
      });
    }
  });
}

function openSettingsModal() {
  initEditDefaultsForm();
  isSettingsOpen.value = true;
}

function closeSettingsModal() {
  isSettingsOpen.value = false;
}

async function saveCustomDefaults() {
  const result = instructionDefaultsService.saveDefaults(editDefaultsForm, true);
  closeSettingsModal();
  statusMessage.value = "DEFAULT VALUES DISIMPAN KE LOCALSTORAGE";

  if (isBleConnected.value) {
    try {
      const syncRes = await result.syncPromise;
      if (syncRes && syncRes.success) {
        statusMessage.value =
          "DEFAULT VALUES DISIMPAN & BERHASIL DISINKRONKAN KE LEANBOT!";
      }
    } catch (e) {
      statusMessage.value = `Default values disimpan, namun sync BLE gagal: ${e.message || e}`;
    }
  }

  setTimeout(() => {
    if (statusMessage.value.includes("DEFAULT VALUES")) {
      statusMessage.value = "";
    }
  }, 3500);
}

function resetDefaultsToStandard() {
  instructionDefaultsService.resetToBaselines(true);
  initEditDefaultsForm();
  statusMessage.value = "DEFAULT VALUE DIRESET KE NILAI STANDAR";
  setTimeout(() => {
    if (statusMessage.value.includes("DIRESET")) {
      statusMessage.value = "";
    }
  }, 2500);
}

async function manualSyncDefaults() {
  try {
    statusMessage.value = "Menyinkronkan default value ke Leanbot...";
    const res = await instructionDefaultsService.syncToLeanbot();
    if (res.success) {
      statusMessage.value = "BERHASIL MENYINKRONKAN DEFAULT VALUE KE LEANBOT!";
    } else {
      statusMessage.value = `Sync gagal: ${res.reason}`;
    }
  } catch (err) {
    statusMessage.value = `Gagal sync ke Leanbot: ${err.message || err}`;
  }
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

async function runProdMission() {
  const str = manualInstructionString.value.trim();
  if (!str) {
    statusMessage.value =
      "String instruksi kosong! Silakan tambahkan instruksi terlebih dahulu.";
    return;
  }
  const prodCmd = str.toUpperCase().startsWith("PROD") ? str : `PROD;${str}`;
  await sendRawBle(prodCmd);
}

async function emergencyStop() {
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
