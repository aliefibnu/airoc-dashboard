import { INSTRUCTION_ALIASES } from "@/data/instruction-aliases.js";
import instructionDefaultsService from "@/services/instruction-defaults.service.js";

const STORAGE_KEY_STATE = "airoc_remote_debug_state";

class RemoteDebugService {
  constructor() {
    this.listeners = new Set();
  }

  getAliases() {
    return [...INSTRUCTION_ALIASES];
  }

  getAlias(nameOrLabel) {
    if (!nameOrLabel) return "UNKNOWN";
    const str = String(nameOrLabel).trim().toUpperCase();
    const list = this.getAliases();

    const directAliasMatch = list.find((item) => item.alias.toUpperCase() === str);
    if (directAliasMatch) return directAliasMatch.alias;

    const exactNameMatch = list.find((item) => item.name.toUpperCase() === str);
    if (exactNameMatch) return exactNameMatch.alias;

    const startsWithName = list.find((item) => str.startsWith(item.name.toUpperCase()));
    if (startsWithName) return startsWithName.alias;

    const includesName = list.find((item) => str.includes(item.name.toUpperCase()));
    if (includesName) return includesName.alias;

    const sanitized = str.replace(/[^A-Z0-9_]/g, "_").replace(/_+/g, "_").slice(0, 10);
    return sanitized || "CMD";
  }

  findActionByAlias(alias) {
    if (!alias) return null;
    const str = String(alias).trim().toUpperCase();
    return this.getAliases().find((item) => item.alias.toUpperCase() === str) || null;
  }

  normalizeParams(params) {
    if (params === undefined || params === null || params === "") {
      return [];
    }
    if (Array.isArray(params)) {
      return params
        .filter((p) => p !== undefined && p !== null && p !== "")
        .map((p) => String(p).trim());
    }
    if (typeof params === "object") {
      return Object.values(params)
        .filter((p) => p !== undefined && p !== null && p !== "")
        .map((p) => String(p).trim());
    }
    return [String(params).trim()];
  }

  formatSingleInstruction(alias, params = []) {
    const cleanParams = this.normalizeParams(params);
    if (cleanParams.length > 0) {
      return `${alias}:${cleanParams.join(",")}`;
    }
    return alias;
  }

  formatInstructions(instructions) {
    if (!Array.isArray(instructions) || instructions.length === 0) {
      return "";
    }
    return instructions
      .map((item) => {
        const alias = item.alias || this.getAlias(item.name || item.label);
        return this.formatSingleInstruction(alias, item.params);
      })
      .join(";");
  }

  resolveInstructionToken(token) {
    const trimmed = String(token || "").trim();
    if (!trimmed) return "";
    const colonIdx = trimmed.indexOf(":");
    let cmd = trimmed;
    let paramPart = "";
    if (colonIdx !== -1) {
      cmd = trimmed.slice(0, colonIdx).trim().toUpperCase();
      paramPart = trimmed.slice(colonIdx + 1).trim();
    } else {
      cmd = trimmed.toUpperCase();
    }

    if (paramPart.length > 0) {
      return `${cmd}:${paramPart}`;
    }

    const defs = instructionDefaultsService.getAllDefaults();
    const defForCmd = defs[cmd];
    if (defForCmd && typeof defForCmd === "object") {
      const vals = Object.values(defForCmd).filter(
        (v) => v !== undefined && v !== null && v !== "",
      );
      if (vals.length > 0) {
        return `${cmd}:${vals.join(",")}`;
      }
    }

    const action = this.findActionByAlias(cmd);
    if (action && Array.isArray(action.params) && action.params.length > 0) {
      const defaultVals = action.params
        .map((p) => p.defaultValue)
        .filter((v) => v !== undefined && v !== null && v !== "");
      if (defaultVals.length > 0) {
        return `${cmd}:${defaultVals.join(",")}`;
      }
    }

    return cmd;
  }

  getPreparedBatches(startBatchNum = 1, startInstNum = 1) {
    const allBatches = this.getAllBatches();
    const preparedBatches = [];
    const minBatch = Math.max(1, Math.min(10, Number(startBatchNum) || 1));
    const minInst = Math.max(1, Number(startInstNum) || 1);

    for (let b = 0; b < 10; b++) {
      const bNum = b + 1;
      if (bNum < minBatch) {
        continue;
      }
      if (!this.isBatchEnabled(b)) {
        continue;
      }
      const raw = String(allBatches[b] || "").trim();
      if (!raw) continue;
      let tokens = raw
        .split(";")
        .map((t) => t.trim())
        .filter(
          (t) =>
            t.length > 0 &&
            t.toUpperCase() !== "END" &&
            t.toUpperCase() !== "CODE" &&
            t.toUpperCase() !== "PROD",
        )
        .map((t) => this.resolveInstructionToken(t));

      if (bNum === minBatch && minInst > 1) {
        tokens = tokens.slice(minInst - 1);
      }

      if (tokens.length > 0) {
        const chunk = tokens.slice(0, 12);
        chunk.push("END");
        preparedBatches.push({
          batchNum: bNum,
          batchIndex: b,
          command: `BATCH:${bNum};${chunk.join(";")}`,
          count: chunk.length - 1,
          tokens: chunk.slice(0, -1),
        });
      }
    }
    return preparedBatches;
  }

