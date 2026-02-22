# BCGlobalDefines Library - Common Library for the WiFiBinaryClock Project

[![GitHub release](https://img.shields.io/github/release/Chris-70/WiFiBinaryClock.svg?style=flat-square)]
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?style=flat-square)]

**Shared interfaces, structures, and definitions for the WiFi Binary Clock project**

This library serves as the foundation for the [WiFiBinaryClock project][WiFiBinaryClock], providing common interfaces, data structures, compile-time definitions, and utility templates used across all other project libraries. It ensures consistent architecture, type safety, and code reusability throughout the entire system.

## Additional Documents
- [**`ClassDiagram.md`**][ClassDiagram] (GitHub: [`ClassDiagram.md`][ClassDiagram_Git]) - Class diagram and structure of the **BCGlobalDefines library**.
- [**`InstallUsage.md`**][InstallUsage] (GitHub: [`InstallUsage.md`][InstallUsage_Git]) - Installation and usage instructions for the **BCGlobalDefines library**.
---

## Overview

**`BCGlobalDefines`** is a header-only library that acts as the central repository for shared components in the Binary Clock ecosystem. It defines the contracts (interfaces) that other libraries implement, establishes common data structures for inter-library communication, configures board-specific settings through compile-time definitions, and provides utility templates for advanced features like FreeRTOS task management.

This library is designed to be platform-agnostic where possible, with conditional compilation directives to adapt to different boards (Arduino UNO R3, UNO R4, ESP32 variants) and capabilities (WiFi, FreeRTOS, STL support).

## Key Components

### Interface Classes
Abstract base classes defining the contracts for Binary Clock functionality:
- **`IBinaryClock`** - Extended interface for full Binary Clock functionality
- **`IBinaryClockBase`** - Core interface defining basic clock operations
- **`IBCButtonBase`** - Interface for button handling and events

These interfaces enable dependency inversion and loose coupling between libraries.

### Data Structures
Common structures shared across the project:
- **`alarmTime`** - Complete alarm configuration (time, melody, repeat frequency, status)
- **`WiFiCredentials`** - WiFi network credentials (SSID, password, security)
- **`NTPSettings`** - NTP server configuration and sync intervals
- **`LedPattern`** - Enumeration of LED display patterns
- **`Note`** - Musical note definition (frequency, duration) for melody playback
- **Various Enums** - Shared enumerations for modes, states, and options

### Compile-Time Definitions
System-wide configuration through preprocessor definitions:
- **Board Selection** - Automatic detection and configuration for supported boards
- **Pin Definitions** - Hardware pin mappings for LEDs, buttons, buzzer, RTC
- **Feature Flags** - Enable/disable features based on board capabilities (WiFi, FreeRTOS, STL)
- **Serial Output Control** - Debugging and development output macros
- **Common Constants** - Timing values, buffer sizes, default settings

### Utility Templates
Generic template classes and functions for advanced features:
- **`TaskWrapper`** - FreeRTOS task creation wrapper for instance methods
- **`TaskGroupBits`** - FreeRTOS event group bit management utilities
- **Method Templates** - Function pointer wrapping for callbacks

## Architecture

```
BCGlobalDefines (Foundation Layer)
        │
        ├─── Interfaces (Abstract Contracts)
        │    ├─── IBinaryClockBase.h
        │    ├─── IBinaryClock.h
        │    └─── IBCButtonBase.h
        │
        ├─── Structures (Shared Data Types)
        │    └─── BinaryClock.Structs.h
        │
        ├─── Definitions (Configuration & Macros)
        │    ├─── BinaryClock.Defines.h
        │    └─── SerialOutput.Defines.h
        │
        └─── Utilities (Templates & Helpers)
             ├─── TaskWrapper.h
             └─── TaskGroupBits.h
```

### Design Philosophy

1. **Single Source of Truth** - All shared definitions in one place
2. **Conditional Compilation** - Code adapts to board capabilities at compile time
3. **Zero Runtime Cost** - Header-only with extensive use of macros and templates
4. **Type Safety** - Strong typing through interfaces and structures
5. **Maintainability** - Centralized changes propagate to all dependent libraries

