import { INSTRUCTION_ALIASES } from "@/data/instruction-aliases.js";

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
      activeBatchIndex: 0,
      instructionString: "",
      instructions: [],
      history: [],
      lastUpdated: null,
    };
  }

  saveState(state) {
    try {
      const batches =
        Array.isArray(state.batches) && state.batches.length === 10
          ? state.batches
          : Array(10).fill("");
      const activeBatchIndex =
        typeof state.activeBatchIndex === "number" &&
        state.activeBatchIndex >= 0 &&
        state.activeBatchIndex < 10
          ? state.activeBatchIndex
          : 0;
      const currentString = batches[activeBatchIndex] || "";
      const instructions = state.instructions || this.parseInstructionString(currentString);

      const normalizedState = {
        batches,
        activeBatchIndex,
        instructionString: currentString,
        instructions,
        history: state.history || [],
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