  parseInstructionString(rawString) {
    if (!rawString || typeof rawString !== "string") return [];
    return rawString
      .split(";")
      .map((seg) => seg.trim())
      .filter(Boolean)
      .map((segment, index) => {
        const colonIndex = segment.indexOf(":");
        if (colonIndex === -1) {
          const alias = segment.trim().toUpperCase();
          const action = this.findActionByAlias(alias);
          return {
            id: `cmd_${Date.now()}_${index}`,
            alias,
            name: action ? action.name : alias,
            params: [],
            formatted: alias,
          };
        }
        const alias = segment.slice(0, colonIndex).trim().toUpperCase();
        const paramStr = segment.slice(colonIndex + 1).trim();
        const params = paramStr.length > 0 ? paramStr.split(",").map((p) => p.trim()) : [];
        const action = this.findActionByAlias(alias);
        return {
          id: `cmd_${Date.now()}_${index}`,
          alias,
          name: action ? action.name : alias,
          params,
          formatted: `${alias}:${params.join(",")}`,
        };
      });
  }

  readState() {
    try {
      const raw = localStorage.getItem(STORAGE_KEY_STATE);
      if (raw) {
        const parsed = JSON.parse(raw);
        let batches = Array(10).fill("");
        if (Array.isArray(parsed.batches) && parsed.batches.length === 10) {
          batches = parsed.batches.map((b) => String(b || ""));
        } else if (parsed.instructionString) {
          const tokens = String(parsed.instructionString)
            .split(";")
            .map((t) => t.trim())
            .filter(Boolean);
          for (let i = 0; i < 10; i++) {
            batches[i] = tokens.slice(i * 12, (i + 1) * 12).join(";");
          }
        }
        let batchesEnabled = Array(10).fill(true);
        if (Array.isArray(parsed.batchesEnabled) && parsed.batchesEnabled.length === 10) {
          batchesEnabled = parsed.batchesEnabled.map((b) => b !== false);
        }
        let batchNames = Array(10).fill("");
        if (Array.isArray(parsed.batchNames) && parsed.batchNames.length === 10) {
          batchNames = parsed.batchNames.map((n) => String(n || ""));
        }

        const activeBatchIndex =
          typeof parsed.activeBatchIndex === "number" &&
          parsed.activeBatchIndex >= 0 &&
          parsed.activeBatchIndex < 10
            ? parsed.activeBatchIndex
            : 0;

        const currentString = batches[activeBatchIndex] || "";
        const instructions = this.parseInstructionString(currentString);

        return {
          batches,
          batchesEnabled,
          batchNames,
          activeBatchIndex,
          instructionString: currentString,
          instructions,
          history: Array.isArray(parsed.history) ? parsed.history : [],
          lastUpdated: parsed.lastUpdated || null,
        };
      }
    } catch (e) {
      console.error(e);
    }
    return {
      batches: Array(10).fill(""),
      batchesEnabled: Array(10).fill(true),
      batchNames: Array(10).fill(""),
      activeBatchIndex: 0,
      instructionString: "",
      instructions: [],
      history: [],
      lastUpdated: null,
    };
  }

  saveState(state) {
    try {
      const currentState = this.readState();
      const batches =
        Array.isArray(state.batches) && state.batches.length === 10
          ? state.batches
          : Array.isArray(currentState.batches) && currentState.batches.length === 10
            ? currentState.batches
            : Array(10).fill("");
      const batchesEnabled =
        Array.isArray(state.batchesEnabled) && state.batchesEnabled.length === 10
          ? state.batchesEnabled.map((b) => b !== false)
          : Array.isArray(currentState.batchesEnabled) && currentState.batchesEnabled.length === 10
            ? currentState.batchesEnabled.map((b) => b !== false)
            : Array(10).fill(true);
      const batchNames =
        Array.isArray(state.batchNames) && state.batchNames.length === 10
          ? state.batchNames.map((n) => String(n || ""))
          : Array.isArray(currentState.batchNames) && currentState.batchNames.length === 10
            ? currentState.batchNames.map((n) => String(n || ""))
            : Array(10).fill("");
      const activeBatchIndex =
        typeof state.activeBatchIndex === "number" &&
        state.activeBatchIndex >= 0 &&
        state.activeBatchIndex < 10
          ? state.activeBatchIndex
          : typeof currentState.activeBatchIndex === "number"
            ? currentState.activeBatchIndex
            : 0;
      const currentString = batches[activeBatchIndex] || "";
      const instructions = state.instructions || this.parseInstructionString(currentString);

      const normalizedState = {
        batches,
        batchesEnabled,
        batchNames,
        activeBatchIndex,
        instructionString: currentString,
        instructions,
        history: state.history || currentState.history || [],
        lastUpdated: new Date().toISOString(),
      };
      localStorage.setItem(STORAGE_KEY_STATE, JSON.stringify(normalizedState));
      this.listeners.forEach((fn) => fn(normalizedState));
      return normalizedState;
    } catch (e) {
      console.error(e);
      return state;
    }
  }

