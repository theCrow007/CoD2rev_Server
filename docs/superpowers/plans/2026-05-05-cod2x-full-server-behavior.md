# CoD2x Full Server Behavior Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add full server-side CoD2x behavior to `CoD2rev_Server` x64 with an explicit optional/required client compatibility policy.

**Architecture:** Port CoD2x behavior as native `src/libcod` modules and thin server lifecycle hooks. Keep existing libcod APIs intact, add CoD2x aliases and features beside them, and use `sv_cod2xClientMode` to choose whether vanilla 1.3 clients are accepted.

**Tech Stack:** C/C++17-compatible server code, existing GNU Make build, Mongoose/OpenSSL for async HTTP/WebSocket, existing CoD2 script VM helpers.

---

## File Structure

- Modify `Makefile` to compile new CoD2x support modules and link TLS/network libraries where available.
- Modify `src/libcod/gsc.cpp` and `src/libcod/gsc.hpp` to register CoD2x GSC functions and lifecycle entry points.
- Modify `src/libcod/gsc_player.cpp` and `src/libcod/gsc_player.hpp` to add CoD2x player identity/view aliases.
- Create `src/libcod/cod2x_state.hpp` and `src/libcod/cod2x_state.cpp` for per-client CoD2x protocol/HWID state.
- Create `src/libcod/gsc_http.hpp` and `src/libcod/gsc_http.cpp` for `http_fetch`.
- Create `src/libcod/gsc_websocket.hpp` and `src/libcod/gsc_websocket.cpp` for WebSocket GSC functions.
- Create `src/libcod/http_client.hpp` and port Mongoose sources under `src/libcod/mongoose/`.
- Create `src/libcod/match.hpp`, `src/libcod/match.cpp`, `src/libcod/gsc_match.hpp`, and `src/libcod/gsc_match.cpp` for match behavior.
- Modify `src/server/sv_client_mp.cpp`, `src/server/sv_init_mp.cpp`, `src/server/sv_main_mp.cpp`, and shutdown/map-change files as needed for lifecycle hooks.
- Add focused tests or compile-time checks where practical under `test/` for pure helper logic.

## Tasks

### Task 1: CoD2x Build And State Foundation

- [ ] Add `test/cod2x_state_fnv1a.cpp` for HWID hash and validation.
- [ ] Run it once before implementation and verify failure.
- [ ] Add `src/libcod/cod2x_state.hpp`.
- [ ] Add `src/libcod/cod2x_state.cpp`.
- [ ] Register `sv_cod2xClientMode` and `g_cod2x` from `RegisterLibcodDvars`.
- [ ] Run the helper test.
- [ ] Run `make ARCH=x64`.

### Task 2: Connection Policy And Identity Capture

- [ ] Parse `protocol_cod2x` and `cl_hwid2` in `SV_DirectConnect`.
- [ ] Reject invalid CoD2x clients when `sv_cod2xClientMode` is required.
- [ ] Preserve vanilla clients when optional mode is active.
- [ ] Use CoD2x HWID hash for GUID identity when CoD2x state is valid.
- [ ] Clear CoD2x state when a client disconnects or slot is reused.
- [ ] Verify optional and required connect paths.

### Task 3: Player GSC Compatibility

- [ ] Register CoD2x-cased player aliases.
- [ ] Implement `getHWID`.
- [ ] Implement `getCDKeyHash`.
- [ ] Implement `getAuthorizationStatus`.
- [ ] Implement `getViewOrigin`.
- [ ] Add `getStance` and `isUsingTurret` aliases.
- [ ] Build x64.

### Task 4: HTTP And WebSocket Runtime

- [ ] Port Mongoose and `HttpClient`.
- [ ] Register `http_fetch` and verify unresolved symbol before implementation.
- [ ] Implement `gsc_http_fetch`.
- [ ] Wire frame polling and shutdown cleanup.
- [ ] Implement WebSocket GSC functions.
- [ ] Verify against a local HTTP endpoint.

### Task 5: Match System

- [ ] Port match data structures.
- [ ] Register match GSC functions and methods.
- [ ] Implement local match data functions.
- [ ] Implement HTTP-backed upload/redownload.
- [ ] Implement player match methods.
- [ ] Wire lifecycle callbacks.
- [ ] Test with latest zPAM.

### Task 6: Server Fixes And Integration Verification

- [ ] Port required map/shutdown/game-type lifecycle fixes.
- [ ] Build x64.
- [ ] Test required mode with latest zPAM.
- [ ] Test optional mode with old zPAM 3.3.6.
- [ ] Run final x64 regression build.
