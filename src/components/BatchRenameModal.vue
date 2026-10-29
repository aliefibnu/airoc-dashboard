<template>
  <teleport to="body">
    <div
      v-if="isOpen"
      class="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/60 backdrop-blur-xs select-none"
      @click.self="closeModal"
    >
      <div
        class="bg-yellow-50 border-4 border-black rounded-3xl p-6 md:p-8 max-w-xl w-full shadow-[8px_8px_0px_0px_rgba(0,0,0,1)] space-y-5 max-h-[90vh] overflow-y-auto"
      >
        <div
          class="flex items-center justify-between border-b-4 border-black pb-3 gap-3"
        >
          <div class="flex items-center gap-2">
            <span class="text-2xl">🏷️</span>
            <h2
              class="text-xl md:text-2xl font-black uppercase tracking-tight text-black"
            >
              RENAME BATCH {{ batchIndex + 1 }}
            </h2>
          </div>
          <button
            type="button"
            @click="closeModal"
            class="border-2 border-black font-black text-xl px-2.5 py-0.5 rounded-xl bg-red-500 text-white hover:bg-red-600 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer shrink-0"
          >
            ✕
          </button>
        </div>

        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div
            class="bg-white border-3 border-black rounded-2xl p-4 shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] space-y-3"
          >
            <div
              class="flex items-center justify-between border-b-2 border-black/20 pb-2"
            >
              <span
                class="text-xs font-black uppercase tracking-wider text-black"
              >
                TITIK ASAL
              </span>
              <span
                class="px-2 py-0.5 rounded-lg border-2 border-black text-xs font-mono font-black shadow-[1px_1px_0px_0px_rgba(0,0,0,1)]"
                :class="
                  originCity === 'CRL'
                    ? 'bg-purple-200 text-purple-950'
                    : 'bg-green-200 text-green-950'
                "
              >
                {{ formattedOrigin }}
              </span>
            </div>

            <div class="space-y-1">
              <label class="text-[11px] font-black uppercase text-gray-700"
                >Area</label
              >
              <select
                v-model="originCity"
                @change="onLocationChange"
                class="w-full bg-yellow-50 border-3 border-black rounded-xl p-2.5 text-sm font-black font-mono focus:outline-none focus:bg-white shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] cursor-pointer uppercase"
              >
                <option value="CRL">CRL (Crisis Response Lab)</option>
                <option v-for="c in CITIES" :key="c" :value="c">
                  Kota {{ c }}
                </option>
              </select>
            </div>

            <div v-if="originCity !== 'CRL'" class="space-y-1">
              <label class="text-[11px] font-black uppercase text-gray-700"
                >Zona</label
              >
              <div class="grid grid-cols-4 gap-1.5">
                <button
                  v-for="z in 4"
                  :key="z"
                  type="button"
                  @click="setOriginZone(z)"
                  class="py-1.5 rounded-lg border-2 border-black font-black text-xs uppercase transition-all cursor-pointer text-center"
                  :class="
                    originZone === z
                      ? 'bg-yellow-400 text-black shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] -translate-y-0.5'
                      : 'bg-white text-gray-600 hover:bg-gray-100'
                  "
                >
                  {{ z }}
                </button>
              </div>
            </div>

            <div
              v-else
              class="py-2.5 px-3 bg-purple-100 border-2 border-purple-400 rounded-xl text-center"
            >
              <p
                class="text-[11px] font-black uppercase text-purple-900 tracking-wide"
              >
                CRL (Tanpa Zona)
              </p>
            </div>
          </div>

          <div
            class="bg-white border-3 border-black rounded-2xl p-4 shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] space-y-3"
          >
            <div
              class="flex items-center justify-between border-b-2 border-black/20 pb-2"
            >
              <span
                class="text-xs font-black uppercase tracking-wider text-black"
              >
                TITIK TUJUAN
              </span>
              <span
                class="px-2 py-0.5 rounded-lg border-2 border-black text-xs font-mono font-black shadow-[1px_1px_0px_0px_rgba(0,0,0,1)]"
                :class="
                  destCity === 'CRL'
                    ? 'bg-purple-200 text-purple-950'
                    : 'bg-blue-200 text-blue-950'
                "
              >
                {{ formattedDest }}
              </span>
            </div>

            <div class="space-y-1">
              <label class="text-[11px] font-black uppercase text-gray-700"
                >Area</label
              >
              <select
                v-model="destCity"
                @change="onLocationChange"
                class="w-full bg-yellow-50 border-3 border-black rounded-xl p-2.5 text-sm font-black font-mono focus:outline-none focus:bg-white shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] cursor-pointer uppercase"
              >
                <option value="CRL">CRL (Crisis Response Lab)</option>
                <option v-for="c in CITIES" :key="c" :value="c">
                  Kota {{ c }}
                </option>
              </select>
            </div>

            <div v-if="destCity !== 'CRL'" class="space-y-1">
              <label class="text-[11px] font-black uppercase text-gray-700"
                >Zona</label
              >
              <div class="grid grid-cols-4 gap-1.5">
                <button
                  v-for="z in 4"
                  :key="z"
                  type="button"
                  @click="setDestZone(z)"
                  class="py-1.5 rounded-lg border-2 border-black font-black text-xs uppercase transition-all cursor-pointer text-center"
                  :class="
                    destZone === z
                      ? 'bg-yellow-400 text-black shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] -translate-y-0.5'
                      : 'bg-white text-gray-600 hover:bg-gray-100'
                  "
                >
                  {{ z }}
                </button>
              </div>
            </div>

            <div
              v-else
              class="py-2.5 px-3 bg-purple-100 border-2 border-purple-400 rounded-xl text-center"
            >
              <p
                class="text-[11px] font-black uppercase text-purple-900 tracking-wide"
              >
                CRL (Tanpa Zona)
              </p>
            </div>
          </div>
        </div>

        <div class="flex flex-wrap items-center justify-center gap-3">
          <div
            class="flex items-center rounded-xl border-3 border-black overflow-hidden shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] bg-white"
          >
            <button
              type="button"
              @click="setConnector('to')"
              class="px-4 py-2 font-black text-xs md:text-sm uppercase transition-colors cursor-pointer"
              :class="
                connector === 'to'
                  ? 'bg-yellow-400 text-black'
                  : 'hover:bg-yellow-100 text-gray-600'
              "
            >
              to
            </button>
            <button
              type="button"
              @click="setConnector('ke')"
              class="px-4 py-2 font-black text-xs md:text-sm uppercase border-l-3 border-black transition-colors cursor-pointer"
              :class="
                connector === 'ke'
                  ? 'bg-yellow-400 text-black'
                  : 'hover:bg-yellow-100 text-gray-600'
              "
            >
              ke
            </button>
          </div>

          <button
            type="button"
            @click="swapLocations"
            class="bg-blue-100 hover:bg-blue-200 text-blue-900 border-3 border-black px-4 py-2 rounded-xl font-black text-xs md:text-sm uppercase shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none flex items-center gap-2 cursor-pointer transition-all"
            title="Tukar Asal dan Tujuan"
          >
            <span class="text-base">⇄</span>
            <span>TUKAR RUTE</span>
          </button>
        </div>

        <div
          class="bg-white border-4 border-black rounded-2xl p-4 shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] space-y-2"
        >
          <div class="flex items-center justify-between">
            <span
              class="text-xs font-black uppercase tracking-wider text-black"
            >
              HASIL NAMA BATCH
            </span>
          </div>

          <input
            v-model="batchNameInput"
            type="text"
            placeholder="Contoh: A1 to B2"
            class="w-full bg-yellow-50 border-3 border-black rounded-xl p-3 font-mono font-black text-lg md:text-xl text-center shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] focus:outline-none focus:bg-white select-text uppercase"
          />
        </div>

        <div class="flex flex-wrap items-center justify-between gap-3 pt-2">
          <button
            type="button"
            @click="resetToDefault"
            class="bg-red-100 hover:bg-red-200 text-red-900 font-black uppercase text-xs md:text-sm py-2.5 px-4 rounded-xl border-2 border-black shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer"
          >
            RESET NAMA
          </button>

          <div class="flex items-center gap-3">
            <button
              type="button"
              @click="closeModal"
              class="bg-slate-200 text-black font-black uppercase text-xs md:text-sm py-2.5 px-4 rounded-xl border-3 border-black shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] hover:bg-slate-300 active:translate-y-0.5 active:shadow-none transition-all cursor-pointer"
            >
              BATAL
            </button>

            <button
              type="button"
              @click="confirmSave"
              class="bg-green-500 text-black font-black uppercase text-xs md:text-sm py-2.5 px-5 rounded-xl border-3 border-black shadow-[3px_3px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-0.5 active:shadow-none transition-all cursor-pointer flex items-center gap-1.5"
            >
              <span>💾</span>
              <span>SIMPAN</span>
            </button>
          </div>
        </div>
      </div>
    </div>
  </teleport>
