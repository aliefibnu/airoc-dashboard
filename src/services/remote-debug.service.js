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
        return {
          instructionString: parsed.instructionString || "",
          instructions: Array.isArray(parsed.instructions) ? parsed.instructions : [],
          history: Array.isArray(parsed.history) ? parsed.history : [],
          lastUpdated: parsed.lastUpdated || null,
        };
      }
    } catch (e) {
      console.error(e);
    }
    return {
      instructionString: "",
      instructions: [],
      history: [],
      lastUpdated: null,
    };
  }

  saveState(state) {
    try {
      const normalizedState = {
        instructionString: state.instructionString || "",
        instructions: state.instructions || [],
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

  updateState(updater) {
    const current = this.readState();
    const updated = typeof updater === "function" ? updater(current) : { ...current, ...updater };
    return this.saveState(updated);
  }

  clearState() {
    const emptyState = {
      instructionString: "",
      instructions: [],
      history: [],
      lastUpdated: new Date().toISOString(),
    };
    return this.saveState(emptyState);
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
    const updatedInstructions = [...currentState.instructions, instructionItem];
    const updatedHistory = [instructionItem, ...currentState.history];
    const updatedString = this.formatInstructions(updatedInstructions);

    return this.saveState({
      instructionString: updatedString,
      instructions: updatedInstructions,
      history: updatedHistory,
    });
  }

  updateInstruction(index, newParams) {
    const currentState = this.readState();
    if (index < 0 || index >= currentState.instructions.length) {
      return currentState;
    }

    const target = { ...currentState.instructions[index] };
    const cleanParams = this.normalizeParams(newParams);
    target.params = cleanParams;
    target.formatted = this.formatSingleInstruction(target.alias, cleanParams);

    const updatedInstructions = [...currentState.instructions];
    updatedInstructions[index] = target;
    const updatedString = this.formatInstructions(updatedInstructions);

    return this.saveState({
      ...currentState,
      instructions: updatedInstructions,
      instructionString: updatedString,
    });
  }

  removeInstruction(index) {
    const currentState = this.readState();
    if (index < 0 || index >= currentState.instructions.length) {
      return currentState;
    }

    const updatedInstructions = currentState.instructions.filter((_, i) => i !== index);
    const updatedString = this.formatInstructions(updatedInstructions);

    return this.saveState({
      ...currentState,
      instructions: updatedInstructions,
      instructionString: updatedString,
    });
  }

  undo() {
    const currentState = this.readState();
    if (currentState.instructions.length === 0) {
      return currentState;
    }
    return this.removeInstruction(currentState.instructions.length - 1);
  }

  clearInstructions() {
    const currentState = this.readState();
    return this.saveState({
      ...currentState,
      instructions: [],
      instructionString: "",
    });
  }

  getInstructionString() {
    return this.readState().instructionString;
  }

  setInstructionString(rawString) {
    const cleanStr = String(rawString || "").trim();
    const parsedInstructions = this.parseInstructionString(cleanStr);
    const currentState = this.readState();

    return this.saveState({
      ...currentState,
      instructions: parsedInstructions,
      instructionString: cleanStr,
    });
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

  getPayloadBuffer() {
    const str = this.getInstructionString();
    return new TextEncoder().encode(str);
  }

  subscribe(listener) {
    this.listeners.add(listener);
    return () => this.listeners.delete(listener);
  }
}

export default new RemoteDebugService();