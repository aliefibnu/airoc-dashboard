import { createRouter, createWebHistory } from "vue-router";
import BleService from "@/services/ble.services.js";

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: "/",
      name: "home",
      component: { template: "<div></div>" },
    },
    {
      path: "/ble-dashboard",
      name: "ble-dashboard",
      component: () => import("../views/BleDashboard.vue"),
    },
    {
      path: "/remote",
      name: "remote",
      component: () => import("../views/Remote.vue"),
    },
    {
      path: "/production",
      alias: ["/production-instruction", "/prod-instruction"],
      name: "production-instruction",
      component: () => import("../views/ProductionInstruction.vue"),
    },
    {
      path: "/iot",
      alias: ["/iot-dashboard"],
      name: "iot-dashboard",
      component: () => import("../views/IotDashboard.vue"),
    },
    {
      path: "/default-settings",
      alias: "/settings",
      name: "default-settings",
      component: () => import("../views/DefaultSettings.vue"),
    },
  ],
});

router.beforeEach((to) => {
  if (to.path === "/") {
    const isConnected =
      BleService.connectedServer && BleService.connectedServer.connected;
    return isConnected ? "/remote" : "/ble-dashboard";
  }
});

export default router;