</template>

<script setup>
import { ref, computed, watch } from "vue";

const props = defineProps({
  isOpen: {
    type: Boolean,
    default: false,
  },
  batchIndex: {
    type: Number,
    default: 0,
  },
  initialName: {
    type: String,
    default: "",
  },
});

const emit = defineEmits(["close", "save"]);

const CITIES = ["A", "B", "C", "D", "E", "F", "G", "H"];

const originCity = ref("A");
const originZone = ref(1);
const connector = ref("to");
const destCity = ref("B");
const destZone = ref(1);
const batchNameInput = ref("");

const formattedOrigin = computed(() => {
  if (originCity.value === "CRL") return "CRL";
  return `${originCity.value}${originZone.value}`;
});

const formattedDest = computed(() => {
  if (destCity.value === "CRL") return "CRL";
  return `${destCity.value}${destZone.value}`;
});

function generateName() {
  return `${formattedOrigin.value} ${connector.value} ${formattedDest.value}`;
}

function updateInputFromName() {
  batchNameInput.value = generateName();
}

function onLocationChange() {
  updateInputFromName();
}

function setOriginZone(z) {
  originZone.value = z;
  updateInputFromName();
}

function setDestZone(z) {
  destZone.value = z;
  updateInputFromName();
}

function setConnector(c) {
  connector.value = c;
  updateInputFromName();
}

