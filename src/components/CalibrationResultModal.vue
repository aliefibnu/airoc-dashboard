<template>
  <teleport to="body">
    <div
      v-if="isOpen"
      class="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/60 backdrop-blur-xs select-none"
      @click.self="emitClose"
    >
      <div
        class="bg-yellow-50 border-4 border-black rounded-3xl p-6 md:p-8 max-w-md w-full shadow-[8px_8px_0px_0px_rgba(0,0,0,1)] space-y-6"
      >
        <div class="flex items-center justify-between border-b-4 border-black pb-3 gap-3">
          <div class="flex items-center gap-2">
            <span class="text-2xl">🎯</span>
            <h2 class="text-xl md:text-2xl font-black uppercase tracking-tight text-black">
              HASIL KALIBRASI :
            </h2>
          </div>
          <button
            type="button"
            @click="emitClose"
            class="border-2 border-black font-black text-xl px-2.5 py-0.5 rounded-xl bg-red-500 text-white hover:bg-red-600 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer shrink-0"
          >
            ✕
          </button>
        </div>

        <div
          v-if="feedbackMessage"
          class="border-3 border-black py-2 px-3 rounded-xl text-center text-xs font-black uppercase tracking-wider"
          :class="feedbackType === 'error' ? 'bg-red-200 text-red-950' : 'bg-green-200 text-green-950'"
        >
          {{ feedbackMessage }}
        </div>

        <div class="space-y-2">
          <div class="flex items-center justify-between text-xs font-black font-mono text-gray-700 uppercase">
            <span>IR LEVEL (0 - 1 - 2 - 3)</span>
            <span v-if="isCalibrating" class="text-amber-700 animate-pulse">MENUNGGU DATA...</span>
          </div>

          <div class="grid grid-cols-4 gap-2 sm:gap-3">
            <div
              v-for="(val, idx) in irLevel"
              :key="idx"
              class="flex flex-col items-center gap-1.5"
            >
              <span class="font-mono font-black text-xs text-gray-600">{{ idx }}</span>
              <input
                type="number"
                v-model.number="irLevel[idx]"
                class="w-full h-16 sm:h-20 text-center font-mono text-xl sm:text-2xl font-black bg-white border-4 border-black rounded-2xl shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] focus:outline-none focus:bg-yellow-100"
              />
            </div>
          </div>
        </div>

        <div class="space-y-3 pt-2">
          <button
            type="button"
            @click="testThreshold"
            :disabled="isTesting"
            class="w-full bg-blue-600 hover:bg-blue-700 text-white text-base md:text-lg font-black py-3.5 px-4 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] active:translate-y-1 active:shadow-none transition-all cursor-pointer disabled:opacity-60 disabled:cursor-not-allowed"
          >
            TEST INSTRUKSI (SET TREESHOLD)
          </button>

          <button
            type="button"
            @click="confirmThreshold"
            class="w-full bg-green-500 hover:bg-green-400 text-black text-base md:text-lg font-black py-3.5 px-4 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] active:translate-y-1 active:shadow-none transition-all cursor-pointer"
          >
            KONFIRMASI INSTRUKSI
          </button>
        </div>
      </div>
    </div>
  </teleport>
</template>

<script setup>
import { ref, watch, onMounted, onUnmounted } from "vue";
import lineSensorService from "@/services/line-sensor.service.js";

const props = defineProps({
  isOpen: {
    type: Boolean,
    default: false,
  },
});

const emit = defineEmits(["close", "confirm"]);

const irLevel = ref([302, 128, 94, 112]);
const isCalibrating = ref(false);
const feedbackMessage = ref("");
const feedbackType = ref("success");
const isTesting = ref(false);

let unsubscribe = null;

onMounted(() => {
  syncValues();
  unsubscribe = lineSensorService.subscribe(() => {
    syncValues();
  });
});

onUnmounted(() => {
  if (unsubscribe) unsubscribe();
});

watch(
  () => props.isOpen,
  (open) => {
    if (open) {
      syncValues();
      feedbackMessage.value = "";
    }
  }
);

function syncValues() {
  const state = lineSensorService.getState();
  irLevel.value = [...state.irLevel];
  isCalibrating.value = state.isCalibrating;
}

function emitClose() {
  emit("close");
}

async function testThreshold() {
  isTesting.value = true;
  feedbackMessage.value = "";
  try {
    const vals = irLevel.value.map((v) => Number(v) || 0);
    await lineSensorService.setThreshold(vals);
    feedbackMessage.value = "SET TREESHOLD BERHASIL DIKIRIM KE ROBOT!";
    feedbackType.value = "success";
  } catch (err) {
    feedbackMessage.value = `GAGAL: ${err.message || err}`;
    feedbackType.value = "error";
  } finally {
    isTesting.value = false;
  }
}

function confirmThreshold() {
  const vals = irLevel.value.map((v) => Number(v) || 0);
  emit("close");
  emit("confirm", vals);
}
</script>