  getAllBatches() {
    return [...this.readState().batches];
  }

  getActiveBatchIndex() {
    return this.readState().activeBatchIndex;
  }

  setActiveBatchIndex(index) {
    const safeIdx = Math.max(0, Math.min(9, Number(index) || 0));
    const currentState = this.readState();
    const currentString = currentState.batches[safeIdx] || "";
    const instructions = this.parseInstructionString(currentString);

    return this.saveState({
      ...currentState,
      activeBatchIndex: safeIdx,
      instructionString: currentString,
      instructions,
    });
  }

  getBatchInstructionString(index) {
    const safeIdx = Math.max(0, Math.min(9, Number(index) || 0));
    return this.readState().batches[safeIdx] || "";
  }

  setBatchInstructionString(index, rawString) {
    const safeIdx = Math.max(0, Math.min(9, Number(index) || 0));
    const currentState = this.readState();
    const batches = [...currentState.batches];
    batches[safeIdx] = String(rawString || "").trim();

    return this.saveState({
      ...currentState,
      batches,
      instructionString: batches[currentState.activeBatchIndex] || "",
      instructions: this.parseInstructionString(batches[currentState.activeBatchIndex] || ""),
    });
  }

  clearBatch(index) {
    return this.setBatchInstructionString(index, "");
  }

  clearAllBatches() {
    const currentState = this.readState();
    return this.saveState({
      ...currentState,
      batches: Array(10).fill(""),
      instructionString: "",
      instructions: [],
    });
  }

  getBatchesEnabled() {
    return [...this.readState().batchesEnabled];
  }

  isBatchEnabled(index) {
    const safeIdx = Math.max(0, Math.min(9, Number(index) || 0));
    return this.readState().batchesEnabled[safeIdx] !== false;
  }

  setBatchEnabled(index, enabled) {
    const safeIdx = Math.max(0, Math.min(9, Number(index) || 0));
    const currentState = this.readState();
    const batchesEnabled = [...currentState.batchesEnabled];
    batchesEnabled[safeIdx] = Boolean(enabled);
    return this.saveState({
      ...currentState,
      batchesEnabled,
    });
  }

  toggleBatchEnabled(index) {
    const safeIdx = Math.max(0, Math.min(9, Number(index) || 0));
    const current = this.isBatchEnabled(safeIdx);
    return this.setBatchEnabled(safeIdx, !current);
  }

  getAllBatchNames() {
    return [...this.readState().batchNames];
  }

  getBatchName(index) {
    const safeIdx = Math.max(0, Math.min(9, Number(index) || 0));
    return this.readState().batchNames[safeIdx] || "";
  }

  setBatchName(index, name) {
    const safeIdx = Math.max(0, Math.min(9, Number(index) || 0));
    const currentState = this.readState();
    const batchNames = [...currentState.batchNames];
    batchNames[safeIdx] = String(name || "").trim();
    return this.saveState({
      ...currentState,
      batchNames,
    });
  }

  exportToJson() {
    const state = this.readState();
    const payload = {
      version: 1,
      exportedAt: new Date().toISOString(),
      batches: state.batches,
      batchesEnabled: state.batchesEnabled,
      batchNames: state.batchNames,
    };
    return JSON.stringify(payload, null, 2);
  }

