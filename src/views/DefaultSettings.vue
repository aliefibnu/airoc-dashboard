<template>
  <div class="max-w-6xl mx-auto space-y-8 select-none pb-12">
    <div
      class="flex flex-col md:flex-row justify-between items-start md:items-center gap-4 border-b-4 border-black pb-6"
    >
      <div class="space-y-1">
        <div class="flex items-center gap-3">
          <h1 class="text-2xl md:text-4xl font-black uppercase tracking-tight">
            PENGATURAN DEFAULT VALUE (KRUSIAL)
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
              isBleConnected
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
        @click="statusMessage = ''"
        class="font-black text-lg px-2 hover:bg-black/10 rounded border-2 border-black cursor-pointer"
      >
        ✕
      </button>
    </div>

    <div class="space-y-8 select-text">
      <div
        v-for="cat in paramCategories"
        :key="cat.id"
        class="bg-yellow-50 border-4 border-black p-6 md:p-8 rounded-3xl shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] space-y-4"
      >
        <div class="border-b-4 border-black pb-3">
          <h2 class="text-xl md:text-2xl font-black uppercase tracking-tight">
            {{ cat.title }}
          </h2>
        </div>

        <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-4">
          <div
            v-for="item in cat.items"
            :key="item.alias"
            class="bg-white border-4 border-black p-4 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] space-y-3"
          >
            <div
              class="flex items-center justify-between border-b-2 border-black pb-2"
            >
              <span class="font-black text-base uppercase truncate">{{
                item.label
              }}</span>
            </div>

            <div v-for="p in item.params" :key="p.name" class="space-y-1.5">
              <div class="flex items-center justify-between">
                <label
                  class="text-xs font-black uppercase tracking-wide text-gray-700 block"
                >
                  {{ p.label || p.name }}
                </label>
                <span
                  v-if="p.defaultValue !== undefined"
                  class="text-[10px] font-mono font-bold text-gray-500"
                >
                  Asli: {{ p.defaultValue }}
                </span>
              </div>

              <input
                v-model="editForm[item.alias][p.name]"
                :type="p.type || 'text'"
                :inputmode="
                  p.inputmode || (p.type === 'number' ? 'numeric' : 'text')
                "
                :min="p.min"
                :max="p.max"
                :step="p.step"
                class="w-full bg-yellow-50 border-3 border-black rounded-xl p-2.5 text-base font-black font-mono focus:outline-none focus:bg-white shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] transition-colors"
              />
            </div>
          </div>
        </div>
      </div>
    </div>

    <div
      class="bg-yellow-50 border-4 border-black p-6 rounded-3xl shadow-[6px_6px_0px_0px_rgba(0,0,0,1)] flex flex-wrap items-center justify-between gap-4 sticky bottom-4 z-40"
    >
      <button
        type="button"
        @click="resetToStandard"
        class="bg-slate-200 text-black font-black uppercase text-sm md:text-base py-3.5 px-6 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-slate-300 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
      >
        <span>RESET KE STANDAR</span>
      </button>

      <div class="flex flex-wrap items-center gap-3">
        <button
          type="button"
          @click="saveDefaults"
          class="bg-green-500 text-black font-black uppercase text-base md:text-lg py-3.5 px-8 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-1 active:shadow-none transition-all cursor-pointer flex items-center gap-2"
        >
          <span>SIMPAN & SINKRONKAN</span>
        </button>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted, onUnmounted } from "vue";
import BleService from "@/services/ble.services.js";
import instructionDefaultsService from "@/services/instruction-defaults.service.js";
import {
  INSTRUCTION_CATEGORIES,
  INSTRUCTION_ALIASES,
} from "@/data/instruction-aliases.js";

const CRUCIAL_PARAMS = {
  SPEED: ["level"],
  MAJU: ["cm"],
  MUNDUR: ["cm"],
  KANAN: ["deg"],
  KIRI: ["deg"],
  BELOK_KANAN: ["deg"],
  BELOK_KIRI: ["deg"],
  FOLLOW_LINE: ["jumlah_simpangan"],
  MAJU_LINE: ["max_cm"],
  MUNDUR_LINE: ["max_cm"],
  TUNGGU: ["ms"],
  BUNYI: ["jumlah"],
};

const isBleConnected = ref(false);
const connectedDeviceName = ref("");
const statusMessage = ref("");

function buildInitialFormData() {
  const currentDefaults = instructionDefaultsService.getAllDefaults();
  const data = {};
  for (const alias in CRUCIAL_PARAMS) {
    data[alias] = {};
    CRUCIAL_PARAMS[alias].forEach((paramName) => {
      const val = currentDefaults[alias]?.[paramName];
      data[alias][paramName] = val !== undefined ? val : "";
    });
  }
  return data;
}

const editForm = reactive(buildInitialFormData());

let removeDisconnectListener = null;

onMounted(() => {
  checkBle();
  initForm();

  removeDisconnectListener = BleService.onDisconnect(() => {
    checkBle();
  });
});

onUnmounted(() => {
  if (removeDisconnectListener) removeDisconnectListener();
});

function checkBle() {
  isBleConnected.value = BleService.isConnected();
  const info = BleService.getConnectedDeviceInfo();
  connectedDeviceName.value = info ? info.name : "";
}

const paramCategories = computed(() => {
  return INSTRUCTION_CATEGORIES.map((cat) => {
    const items = INSTRUCTION_ALIASES.filter(
      (item) => item.categoryId === cat.id && CRUCIAL_PARAMS[item.alias],
    ).map((item) => {
      const allowed = CRUCIAL_PARAMS[item.alias] || [];
      return {
        ...item,
        params: item.params.filter((p) => allowed.includes(p.name)),
      };
    }).filter((item) => item.params.length > 0);

    return {
      id: cat.id,
      title: cat.title,
      items,
    };
  }).filter((cat) => cat.items.length > 0);
});

function initForm() {
  const currentDefaults = instructionDefaultsService.getAllDefaults();
  for (const alias in CRUCIAL_PARAMS) {
    if (!editForm[alias]) {
      editForm[alias] = {};
    }
    CRUCIAL_PARAMS[alias].forEach((paramName) => {
      const val = currentDefaults[alias]?.[paramName];
      editForm[alias][paramName] = val !== undefined ? val : "";
    });
  }
}

async function saveDefaults() {
  const result = instructionDefaultsService.saveDefaults(editForm, true);
  statusMessage.value = "DEFAULT VALUE BERHASIL DISIMPAN KE LOCALSTORAGE";

  if (isBleConnected.value) {
    try {
      const syncRes = await result.syncPromise;
      if (syncRes && syncRes.success) {
        statusMessage.value =
          "DEFAULT VALUE DISIMPAN & BERHASIL DISINKRONKAN KE LEANBOT!";
      } else {
        statusMessage.value = `Tersimpan, namun sync robot gagal: ${syncRes?.reason || "Timeout"}`;
      }
    } catch (e) {
      statusMessage.value = `Tersimpan, namun sync BLE error: ${e.message || e}`;
    }
  }

  setTimeout(() => {
    if (statusMessage.value.includes("BERHASIL")) {
      statusMessage.value = "";
    }
  }, 4000);
}

function resetToStandard() {
  instructionDefaultsService.resetToBaselines(true);
  initForm();
  statusMessage.value = "SEMUA DEFAULT VALUE DIRESET KE NILAI AWAL STANDAR";
  setTimeout(() => {
    if (statusMessage.value.includes("DIRESET")) {
      statusMessage.value = "";
    }
  }, 3000);
}
</script>
