# BCGlobalDefines - Installation and Usage Guide

**Comprehensive guide for installing and using the BCGlobalDefines shared library**

## Table of Contents
1. [Installation](#installation)
   - [PlatformIO](#platformio)
   - [Arduino IDE](#arduino-ide)
2. [Quick Start](#quick-start)
3. [What's Included](#whats-included)
4. [Configuration](#configuration)
   - [Board Selection](#board-selection)
   - [Feature Flags](#feature-flags)
   - [Custom Board Definition](#custom-board-definition)
5. [Component Reference](#component-reference)
   - [Interface Classes](#interface-classes)
   - [Data Structures](#data-structures)
   - [Compile-Time Definitions](#compile-time-definitions)
   - [Utility Templates](#utility-templates)
6. [Advanced Usage](#advanced-usage)
   - [Custom Pin Mappings](#custom-pin-mappings)
   - [Serial Output Control](#serial-output-control)
   - [FreeRTOS Task Wrappers](#freertos-task-wrappers)
7. [Best Practices](#best-practices)
8. [Troubleshooting](#troubleshooting)

---

## Installation

### PlatformIO

BCGlobalDefines is automatically included as a dependency when you install other WiFiBinaryClock libraries. To explicitly include it in your project:

1. **Add to `platformio.ini`:**
   ```ini
   [env:your_board]
   platform = espressif32  ; or your platform
   board = wemos_d1_uno32  ; or your board
   framework = arduino
   
   lib_deps =
       https://github.com/Chris-70/WiFiBinaryClock.git#lib/BCGlobalDefines
       adafruit/RTClib@^2.1.4  ; or RTClibPlus
   ```

2. **Or add to project `lib/` folder:**
   ```bash
   cd your_project/lib
   git clone https://github.com/Chris-70/WiFiBinaryClock.git
   # Copy only the BCGlobalDefines folder
   cp -r WiFiBinaryClock/lib/BCGlobalDefines .
   ```

3. **Install dependencies:**
   ```bash
   pio lib install
   ```

### Arduino IDE

1. **Download the library:**
   - Visit [WiFiBinaryClock on GitHub](https://github.com/Chris-70/WiFiBinaryClock)
   - Download the repository as ZIP or clone it
   - Extract files to a working directory

2. **Install BCGlobalDefines:**
   - Navigate to `WiFiBinaryClock/lib/BCGlobalDefines`
   - Copy the entire `BCGlobalDefines` folder
   - Paste into Arduino libraries folder:
     - **Windows:** `Documents\Arduino\libraries\`
     - **macOS:** `~/Documents/Arduino/libraries/`
     - **Linux:** `~/Arduino/libraries/`

3. **Install required dependencies:**
   - Open Arduino IDE
   - Go to **Sketch > Include Library > Manage Libraries...**
   - Search for and install:
     - **RTClib** by Adafruit (v2.1.4 or later) - or use RTClibPlus
     - **Streaming** by Mikal Hart (optional, for enhanced serial output)

4. **Verify installation:**
   - Restart Arduino IDE
   - Go to **Sketch > Include Library**
   - Look for "BCGlobalDefines" in the list

---

## Quick Start

### Understanding BCGlobalDefines

BCGlobalDefines is a **passive dependency library** - you typically don't use it directly. It's automatically included by other libraries in the WiFiBinaryClock ecosystem:

```cpp
// Other libraries include BCGlobalDefines
#include <BinaryClock.h>        // Includes BCGlobalDefines interfaces
#include <BinaryClockWiFi.h>    // Includes BCGlobalDefines structures
```

### When to Use Directly

You'll interact with BCGlobalDefines directly when:

1. **Implementing the interfaces** - Creating your own clock implementation
2. **Using shared structures** - Working with AlarmTime, APCreds, WiFiInfo, etc.
3. **Configuring the system** - Customizing board settings via `board_select.h`
4. **Developing extensions** - Adding new features that integrate with existing code

### Basic Usage Example

```cpp
#include <BinaryClock.Defines.h>    // System definitions
#include <BinaryClock.Structs.h>    // Shared structures
#include <IBinaryClock.h>           // Interface definition

using namespace BinaryClockShield;

void setup() {
    Serial.begin(115200);
    
    // Access the BinaryClock singleton through interface
    IBinaryClock* clock = &BinaryClock::get_Instance();
    
    // Use shared structures
    AlarmTime alarm;
    alarm.number = 1;
    alarm.time = DateTime(2026, 2, 16, 7, 30, 0);
    alarm.melody = 0;
    alarm.status = 1;  // Active
    alarm.freq = AlarmTime::Repeat::Daily;
    
    // Configure the alarm using the interface
    clock->set_Alarm(alarm);
    
    // Display a pattern
    clock->DisplayLedPattern(LedPattern::onColors, 5000);
}

void loop() {
    // Your code here
}
```

---

## What's Included

### File Structure

```
BCGlobalDefines/
├── library.json                        # Library metadata
├── README.md                           # Library overview
├── BCGlobalDefines_LibInstallUsage.md  # This file
├── BCGlobalDefines_LibClassDiagram.md  # Architecture diagram
└── src/
    ├── IBinaryClockBase.h              # Base interface for clock
    ├── IBinaryClock.h                  # Extended clock interface
    ├── IBCButtonBase.h                 # Button interface
    ├── BinaryClock.Structs.h           # Shared data structures
    ├── BinaryClock.Defines.h           # System-wide definitions
    ├── SerialOutput.Defines.h          # Debug output macros
    ├── TaskWrapper.h                   # FreeRTOS task utilities
    └── TaskGroupBits.h                 # FreeRTOS event bits
```

### Component Categories

| Category | Purpose | Files |
|----------|---------|-------|
| **Interfaces** | Abstract contracts for implementations | `IBinaryClock*.h`, `IBCButtonBase.h` |
| **Structures** | Shared data types and enums | `BinaryClock.Structs.h` |
| **Definitions** | Compile-time configuration | `BinaryClock.Defines.h` |
| **Utilities** | Helper templates and macros | `TaskWrapper.h`, `SerialOutput.Defines.h` |

---

## Configuration

### Board Selection

The library automatically detects your board from Arduino/PlatformIO settings. For custom configurations, create `board_select.h` in your project:

**Location:** `lib/BinaryClock/src/board_select.h` (or main `src/` folder)

#### Supported Boards

```cpp
// Uncomment ONE of these in board_select.h:

// ESP32 Boards
// #define ESP32_D1_R32_UNO   // Wemos D1 R32 UNO (ESP32)
// #define METRO_ESP32_S3     // Adafruit Metro ESP32-S3
// #define ESP32_S3_UNO       // Generic ESP32-S3 UNO form factor

// Arduino Boards
// #define UNO_R4_WIFI        // Arduino UNO R4 WiFi
// #define UNO_R4_MINIMA      // Arduino UNO R4 Minima
// #define UNO_R3             // Arduino UNO R3 (limited features)

// Custom boards
// #define CUSTOM_UNO  true   // Use custom board definition (see below)
```

### Feature Flags

Key features controlled by compile-time definitions:

```cpp
// Automatically set based on board, or override in board_select.h

// WiFi Support
#define WIFI         true   // General WiFi availability
#define ESP32_WIFI   true   // ESP32 WiFi (uses WiFi.h)
#define WIFIS3       false  // Arduino UNO R4 WiFi (uses WiFiS3.h)

// Operating System
#define FREE_RTOS    true   // FreeRTOS task support
#define STL_USED     true   // C++ STL (std::vector, etc.) available

// Serial Debugging
#define SERIAL_OUTPUT  true   // Enable serial output statements
#define DEBUG_OUTPUT   false  // Enable debug-level output
#define DEV_CODE       false  // Enable development code sections

// Features
#define MELODY_REGISTRY  true   // Multiple melody support
#define RTC_CENTURY      true   // Century support in RTC
```

### Custom Board Definition

To define a custom board, create or edit `board_select.h`:

```cpp
// board_select.h - Custom Board Example

#define CUSTOM_UNO  true

// Board Capabilities
#define WIFI               false
#define FREE_RTOS          false
#define STL_USED           false
#define MELODY_REGISTRY    false

// Pin Definitions
#define LED_DATA_PIN       6     // NeoPixel data pin
#define S1                 A0    // Button 1
#define S2                 A1    // Button 2  
#define S3                 A2    // Button 3
#define PIEZO              9     // Buzzer pin

// I2C Pins (for RTC)
#define I2C_SDA_PIN        SDA
#define I2C_SCL_PIN        SCL

// Button Configuration
#define BUTTON_WIRING      CA_ON    // Common Anode

// Hardware Specs
#define NUMBER_LEDS        17       // 17 LEDs for binary time

// Serial Configuration
#define SERIAL_OUTPUT      true
#define DEBUG_OUTPUT       false
#define SERIAL_BAUD        115200
```

---

## Component Reference

### Interface Classes

#### IBinaryClockBase

Base interface defining core clock functionality.

**Key Methods:**
```cpp
// Time Management
virtual void set_Time(DateTime value) = 0;
virtual DateTime get_Time() const = 0;
virtual DateTime ReadTime() = 0;

// Alarm Management
virtual void set_Alarm(AlarmTime value) = 0;
virtual AlarmTime get_Alarm() const = 0;

// Display Control
virtual void DisplayLedPattern(LedPattern patternType) = 0;
virtual void DisplayBinaryTime(int hours, int minutes, int seconds, bool use12Hour = false) = 0;

// Callbacks
virtual bool RegisterTimeCallback(void (*callback)(const DateTime&)) = 0;
virtual bool RegisterAlarmCallback(void (*callback)(const DateTime&)) = 0;
virtual void PlayAlarm(const AlarmTime& alarm) const = 0;
```

#### IBinaryClock

Extended interface with additional functionality.

**Additional Methods:**
```cpp
// Extended Display
virtual void DisplayLedPattern(LedPattern patternType, unsigned long duration) = 0;

// Melody Management (when STL_USED == true)
virtual bool PlayMelody(size_t id) const = 0;
virtual size_t RegisterMelody(const std::vector<Note>& melody) = 0;
virtual const std::vector<Note>& GetMelodyById(size_t id) const = 0;
```

#### IBCButtonBase

Interface for button handling.

**Key Methods:**
```cpp
// Button State
virtual bool IsPressed() = 0;              // Check if button is currently pressed (with debounce)
virtual bool IsPressedRaw() const = 0;     // Check if button is pressed (no debounce)
virtual bool IsPressedNew() = 0;           // Check if button was just pressed (new press event)

// State Management
virtual void ClearPressedNew() = 0;        // Clear the pressed state
virtual void Reset() = 0;                  // Reset button state
virtual void Initialize() = 0;             // Initialize the button pin

// Properties
virtual uint8_t get_Pin() const = 0;       // Get the GPIO pin number
```

### Data Structures

#### AlarmTime

Complete alarm configuration.

```cpp
struct alarmTime {  // typedef as AlarmTime
    enum Repeat {
        Never = 0,    // Turn off after firing
        Hourly,       // Every hour
        Daily,        // Every day (default)
        Weekly,       // Every week
        Monthly,      // Every month
        endTag
    };
    
    uint8_t  number;   // 1 or 2
    DateTime time;     // Alarm time
    uint8_t  melody;   // Melody ID (0 = default)
    uint8_t  status;   // 0 = inactive, 1 = active
    Repeat   freq;     // Repeat frequency
    bool     fired;    // Currently ringing
    
    void clear();      // Reset all except number
} AlarmTime;  // typedef for easier use
```

#### WiFi Structures (when WIFI == true)

WiFi-related structures for managing access points and credentials.

**APNames** - Basic AP identification:
```cpp
struct APNames {
    String ssid;               // Network SSID
    String bssid;              // MAC address (can be empty for wildcard)
    
    bool bssidToBytes(uint8_t(&bssidArray)[6]) const;  // Convert BSSID to bytes
};
```

**WiFiInfo** - AP information from scan:
```cpp
struct WiFiInfo : public APNames {
    int32_t           rssi;       // Signal strength in dBm
    int32_t           channel;    // WiFi channel
    wifi_auth_mode_t  authMode;   // Security type (OPEN, WPA2, etc.)
};
```

**APCreds** - AP credentials for connection:
```cpp
struct APCreds : public APNames {
    String pw;                 // Network password (can be empty for open networks)
};
```

#### Note

Musical note definition for melodies.

```cpp
struct Note {
    unsigned      tone;         // Note frequency in Hz (0 = rest)
    unsigned long duration;     // Note duration in milliseconds
};
```

#### LedPattern Enum

LED pattern indices for color arrays.

**Note:** These are indices into color arrays, not direct display patterns. The BinaryClock class uses these to access predefined color patterns.

```cpp
enum class LedPattern : uint8_t {
    onColors = 0,   // LED colors when bit is ON (hours; minutes; seconds)
    offColors,      // LED colors when bit is OFF (usually black)
    onText,         // Big Green "O" for On pattern
    offTxt,         // Big RED sideways "F" for oFF pattern  
    xAbort,         // Big Pink "X" for abort/cancel pattern
    okText,         // Big Lime "✓" for okay/good pattern
    rainbow,        // Rainbow colors on diagonal pattern
    
    // WiFi patterns (when WIFI == true)
    wText,          // Big RoyalBlue "W" for WPS/WiFi pattern
    aText,          // Big Indigo "A" for AP Access web page pattern
    pText,          // Big Orange "P" for Phone app pattern
    nText,          // Big Yellow "N" for NTP sync pattern
    
    endTAG          // End marker and pattern count
};
```

### Compile-Time Definitions

#### Common Macros

**Serial Output Macros:**
```cpp
// General output (controlled by SERIAL_OUTPUT flag)
SERIAL_OUT_PRINT(x)        // Print x to Serial
SERIAL_OUT_PRINTLN(x)      // Print x with newline
SERIAL_OUT_STREAM(x)       // Stream output using << operator
SERIAL_OUT_PRINTF(...)     // Printf-style output

// Development code (controlled by DEV_CODE flag)
SERIAL_PRINT(x)            // Print during development
SERIAL_PRINTLN(x)          // Print with newline during development
SERIAL_STREAM(x)           // Stream output during development
SERIAL_PRINTF(...)         // Printf during development

// Debug/temporary output (controlled by DEBUG_OUTPUT flag)
DEBUG_PRINT(x)             // Temporary debug print
DEBUG_PRINTLN(x)           // Temporary debug print with newline
DEBUG_STREAM(x)            // Temporary debug stream
DEBUG_PRINTF(...)          // Temporary debug printf
```

**Note:** All these macros expand to nothing (whitespace) when their control flag is `false`, removing the code from compilation entirely.

**Conditional Compilation:**
```cpp
STL_USED               // C++ STL available (std::vector, etc.)
WIFI                   // WiFi functionality available (ESP32_WIFI || WIFIS3)
ESP32_WIFI             // ESP32-based WiFi
WIFIS3                 // Arduino UNO R4 WiFi
FREE_RTOS              // FreeRTOS available
```

**Utility Macros:**
```cpp
FOREVER                // Infinite loop: while(true)
```

#### Pin Definitions

Default pins (can be overridden in `board_select.h`):

```cpp
// LED Strip
LED_DATA_PIN           // WS2812B data pin (NeoPixels)
NUMBER_LEDS            // Number of LEDs (default: 17)

// Buttons
S1                     // Time set & Decrement button
S2                     // Select & Confirm/Save button
S3                     // Alarm set & Increment button

// Buzzer
PIEZO                  // Piezo buzzer PWM pin

// I2C (RTC)
I2C_SDA_PIN            // I2C data
I2C_SCL_PIN            // I2C clock
I2C_SPEED              // Bus speed (100000 or 400000 Hz)

// Interrupt
RTC_INT                // RTC interrupt/square wave pin
```

### Utility Templates

#### TaskWrapper

Create FreeRTOS tasks from instance methods (when FREE_RTOS == true).

**Basic Usage:**
```cpp
#include <TaskWrapper.h>

class MyClass {
public:
    void myTaskMethod(int param1, const char* param2) {
        FOREVER {
            // Task code here
            Serial.print("Param: ");
            Serial.println(param1);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
};

void setup() {
    MyClass instance;
    
    // Create task from instance method
    TaskHandle_t handle = CreateInstanceTask(
        &instance,                     // Instance pointer
        &MyClass::myTaskMethod,        // Method pointer
        "MyTask",                      // Task name
        4096,                          // Stack size
        NULL,                          // Task parameters (optional)
        1,                             // Priority
        42,                            // Method arg 1
        "Hello"                        // Method arg 2
    );
    
    if (handle == NULL) {
        Serial.println("Task creation failed!");
    }
}
```

**Static Method Wrapper:**
```cpp
void staticFunction(int x, float y) {
    FOREVER {
        Serial.printf("Values: %d, %.2f\n", x, y);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void setup() {
    // Create task from static/free function
    TaskHandle_t handle = CreateMethodTask(
        staticFunction,                // Function pointer
        "StaticTask",                  // Task name
        2048,                          // Stack size
        NULL,                          // Task parameters
        1,                             // Priority
        100,                           // Arg 1
        3.14f                          // Arg 2
    );
}
```

#### TaskGroupBits

Helper utilities for FreeRTOS event groups (when FREE_RTOS == true).

**Usage Example:**
```cpp
#include <TaskGroupBits.h>

EventGroupHandle_t eventGroup;

#define BIT_WIFI_CONNECTED    (1 << 0)
#define BIT_TIME_SYNCED       (1 << 1)
#define BIT_ALARM_ACTIVE      (1 << 2)

void setup() {
    eventGroup = xEventGroupCreate();
    
    // Set bits
    xEventGroupSetBits(eventGroup, BIT_WIFI_CONNECTED);
    
    // Wait for multiple bits
    EventBits_t bits = xEventGroupWaitBits(
        eventGroup,
        BIT_WIFI_CONNECTED | BIT_TIME_SYNCED,  // Bits to wait for
        pdFALSE,                                // Don't clear on exit
        pdTRUE,                                 // Wait for all bits
        portMAX_DELAY                           // Wait forever
    );
    
    // Check if specific bit is set
    if (bits & BIT_TIME_SYNCED) {
        Serial.println("Time synchronized!");
    }
}
```

---

## Advanced Usage

### Custom Pin Mappings

Override default pins for custom hardware in `board_select.h`:

```cpp
// board_select.h

// Example: Different NeoPixel pin
#undef  LED_DATA_PIN
#define LED_DATA_PIN  12

// Example: Analog buttons on different pins
#undef  S1
#undef  S2
#define S1  A4
#define S2  A5

// Example: Different I2C pins (ESP32)
#undef  I2C_SDA_PIN
#undef  I2C_SCL_PIN
#define I2C_SDA_PIN  21
#define I2C_SCL_PIN  22
```

### Serial Output Control

Fine-tune debugging output in `board_select.h`:

```cpp
// Enable different levels of output
#define SERIAL_OUTPUT   true    // General serial output
#define DEBUG_OUTPUT    true    // Debug messages
#define DEV_CODE        false   // Development code sections

// Configure serial settings
#define SERIAL_BAUD     115200  // Baud rate

// Example usage in your code:
SERIAL_OUT_PRINTLN("This prints when SERIAL_OUTPUT==true");
DEBUG_PRINTLN("This only prints when DEBUG_OUTPUT==true");
SERIAL_PRINTLN("This prints when DEV_CODE==true");
```

### FreeRTOS Task Wrappers

#### Advanced Task Creation

**With Custom Stack and Priority:**
```cpp
class ClockManager {
    void updateTask(uint32_t interval) {
        TickType_t lastWakeTime = xTaskGetTickCount();
        FOREVER {
            // Update clock display
            updateDisplay();
            
            // Delay until next period
            vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(interval));
        }
    }
    
public:
    void start() {
        CreateInstanceTask(
            this,
            &ClockManager::updateTask,
            "ClockUpdate",
            8192,              // 8KB stack
            NULL,
            2,                 // Priority 2 (higher = more important)
            1000               // 1000ms interval
        );
    }
};
```

#### Task with Multiple Parameters

```cpp
class DataProcessor {
    void processTask(const char* name, int id, float threshold, bool enabled) {
        FOREVER {
            if (enabled && checkThreshold(threshold)) {
                Serial.printf("[%s:%d] Processing data\n", name, id);
                processData();
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    
public:
    void startProcessor(const char* processorName, int processorId) {
        CreateInstanceTask(
            this,
            &DataProcessor::processTask,
            "DataProc",
            4096,
            NULL,
            1,
            processorName,     // Arg 1: const char*
            processorId,       // Arg 2: int
            3.14f,             // Arg 3: float
            true               // Arg 4: bool
        );
    }
};
```

### Custom Interface Implementation

Implement the interfaces for custom clock variants:

```cpp
#include <IBinaryClock.h>

class MyCustomClock : public IBinaryClock {
private:
    DateTime currentTime;
    
public:
    // Implement required interface methods
    DateTime get_Time() const override {
        return currentTime;
    }
    
    void set_Time(DateTime value) override {
        currentTime = value;
    }
    
    DateTime ReadTime() override {
        // Read from RTC hardware
        return currentTime;
    }
    
    void DisplayLedPattern(LedPattern pattern) override {
        // Your custom display logic
        switch (pattern) {
            case LedPattern::onColors:
                displayBinaryTime();
                break;
            case LedPattern::rainbow:
                displayRainbow();
                break;
            // ... other patterns
        }
    }
    
    // Implement all other pure virtual methods from IBinaryClockBase...
    // set_Alarm(), get_Alarm(), DisplayBinaryTime(), etc.
};
```

---

## Best Practices

### 1. Configuration Management

✅ **DO:**
- Keep all customizations in `board_select.h`
- Use `#undef` before redefining existing macros
- Document your custom settings
- Version control your `board_select.h`

❌ **DON'T:**
- Modify library header files directly
- Hardcode values in multiple places
- Mix configuration between files

### 2. Interface Usage

✅ **DO:**
- Program to interfaces, not implementations
- Use `IBinaryClock*` for function parameters
- Keep dependencies on abstractions

```cpp
// Good: Depends on interface
void updateDisplay(IBinaryClock* clock) {
    clock->DisplayLedPattern(LedPattern::onColors);
}

// Better: Use reference for non-null
void updateDisplay(IBinaryClock& clock) {
    clock.DisplayLedPattern(LedPattern::onColors);
}
```

❌ **DON'T:**
- Depend on concrete class types unnecessarily
- Cast interfaces back to implementations

### 3. Structure Usage

✅ **DO:**
- Initialize structures properly
- Use `.clear()` methods when available
- Validate data before using

```cpp
AlarmTime alarm;
alarm.clear();                          // Initialize
alarm.number = 1;
alarm.time = DateTime(2026, 2, 16, 7, 30, 0);
alarm.status = 1;

// Validate before using
if (alarm.status && alarm.time.isValid()) {
    setAlarm(alarm);
}
```

### 4. Conditional Compilation

✅ **DO:**
- Check feature flags before using optional features
- Provide alternatives for limited boards

```cpp
#if STL_USED
    std::vector<Note> melody;
    Note note;
    note.tone = 440;         // 440 Hz (A4)
    note.duration = 250;     // 250ms
    melody.push_back(note);
#else
    // Fallback for boards without STL
    playDefaultMelody();
#endif
```

### 5. Serial Output

✅ **DO:**
- Use SERIAL_OUT_*/SERIAL_*/DEBUG_* macros instead of direct Serial calls
- They automatically remove code when disabled

```cpp
// Good: Compiled out when SERIAL_OUTPUT==false
SERIAL_OUT_PRINTLN("Initializing...");
DEBUG_PRINTLN("Debug value: " + String(value));

// Bad: Always compiled in
Serial.println("Initializing...");
```

---

## Troubleshooting

### Compilation Errors

**Error: "IBinaryClock.h: No such file or directory"**
- **Cause:** Library not installed or not in library path
- **Solution:** 
  - Verify BCGlobalDefines is in `lib/` folder or Arduino libraries
  - Check library.json or library.properties exists
  - Restart IDE and rebuild

**Error: "std::vector not found"**
- **Cause:** Trying to use STL on unsupported board (e.g., UNO R3)
- **Solution:**
  - Check board selection
  - Wrap STL code in `#if STL_USED` blocks
  - Use alternative data structures for limited boards

**Error: "LED_DATA_PIN was not declared"**
- **Cause:** Missing board definition or board_select.h
- **Solution:**
  - Create `board_select.h` with appropriate board define
  - Or define `CUSTOM_UNO true` with all required pins

**Error: Multiple definition of...**
- **Cause:** Including implementation files (.cpp) or missing include guards
- **Solution:**
  - Only include header files (.h)
  - Verify all headers have `#pragma once` or include guards

### Runtime Issues

**WiFi structures undefined**
- **Symptom:** `WiFiCredentials` not found
- **Cause:** WiFi features disabled for current board
- **Solution:**
  - Verify your board supports WiFi
  - Check `WIFI`, `ESP32_WIFI`, or `WIFIS3` defines
  - Conditional compilation around WiFi code

**Interface methods not available**
- **Symptom:** Method calls fail on interface pointer
- **Cause:** Using wrong interface type
- **Solution:**
  - Use `IBinaryClock*` for extended functionality
  - Use `IBinaryClockBase*` for basic operations
  - Check method availability in interface definition

**FreeRTOS functions not found**
- **Symptom:** `xTaskCreate`, `vTaskDelay` undefined
- **Cause:** Board doesn't support FreeRTOS
- **Solution:**
  - Check `FREE_RTOS` define value
  - Use appropriate board (ESP32, UNO R4)
  - Wrap FreeRTOS code in `#if FREE_RTOS` blocks

### Configuration Issues

**Custom pins not working**
- **Cause:** board_select.h not included or overridden later
- **Solution:**
  - Ensure board_select.h is in correct location
  - Use `#undef` before redefining
  - Check library inclusion order

**Serial output not appearing**
- **Cause:** SERIAL_OUTPUT disabled or wrong baud rate
- **Solution:**
  - Check `SERIAL_OUTPUT` definition
  - Verify serial monitor baud rate matches `SERIAL_BAUD`
  - Ensure `Serial.begin()` called in setup()

### Memory Issues (UNO R3)

**Sketch too large**
- **Cause:** Too many features enabled for limited memory
- **Solution:**
  - Disable unused features in board_select.h:
    ```cpp
    #define SERIAL_OUTPUT  false  // Saves ~2KB
    #define DEBUG_OUTPUT   false
    #define DEV_CODE       false
    ```
  - Reduce melody count
  - Simplify display patterns

**Stack overflow / crashes**
- **Cause:** Insufficient stack for variables or recursion
- **Solution:**
  - Use static or global variables instead of local
  - Avoid deep recursion
  - Reduce string/buffer sizes

---

## Additional Resources

- [BCGlobalDefines Class Diagram](BCGlobalDefines_LibClassDiagram.md)
- [WiFiBinaryClock Project Documentation](https://github.com/Chris-70/WiFiBinaryClock)
- [BinaryClock Library Usage Guide](../BinaryClock/BinaryClock_LibInstallUseage.md)
- [BinaryClockWiFi Library Usage Guide](../BinaryClockWiFi/BinaryClockWiFi_LibInstallUsage.md)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [Arduino Reference](https://www.arduino.cc/reference/en/)

---

*For questions, issues, or contributions, please visit the [WiFiBinaryClock repository](https://github.com/Chris-70/WiFiBinaryClock).*
