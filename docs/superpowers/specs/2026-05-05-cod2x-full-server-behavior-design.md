# CoD2x Full Server Behavior Design

## Goal

Port full server-side CoD2x behavior into `CoD2rev_Server` x64 while preserving an explicit compatibility mode for vanilla 1.3 clients.

## Client Policy

Add a server dvar:

```text
sv_cod2xClientMode 0
```

`0` means optional mode. Vanilla 1.3 clients and CoD2x clients can connect. CoD2x-aware clients get full identity and match behavior. Vanilla clients are marked as non-CoD2x and identity-dependent APIs return empty or undefined values safely.

`1` means required mode. The server enforces upstream CoD2x behavior: clients must send a compatible `protocol_cod2x` value and a valid 32-character `cl_hwid2`. Missing or old CoD2x clients are rejected during connection.

Do not infer this policy from `PRODUCT_VERSION`. That value describes the binary build, not the active mod or desired server admission policy.

## GSC Surface

Expose the CoD2x GSC functions and methods required by current zPAM:

Global functions:

- `http_fetch`
- `websocket_connect`
- `websocket_sendText`
- `websocket_close`
- `matchUploadData`
- `matchSetData`
- `matchGetData`
- `matchRedownloadData`
- `matchClearData`
- `matchIsActivated`
- `matchCancel`
- `matchFinish`

Player methods:

- `getIp`
- `getHWID`
- `getCDKeyHash`
- `getAuthorizationStatus`
- `getViewOrigin`
- `getStance`
- `isUsingTurret`
- `matchPlayerGetData`
- `matchPlayerSetData`
- `matchPlayerIsAllowed`

Keep existing libcod lowercase names and add CoD2x casing aliases where needed.

## Identity

Store per-client CoD2x state in native server data:

- Whether the client sent `protocol_cod2x`.
- The CoD2x protocol value.
- The raw `cl_hwid2`.
- A 32-bit HWID hash matching upstream CoD2x FNV-1a behavior.
- CD-key hash and authorization status using the existing challenge/client fields.

In required mode, the HWID hash becomes the ban/guid identity as in CoD2x. In optional mode, vanilla clients keep existing identity behavior.

## HTTP And WebSocket

Port the Mongoose-backed CoD2x async HTTP and WebSocket clients into `src/libcod`.

The server must poll these clients once per frame and cleanly drain or close them on shutdown and map changes. GSC callbacks must only execute when the script VM is active.

## Match System

Port the CoD2x match data model and JSON upload/download behavior. Match state must survive ordinary frame polling and be cleared or uploaded according to the same map restart/shutdown hooks CoD2x uses.

Player matching uses `match_login` from userinfo. If no matching UUID is found, it falls back to GUID or name for progress data keys, matching upstream behavior.

## Server Lifecycle

Integrate the lifecycle points needed by CoD2x:

- Startup dvar registration and library initialization.
- Per-frame polling for HTTP, WebSocket, match, and rate limiter stats.
- Player connect handling.
- Game type start/stop callbacks.
- Map change, map restart, fast restart, map rotate, and shutdown interception.
- `g_cod2x` systeminfo propagation for CoD2x clients.

## Verification

Build the x64 server and run two integration profiles:

- Required mode with latest zPAM and CoD2x clients.
- Optional mode with old zPAM 3.3.6 and vanilla 1.3 clients.

The server should compile without warnings introduced by the port, expose the expected GSC names, and preserve existing libcod behavior.