### Board Support

The library automatically configures itself based on the target board:

| Board | WiFi | FreeRTOS | STL | Melody Registry |
|-------|------|----------|-----|-----------------|
| Arduino UNO R3 | ✗ | ✗ | ✗ | Limited |
| Arduino UNO R4 | ✗ | ✓ | ✓ | Full |
| ESP32 (all variants) | ✓ | ✓ | ✓ | Full |

Board selection is automatic when using standard Arduino board definitions, or can be customized via [`board_select.h`][boardselect].

## Dependencies

This library has minimal external dependencies:

- **Arduino Core** - Platform-specific Arduino framework
- **RTClibPlus** - Required for `DateTime` and `TimeSpan` classes
- **Streaming** - Optional, for enhanced serial output (janelia-arduino/Streaming)
- **WiFi Libraries** - Conditional on board type:
  - ESP32: `WiFi.h`
  - Arduino UNO R4 WiFi: `WiFiS3.h`

## Installation and Usage

📚 For detailed installation instructions, quick start guide, and usage examples, see [**`InstallUsage.md`**][InstallUsage]

The usage guide includes:
- PlatformIO and Arduino IDE installation instructions
- Quick start for using interfaces and structures
- Configuration through [`board_select.h`][boardselect]
- Advanced topics (custom boards, feature flags, debugging)
- API reference for all components

## Quick Start

Since this is a foundational library used as a dependency, you typically won't interact with it directly. It's automatically included by other libraries in the project.

However, if you're developing your own extensions or using the interfaces:

```cpp
#include <BinaryClock.Defines.h>    // System-wide definitions and macros
#include <BinaryClock.Structs.h>    // Common data structures
#include <IBinaryClock.h>           // Interface for BinaryClock functionality

using namespace BinaryClockShield;

// Example: Using the interface in your code
void updateClock(IBinaryClock* clock) {
    DateTime now = clock->getCurrentTime();
    clock->DisplayLedPattern(LedPattern::Time, 5000);
}

// Example: Creating an alarm structure
alarmTime myAlarm;
myAlarm.number = 1;
myAlarm.time = DateTime(2026, 2, 16, 7, 30, 0);  // 7:30 AM
myAlarm.melody = 0;  // Default melody
myAlarm.status = 1;  // Active
myAlarm.freq = alarmTime::Repeat::Daily;
```

## Version History

- **v0.9.4** (2026) - Current release
  - Added TaskWrapper and TaskGroupBits utilities for FreeRTOS
  - Enhanced interface hierarchy with IBinaryClock
  - Extended structures for WiFi credentials and NTP settings
  - Comprehensive board support with conditional compilation
  - Serial output control macros for debugging

## Authors

- **Chris-70** (2025, 2026)
- **Chris-80** (2025)

## License

This library is part of the WiFiBinaryClock project and is licensed under the **GNU General Public License v3.0**. See the [LICENSE][license] file in the project root for details.

## Links

- [WiFiBinaryClock Project][WiFiBinaryClock]
- [BCGlobalDefines Library][BCGlobalDefines]
- [Installation and Usage Guide][InstallUsage]
- [Class Diagram][ClassDiagram]
- [BinaryClock Library][BinaryClock]
- [BinaryClockWiFi Library][BinaryClockWiFi]
- [MorseCodeLED Library][MorseCodeLED]
- [RTClibPlus Library][RTClibPlus]

---

*For questions, issues, or contributions, please visit the [project repository][WiFiBinaryClock].*

<!-- Reference Links -->
[BCGlobalDefines]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines
[BinaryClock]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock
[BinaryClockWiFi]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi
[boardselect]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/board_select.h
[ClassDiagram]: ClassDiagram.md
[ClassDiagram_Git]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BCGlobalDefines/ClassDiagram.md
[InstallUsage]: InstallUsage.md
[InstallUsage_Git]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BCGlobalDefines/InstallUsage.md
[license]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/LICENSE
[MorseCodeLED]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/MorseCodeLED
[RTClibPlus]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/RTClibPlus
[WiFiBinaryClock]: https://github.com/Chris-70/WiFiBinaryClock