  importFromJson(jsonInput) {
    let parsed = null;
    if (typeof jsonInput === "string") {
      try {
        parsed = JSON.parse(jsonInput.trim());
      } catch (err) {
        throw new Error("Format JSON tidak valid! Pastikan sintaks JSON benar.");
      }
    } else if (typeof jsonInput === "object" && jsonInput !== null) {
      parsed = jsonInput;
    } else {
      throw new Error("Data impor tidak valid!");
    }

    let newBatches = Array(10).fill("");
    let newBatchesEnabled = Array(10).fill(true);
    let newBatchNames = Array(10).fill("");

    if (Array.isArray(parsed)) {
      parsed.slice(0, 10).forEach((item, idx) => {
        newBatches[idx] = String(item || "").trim();
      });
    } else if (Array.isArray(parsed.batches)) {
      parsed.batches.slice(0, 10).forEach((item, idx) => {
        newBatches[idx] = String(item || "").trim();
      });
      if (Array.isArray(parsed.batchesEnabled)) {
        parsed.batchesEnabled.slice(0, 10).forEach((item, idx) => {
          newBatchesEnabled[idx] = item !== false;
        });
      }
      if (Array.isArray(parsed.batchNames)) {
        parsed.batchNames.slice(0, 10).forEach((item, idx) => {
          newBatchNames[idx] = String(item || "").trim();
        });
      }
    } else if (typeof parsed === "object") {
      let foundAny = false;
      for (let i = 0; i < 10; i++) {
        const keyNum = String(i + 1);
        const keyBatch = `batch${i + 1}`;
        const keyB = `b${i + 1}`;
        if (parsed[keyNum] !== undefined) {
          newBatches[i] = String(parsed[keyNum] || "").trim();
          foundAny = true;
        } else if (parsed[keyBatch] !== undefined) {
          newBatches[i] = String(parsed[keyBatch] || "").trim();
          foundAny = true;
        } else if (parsed[keyB] !== undefined) {
          newBatches[i] = String(parsed[keyB] || "").trim();
          foundAny = true;
        }
      }
      if (Array.isArray(parsed.batchNames)) {
        parsed.batchNames.slice(0, 10).forEach((item, idx) => {
          newBatchNames[idx] = String(item || "").trim();
        });
      }
      if (!foundAny) {
        throw new Error("Struktur JSON tidak dikenali! JSON harus memiliki format batch yang sesuai.");
      }
    }

    this.clearAllBatches();
    const currentState = this.readState();

    return this.saveState({
      ...currentState,
      batches: newBatches,
      batchesEnabled: newBatchesEnabled,
      batchNames: newBatchNames,
      activeBatchIndex: 0,
      instructionString: newBatches[0] || "",
      instructions: this.parseInstructionString(newBatches[0] || ""),
    });
  }

  addInstruction(nameOrLabel, params = []) {
    const alias = this.getAlias(nameOrLabel);
    const cleanParams = this.normalizeParams(params);
    const formatted = this.formatSingleInstruction(alias, cleanParams);
    const id = `inst_${Date.now()}_${Math.random().toString(36).substring(2, 6)}`;

    const instructionItem = {
      id,
      alias,
      name: String(nameOrLabel).trim(),
      params: cleanParams,
      formatted,
      timestamp: new Date().toISOString(),
      formattedTime: new Date().toLocaleTimeString(),
    };

    const currentState = this.readState();
    let targetIdx = currentState.activeBatchIndex;
    const batches = [...currentState.batches];

    let tokens = batches[targetIdx]
      ? batches[targetIdx].split(";").map((t) => t.trim()).filter(Boolean)
      : [];

    if (tokens.length >= 12) {
      let foundSlot = -1;
      for (let next = targetIdx + 1; next < 10; next++) {
        const nextTokens = batches[next]
          ? batches[next].split(";").map((t) => t.trim()).filter(Boolean)
          : [];
        if (nextTokens.length < 12) {
          foundSlot = next;
          break;
        }
      }
      if (foundSlot === -1) {
        for (let prev = 0; prev < targetIdx; prev++) {
          const prevTokens = batches[prev]
            ? batches[prev].split(";").map((t) => t.trim()).filter(Boolean)
            : [];
          if (prevTokens.length < 12) {
            foundSlot = prev;
            break;
          }
        }
      }

      if (foundSlot !== -1) {
        targetIdx = foundSlot;
        tokens = batches[targetIdx]
          ? batches[targetIdx].split(";").map((t) => t.trim()).filter(Boolean)
          : [];
      } else {
        return currentState;
      }
    }

    tokens.push(formatted);
    batches[targetIdx] = tokens.join(";");

    const updatedHistory = [instructionItem, ...currentState.history];

    return this.saveState({
      ...currentState,
      batches,
      activeBatchIndex: targetIdx,
      instructionString: batches[targetIdx],
      instructions: this.parseInstructionString(batches[targetIdx]),
      history: updatedHistory,
    });
  }

  getInstructionString() {
    return this.readState().instructionString;
  }

  setInstructionString(rawString) {
    const currentState = this.readState();
    return this.setBatchInstructionString(currentState.activeBatchIndex, rawString);
  }

  getInstructions() {
    return this.readState().instructions;
  }

  getHistory() {
    return this.readState().history;
  }

  clearHistory() {
    const currentState = this.readState();
    return this.saveState({
      ...currentState,
      history: [],
    });
  }

  subscribe(listener) {
    this.listeners.add(listener);
    return () => this.listeners.delete(listener);
  }
}

export default new RemoteDebugService();
