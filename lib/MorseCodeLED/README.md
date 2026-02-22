# MorseCodeLED Library

[![GitHub release](https://img.shields.io/github/release/Chris-70/MorseCodeLED.svg?style=flat-square)]
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?style=flat-square)]

A simple yet powerful Arduino library for flashing Morse code messages on an LED. Perfect for error signaling, status messages, or just having fun with Morse code!
___

## Additional Documents
- [**`ClassDiagram.md`**][ClassDiagram] (GitHub: [`ClassDiagram.md`][ClassDiagram_Git]) - Class diagram and structure of the **MorseCodeLED library**.
- [**`InstallUsage.md`**][INSTALL] (GitHub: [`InstallUsage.md`][INSTALL_Git]) - Installation and usage instructions for the **MorseCodeLED library**.
---

## Overview

**MorseCodeLED** provides an easy way to communicate messages through an LED using international Morse code. Originally created for the [WiFi Binary Clock](https://github.com/Chris-70/WiFiBinaryClock) project to signal "CQD NO RTC" errors, it has grown into a full-featured Morse code library supporting text, prosigns, and custom messages.

### Key Features

- ✅ **Standard Morse Code**: Flash letters (A-Z), numbers (0-9), and punctuation
- ✅ **Prosign Support**: 24 Morse prosigns (SOS, CQD, AR, SK, etc.)
- ✅ **Text Messages**: Flash complete strings of text
- ✅ **Custom Messages**: Flash raw Morse code patterns
- ✅ **Efficient Encoding**: 16-bit packed pattern storage
- ✅ **Resource Adaptive**: Scaled-down version for memory-constrained boards (Arduino UNO R3)
- ✅ **Arduino Compatible**: Works with any Arduino-compatible board

## Installation

### PlatformIO

Add to your `platformio.ini`:
```ini
lib_deps = 
    https://github.com/Chris-70/WiFiBinaryClock.git#lib/MorseCodeLED
```

### Arduino IDE

1. Download this library
2. Copy to your Arduino `libraries` folder
3. Restart Arduino IDE

## Quick Start

```cpp
#include <MorseCodeLED.h>

MorseCodeLED morse(LED_BUILTIN);  // Use built-in LED

void setup() {
    morse.Begin();
    morse.FlashString("HELLO WORLD");
}

void loop() {
    // Your code here
}
```

## Usage Examples

### Basic Text Messages

```cpp
MorseCodeLED morse(13);  // Pin 13
morse.Begin();

morse.FlashString("CQD");           // Flash "CQD" word for an error message
morse.FlashString("HELLO WORLD");   // Flash complete phrase
morse.FlashCharacter('A');          // Flash single character
```

### Prosigns (Procedural Signals)

```cpp
morse.FlashProsign(Prosign::Error);      // Error/Correction (HH)
morse.FlashProsign(Prosign::Start);      // Start transmission (KA)
morse.FlashProsign(Prosign::End);        // End message (AR)
morse.FlashProsign(Prosign::SK);         // Out / End of contact

// Flash by keyword
morse.FlashProsignWord("START");         // KA: -.-.-
morse.FlashProsignWord("ERROR");         // HH: ........
morse.FlashProsignWord("OVER");          // K: -.-
```

### Error Message (All Boards)

```cpp
morse.Flash_CQD();  // Flash "CQD" as an error indicator message
```

This predefined message works even on memory-constrained boards like the Arduino UNO R3.

### Raw Morse Code Arrays (All Boards)

```cpp
// Define custom pattern using MC enum
const MC customMessage[] = {
    MC::Dash, MC::Dot, MC::Space,      // N
    MC::Dash, MC::Dash, MC::Dash, MC::Space,  // O
    MC::EndMarker
};

morse.FlashMorseCode(customMessage);
```

## Morse Code Reference

### Timing

The library uses standard Morse code timing:

| Element | Duration | Description |
|---------|----------|-------------|
| **Dot (dit)** | 200 ms | Base unit: **·** |
| **Dash (dah)** | 600 ms | 3× dot duration: **−** |
| **Intra-character gap** | 200 ms | Between dots/dashes in same letter |
| **Inter-character gap** | 600 ms | Between letters (3× dot) |
| **Word space** | 1400 ms | Between words (7× dot) |

### Supported Characters

#### Letters
```
A ·−    B −···  C −·−·  D −··   E ·     F ··−·  G −−·   H ····
I ··    J ·−−−  K −·−   L ·−··  M −−    N −·    O −−−   P ·−−·
Q −−·−  R ·−·   S ···   T −     U ··−   V ···−  W ·−−   X −··−
Y −·−−  Z −−··
```

#### Numbers
```
0 −−−−−    1 ·−−−−    2 ··−−−    3 ···−−    4 ····−
5 ·····    6 −····    7 −−···    8 −−−··    9 −−−−·
```

#### Punctuation (Full-featured boards only)
Period, comma, question mark, apostrophe, exclamation, slash, parentheses, colon, quotes, and more.

### Prosigns

| Prosign | Morse Code | Meaning | Alternate Names |
|---------|------------|---------|-----------------|
| **SOS** | `···−−−···` | Life Emergency distress signal | |
| **CQD** | `−·−· −−·− −··` | Come Quick - Distress (old/non-life-emergency) | |
| **AR** | `·−·−·` | End of message | End |
| **SK** | `···−·−` | End of contact | Out, EndWork |
| **KA** | `−·−·−` | Start of transmission | Start |
| **K** | `−·−` | Invitation to transmit | Over, Invite |
| **R** | `·−·` | Received OK | Roger |
| **VE** | `···−·` | Understood | Verified |
| **HH** | `········` | Error/Correction | |
| **BT** | `−···−` | New paragraph separator | Break |
| **AS** | `·−···` | Wait | |

See [MorseCodeLED.h](src/MorseCodeLED.h) for complete prosign list (24 total).

## API Reference

### Constructor

```cpp
MorseCodeLED(int ledPin)
```
Creates Morse code controller for specified LED pin.

- **ledPin**: GPIO pin number where LED is connected

### Methods

#### Core Methods (All Boards)

```cpp
void Begin()
```
Initialize the LED pin. Call in `setup()` before using.

```cpp
void Flash_CQD_NO_RTC()
```
Flash the predefined "CQD NO RTC" emergency message.

```cpp
void FlashMorseCode(const MC* morseData)
```
Flash raw Morse code component array (dots, dashes, spaces).

#### Extended Methods (Full-featured Boards)

Not available on UNO R3 due to memory constraints.

```cpp
void FlashCharacter(char c)
```
Flash a single character (A-Z, 0-9, punctuation).

```cpp
void FlashString(const String& text)
```
Flash complete text string. Handles spaces between words automatically.

```cpp
void FlashProsign(Prosign sign)
```
Flash a Morse prosign by enum value.

```cpp
void FlashProsignWord(String keyword)
```
Flash a prosign by keyword (e.g., "START", "SOS", "OVER").

Supported keywords:
- "START" / "STARTING" → KA
- "END" / "OK" → AR
- "OUT" / "ENDWORK" → SK
- "OVER" / "INVITE" → K
- "ROGER" → R
- "UNDERSTOOD" → VE
- "ERROR" / "CORRECTION" → HH
- "YES" / "CORRECT" / "CONFIRM" → C
- "NO" / "NEGATIVE" → N
- "SOS" → SOS

## Advanced Usage

### Data Structures

#### MCode - Packed Morse Pattern

```cpp
union MCode {
    uint16_t pattern;        // Full 16-bit value
    struct {
        uint16_t code : 12;  // Pattern bits (0-11)
        uint16_t len  :  4;  // Length (12-15)
    };
};
```

Efficiently stores Morse patterns in 16 bits:
- **12 bits** for pattern (max 12 elements)
- **4 bits** for length (0-12)

**Example:**
```cpp
MCode letterA(2, 0b01);  // A = ·− = length 2, pattern 01
// Bit 0 (rightmost) = first element: 1 = dash, 0 = dot
```

#### MC Enum - Morse Components

```cpp
enum class MC : uint8_t {
    Dot = 0,        // · (dit)
    Dash = 1,       // − (dah)
    Space = 2,      // Letter separation
    Word = 3,       // Word separation
    EndMarker = 255 // End of sequence
};
```

### Board-Specific Features

#### Full-Featured Boards
**ESP32, Arduino R4, and other boards with sufficient memory**

- Complete character set (A-Z, 0-9, punctuation)
- All 24 prosigns
- Text string support
- Keyword prosign support

#### Resource-Constrained Boards
**Arduino UNO R3 (ATmega328P)**

- Emergency message only: `Flash_CQD_NO_RTC()`
- Raw Morse code arrays: `FlashMorseCode()`
- Reduced memory footprint

The library automatically adapts based on `UNO_R3` board definition.

## Historical Note: CQD vs SOS

This library uses **CQD** ("Come Quick - Distress") for equipment failures rather than **SOS**.

**Why?**
- **CQD** was the original maritime distress signal (pre-1908)
- **SOS** replaced CQD and is reserved for life-threatening emergencies
- Using CQD for equipment failures follows international convention
- Morse operators will recognize no life threatening emergency

The library includes both:
- Use `CQD` for equipment issues (e.g., "CQD NO RTC")
- Use `SOS` prosign only for actual emergencies

## Integration Example

From the WiFi Binary Clock project:

```cpp
#include <MorseCodeLED.h>

MorseCodeLED morse(LED_BUILTIN);

void setup() {
    morse.Begin();
    
    if (!rtc.begin()) {
        // RTC not found - signal distress
        while (true) {
            morse.Flash_CQD_NO_RTC();
            delay(3000);  // Pause between messages
        }
    }
    
    // RTC OK - continue normally
    morse.FlashProsignWord("START");
}
```

## Memory Usage

Approximate flash memory usage:

| Board Type | Flash Usage | Features |
|------------|-------------|----------|
| UNO R3 | ~1-2 KB | Emergency message only |
| ESP32 / R4 | ~3-5 KB | Full featured |

RAM usage is minimal (< 100 bytes) for both versions.

## Limitations

- Non-blocking operation not supported (uses `delay()`)
- Fixed timing (200ms dot duration)
- LED must be connected to GPIO output pin
- Case insensitive (converts to uppercase)
- Extended character support varies by board

## Dependencies

- **Arduino.h**: Arduino core library

No other dependencies required.

## Contributing

Contributions welcome! This library is part of the [WiFi Binary Clock](https://github.com/Chris-70/WiFiBinaryClock) project.

Areas for enhancement:
- Non-blocking operation
- Adjustable timing
- PWM brightness control
- Additional character sets
- Custom prosign definitions

## Version History

- **v0.9.4** - Current version
  - Full Morse code library for Binary Clock project
  - 24 prosigns supported
  - Resource-adaptive design
  - Extended character support

## Author

**Chris-70** (2025)

Created for the WiFi Binary Clock Shield project.

### Author's Note

> *"This was my first attempt at using CoPilot to generate code, and the results were mixed. Some code was good, for example packing each Morse code 'character' into a 16-bit value with the length in the upper 4 bits and the pattern in the lower 12 bits. However, when I asked it to generate the lookup table for the characters, it made several mistakes over and over again. None of the code was robust, and there was no error checking. I had to rewrite most of the code to make it work properly. I also added the ability to flash arbitrary strings of text, which was not in the original code. Overall, it was a good learning experience, but I would not rely on CoPilot to generate production code."*

## License

Part of the WiFi Binary Clock project. See project repository for license details.

## Links

- **WiFi Binary Clock Project**: https://github.com/Chris-70/WiFiBinaryClock
- **MorseCodeLED Library**: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/MorseCodeLED
- **Class Diagram**: [ClassDiagram.md](ClassDiagram.md)
- **Binary Clock Shield**: https://nixietester.com/product/binary-clock-shield-for-arduino/

---

**Flash responsibly. Use SOS only for actual emergencies! 🚨📡**

<!-- Reference Links -->
[ClassDiagram]: ClassDiagram.md
[ClassDiagram_Git]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/MorseCodeLED/ClassDiagram.md
[INSTALL]: InstallUsage.md
[INSTALL_Git]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/MorseCodeLED/InstallUsage.md
