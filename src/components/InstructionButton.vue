<template>
  <button
    type="button"
    :disabled="disabled"
    @click="openModal"
    class="font-black uppercase text-xl py-4 px-6 rounded-2xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] active:translate-y-1 active:shadow-none transition-all cursor-pointer disabled:bg-gray-400 disabled:shadow-none disabled:cursor-not-allowed select-none"
    :class="buttonColorClass"
  >
    <slot>
      <span class="tracking-tight">{{ displayLabel }}</span>
    </slot>
  </button>

  <teleport to="body">
    <div
      v-if="isOpen"
      class="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/60 backdrop-blur-xs select-none"
      @click.self="closeModal"
    >
      <div
        class="bg-yellow-50 border-4 border-black rounded-3xl p-6 md:p-8 max-w-md w-full shadow-[8px_8px_0px_0px_rgba(0,0,0,1)] space-y-6"
      >
        <div
          class="flex items-center justify-between border-b-4 border-black pb-4 gap-3"
        >
          <h2
            class="text-2xl font-black uppercase tracking-tight text-black truncate"
          >
            {{ displayLabel }}
          </h2>
          <button
            type="button"
            @click="closeModal"
            class="border-2 border-black font-black text-xl px-2.5 py-0.5 rounded-xl bg-red-500 text-white hover:bg-red-600 shadow-[2px_2px_0px_0px_rgba(0,0,0,1)] active:translate-y-0.5 active:shadow-none transition-all cursor-pointer shrink-0"
          >
            ✕
          </button>
        </div>

        <div v-if="configuredParams.length > 0" class="space-y-4">
          <div
            v-for="param in configuredParams"
            :key="param.name"
            class="space-y-1.5"
          >
            <div class="flex items-center justify-between">
              <h3
                class="text-sm font-black uppercase tracking-wider text-black"
              >
                {{ param.label || param.name }}
              </h3>
            </div>

            <input
              v-model="inputValues[param.name]"
              :type="param.type || 'text'"
              :inputmode="getInputMode(param)"
              :placeholder="
                param.placeholder ||
                (param.defaultValue !== undefined && param.defaultValue !== ''
                  ? 'Default: ' + param.defaultValue
                  : 'PARAM VALUE...')
              "
              :min="param.min"
              :max="param.max"
              :step="param.step"
              class="w-full bg-white border-4 border-black rounded-xl p-3 text-xl font-black font-mono focus:outline-none focus:bg-yellow-100 shadow-[3px_3px_0px_0px_rgba(0,0,0,1)]"
            />
          </div>
        </div>

        <div
          v-if="testExecuted"
          class="bg-blue-300 border-2 border-black py-2 px-3 rounded-xl text-center"
        >
          <h4
            class="text-xs font-black uppercase tracking-widest text-blue-950"
          >
            {{ testFeedbackText }}
          </h4>
        </div>

        <div class="space-y-3 pt-2">
          <button
            type="button"
            @click="handleTest"
            class="w-full bg-blue-600 text-white text-lg font-black py-3.5 px-4 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-blue-700 active:translate-y-1 active:shadow-none transition-all cursor-pointer"
          >
            <h3 class="uppercase tracking-wide">TEST INSTRUCTION</h3>
          </button>

          <button
            type="button"
            @click="handleConfirm"
            class="w-full bg-green-500 text-black text-lg font-black py-3.5 px-4 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-1 active:shadow-none transition-all cursor-pointer"
          >
            <h3 class="uppercase tracking-wide">KONFIRMASI</h3>
          </button>
        </div>
      </div>
    </div>
  </teleport>
</template>

<script setup>
import { ref, reactive, computed, onMounted, onUnmounted } from "vue";
import remoteDebugService from "@/services/remote-debug.service.js";
import instructionDefaultsService from "@/services/instruction-defaults.service.js";
import { INSTRUCTION_ALIASES } from "@/data/instruction-aliases.js";

