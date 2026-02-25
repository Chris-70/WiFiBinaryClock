# **WiFiBinaryClock** - Release Notes 

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/Chris-70/WiFiBinaryClock?style=flat-square)]
[![GitHub license](https://img.shields.io/github/license/Chris-70/WiFiBinaryClock?style=flat-square)]
[![GitHub issues](https://img.shields.io/github/issues/Chris-70/WiFiBinaryClock?style=flat-square)]
[![GitHub pull requests](https://img.shields.io/github/issues-pr/Chris-70/WiFiBinaryClock?style=flat-square)]
[![GitHub last commit](https://img.shields.io/github/last-commit/Chris-70/WiFiBinaryClock?style=flat-square)]
[![GitHub repo size](https://img.shields.io/github/repo-size/Chris-70/WiFiBinaryClock?style=flat-square)]

---

# *Software to Supercharge The Binary Clock Shield for Arduino*

## **Version: v0.9.4**  (2026-02-23)  
[Full Release Notes for v0.9.4][Release_0_9_4] (Git: [`Release_0.9.4.md`][Release_0_9_4_Git]) 

Version 0.9.4 is a **critical bug fix and documentation release** that resolves production-breaking issues on ESP32 and Arduino UNO R3 platforms while adding comprehensive documentation for all five project libraries.

### Critical Fixes

**ESP32 NTP Crash - FIXED** ⚠️
- **Issue:** Guru Meditation Error (LoadProhibited exception) during SNTP initialization on all ESP32 boards
- **Impact:** Complete WiFi/NTP functionality failure - boards would crash immediately after attempting time sync
- **Status:** ✅ Resolved - ESP32 boards now stable and production-ready

**Arduino UNO R3 Build Failure - FIXED** ⚠️  
- **Issue:** Compilation failure due to flash memory overflow (102.4% usage, 790 bytes over limit)
- **Impact:** UNO R3 completely unusable - builds would fail
- **Status:** ✅ Resolved - Flash optimized to 98.5% usage with 488 bytes headroom

### New in v0.9.4

- ✅ **Stable ESP32 WiFi/NTP**: Fixed critical crash affecting all ESP32-based boards
- ✅ **UNO R3 Support Restored**: Flash memory optimizations enable successful builds
- ✅ **Comprehensive Documentation**: 5,000+ lines of installation guides, API references, and architecture diagrams
- ✅ **Library Updates**: All 5 libraries updated from v0.8.0 to v0.9.4
- ✅ **Code Quality**: Enhanced task management, improved LED control, better debug output

### Upgrade Recommendation

**Immediate upgrade recommended** if you are using:
- Any ESP32-based board (Metro ESP32-S3, ESP32-S3 UNO, Wemos D1)
- Arduino UNO R3

**No breaking API changes** - fully backward compatible with v0.8.5

---

## **Version: 0.8.5** (2025-12-17) 
[Full Release Notes for v0.8.5][Release_0_8_5] (Git: [`Release_0.8_5.md`][Release_0_8_5_Git])

Version 0.8.5 is a major architecture update focused on modularization, WiFi/NTP capability, and ESP32 task orchestration.
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

---

## **Version: 0.7.1** (2025-09-18)
[Full Release Notes for v0.7.1][Release_0_7_1] (Git: [`Release_0.7_1.md`][Release_0_7_1_Git])

Binary Cock base
This is the base Binary Clock software working on the UNO_R3 (as well as all other supported boards).  
This is a standalone version, no WiFi, and compact enough to fit on the Arduino UNO R3.  
All non-WiFi functionality has been implemented. The Arduino UNO R3 and R4 Minima will remain at this level while development continues with the WiFi and other communications supported by the ESP32 family of chips.

1. Fully supports 12 hour mode with AM/PM indicators (on Hour bit 5), in addition to the standard 24 hour mode.
2. Alarm time setting is displayed in the selected mode.
3. Cancel settings and return is available for both Time and Alarm. This is the first menu item along with 12/24 mode selection for Time or ON/OFF for Alarm.
4. Added feedback during settings:
    - Green checkmark, [✅], is displayed when the settings are saved, or
    - Red X, [❌], is displayed when the changes are aborted.
    - rainbow screen is displayed to indicate the end of the settings.
5. The user can change, through property calls:
  5.1 Individual display colors for:  
        - Hours
        - Minutes
        - Seconds
        - AM and PM indicators.    
  5.2 Users can upload different alarm melodies
6. Users can define their own UNO board in the optional [board_select.h](https://github.com/Chris-70/WiFiBinaryClock/blob/UNO_R3_v0.7.0/lib/BinaryClock/src/board_select.h) file. To use the custom UNO board, just define CUSTOM_UNO true at the start of the file, before the definition section.

- The code is fully commented.
- Doxygen files will be published in a future release.

---

<!-- Reference Links -->
[Release_0_7_1]: History/Release_V0.7.1.md
[Release_0_7_1_Git]: https://github.com/Chris-70/WiFiBinaryClock/releases/tag/v0.7.1
[Release_0_8_5]: History/Release_V0.8.5.md
[Release_0_8_5_Git]: https://github.com/Chris-70/WiFiBinaryClock/releases/tag/v0.8.5
[Release_0_9_4]: History/Release_V0.9.4.md
[Release_0_9_4_Git]: https://github.com/Chris-70/WiFiBinaryClock/releases/tag/v0.9.4
