<template>
  <div class="max-w-5xl mx-auto space-y-8 select-none">
    <div
      class="flex flex-col md:flex-row justify-between items-start md:items-center gap-4 border-b-4 border-black pb-6"
    >
      <div>
        <h1 class="text-3xl md:text-4xl font-black uppercase tracking-tight">
          REMOTE DEBUGGING CONTROLLER
        </h1>
      </div>

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

    <div class="space-y-3">
      <textarea
        v-model="manualInstructionString"
        rows="3"
        placeholder="KETIK ATAU EDIT INSTRUCTION STRING (CONTOH: FWD:150;LEFT:90;STP)..."
        class="w-full font-mono text-base md:text-lg font-black bg-white border-4 border-black p-4 rounded-2xl shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] focus:outline-none focus:bg-yellow-50 select-text"
      ></textarea>

      <div class="flex flex-wrap items-center gap-3">
        <button
          type="button"
          @click="saveManualInstruction"
          class="bg-green-500 text-black font-black uppercase text-base md:text-lg py-3 px-6 rounded-xl border-4 border-black shadow-[4px_4px_0px_0px_rgba(0,0,0,1)] hover:bg-green-400 active:translate-y-1 active:shadow-none transition-all cursor-pointer"
        >
          <h3>SIMPAN INSTRUKSI</h3>
        </button>

        <span
          v-if="saveNotification"
          class="text-xs md:text-sm font-black bg-green-200 text-green-900 border-2 border-black px-3 py-2 rounded-lg uppercase"
        >
          {{ saveNotification }}
        </span>
      </div>
    </div>

    <div class="space-y-4">
      <div class="border-b-4 border-black pb-2">
        <h2 class="text-2xl font-black uppercase tracking-tight">
          PANEL TOMBOL DEBUG INSTRUCTION
        </h2>
      </div>

      <div class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 gap-4">
        <InstructionButton
          label="MAJU (SPEED)"
          color="green"
          :has-param="true"
          param-label="SPEED PWM (0 - 255)"
          param-type="number"
          param-inputmode="numeric"
          :param-default="150"
          :param-min="0"
          :param-max="255"
          param-placeholder="150"
          :func="(p) => logAction('MAJU', p)"
          :reverse-function="(p) => logAction('MUNDUR', p)"
        />

        <InstructionButton
          label="MUNDUR (SPEED)"
          color="yellow"
          :has-param="true"
          param-label="SPEED PWM (0 - 255)"
          param-type="number"
          param-inputmode="numeric"
          :param-default="150"
          :param-min="0"
          :param-max="255"
          param-placeholder="150"
          :func="(p) => logAction('MUNDUR', p)"
          :reverse-function="(p) => logAction('MAJU', p)"
        />

        <InstructionButton
          label="STOP ROBOT"
          color="red"
          :func="() => logAction('STOP')"
        />

        <InstructionButton
          label="BELOK KIRI"
          color="blue"
          :has-param="true"
          param-label="SUDUT / DERAJAT"
          param-type="number"
          param-inputmode="numeric"
          :param-default="90"
          param-placeholder="90"
          :func="(p) => logAction('KIRI', p)"
          :reverse-function="(p) => logAction('KANAN', p)"
        />

        <InstructionButton
          label="BELOK KANAN"
          color="blue"
          :has-param="true"
          param-label="SUDUT / DERAJAT"
          param-type="number"
          param-inputmode="numeric"
          :param-default="90"
          param-placeholder="90"
          :func="(p) => logAction('KANAN', p)"
          :reverse-function="(p) => logAction('KIRI', p)"
        />

        <InstructionButton
          label="SET SERVO"
          color="purple"
          :params="[
            {
              name: 'angle',
              label: 'SUDUT (0-180)',
              type: 'number',
              inputmode: 'numeric',
              defaultValue: 90,
              min: 0,
              max: 180,
            },
            {
              name: 'delay',
              label: 'DELAY (MS)',
              type: 'number',
              inputmode: 'numeric',
              defaultValue: 300,
            },
          ]"
          :func="(p) => logAction('SERVO', p)"
          :reverse-function="() => logAction('SERVO', 90)"
        />

        <InstructionButton
          label="DELAY WAKTU"
          color="yellow"
          :has-param="true"
          param-label="MILLISECONDS (MS)"
          param-type="number"
          param-inputmode="numeric"
          :param-default="500"
          param-placeholder="500"
          :func="(p) => logAction('DELAY', p)"
        />

        <InstructionButton
          label="BUKA GRIPPER"
          color="green"
          :func="() => logAction('GRIPPER_OPEN')"
          :reverse-function="() => logAction('GRIPPER_CLOSE')"
        />

        <InstructionButton
          label="TUTUP GRIPPER"
          color="red"
          :func="() => logAction('GRIPPER_CLOSE')"
          :reverse-function="() => logAction('GRIPPER_OPEN')"
        />
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from "vue";
import InstructionButton from "@/components/InstructionButton.vue";
import remoteDebugService from "@/services/remote-debug.service.js";
import BleService from "@/services/ble.services.js";

const manualInstructionString = ref("");
const instructionList = ref([]);
const isBleConnected = ref(false);
const connectedDeviceName = ref("");
const saveNotification = ref("");

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

function logAction(name, params) {
  console.log(`[ACTION] ${name}:`, params);
}

function saveManualInstruction() {
  remoteDebugService.setInstructionString(manualInstructionString.value);
  refreshState();
  saveNotification.value = "INSTRUKSI BERHASIL DISIMPAN";
  setTimeout(() => {
    saveNotification.value = "";
  }, 2500);
}
</script>
