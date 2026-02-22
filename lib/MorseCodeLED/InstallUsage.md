# MorseCodeLED - Installation and Usage Guide

This document provides detailed instructions on how to install and use the MorseCodeLED library for Arduino. 
Whether you're using PlatformIO or the Arduino IDE, this guide will help you get up and running quickly.

## Additional Documents
- [**`README.md`**][README] (GitHub: [`README.md`][README_Git]) - Overview and documentation for the **MorseCodeLED library**.
- [**`ClassDiagram.md`**][ClassDiagram] (GitHub: [`ClassDiagram.md`][ClassDiagram_Git]) - Class diagram and structure of the **MorseCodeLED library**.
---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Installation](#installation)
   - [PlatformIO](#platformio)
   - [Arduino IDE](#arduino-ide)
3. [Basic Usage](#basic-usage)
4. [API Reference](#api-reference)
5. [Board-Specific Features](#board-specific-features)
6. [Examples](#examples)
7. [Troubleshooting](#troubleshooting)

---

## Quick Start

Get up and running in 3 steps:

```cpp
#include <MorseCodeLED.h>

MorseCodeLED morse(LED_BUILTIN);  // Use built-in LED

void setup() {
    morse.Begin();                     // Initialize
    morse.FlashString("HELLO WORLD");  // Flash message
}

void loop() {
    // Your code here
}
```

That's it! Your LED is now speaking Morse code.

---

## Installation

### PlatformIO

**Method 1: Direct from GitHub (Recommended)**

Add to your `platformio.ini` file:

```ini
[env:your_board]
platform = ...
board = ...
framework = arduino

lib_deps = 
    https://github.com/Chris-70/WiFiBinaryClock.git#lib/MorseCodeLED
```

**Method 2: Local Library**

If you have the library locally:

```ini
lib_deps = 
    MorseCodeLED
```

Then copy the `MorseCodeLED` folder to your project's `lib/` directory.

**Method 3: Git Submodule**

For version control integration:

```bash
cd your_project
git submodule add https://github.com/Chris-70/WiFiBinaryClock.git lib/WiFiBinaryClock
```

Reference in `platformio.ini`:
```ini
lib_deps = 
    ${PROJECT_DIR}/lib/WiFiBinaryClock/lib/MorseCodeLED
```

---

### Arduino IDE

**Method 1: Manual Installation**

1. Download or clone the [WiFi Binary Clock repository](https://github.com/Chris-70/WiFiBinaryClock)
2. Navigate to `lib/MorseCodeLED` folder
3. Copy the entire `MorseCodeLED` folder to your Arduino libraries directory:
   - **Windows**: `C:\Users\<username>\Documents\Arduino\libraries\`
   - **macOS**: `~/Documents/Arduino/libraries/`
   - **Linux**: `~/Arduino/libraries/`
4. Restart Arduino IDE
5. Verify installation: Sketch → Include Library → MorseCodeLED should appear

**Method 2: ZIP Import**

1. Download the library as ZIP
2. In Arduino IDE: Sketch → Include Library → Add .ZIP Library
3. Select the downloaded ZIP file
4. Restart Arduino IDE

**Verification**

After installation, try the basic example:

```cpp
#include <MorseCodeLED.h>

MorseCodeLED morse(LED_BUILTIN);

void setup() {
    morse.Begin();
    morse.FlashString("TEST");
}

void loop() {}
```

---

## Basic Usage

### 1. Include the Library

```cpp
#include <MorseCodeLED.h>
```

### 2. Create Morse Object

```cpp
// Use built-in LED
MorseCodeLED morse(LED_BUILTIN);

// Or specify a pin
MorseCodeLED morse(13);  // Use pin 13
```

### 3. Initialize in setup()

```cpp
void setup() {
    morse.Begin();  // Must call before using
}
```

### 4. Flash Messages

**Simple Text:**
```cpp
morse.FlashString("SOS");
morse.FlashString("HELLO WORLD");
```

**Single Character:**
```cpp
morse.FlashCharacter('A');  // Flash letter A (·−)
```

**Prosigns (Procedural Signals):**
```cpp
morse.FlashProsign(Prosign::SOS);      // Distress signal
morse.FlashProsign(Prosign::Start);    // KA - Start transmission
morse.FlashProsign(Prosign::End);      // AR - End of message
```

**Prosigns by Keyword:**
```cpp
morse.FlashProsignWord("START");   // KA
morse.FlashProsignWord("SOS");     // SOS
morse.FlashProsignWord("OVER");    // K
```

---

## API Reference

### Constructor

```cpp
MorseCodeLED(int ledPin)
```

**Parameters:**
- `ledPin` - GPIO pin number where LED is connected (e.g., `LED_BUILTIN`, `13`)

**Example:**
```cpp
MorseCodeLED morse(LED_BUILTIN);
```

---

### Methods

#### `void Begin()`

Initialize the LED pin. **Must be called in `setup()` before using any other methods.**

```cpp
void setup() {
    morse.Begin();
}
```

---

#### `void FlashString(const String& text)`

Flash a complete text string in Morse code.

**Parameters:**
- `text` - String to flash (A-Z, 0-9, punctuation, spaces)

**Features:**
- Automatically converts to uppercase
- Handles word spacing
- Supports letters, numbers, and punctuation (board-dependent)

**Example:**
```cpp
morse.FlashString("HELLO");
morse.FlashString("SOS 911");
morse.FlashString("42");
```

**Availability:** Full-featured boards only (not UNO R3)

---

#### `void FlashCharacter(char c)`

Flash a single character in Morse code.

**Parameters:**
- `c` - Character to flash (A-Z, 0-9, punctuation)

**Example:**
```cpp
morse.FlashCharacter('A');  // ·−
morse.FlashCharacter('5');  // ·····
morse.FlashCharacter('?');  // ··−−··
```

**Availability:** Full-featured boards only (not UNO R3)

---

#### `void FlashProsign(Prosign sign)`

Flash a Morse prosign (procedural signal).

**Parameters:**
- `sign` - Prosign enum value

**Available Prosigns:**

| Prosign | Morse Code | Meaning | Example |
|---------|------------|---------|---------|
| `Prosign::SOS` | `···−−−···` | Emergency distress | `morse.FlashProsign(Prosign::SOS);` |
| `Prosign::Start` | `−·−·−` | Start transmission (KA) | `morse.FlashProsign(Prosign::Start);` |
| `Prosign::End` | `·−·−·` | End of message (AR) | `morse.FlashProsign(Prosign::End);` |
| `Prosign::Over` | `−·−` | Invitation to transmit (K) | `morse.FlashProsign(Prosign::Over);` |
| `Prosign::Error` | `········` | Error/Correction (HH) | `morse.FlashProsign(Prosign::Error);` |
| `Prosign::R` | `·−·` | Received OK (Roger) | `morse.FlashProsign(Prosign::R);` |

**Full list:** See [MorseCodeLED.h](src/MorseCodeLED.h) for all 24 prosigns.

**Example:**
```cpp
morse.FlashProsign(Prosign::Start);     // Begin transmission
morse.FlashString("HELLO");             // Message
morse.FlashProsign(Prosign::End);       // End transmission
```

**Availability:** Full-featured boards only (not UNO R3)

---

#### `void FlashProsignWord(String keyword)`

Flash a prosign using a keyword string.

**Parameters:**
- `keyword` - Prosign keyword (case-insensitive)

**Supported Keywords:**

| Keyword | Prosign | Morse | Meaning |
|---------|---------|-------|---------|
| `"START"`, `"STARTING"` | KA | `−·−·−` | Start transmission |
| `"END"`, `"OK"` | AR | `·−·−·` | End of message |
| `"OUT"`, `"ENDWORK"` | SK | `···−·−` | End of contact |
| `"OVER"`, `"INVITE"` | K | `−·−` | Over to you |
| `"ROGER"` | R | `·−·` | Received |
| `"UNDERSTOOD"` | VE | `···−·` | Understood |
| `"ERROR"`, `"CORRECTION"` | HH | `········` | Error |
| `"YES"`, `"CORRECT"`, `"CONFIRM"` | C | `−·−·` | Affirmative |
| `"NO"`, `"NEGATIVE"` | N | `−·` | Negative |
| `"SOS"` | SOS | `···−−−···` | Distress |

**Example:**
```cpp
morse.FlashProsignWord("START");
morse.FlashString("TEST");
morse.FlashProsignWord("END");
```

**Availability:** Full-featured boards only (not UNO R3)

---

#### `void FlashMorseCode(const MC* morseData)`

Flash raw Morse code using component array (dots, dashes, spaces).

**Parameters:**
- `morseData` - Array of `MC` enum values terminated with `MC::EndMarker`

**MC Enum Values:**
- `MC::Dot` - Dot (dit) `·`
- `MC::Dash` - Dash (dah) `−`
- `MC::Space` - Letter space
- `MC::Word` - Word space
- `MC::EndMarker` - End of sequence (required)

**Example:**
```cpp
// Custom message: "NO" (N=−·, O=−−−)
const MC customMessage[] = {
    MC::Dash, MC::Dot, MC::Space,           // N
    MC::Dash, MC::Dash, MC::Dash, MC::Space,// O
    MC::EndMarker                            // Required
};

morse.FlashMorseCode(customMessage);
```

**Availability:** All boards (including UNO R3)

---

#### `void Flash_CQD_NO_RTC()`

Flash the predefined "CQD NO RTC" emergency message.

**Example:**
```cpp
if (!rtc.begin()) {
    morse.Flash_CQD_NO_RTC();  // Signal RTC failure
}
```

**Note:** This method is only available when `BINARY_CLOCK_LIB` is defined (Binary Clock project specific).

**Availability:** All boards (including UNO R3)

---

## Board-Specific Features

The library automatically adapts to your board's capabilities:

### Full-Featured Boards

**Supported Boards:**
- ESP32 / ESP8266
- Arduino R4 (Renesas)
- Arduino Mega
- STM32
- Teensy
- Most modern Arduino-compatible boards

**Available Features:**
✅ Complete character set (A-Z, 0-9, punctuation)  
✅ All 24 prosigns  
✅ Text string support (`FlashString`)  
✅ Single character support (`FlashCharacter`)  
✅ Keyword prosign support (`FlashProsignWord`)  
✅ Raw Morse code arrays (`FlashMorseCode`)  

**Memory Usage:**
- Flash: ~3-5 KB
- RAM: < 100 bytes

---

### Resource-Constrained Boards

**Supported Boards:**
- Arduino UNO R3 (ATmega328P)

**Available Features:**
✅ Emergency message (`Flash_CQD_NO_RTC`)  
✅ Raw Morse code arrays (`FlashMorseCode`)  
❌ Text strings (memory limited)  
❌ Extended character set  
❌ Prosign enums  

**Memory Usage:**
- Flash: ~1-2 KB
- RAM: < 50 bytes

**UNO R3 Example:**
```cpp
#include <MorseCodeLED.h>

MorseCodeLED morse(13);

void setup() {
    morse.Begin();
    
    // Option 1: Emergency message
    morse.Flash_CQD_NO_RTC();
    
    // Option 2: Custom raw Morse
    const MC sos[] = {
        MC::Dot, MC::Dot, MC::Dot, MC::Space,      // S
        MC::Dash, MC::Dash, MC::Dash, MC::Space,   // O
        MC::Dot, MC::Dot, MC::Dot,                 // S
        MC::EndMarker
    };
    morse.FlashMorseCode(sos);
}

void loop() {}
```

---

## Examples

### Example 1: Simple Error Signaling

Flash an error when something goes wrong:

```cpp
#include <MorseCodeLED.h>

MorseCodeLED morse(LED_BUILTIN);

void setup() {
    morse.Begin();
    
    if (!initializeHardware()) {
        // Signal error
        morse.FlashString("ERROR");
        while(true) {
            delay(1000);
            morse.FlashProsignWord("ERROR");
        }
    }
    
    // Success!
    morse.FlashString("OK");
}

bool initializeHardware() {
    // Your initialization code
    return true;
}

void loop() {}
```

---

### Example 2: Status Messages

Flash different status messages:

```cpp
#include <MorseCodeLED.h>

MorseCodeLED morse(LED_BUILTIN);

void setup() {
    Serial.begin(115200);
    morse.Begin();
    
    morse.FlashProsignWord("START");
    
    Serial.println("Initializing WiFi...");
    if (initWiFi()) {
        morse.FlashString("WIFI OK");
    } else {
        morse.FlashString("WIFI FAIL");
    }
    
    Serial.println("Initializing sensors...");
    if (initSensors()) {
        morse.FlashString("SENSORS OK");
    } else {
        morse.FlashString("SENSOR FAIL");
    }
    
    morse.FlashProsignWord("END");
}

void loop() {}
```

---

### Example 3: Binary Clock Integration

From the WiFi Binary Clock project:

```cpp
#include <MorseCodeLED.h>
#include <RTClib.h>

RTC_DS3231 rtc;
MorseCodeLED morse(LED_BUILTIN);

void setup() {
    morse.Begin();
    
    if (!rtc.begin()) {
        // RTC not found - enter error state
        while (true) {
            morse.Flash_CQD_NO_RTC();  // "CQD NO RTC"
            delay(5000);                // Wait 5 seconds
        }
    }
    
    // RTC OK
    morse.FlashProsignWord("START");
}

void loop() {
    // Normal operation
}
```

---

### Example 4: Interactive Morse Communicator

Send messages based on Serial input:

```cpp
#include <MorseCodeLED.h>

MorseCodeLED morse(LED_BUILTIN);

void setup() {
    Serial.begin(115200);
    morse.Begin();
    
    Serial.println("Morse Code Communicator");
    Serial.println("Type message and press Enter");
    morse.FlashProsignWord("START");
}

void loop() {
    if (Serial.available()) {
        String message = Serial.readStringUntil('\n');
        message.trim();
        
        if (message.length() > 0) {
            Serial.print("Sending: ");
            Serial.println(message);
            
            morse.FlashString(message);
            morse.FlashProsignWord("END");
        }
    }
}
```

---

### Example 5: Custom Raw Morse Pattern

Create custom patterns for UNO R3:

```cpp
#include <MorseCodeLED.h>

MorseCodeLED morse(13);

// Define "SOS" manually
const MC sosPattern[] = {
    MC::Dot, MC::Dot, MC::Dot, MC::Space,      // S
    MC::Dash, MC::Dash, MC::Dash, MC::Space,   // O
    MC::Dot, MC::Dot, MC::Dot,                 // S
    MC::EndMarker
};

// Define "HELP" manually
const MC helpPattern[] = {
    // H = ····
    MC::Dot, MC::Dot, MC::Dot, MC::Dot, MC::Space,
    // E = ·
    MC::Dot, MC::Space,
    // L = ·−··
    MC::Dot, MC::Dash, MC::Dot, MC::Dot, MC::Space,
    // P = ·−−·
    MC::Dot, MC::Dash, MC::Dash, MC::Dot,
    MC::EndMarker
};

void setup() {
    morse.Begin();
    
    morse.FlashMorseCode(sosPattern);
    delay(2000);
    morse.FlashMorseCode(helpPattern);
}

void loop() {}
```

---

## Morse Code Reference

### Timing Standards

The library follows international Morse code timing:

| Element | Duration | Multiple |
|---------|----------|----------|
| **Dot (dit)** | 200 ms | 1× (base unit) |
| **Dash (dah)** | 600 ms | 3× dot |
| **Intra-character gap** | 200 ms | 1× dot |
| **Inter-character gap** | 600 ms | 3× dot |
| **Word space** | 1400 ms | 7× dot |

### Character Set

**Letters (A-Z):**
```
A ·−    B −···  C −·−·  D −··   E ·     F ··−·  G −−·   H ····
I ··    J ·−−−  K −·−   L ·−··  M −−    N −·    O −−−   P ·−−·
Q −−·−  R ·−·   S ···   T −     U ··−   V ···−  W ·−−   X −··−
Y −·−−  Z −−··
```

**Numbers (0-9):**
```
0 −−−−−    1 ·−−−−    2 ··−−−    3 ···−−    4 ····−
5 ·····    6 −····    7 −−···    8 −−−··    9 −−−−·
```

**Punctuation (Full-featured boards):**
```
. ·−·−·−    , −−··−−    ? ··−−··    ' ·−−−−·
! −·−·−−    / −··−·     ( −·−−·     ) −·−−·−
: −−−···    ; −·−·−·    = −···−     + ·−·−·
- −····−    _ ··−−·−    " ·−··−·    $ ···−··−
@ ·−−·−·
```

---

## Troubleshooting

### LED Not Flashing

**Problem:** LED doesn't light up

**Solutions:**
1. Verify pin number is correct
   ```cpp
   MorseCodeLED morse(LED_BUILTIN);  // Try built-in LED first
   ```

2. Check that `Begin()` was called
   ```cpp
   void setup() {
       morse.Begin();  // Required!
   }
   ```

3. Test LED manually
   ```cpp
   pinMode(LED_BUILTIN, OUTPUT);
   digitalWrite(LED_BUILTIN, HIGH);  // Should turn on
   ```

4. Check LED polarity if using external LED

---

### Compile Errors

**Problem:** `FlashString` not found

**Cause:** Using UNO R3 or `UNO_R3` is defined

**Solution:** Use raw Morse arrays instead:
```cpp
// Instead of:
morse.FlashString("SOS");  // ❌ Not available on UNO R3

// Use:
const MC sos[] = {
    MC::Dot, MC::Dot, MC::Dot, MC::Space,
    MC::Dash, MC::Dash, MC::Dash, MC::Space,
    MC::Dot, MC::Dot, MC::Dot,
    MC::EndMarker
};
morse.FlashMorseCode(sos);  // ✅ Available on all boards
```

---

**Problem:** Library not found

**Solutions:**

*PlatformIO:*
1. Check `platformio.ini` syntax
2. Run: `pio lib install`
3. Clean and rebuild: `pio run --target clean`

*Arduino IDE:*
1. Verify library in `Arduino/libraries/` folder
2. Restart Arduino IDE
3. Check: Sketch → Include Library → should show MorseCodeLED

---

### Runtime Issues

**Problem:** Messages flash too fast/slow

**Solution:** Timing is fixed at 200ms dot duration. To modify, edit `MorseCodeLED.cpp`:
```cpp
// Find these constants and adjust:
const int dotDuration = 200;      // Change to desired ms
const int dashDuration = 600;     // Should be 3× dot
```

---

**Problem:** Blocking behavior stops other code

**Explanation:** The library uses `delay()` internally and is blocking.

**Workaround:** For non-blocking operation, refactor needed:
```cpp
// Current (blocking):
morse.FlashString("HELLO");  // Blocks until complete

// Workaround: Flash in batches
void loop() {
    static unsigned long lastFlash = 0;
    
    if (millis() - lastFlash > 10000) {  // Every 10 seconds
        morse.FlashString("STATUS");
        lastFlash = millis();
    }
    
    // Other code runs between flashes
}
```

---

### Memory Issues

**Problem:** Out of memory on UNO R3

**Solution 1:** Use minimal features only
```cpp
// ❌ Avoid on UNO R3:
morse.FlashString("LONG MESSAGE HERE");

// ✅ Use on UNO R3:
morse.Flash_CQD_NO_RTC();  // Predefined
```

**Solution 2:** Use PROGMEM for patterns
```cpp
const PROGMEM MC pattern[] = { /* ... */ };
morse.FlashMorseCode(pattern);
```

---

## Advanced Topics

### Data Structures

#### MCode Union

Efficient 16-bit Morse pattern storage:

```cpp
union MCode {
    uint16_t pattern;        // Full 16-bit value
    struct {
        uint16_t code : 12;  // Pattern (bits 0-11)
        uint16_t len  :  4;  // Length (bits 12-15)
    };
};
```

**Example:**
```cpp
// Letter 'A' = ·− = dot-dash
// Pattern: 01 (right to left: dash=1, dot=0)
// Length: 2
MCode letterA(2, 0b01);

// Letter 'S' = ··· = dot-dot-dot
// Pattern: 000
// Length: 3
MCode letterS(3, 0b000);
```

---

### Creating Custom Lookup Tables

For advanced users wanting to extend the character set:

```cpp
struct XcLookup {
    char character;
    MCode mc;
};

// Add custom character
const XcLookup customChars[] = {
    {'@', MCode(6, 0b101001)},  // @ = ·−−·−·
    {'\0', MCode(0, 0)}          // Terminator
};
```

---

## Best Practices

1. **Always call `Begin()`** before using any flash methods
2. **Use `LED_BUILTIN`** for initial testing
3. **Keep messages short** - long messages take time to flash
4. **Use prosigns** for standardized communication
5. **Test on target hardware** - memory usage varies by board
6. **Add delays between messages** for readability
7. **Use `Flash_CQD_NO_RTC()`** on UNO R3 when possible
8. **Document custom patterns** with comments

---

## Performance Considerations

**Execution Time Examples:**

| Message | Approximate Duration |
|---------|---------------------|
| Single letter 'A' | ~1 second |
| "SOS" | ~4 seconds |
| "HELLO" | ~8 seconds |
| "HELLO WORLD" | ~18 seconds |

**Calculation:**
- Each dot: 200ms × 2 (on + off) = 400ms
- Each dash: 600ms × 2 (on + off) = 1200ms
- Letter space: 600ms
- Word space: 1400ms

---

## Frequently Asked Questions

**Q: Can I use multiple LEDs?**  
A: Yes, create multiple `MorseCodeLED` objects with different pins.

```cpp
MorseCodeLED led1(12);
MorseCodeLED led2(13);

led1.Begin();
led2.Begin();

led1.FlashString("LEFT");
led2.FlashString("RIGHT");
```

**Q: Can I change the speed?**  
A: Timing is hardcoded but can be modified in the source code (`MorseCodeLED.cpp`).

**Q: Does it work with active-low LEDs?**  
A: Currently designed for active-high. Modification needed for active-low.

**Q: Is non-blocking mode supported?**  
A: Not currently. Uses `delay()` for timing.

**Q: Can I flash to Serial instead of LED?**  
A: Library is LED-specific, but concept can be adapted.

---

## Additional Resources

- **Main README:** [README.md](README.md) - Full library documentation
- **Class Diagram:** [ClassDiagram.md](ClassDiagram.md) - Architecture overview
- **Source Code:** [MorseCodeLED.h](src/MorseCodeLED.h) / [MorseCodeLED.cpp](src/MorseCodeLED.cpp)
- **WiFi Binary Clock Project:** https://github.com/Chris-70/WiFiBinaryClock
- **Binary Clock Shield:** https://nixietester.com/product/binary-clock-shield-for-arduino/

---

## Support and Contributing

This library is part of the WiFi Binary Clock project.

**Report Issues:**
- GitHub: https://github.com/Chris-70/WiFiBinaryClock/issues

**Contributions Welcome:**
- Non-blocking operation
- Adjustable timing
- PWM brightness control
- Additional character sets
- Active-low LED support

---

## License

Part of the WiFi Binary Clock project. See project repository for license details.

---

<!-- Reference Links -->
[ClassDiagram]: ClassDiagram.md
[ClassDiagram_Git]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/MorseCodeLED/ClassDiagram.md
[README]: README.md
[README_Git]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/MorseCodeLED/README.md
