# Release Notes: BinaryClock_v0.8.5

**Release Date:** December 17, 2025  
---
Note: **The Current Version is:** 
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/Chris-70/WiFiBinaryClock?style=flat-square)]
[![GitHub license](https://img.shields.io/github/license/Chris-70/WiFiBinaryClock?style=flat-square)]

---

## Executive Summary

- This release is a major architecture update focused on modularization, WiFi/NTP capability, and ESP32 task orchestration.
- A new shared library, `lib/BCGlobalDefines`, was added to centralize global types, interfaces, task helpers, and serial-output macros.
- Global headers were split and relocated, including `BinaryClock.Defines.h`, `BinaryClock.Structs.h`, `IBCButtonBase.h`, and `SerialOutput.Defines.h`.
- The base clock interface was renamed/migrated from `IBinaryClock.h` to `IBinaryClockBase.h` and is now consumed by menu and support classes.
- A new WiFi subsystem library, `lib/BinaryClockWiFi`, was introduced with four major classes: `BinaryClockWAN`, `BinaryClockNTP`, `BinaryClockWPS`, and `BinaryClockSettings`.
- `BinaryClockWAN` now manages access-point discovery, connection attempts, and integration with NTP-based time updates.
- WiFi connection flow now attempts stored local credentials first, then falls back to WPS enrollment when local reconnect fails.
- Successful WPS enrollment persists credentials for future reconnects.
- `BinaryClockSettings` adds NVS-backed persistence for AP credentials and timezone settings on ESP32.
- WiFi credential handling now supports SSID+BSSID matching, credential IDs, add/update/delete staging, and explicit save semantics.
- `BinaryClockNTP` adds SNTP initialization, direct NTP sync helpers, timezone handling, callback registration, and sync-status tracking.
- NTP logic now includes dedicated event-bit support (`NTPEventBits`) for task/event-group coordination.
- NTP initialization was moved to an async FreeRTOS task path with explicit callback gating and lifecycle guards.
- `BinaryClockWPS` now includes explicit WPS setup, event handling, timeout/error flow, and cleanup routines.
- The main ESP32 application (`src/BinaryClock_ESP32.cpp`) now coordinates startup using FreeRTOS synchronization primitives.
- Event-group signaling is used to sequence splash completion, WiFi setup, and NTP completion states.
- `TaskWrapper.h` was added to provide generic template wrappers for invoking instance/free methods through `xTaskCreate()`.
- Task wrapper support includes parameter packaging, argument forwarding, cleanup, and self-deleting task entry points.
- The settings/menu class was renamed from `BCSettings` to `BCMenu`, with corresponding source/header rename and interface alignment.
- Core BinaryClock library files (`BinaryClock.h/.cpp`, button/menu files, board selection header) received broad refactoring and integration updates.
- `platformio.ini` was updated to use **GNU++17** (explicitly replacing gnu++11 defaults).
- Build configuration now uses expanded per-environment source filtering to include modular library source trees.
- Non-WiFi board environments now explicitly ignore WiFi-focused libraries/dependencies where appropriate.
- Multiple libraries now include explicit PlatformIO metadata via new `library.json` manifests.
- `lib/BinaryClock/library.json`, `lib/BinaryClockWiFi/library.json`, `lib/BCGlobalDefines/library.json`, and `lib/MorseCodeLED/library.json` were added.
- `lib/RTClibPlus/library.json` was also added, and `library.properties` was revised.
- `RTClibPlus` was significantly restructured by moving `DateTime`/`TimeSpan` into a dedicated `DateTime.h` header.
- `RTClibPlus` now exposes expanded timestamp formatting options and additional 12-hour/AM-PM format support.
- `DateTime` weekday handling was extended via `FIRST_WEEKDAY`-driven offset/epoch logic.
- RTClib-related README documentation was expanded to describe fork behavior and feature differences.
- The standalone `MorseCodeLED` library was updated and now ships with manifest metadata.
- New diagnostics/testing support files were added: `test/Diagnostic_ESP32.cpp`, `test/Diagnostics.h`, and dummy clock test doubles.
- The test scaffold now includes `DummyBinaryClock.cpp/.h` under `test/` for interface-level testing and WAN/NTP integration scenarios.
- Top-level README content was heavily revised and expanded.
- Project assets were updated with a new image and an updated shield image.
- Overall, `v0.8.5` transitions the project from a primarily clock-focused codebase to a modular clock + networked-time platform.
- This release establishes the foundation for persistent WiFi onboarding, asynchronous SNTP synchronization, and more testable component boundaries.

## Diff Metrics

- Tags compared: `BinaryClock_v0.7.1` .. `BinaryClock_v0.8.5`
- Commits in range: 16
- Files changed: 42
- Insertions: 8295
- Deletions: 1742

## Notable Additions (new files/directories)

- `lib/BinaryClockWiFi/src/BinaryClockWAN.*`
- `lib/BinaryClockWiFi/src/BinaryClockNTP.*`
- `lib/BinaryClockWiFi/src/BinaryClockWPS.*`
- `lib/BinaryClockWiFi/src/BinaryClockSettings.*`
- `lib/BCGlobalDefines/src/BinaryClock.Structs.h`
- `lib/BCGlobalDefines/src/IBCButtonBase.h`
- `lib/BCGlobalDefines/src/SerialOutput.Defines.h`
- `lib/BCGlobalDefines/src/TaskWrapper.h`
- `lib/RTClibPlus/src/DateTime.h`
- `test/Diagnostic_ESP32.cpp`
- `test/Diagnostics.h`
- `test/DummyBinaryClock.cpp`
- `test/DummyBinaryClock.h`

## Upgrade Notes / Breaking Changes

- **Interface rename:** `IBinaryClock.h` was replaced by `IBinaryClockBase.h` (now under `lib/BCGlobalDefines/src`).
- **Menu class rename:** `BCSettings` was renamed to `BCMenu` (`BCSettings.cpp/.h` → `BCMenu.cpp/.h`).
- **Global header relocation:** shared headers moved out of `lib/BinaryClock/src` into `lib/BCGlobalDefines/src`.
- **New WiFi module boundary:** WiFi/NTP/WPS/settings logic is now in `lib/BinaryClockWiFi` (`BinaryClockWAN`, `BinaryClockNTP`, `BinaryClockWPS`, `BinaryClockSettings`).
- **RTClib include split:** `DateTime` and `TimeSpan` now live in `lib/RTClibPlus/src/DateTime.h`; projects that only need date/time types can include this directly.
- **Toolchain standard change:** project configuration moved to GNU++17, so older GNU++11-only assumptions should be removed.
- **Build filtering changes:** source inclusion is now environment-specific in `platformio.ini`; board environments may include/ignore different libraries.
- **Non-WiFi targets:** UNO R3 / UNO R4 non-WiFi environments now explicitly ignore WiFi-oriented libraries in build config.
- **Library manifests:** multiple internal libraries now ship with `library.json`; dependency resolution and local library discovery behavior may differ from `v0.7.1`.

### Migration Checklist (from v0.7.1)

1. Replace includes/references from `IBinaryClock` to `IBinaryClockBase` where applicable.
2. Replace `BCSettings` symbol/file references with `BCMenu`.
3. Update include paths for moved global headers (now in `lib/BCGlobalDefines/src`).
4. If using WiFi features, migrate integrations to `BinaryClockWAN` + `BinaryClockSettings` + `BinaryClockNTP` APIs.
5. Confirm your PlatformIO environment inherits GNU++17 and the intended `build_src_filter` / `lib_ignore` behavior.
