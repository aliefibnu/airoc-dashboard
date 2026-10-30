export const PRESET_INSTRUCTIONS = {
  PINGGIR_AMBIL: {
    KANAN: {
      JAUH: "FOLLOW_LINE:1,7,MAJU;BELOK_KANAN:40,5,MAJU;MAJU:20,10;CAPIT;MUNDUR:21,10;MUNDUR_LINE:50,5",
      DEKAT: "",
    },
    KIRI: {
      JAUH: "",
      DEKAT: "",
    },
  },
  PINGGIR_MASUK: {
    KANAN: {
      JAUH: "",
      DEKAT: "",
    },
    KIRI: {
      JAUH: "",
      DEKAT: "",
    },
  },
  TENGAH_AMBIL: {
    KANAN: "",
    KIRI: "",
  },
  TENGAH_MASUK: {
    KANAN: "",
    KIRI: "",
  },
};

export function getPresetString(presetNameOrAlias, params) {
  if (!presetNameOrAlias) return "";
  const key = String(presetNameOrAlias).trim().toUpperCase();
  const entry = PRESET_INSTRUCTIONS[key];
  if (!entry) return "";

  let arah = "KANAN";
  let jarak = "JAUH";

  if (typeof params === "string") {
    const parts = params.split(",").map((p) => p.trim().toUpperCase());
    if (parts[0]) arah = parts[0];
    if (parts[1]) jarak = parts[1];
  } else if (Array.isArray(params)) {
    if (params[0]) arah = String(params[0]).trim().toUpperCase();
    if (params[1]) jarak = String(params[1]).trim().toUpperCase();
  } else if (params && typeof params === "object") {
    if (params.arah) arah = String(params.arah).trim().toUpperCase();
    if (params.jarak) jarak = String(params.jarak).trim().toUpperCase();
  }

  if (typeof entry[arah] === "object" && entry[arah] !== null) {
    return entry[arah][jarak] || "";
  }

  if (typeof entry[arah] === "string") {
    return entry[arah];
  }

  return "";
}

export function isPresetInstruction(nameOrAlias) {
  if (!nameOrAlias) return false;
  const key = String(nameOrAlias).trim().toUpperCase();
  return Boolean(PRESET_INSTRUCTIONS[key]);
}
