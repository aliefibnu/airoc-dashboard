# AI Coding Agent Directives

## CRITICAL RULE: NO COMMENTS IN CODE
All AI coding agents working on this project (including Claude, Gemini, Antigravity, Cursor, Copilot, ChatGPT, and any other agents) MUST strictly adhere to the following rules:

1. **NEVER WRITE ANY COMMENTS**:
   - Do NOT add single-line comments (`//`).
   - Do NOT add multi-line comments (`/* ... */`).
   - Do NOT add HTML / template comments (`<!-- ... -->`).
   - Do NOT add docstrings or header comments.
   - Do NOT add explanatory, TODO, or temporary comments.

2. **APPLIES TO ALL LANGUAGES & FILE TYPES**:
   - JavaScript / TypeScript (`.js`, `.ts`, `.mjs`, `.cjs`)
   - Vue single-file components (`.vue`)
   - C / C++ / Arduino firmware (`.cpp`, `.h`, `.ino`)
   - HTML / CSS / Tailwind (`.html`, `.css`, `.scss`)
   - Configuration files (`vite.config.js`, `tailwind.config.js`, etc.)

3. **CLEAN CODE ONLY**:
   - Write clear, self-documenting code with meaningful variable and function names instead of comments.
   - When editing or refactoring code, always remove any existing comments encountered in edited areas.
