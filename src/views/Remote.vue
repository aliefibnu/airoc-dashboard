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

    <div class="space-y-6">
      <div class="space-y-4">
        <h3
          class="text-lg font-black uppercase tracking-wider text-black border-l-4 border-black pl-3"
        >
          1. GERAKAN DASAR
        </h3>
        <div
          class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-4"
        >
          <InstructionButton
            label="MAJU"
            alias="MAJU"
            color="green"
            :has-param="true"
            param-label="JARAK MAJU (CM)"
            param-type="number"
            param-inputmode="numeric"
            param-default="10"
            param-placeholder="10 (DEFAULT)"
            :func="(p) => executeTestInstruction('MAJU', p)"
          />

          <InstructionButton
            label="MUNDUR"
            alias="MUNDUR"
            color="yellow"
            :has-param="true"
            param-label="JARAK MUNDUR (CM)"
            param-type="number"
            param-inputmode="numeric"
            param-default="10"
            param-placeholder="10 (DEFAULT)"
            :func="(p) => executeTestInstruction('MUNDUR', p)"
          />

          <InstructionButton
            label="BELOK KIRI"
            alias="KIRI"
            color="blue"
            :has-param="true"
            param-label="SUDUT PUTAR (DERAJAT)"
            param-type="number"
            param-inputmode="numeric"
            param-default="90"
            param-placeholder="90 (DEFAULT)"
            :func="(p) => executeTestInstruction('KIRI', p)"
          />

          <InstructionButton
            label="BELOK KANAN"
            alias="KANAN"
            color="blue"
            :has-param="true"
            param-label="SUDUT PUTAR (DERAJAT)"
            param-type="number"
            param-inputmode="numeric"
            param-default="90"
            param-placeholder="90 (DEFAULT)"
            :func="(p) => executeTestInstruction('KANAN', p)"
          />

          <InstructionButton
            label="STOP MOTOR"
            alias="STOP"
            color="red"
            :has-param="false"
            :func="() => executeTestInstruction('STOP')"
          />

          <InstructionButton
            label="KECEPATAN (SPEED)"
            alias="SPEED"
            color="purple"
            :has-param="true"
            param-label="LEVEL (1-10)"
            param-type="number"
            param-inputmode="numeric"
            param-default="5"
            param-min="1"
            param-max="10"
            param-placeholder="5 (DEFAULT)"
            :func="(p) => executeTestInstruction('SPEED', p)"
          />

          <InstructionButton
            label="TUNGGU (DELAY)"
            alias="TUNGGU"
            color="yellow"
            :has-param="true"
            param-label="DURASI TUNGGU (MS)"
            param-type="number"
            param-inputmode="numeric"
            param-default="1000"
            param-placeholder="1000 (DEFAULT)"
            :func="(p) => executeTestInstruction('TUNGGU', p)"
          />
        </div>
      </div>

      <div class="space-y-4">
        <h3
          class="text-lg font-black uppercase tracking-wider text-black border-l-4 border-black pl-3"
        >
          2. ULTRASONIC & PENCARIAN OBJEK
        </h3>
        <div
          class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-4"
        >
          <InstructionButton
            label="UKUR JARAK"
            alias="UKUR_JARAK"
            color="white"
            :has-param="false"
            :func="() => executeTestInstruction('UKUR_JARAK')"
          />

          <InstructionButton
            label="SCAN KANAN"
            alias="SCAN_KANAN"
            color="blue"
            :params="[
              {
                name: 'jarak_cm',
                label: 'JARAK MAKSIMUM (CM)',
                type: 'number',
                inputmode: 'numeric',
                defaultValue: 20,
                placeholder: '20 (DEFAULT)',
              },
              {
                name: 'sudut_maks',
                label: 'SUDUT SCAN (DEG)',
                type: 'number',
                inputmode: 'numeric',
                defaultValue: 90,
                placeholder: '90 (DEFAULT)',
              },
            ]"
            :func="(p) => executeTestInstruction('SCAN_KANAN', p)"
          />

          <InstructionButton
            label="SCAN KIRI"
            alias="SCAN_KIRI"
            color="blue"
            :params="[
              {
                name: 'jarak_cm',
                label: 'JARAK MAKSIMUM (CM)',
                type: 'number',
                inputmode: 'numeric',
                defaultValue: 20,
                placeholder: '20 (DEFAULT)',
              },
              {
                name: 'sudut_maks',
                label: 'SUDUT SCAN (DEG)',
                type: 'number',
                inputmode: 'numeric',
                defaultValue: 90,
                placeholder: '90 (DEFAULT)',
              },
            ]"
            :func="(p) => executeTestInstruction('SCAN_KIRI', p)"
          />

          <InstructionButton
            label="DEKATI OBJEK"
            alias="DEKATI_OBJEK"
            color="green"
            :has-param="true"
            param-label="JARAK TARGET AKHIR (CM)"
            param-type="number"
            param-inputmode="numeric"
            param-default="6"
            param-placeholder="6 (DEFAULT)"
            :func="(p) => executeTestInstruction('DEKATI_OBJEK', p)"
          />

          <InstructionButton
            label="JAUHI OBJEK"
            alias="JAUHI_OBJEK"
            color="yellow"
            :has-param="true"
            param-label="JARAK TARGET AKHIR (CM)"
            param-type="number"
            param-inputmode="numeric"
            param-default="20"
            param-placeholder="20 (DEFAULT)"
            :func="(p) => executeTestInstruction('JAUHI_OBJEK', p)"
          />

          <InstructionButton
            label="CEK OBJEK"
            alias="CEK_OBJEK"
            color="white"
            :has-param="true"
            param-label="JARAK CEK (CM)"
            param-type="number"
            param-inputmode="numeric"
            param-default="20"
            param-placeholder="20 (DEFAULT)"
            :func="(p) => executeTestInstruction('CEK_OBJEK', p)"
          />

          <InstructionButton
            label="BATAS SCAN"
            alias="BATAS_SCAN"
            color="purple"
            :has-param="true"
            param-label="BATAS SUDUT (DEG)"
            param-type="number"
            param-inputmode="numeric"
            param-default="90"
            param-placeholder="90 (DEFAULT)"
            :func="(p) => executeTestInstruction('BATAS_SCAN', p)"
          />

          <InstructionButton
            label="LAPOR JARAK"
            alias="LAPOR_JARAK"
            color="white"
            :has-param="false"
            :func="() => executeTestInstruction('LAPOR_JARAK')"
          />
        </div>
      </div>

      <div class="space-y-4">
        <h3
          class="text-lg font-black uppercase tracking-wider text-black border-l-4 border-black pl-3"
        >
          3. GRIPPER & MANIPULASI
        </h3>
        <div
          class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-4"
        >
          <InstructionButton
            label="CAPIT (TUTUP)"
            alias="CAPIT"
            color="red"
            :has-param="false"
            :func="() => executeTestInstruction('CAPIT')"
          />

          <InstructionButton
            label="LEPAS (BUKA)"
            alias="LEPAS"
            color="green"
            :has-param="false"
            :func="() => executeTestInstruction('LEPAS')"
          />
        </div>
      </div>

      <div class="space-y-4">
        <h3
          class="text-lg font-black uppercase tracking-wider text-black border-l-4 border-black pl-3"
        >
          4. MANUVER ZONA
        </h3>
        <div
          class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-4"
        >
          <InstructionButton
            label="GESER KANAN"
            alias="GESER_KANAN"
            color="blue"
            :has-param="true"
            param-label="JARAK GESER (CM)"
            param-type="number"
            param-inputmode="numeric"
            param-default="10"
            param-placeholder="10 (DEFAULT)"
            :func="(p) => executeTestInstruction('GESER_KANAN', p)"
          />

          <InstructionButton
            label="GESER KIRI"
            alias="GESER_KIRI"
            color="blue"
            :has-param="true"
            param-label="JARAK GESER (CM)"
            param-type="number"
            param-inputmode="numeric"
            param-default="10"
            param-placeholder="10 (DEFAULT)"
            :func="(p) => executeTestInstruction('GESER_KIRI', p)"
          />
        </div>
      </div>

      <div class="space-y-4">
        <h3
          class="text-lg font-black uppercase tracking-wider text-black border-l-4 border-black pl-3"
        >
          5. BUZZER & LED FEEDBACK
        </h3>
        <div
          class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-4"
        >
          <InstructionButton
            label="BUNYI BUZZER"
            alias="BUNYI"
            color="yellow"
            :has-param="true"
            param-label="JUMLAH KALI BUNYI"
            param-type="number"
            param-inputmode="numeric"
            param-default="1"
            param-placeholder="1 (DEFAULT)"
            :func="(p) => executeTestInstruction('BUNYI', p)"
          />

          <InstructionButton
            label="BEEP NADA"
            alias="BEEP"
            color="yellow"
            :params="[
              {
                name: 'frekuensi',
                label: 'FREKUENSI (HZ)',
                type: 'number',
                inputmode: 'numeric',
                defaultValue: 1500,
                placeholder: '1500 (DEFAULT)',
              },
              {
                name: 'durasi_ms',
                label: 'DURASI (MS)',
                type: 'number',
                inputmode: 'numeric',
                defaultValue: 100,
                placeholder: '100 (DEFAULT)',
              },
            ]"
            :func="(p) => executeTestInstruction('BEEP', p)"
          />

          <InstructionButton
            label="LED WARNA"
            alias="LED_WARNA"
            color="purple"
            :has-param="true"
            param-label="WARNA (MERAH/HIJAU/BIRU/KUNING/PUTIH/OFF)"
            param-type="text"
            param-default="PUTIH"
            param-placeholder="PUTIH (DEFAULT)"
            :func="(p) => executeTestInstruction('LED_WARNA', p)"
          />

          <InstructionButton
            label="LED KEDIP"
            alias="LED_KEDIP"
            color="purple"
            :params="[
              {
                name: 'jumlah',
                label: 'JUMLAH KEDIP',
                type: 'number',
                inputmode: 'numeric',
                defaultValue: 3,
                placeholder: '3 (DEFAULT)',
              },
              {
                name: 'interval_ms',
                label: 'INTERVAL (MS)',
                type: 'number',
                inputmode: 'numeric',
                defaultValue: 200,
                placeholder: '200 (DEFAULT)',
              },
            ]"
            :func="(p) => executeTestInstruction('LED_KEDIP', p)"
          />

          <InstructionButton
            label="MODE LED"
            alias="LED"
            color="white"
            :has-param="true"
            param-label="MODE (ON/OFF/SMILE/FROWN/LEFT/RIGHT)"
            param-type="text"
            param-default="ON"
            param-placeholder="ON (DEFAULT)"
            :func="(p) => executeTestInstruction('LED', p)"
          />
        </div>
      </div>

      <div class="space-y-4">
        <h3
          class="text-lg font-black uppercase tracking-wider text-black border-l-4 border-black pl-3"
        >
          6. KESELAMATAN & AKHIR EKSEKUSI
        </h3>
        <div
          class="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-4"
        >
          <InstructionButton
            label="EMERGENCY STOP"
            alias="EMERGENCY_STOP"
            color="red"
            :has-param="false"
            :func="() => executeTestInstruction('EMERGENCY_STOP')"
          />

          <InstructionButton
            label="END EKSEKUSI"
            alias="END"
            color="white"
            :has-param="false"
            :func="() => executeTestInstruction('END')"
          />
        </div>
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
