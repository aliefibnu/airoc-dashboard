<template>
  <div class="min-h-screen bg-yellow-50 flex flex-col text-gray-900">
    <header class="hidden md:block bg-yellow-300 text-black p-4 shadow-md">
      <div class="max-w-7xl mx-auto flex justify-between items-center">
        <h1 class="text-xl font-black tracking-wider">Airoc Dashboard</h1>
        <nav class="flex space-x-6">
          <RouterLink
            to="/ble-dashboard"
            class="text-lg font-bold uppercase hover:text-yellow-900 transition-colors"
            active-class="text-yellow-600"
            >BLE</RouterLink
          >
          <RouterLink
            to="/remote"
            class="text-lg font-bold uppercase hover:text-yellow-900 transition-colors"
            active-class="text-yellow-600"
            >REMOTE</RouterLink
          >
          <RouterLink
            to="/iot-dashboard"
            class="text-lg font-bold uppercase hover:text-yellow-900 transition-colors"
            active-class="text-yellow-600"
            >IOT</RouterLink
          >
        </nav>
      </div>
    </header>

    <main class="flex-1 p-6 md:p-12 pb-32 md:pb-12">
      <RouterView />
    </main>

    <nav
      class="md:hidden fixed bottom-0 left-0 right-0 bg-yellow-200 text-black p-6 grid grid-cols-3 text-center border-t-4 border-yellow-400 shadow-2xl z-50"
    >
      <RouterLink
        to="/ble-dashboard"
        class="text-lg font-black uppercase"
        active-class="text-blue-600"
        >BLE</RouterLink
      >
      <RouterLink
        to="/remote"
        class="text-lg font-black uppercase"
        active-class="text-blue-600"
        >REMOTE</RouterLink
      >
    </nav>
  </div>
</template>

<script setup>
import { RouterView, RouterLink } from "vue-router";
</script>