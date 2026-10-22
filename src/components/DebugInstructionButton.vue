<template>
  <InstructionButton v-bind="$attrs">
    <slot />
  </InstructionButton>
</template>

<script setup>
import InstructionButton from "./InstructionButton.vue";
</script>