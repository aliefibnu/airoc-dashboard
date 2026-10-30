export const PRESET_INSTRUCTIONS = {
  PINGGIR_AMBIL: {
    KANAN: {
      JAUH: "BELOK_KANAN:40,5,MAJU;MAJU:20,10;CAPIT;MUNDUR:22,10;MUNDUR_LINE:10,5",
      DEKAT: "BELOK_KANAN:40,5,MAJU;MAJU:5,10;CAPIT;MUNDUR:10,5",
    },
    KIRI: {
      JAUH: "BELOK_KIRI:40,5,MAJU;MAJU:20,10;CAPIT;MUNDUR:22,10;MUNDUR_LINE:10,5",
      DEKAT: "BELOK_KIRI:40,5,MAJU;MAJU:5,10;CAPIT;MUNDUR:10,5",
    },
  },
  PINGGIR_MASUK: {
    KANAN: {
      JAUH: "BELOK_KANAN:40,5,MAJU;MAJU:20,10;CAPIT;MUNDUR:5,10;LEPAS;MUNDUR:17,10;MUNDUR_LINE:10,5",
      DEKAT: "BELOK_KANAN:40,5,MAJU;MAJU:12,10;MUNDUR:17,5",
    },
    KIRI: {
      JAUH: "BELOK_KIRI:40,5,MAJU;MAJU:20,10;CAPIT;MUNDUR:5,10;LEPAS;MUNDUR:17,10;MUNDUR_LINE:10,5",
      DEKAT: "BELOK_KIRI:40,5,MAJU;MAJU:12,10;MUNDUR:17,5",
    },
  },
  TENGAH_AMBIL: {
    KANAN: "KANAN:40,5;MAJU:7,10;CAPIT;MUNDUR:7,10;KIRI:40,5;MAJU:1,10;",
    KIRI: "KIRI:40,5;MAJU:7,10;CAPIT;MUNDUR:7,10;KANAN:40,5;MAJU:1,10;",
  },
  TENGAH_MASUK: {
    KANAN:
      "LEPAS;FOLLOW_LINE:1,7,;MAJU:3,10;BELOK_KANAN:100,10,MAJU;BELOK_KANAN:100,10,MUNDUR",
    KIRI: "LEPAS;FOLLOW_LINE:1,7,;MAJU:3,10;BELOK_KIRI:100,10,MAJU;BELOK_KIRI:100,10,MUNDUR",
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