const props = defineProps({
  label: {
    type: String,
    default: "",
  },
  alias: {
    type: String,
    default: null,
  },
  func: {
    type: Function,
    default: null,
  },
  params: {
    type: [Array, Object],
    default: () => [],
  },
  hasParam: {
    type: Boolean,
    default: false,
  },
  paramLabel: {
    type: String,
    default: "PARAMETER VALUE",
  },
  paramType: {
    type: String,
    default: "text",
  },
  paramInputmode: {
    type: String,
    default: null,
  },
  paramDefault: {
    type: [String, Number, Boolean],
    default: null,
  },
  paramPlaceholder: {
    type: String,
    default: "",
  },
  paramMin: {
    type: [String, Number],
    default: null,
  },
  paramMax: {
    type: [String, Number],
    default: null,
  },
  paramStep: {
    type: [String, Number],
    default: null,
  },
  storageKey: {
    type: String,
    default: "robot_instruction_history",
  },
  color: {
    type: String,
    default: null,
  },
  disabled: {
    type: Boolean,
    default: false,
  },
});

const emit = defineEmits(["test", "confirm"]);

const isOpen = ref(false);
const testExecuted = ref(false);
const testFeedbackText = ref("TEST EXECUTED");
const inputValues = reactive({});
const defaultsVersion = ref(0);

let unsubscribeDefaults = null;

onMounted(() => {
  unsubscribeDefaults = instructionDefaultsService.subscribe(() => {
    defaultsVersion.value++;
  });
});

onUnmounted(() => {
  if (unsubscribeDefaults) unsubscribeDefaults();
});

const targetAlias = computed(() => {
  return props.alias || props.label;
});

const instructionDef = computed(() => {
  if (!targetAlias.value) return null;
  const aliasUpper = targetAlias.value.trim().toUpperCase();
  return (
    INSTRUCTION_ALIASES.find(
      (a) =>
        a.alias.toUpperCase() === aliasUpper ||
        a.name.toUpperCase() === aliasUpper,
    ) || null
  );
});

const displayLabel = computed(() => {
  if (props.label) return props.label;
  if (instructionDef.value?.label) return instructionDef.value.label;
  return targetAlias.value || "CMD";
});

const configuredParams = computed(() => {
  // Dependency tracking for reactive updates when defaults change
  // eslint-disable-next-line no-unused-vars
  const _v = defaultsVersion.value;
  const alias = targetAlias.value;

  // 1. If explicit props.params array provided
  if (Array.isArray(props.params) && props.params.length > 0) {
    return props.params.map((p) => {
      const dynamicDef =
        alias && instructionDefaultsService.getDefault(alias, p.name);
      const defVal =
        dynamicDef !== undefined && dynamicDef !== ""
          ? dynamicDef
          : p.defaultValue !== undefined
            ? p.defaultValue
            : "";
      return {
        ...p,
        defaultValue: defVal,
      };
    });
  }

  // 2. If single param explicitly passed via props
  if (
    props.hasParam ||
    props.paramDefault !== null ||
    (props.paramType && props.paramType !== "text") ||
    props.paramInputmode
  ) {
    const dynamicDef =
      alias && instructionDefaultsService.getDefault(alias, "param");
    const defVal =
      dynamicDef !== undefined && dynamicDef !== ""
        ? dynamicDef
        : props.paramDefault !== null
          ? props.paramDefault
          : "";
    return [
      {
        name: "param",
        label: props.paramLabel,
        type: props.paramType,
        inputmode: props.paramInputmode,
        defaultValue: defVal,
        placeholder: props.paramPlaceholder,
        min: props.paramMin,
        max: props.paramMax,
        step: props.paramStep,
      },
    ];
  }

  // 3. Automatically get params from instruction metadata
  if (instructionDef.value?.params && instructionDef.value.params.length > 0) {
    return instructionDef.value.params.map((p) => {
      const dynamicDef =
        alias && instructionDefaultsService.getDefault(alias, p.name);
      const defVal =
        dynamicDef !== undefined && dynamicDef !== ""
          ? dynamicDef
          : p.defaultValue !== undefined
            ? p.defaultValue
            : "";
      return {
        ...p,
        defaultValue: defVal,
      };
    });
  }

  return [];
});

