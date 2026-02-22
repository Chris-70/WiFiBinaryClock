# BinaryClockWiFi Library - WiFi Connectivity and SNTP Time Sync for The Binary Clock

[![GitHub release](https://img.shields.io/github/release/Chris-70/WiFiBinaryClock.svg?style=flat-square)]
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?style=flat-square)]

A comprehensive WiFi connectivity and network time synchronization library for ESP32-based [WiFi Binary Clock][WiFiBinaryClock] project. Provides integrated WiFi management, WPS setup, SNTP time synchronization, and persistent credential storage. The ESP32-based WiFi UNO style boards allow the [Binary Clock Shield for Arduino][shield] to connect to WiFi networks and synchronize time automatically including Daylight Savings adjustment.

## Additional Documents
- [**`ClassDiagram.md`**][CLASS_DIAGRAM] (GitHub: [`ClassDiagram.md`][CLASS_DIAGRAM_GIT]) - Class diagram and structure of the **BinaryClockWiFi library**.
- [**`InstallUsage.md`**][INSTALL] (GitHub: [`InstallUsage.md`][INSTALL_Git]) - Installation and usage instructions for the **BinaryClockWiFi library**.
---

## Overview

**`BinaryClockWiFi`** is a complete networking solution developed for the [WiFi Binary Clock][WiFiBinaryClock] project. It combines four singleton classes that work together to provide robust WiFi connectivity, automatic time synchronization, and easy credential management. The library is designed to be modular and extensible, allowing it to be used in other ESP32 projects that require WiFi and SNTP functionality. This library is currently requires an implementation instance of the [`IBinaryClock`][IBinaryClock] interface to coordinate WiFi operations with the clock's state and display.

`BinaryClockWAN::Begin()` is the main entry point to initialize the WiFi system. It first initializes the [`IBinaryClockSettings`][settings] interface to load stored WiFi credentials and settings. Then it scans for available networks and can automatically connect to available networks using the stored credentials if a match is found. If no credentials are available, it can use [`BinaryClockWPS`][BinaryClockWPS] for push-button setup. Once connected, [`BinaryClockNTP`][BinaryClockNTP] handles time synchronization with configurable NTP servers and timezone support. All WiFi credentials and settings are stored persistently in ESP32 NVS using [`BinaryClockSettings`][BinaryClockSettings] after successful connections are made.

### Key Features

- ✅ **Automatic WiFi Connection**: Scans for available networks and connects to stored credentials
- ✅ **WPS Support**: Push-button WiFi setup without entering passwords
- ✅ **SNTP Time Sync**: Automatic time synchronization with configurable NTP servers
- ✅ **Persistent Storage**: WiFi credentials saved in ESP32 NVS (Non-Volatile Storage)
- ✅ **Multiple AP Support**: Store and manage multiple access point credentials
- ✅ **Timezone Management**: Full timezone support with automatic DST adjustment
- ✅ **Event Integration**: FreeRTOS EventGroup support for task coordination
- ✅ **Callback System**: Asynchronous notifications for connection and sync events
- ✅ **Singleton Pattern**: Thread-safe, single-instance design

## Architecture

The library consists of four main singleton classes:

```
BinaryClockWAN (Coordinator)
    ├── BinaryClockSettings (NVS Storage)
    ├── BinaryClockNTP (Time Sync)
    └── BinaryClockWPS (WPS Setup)
```

### Class Responsibilities

| Class | Purpose | Key Features |
|-------|---------|--------------|
| **BinaryClockWAN** | Central WiFi manager | Connection coordination, network scanning, time updates |
| **BinaryClockNTP** | SNTP time sync | Multiple NTP servers, timezone support, periodic sync |
| **BinaryClockWPS** | WPS connection | Push-button setup, credential retrieval |
| **BinaryClockSettings** | Credential storage | NVS persistence, multiple APs, timezone storage |

## Installation and Usage

For complete installation instructions, API reference, usage examples, and troubleshooting, see:

**📚 [BinaryClockWiFi Installation & Usage Guide](InstallUsage.md)**

The guide includes:
- Installation instructions for PlatformIO and Arduino IDE
- Quick start examples
- Complete API reference for all four classes
- Advanced usage patterns
- Data structures
- Design patterns
- Best practices
- Troubleshooting
- Performance considerations

## Quick Start

Here's a minimal example to get started. For complete examples and detailed documentation, see the [Installation & Usage Guide (`InstallUsage.md`)](InstallUsage.md).

```cpp
#include <BinaryClock.h>
#include <BinaryClockWAN.h>

BinaryClock& clock = BinaryClock::get_Instance();
BinaryClockWAN& wan = BinaryClockWAN::get_Instance();

void setup() {
    Serial.begin(115200);
    clock.setup();
    
    // Begin WiFi with auto-connect
    if (wan.Begin(clock, true)) {
        Serial.println("WiFi connected!");
        Serial.println(wan.get_LocalIP());
    }
}

void loop() {
    clock.loop();
}
```

## Version History

- **v0.9.4** - Current version
  - Full WiFi, WPS, SNTP, and Settings integration
  - FreeRTOS EventGroup support
  - Multiple NTP server support
  - Persistent NVS storage
  - Timezone management

## Authors

**Chris-70** (2025, 2026)  
**Chris-80** (2025)

Created for the WiFi Binary Clock Shield project.

## License

Part of the WiFi Binary Clock project. See project repository for license details.

## Links

- **WiFi Binary Clock Project**: https://github.com/Chris-70/WiFiBinaryClock
- **BinaryClockWiFi Library**: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi
- **Installation & Usage Guide**: [InstallUsage.md](InstallUsage.md)
- **Class Diagram**: [ClassDiagram.md](ClassDiagram.md)
- **Dependencies**:
  - [BCGlobalDefines](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines)
  - [BinaryClock](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock)
  - [RTClibPlus](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/RTClibPlus)

---

**Stay connected. Stay synchronized. ⏰📡**

<!-- Reference Links -->
[BinaryClockNTP]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi/src/BinaryClockNTP.h
[BinaryClockSettings]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi/src/BinaryClockSettings.h
[BinaryClockWAN]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi/src/BinaryClockWAN.h
[BinaryClockWPS]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi/src/BinaryClockWPS.h
[CLASS_DIAGRAM]: ClassDiagram.md
[CLASS_DIAGRAM_GIT]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClockWiFi/ClassDiagram.md
[IBinaryClock]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines/src/IBinaryClock.h
[INSTALL]: InstallUsage.md
[INSTALL_Git]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClockWiFi/InstallUsage.md
[settings]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi/src/BinaryClockSettings.h
[shield]: https://nixietester.com/product/binary-clock-shield-for-arduino/
[WiFiBinaryClock]: https://github.com/Chris-70/WiFiBinaryClock