function swapLocations() {
  const tempCity = originCity.value;
  const tempZone = originZone.value;
  originCity.value = destCity.value;
  originZone.value = destZone.value;
  destCity.value = tempCity;
  destZone.value = tempZone;
  updateInputFromName();
}

function parseLocation(str) {
  const upper = String(str || "")
    .trim()
    .toUpperCase();
  if (upper === "CRL") {
    return { city: "CRL", zone: 1 };
  }
  const city = upper[0];
  const zone = parseInt(upper[1], 10);
  if (CITIES.includes(city) && zone >= 1 && zone <= 4) {
    return { city, zone };
  }
  return null;
}

function parseName(str) {
  if (!str) return null;
  const match = String(str)
    .trim()
    .match(/^(CRL|[A-Ha-h][1-4])\s+(to|ke)\s+(CRL|[A-Ha-h][1-4])$/i);
  if (!match) return null;
  const orig = parseLocation(match[1]);
  const dest = parseLocation(match[3]);
  if (!orig || !dest) return null;
  return {
    origin: orig,
    connector: match[2].toLowerCase(),
    destination: dest,
  };
}

function initFromProps() {
  const parsed = parseName(props.initialName);
  if (parsed) {
    originCity.value = parsed.origin.city;
    originZone.value = parsed.origin.zone;
    connector.value = parsed.connector;
    destCity.value = parsed.destination.city;
    destZone.value = parsed.destination.zone;
    batchNameInput.value = props.initialName.trim();
  } else if (props.initialName && props.initialName.trim().length > 0) {
    originCity.value = "A";
    originZone.value = 1;
    connector.value = "to";
    destCity.value = "B";
    destZone.value = 1;
    batchNameInput.value = props.initialName.trim();
  } else {
    originCity.value = "A";
    originZone.value = 1;
    connector.value = "to";
    destCity.value = "B";
    destZone.value = 1;
    batchNameInput.value = generateName();
  }
}

watch(
  () => props.isOpen,
  (newVal) => {
    if (newVal) {
      initFromProps();
    }
  },
  { immediate: true },
);

function closeModal() {
  emit("close");
}

function resetToDefault() {
  emit("save", "");
  closeModal();
}

function confirmSave() {
  emit("save", batchNameInput.value.trim());
  closeModal();
}
</script>
