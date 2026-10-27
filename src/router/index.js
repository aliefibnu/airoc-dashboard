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
