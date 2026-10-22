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
      path: "/iot-dashboard",
      name: "iot-dashboard",
      component: () => import("../views/IoTDashboard.vue"),
    },
  ],
});

router.beforeEach((to) => {
  if (to.path === "/") {
    const isConnected =
      BleService.connectedServer && BleService.connectedServer.connected;
    return isConnected ? "/iot-dashboard" : "/ble-dashboard";
  }
});

export default router;
