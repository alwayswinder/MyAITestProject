# Copilot Instructions — MyAITestProject

## Project Overview

An **Unreal Engine 5.7** project that serves as a testbed for AI-to-editor automation via the **Model Context Protocol (MCP)**. The project hosts three plugins that bridge AI assistants (Claude, Cursor, etc.) to the Unreal Editor:

| Plugin | Transport | Purpose |
|--------|-----------|---------|
| `McpAutomationBridge` | WebSocket (port 8091) | 200+ C++ automation handlers |
| `SpecialAgentPlugin` | HTTP/SSE (port 8767) | Python execution + 71+ level-design tools |
| `AIBpAnalyze` | (editor plugin) | Blueprint/Material analysis workflow scripts |

## Build

This is a standard UE5 C++ project. There is no separate build script — use Unreal's native tooling:

- **Regenerate project files**: Right-click `MyAITestProject.uproject` → *Generate Visual Studio project files*
- **Build**: Open `MyAITestProject.sln` in Visual Studio and build the `Development Editor` configuration
- **Clean build** (required after plugin additions or persistent errors):
  ```
  Delete: Binaries/, Intermediate/, Saved/
  Then regenerate project files and rebuild
  ```

The main game module uses `PCHUsage = UseExplicitOrSharedPCHs`.  
`McpAutomationBridge` uses `PCHUsageMode.NoPCHs` to prevent heap exhaustion (`C1060`/`C3859`) caused by its 50+ handler files.

## Architecture

### McpAutomationBridge

Editor-only `UEditorSubsystem` that listens for WebSocket messages and dispatches automation requests on the game thread.

```
WebSocket frame
  → McpBridgeWebSocket
  → McpConnectionManager (rate limiting, handshake)
  → McpAutomationBridgeSubsystem::Tick (game thread dispatch)
  → McpAutomationBridge_ProcessRequest.cpp (action routing)
  → *Handlers.cpp (56 domain-specific handler files)
```

- **Handler registration**: Each handler is declared in `McpAutomationBridgeSubsystem.h` and registered in `InitializeHandlers()` inside `McpAutomationBridgeSubsystem.cpp`.
- **Cross-version compat**: All UE 5.0–5.7 API differences are abstracted in `McpAutomationBridgeHelpers.h` using compile-time macros (e.g., `MCP_GET_MATERIAL_EXPRESSIONS`, `MCP_DATALAYER_TYPE`).

### SpecialAgentPlugin

Editor-only plugin using native HTTP/SSE (JSON-RPC 2.0). No Node.js dependency.

```
MCP Client → HTTP/SSE → MCPServer → MCPRequestRouter
  → GameThreadDispatcher → UE5 Python API or service handlers
```

Services live under `Source/SpecialAgent/Public/Services/`. Configuration is in `Config/DefaultSpecialAgent.ini`.

### AIBpAnalyze

Lightweight editor plugin with toolbar commands. The `ai-tools/` directory contains reusable prompt scripts (`.md` files) and batch launchers (`.bat`) for AI-driven Blueprint and Material analysis tasks. Output goes to `ai-tools/ai-out/`.

## Key Conventions (McpAutomationBridge)

### Asset Saving
**Never** use `UPackage::SavePackage()` or `UEditorAssetLibrary::SaveAsset()` (the latter triggers modal dialogs that crash D3D12).  
Always use the safe helper:
```cpp
McpSafeAssetSave(Asset);   // defined in McpAutomationBridgeHelpers.h
McpSafeLevelSave(World);   // for level saves (prevents Intel GPU crash)
```

### Object Lookups
`ANY_PACKAGE` is deprecated in UE 5.1+. Use `nullptr` for path-based lookups:
```cpp
FindObject<UClass>(nullptr, *Path);
```

### SCS Components (UE 5.7+)
Component templates must be owned by the `SCS_Node`, not the Blueprint CDO:
```cpp
USCS_Node* Node = SCS->CreateNode(ComponentClass, NodeName);
// Do NOT attach templates directly to the Blueprint object
```

### Path Security
All incoming asset paths must go through validation helpers before use:
```cpp
SanitizeProjectRelativePath(InPath);   // blocks traversal attacks
SanitizeProjectFilePath(InPath);
ValidateAssetCreationPath(InPath);
```

### Thread Safety
- WebSocket frame processing must never block the game thread.
- Handlers are automatically dispatched to the game thread by the subsystem's ticker — handler code may safely call editor/engine APIs without additional `AsyncTask` wrapping.

### JSON
Use `FJsonObjectConverter` for struct ↔ JSON serialization (UE standard).

## Adding a New Handler (McpAutomationBridge)

1. Add handler method declaration to `McpAutomationBridgeSubsystem.h`
2. Implement in an appropriate `*Handlers.cpp` (or create a new one)
3. Register the action string in `InitializeHandlers()` in `McpAutomationBridgeSubsystem.cpp`

## MCP Server Quick Reference

| Server | Config | Health Check |
|--------|--------|--------------|
| McpAutomationBridge | Edit → Project Settings → MCP Automation Bridge | WebSocket port 8091 |
| SpecialAgent | `Plugins/SpecialAgentPlugin-main/Config/DefaultSpecialAgent.ini` | `curl http://localhost:8767/health` |

Both servers start automatically when the editor opens (disabled during commandlet/cook runs).