function getInputMode(param) {
  if (param.inputmode) return param.inputmode;
  if (param.type === "number") return "numeric";
  if (param.type === "tel") return "tel";
  if (param.type === "email") return "email";
  if (param.type === "url") return "url";
  return "text";
}

function initInputValues() {
  configuredParams.value.forEach((p) => {
    inputValues[p.name] = "";
  });
}

function openModal() {
  initInputValues();
  testExecuted.value = false;
  isOpen.value = true;
}

function closeModal() {
  isOpen.value = false;
  testExecuted.value = false;
}

function getPayload() {
  if (configuredParams.value.length === 0) return null;
  if (configuredParams.value.length === 1) {
    const p = configuredParams.value[0];
    const raw = inputValues[p.name];
    if (raw !== undefined && raw !== null && String(raw).trim() !== "") {
      return String(raw).trim();
    }
    return p.defaultValue !== undefined && p.defaultValue !== null
      ? String(p.defaultValue).trim()
      : "";
  }
  const result = {};
  configuredParams.value.forEach((p) => {
    const raw = inputValues[p.name];
    if (raw !== undefined && raw !== null && String(raw).trim() !== "") {
      result[p.name] = String(raw).trim();
    } else if (p.defaultValue !== undefined && p.defaultValue !== null) {
      result[p.name] = String(p.defaultValue).trim();
    } else {
      result[p.name] = "";
    }
  });
  return result;
}

async function handleTest() {
  const payload = getPayload();
  try {
    if (typeof props.func === "function") {
      await props.func(payload, { ...inputValues });
    }
    emit("test", payload, { ...inputValues });
    testFeedbackText.value = "TEST INSTRUCTION BERHASIL DIJALANKAN";
  } catch (err) {
    testFeedbackText.value = `TEST GAGAL: ${err.message || "ERROR"}`;
  }
  testExecuted.value = true;
  setTimeout(() => {
    testExecuted.value = false;
  }, 2500);
}

async function handleConfirm() {
  const payload = getPayload();
  const targetLabelOrAlias = targetAlias.value;

  const debugState = remoteDebugService.addInstruction(
    targetLabelOrAlias,
    payload,
  );

  const historyItem = {
    id: `${Date.now()}-${Math.random().toString(36).substring(2, 6)}`,
    label: displayLabel.value,
    alias: remoteDebugService.getAlias(targetLabelOrAlias),
    params: payload,
    instructionString: debugState.instructionString,
    timestamp: new Date().toISOString(),
    formattedTime: new Date().toLocaleTimeString(),
  };

  try {
    const raw = localStorage.getItem(props.storageKey);
    const historyList = raw ? JSON.parse(raw) : [];
    historyList.unshift(historyItem);
    localStorage.setItem(props.storageKey, JSON.stringify(historyList));
  } catch (err) {
    console.error(err);
  }

  emit("confirm", historyItem, payload);
  closeModal();
}

// Warna tombol berasal dari data instruction-aliases.js jika tidak dispesifikasikan di props
const buttonColorClass = computed(() => {
  const colorName = props.color || instructionDef.value?.color || "neutral";
  switch (colorName.toLowerCase()) {
    case "green":
      return "bg-green-500 text-black hover:bg-green-400";
    case "red":
      return "bg-red-600 text-white hover:bg-red-700";
    case "blue":
      return "bg-blue-600 text-white hover:bg-blue-700";
    case "amber":
    case "orange":
      return "bg-amber-400 text-black hover:bg-amber-300";
    case "purple":
      return "bg-purple-600 text-white hover:bg-purple-700";
    case "white":
      return "bg-white text-black hover:bg-gray-100";
    case "neutral":
    default:
      return "bg-slate-200 text-black hover:bg-slate-300";
  }
});
</script>
