# BinaryClock Library - Installation and Usage Guide

This document provides detailed instructions on how to install and use the BinaryClock library, which is the core component of the WiFi Binary Clock project. It covers installation steps for both PlatformIO and Arduino IDE, as well as examples of how to utilize the library's features for time management, alarm configuration, LED display control, button handling, and melody playback.

## Additional Documents
- [**Project `README.md`**][README_Project] (GitHub: [`README.md`][README_ProjectGit]) - Overview of the **WiFi Binary Clock project**.
- [**Library `README.md`**][README] (GitHub: [`README.md`][README_Git]) - Overview and usage instructions for the **BinaryClock library**.
- [**Library `ClassDiagram.md`**][CLASS_DIAGRAM] (GitHub: [`ClassDiagram.md`][CLASS_DIAGRAM_GIT]) - Class diagram and structure of the **BinaryClock library**.
---

## Installation

### PlatformIO

Add to your `platformio.ini`:
```ini
lib_deps = 
    https://github.com/Chris-70/WiFiBinaryClock.git#lib/BinaryClock
```

### Arduino IDE

1. **Download the Repository**:
   - Go to https://github.com/Chris-70/WiFiBinaryClock
   - Click the green **Code** button and select **Download ZIP**
   - Extract the ZIP file to a temporary location

2. **Install the BinaryClock Library**:
   - Open Arduino IDE
   - Go to **Sketch → Include Library → Add .ZIP Library...**
   - Navigate to the extracted folder: `WiFiBinaryClock-main/lib/BinaryClock`
   - Compress the `BinaryClock` folder into a ZIP file if needed
   - Select the ZIP file and click **Open**

3. **Install Dependencies**:
   Repeat step 2 for each required dependency:
   - `lib/BCGlobalDefines` (from the same repository)
   - `lib/MorseCodeLED` (from the same repository)
   - `lib/RTClibPlus` (from the same repository)

4. **Install External Libraries**:
   - Go to **Sketch → Include Library → Manage Libraries...**
   - Search for and install:
     - **FastLED** (by Daniel Garcia)
     - **Streaming** (by Mikal Hart)
     - **Adafruit BusIO** (by Adafruit)

5. **Verify Installation**:
   - Go to **File → Examples → BinaryClock**
   - Open an example sketch and compile to verify installation

**Note:** For easier dependency management and better build configuration, we recommend using PlatformIO instead of Arduino IDE.

## Quick Start

### Basic Clock Setup

```cpp
#include <BinaryClock.h>

BinaryClock& clock = BinaryClock::get_Instance();

void setup() {
    Serial.begin(115200);
    
    // Initialize clock with LED test patterns
    clock.setup(true);  // true = show test patterns on startup
    
    Serial.println("Binary Clock Ready!");
}

void loop() {
    clock.loop();  // Handles button input and menu system
}
```
___

## Configuration

Edit the file [board_select.h][boardselect] to customize for your hardware:

### Board-Specific Configuration

To use one of the supported boards, select one of the defines below and add it to the [`board_select.h`][boardselect] file to enable the configuration for your board. This will automatically set the appropriate pin definitions and capabilities for that board. Only one board define can be active at a time.
```cpp
#define ESP32_D1_R32_UNO     // If defined, the code will use Wemos D1 R32 ESP32 UNO board definitions     (ESP32 WiFi)
#define METRO_ESP32_S3       // If defined, the code will use Adafruit Metro ESP32-S3 board definitions    (ESP32 WiFi)
#define METRO_ESP32_S2       // If defined, the code will use Adafruit Metro ESP32-S2 board definitions    (ESP32 WiFi)
#define ESP32_S3_UNO         // If defined, the code will use generic ESP32-S3 UNO board definitions       (ESP32 WiFi)
#define UNO_R4_WIFI          // If defined, the code will use Arduino UNO R4 WiFi board definitions        (WiFiS3)
#define UNO_R4_MINIMA        // If defined, the code will use Arduino UNO R4 Minima board definitions      (No WiFi)
#define UNO_R3               // If defined, the code will use Arduino UNO R3 (ATMEL 328) board definitions (NO WiFi)
```

To change the configuration defaults for your board/application, make these changes in the [`board_select.h`][boardselect] file.

```cpp
// Board capabilities and configuration
#define ESP32_WIFI            true    ///< Set to true if the board has onboard ESP32 based WiFi; false otherwise.
#define WIFIS3                false   ///< Set to true if the board has onboard WIFIS3 based WiFi (UNO R4 WiFi); false otherwise.
#define FREE_RTOS             true    ///< Set to true if the board is running FreeRTOS, e.g. boards with an ESP32.
#define STL_USED              true    ///< Set to true if the board can use the C++ STL library (i.e. has enough memory).
#define LED_HEART      LED_BUILTIN    ///< Heartbeat LED to show working software, errors or messages.
#define PRINTF_OK             true    ///< Use printf style code if supported, usually true.
#define ESP32_INPUT_PULLDOWN  INPUT_PULLDOWN   ///< Pin has an internal pull-down resistor (e.g. ESP32) or just use `INPUT` (e.g. Arduino).

// Initial default values for properties that can be changed at run-time.
#define DEFAULT_DEBOUNCE_DELAY    75   ///< The default debounce delay in milliseconds for the buttons
#define DEFAULT_BRIGHTNESS        30   ///< The best tested LEDs brightness range: 20-60
#define DEFAULT_ALARM_REPEAT       3   ///< How many times to play the melody alarm
#define DEFAULT_SERIAL_SPEED   115200  ///< Default serial output speed in bps
// Time format string definitions names and values used for formatting time and alarm display. 
// #define TIME_FORMAT_24HR    "HH:mm:ss"    ///< 24 Hour time format string
// #define TIME_FORMAT_AMPM    "HH:mm:ss AP" ///< 12 Hour time format string with AM/PM
// #define ALARM_FORMAT_24HR   "HH:mm"       ///< 24 Hour alarm format string
// #define ALARM_FORMAT_AMPM   "HH:mm AP"    ///< 12 Hour alarm format string with AM/PM
#define DEFAULT_TIME_MODE    AMPM_MODE    ///< Default time mode: AMPM_MODE or HR24_MODE
#define DEFAULT_TIME_FORMAT  TIME_FORMAT_AMPM  ///< Default time  format, or TIME_FORMAT_24HR 
#define DEFAULT_ALARM_FORMAT ALARM_FORMAT_AMPM ///< Default alarm format, or ALARM_FORMAT_24HR
```
### Custom Board Support

If your board is not supported out of the box, you can create a custom configuration by defining `CUSTOM_UNO` and providing the necessary pin definitions and capabilities in the [`board_select.h`][boardselect] file. This allows you to use the library with a wide range of hardware by simply specifying the correct pin mappings and features.

```cpp
#define CUSTOM_UNO true

// Define all required pins and capabilities
// example Arduino UNO based pin definitions (R3 & R4). Modify for your board.
#define RTC_INT            3   ///< Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
#define PIEZO             11   ///< The number of the Piezo pin
#define LED_DATA_PIN      A3   ///< Data Out pin that the LED data will be written to.

// Push buttons S1; S2; and S3 connected to the: A2, A1, A0 Arduino pins
#define S1                A2   ///< A2: S1 button: Time set  & Decrement button   
#define S2                A1   ///< A1: S2 button: Select    & Confirm/Save button  
#define S3                A0   ///< A0: S3 button: Alarm set & Increment button  

// I2C pins used by the shield on Arduino boards
#define I2C_SDA_PIN      PC4   ///< SDA pin (A4) for Arduino UNO_R3, PC4 position near Reset button.
#define I2C_SCL_PIN      PC5   ///< SCL pin (A5) for Arduino UNO_R3, PC5 position near Reset button.
```
---

### Setting the Time and Alarm

The alarm and time can be set by pressing either the __`S3`__ or __`S1`__ buttons respectivly.  

Alternatively, you can set the time programmatically using the `set_Alarm()` or `set_Time()` methods of the `BinaryClock` class. The [`BinaryClockWiFi`][BinaryClockWiFi] library uses these methods to automatically synchronize time using SNTP when connected to WiFi.

#### Setting Initial Time Example

```cpp
#include <BinaryClock.h>
#include <DateTime.h>

void setClockTime() {
    BinaryClock& clock = BinaryClock::get_Instance();
    
    // Create DateTime object (year, month, day, hour24, minute, second)
    DateTime newTime(2026, 2, 15, 14, 30, 0);
    
    // Set clock time
    clock.set_Time(newTime);
    
    Serial.print("Time set to: ");
    char buffer[32];
    Serial.println(clock.DateTimeToString(newTime, buffer, 32, "YYYY-MM-DD HH:mm:ss"));
}
```

#### Setting an Alarm Example

```cpp
#include <BinaryClock.h>

void setAlarm() {
    BinaryClock& clock = BinaryClock::get_Instance();

    // Set a daily alarm for 7:30 (i.e. 7:30 AM)
    AlarmTime alarm(7, 30);                   // Time: (hour24, minute)
    clock.set_Alarm(alarm);

    Serial.println("Alarm set for 7:30 AM");
} 

void setWeeklyAlarm() {
    BinaryClock& clock = BinaryClock::get_Instance();

    // Set a weekly alarm for Friday at 14:45 (i.e. 2:45 PM)
    // RTC_FRIDAY is always Friday regardless of the first day of the week configuration.
    // See: `FIRST_WEEKDAY` definition in `DateTime.h`
    AlarmTime alarm(14, 45, AlarmTime::Weekly, RTC_FRIDAY); 
    clock.set_Alarm(alarm);

    Serial.println("Alarm set for 2:45 PM");
}
```

## Core API Reference

### BinaryClock Class

The main singleton class managing all clock functionality.

#### Singleton Access

```cpp
// Get singleton instance
static BinaryClock& get_Instance();
```

#### Initialization

```cpp
// Initialize clock hardware and display
void setup(bool testLeds = false);

// Main processing loop (call in Arduino loop())
void loop();
```

**Example:**
```cpp
void setup() {
    BinaryClock& clock = BinaryClock::get_Instance();
    clock.setup(true);  // Show LED test patterns
}

void loop() {
    clock.loop();
}
```

---

#### Time Management

```cpp
// Set current time
void set_Time(DateTime value);

// Get current time
DateTime get_Time();

// Read time directly from RTC
DateTime ReadTime();

// Format DateTime to string
static char* DateTimeToString(DateTime time, char* buffer, 
                              size_t size, const char* format);
```

**Example: Reading and Formatting Time**
```cpp
BinaryClock& clock = BinaryClock::get_Instance();

DateTime now = clock.get_Time();
char buffer[32];

// Format as "2026-02-15 02:30:00"
clock.DateTimeToString(now, buffer, 32, "YYYY-MM-DD HH:mm:ss");
Serial.println(buffer);

// Format as "02/15/2026 02:30 AM"
clock.DateTimeToString(now, buffer, 32, "MM/DD/YYYY hh:mm AP");
Serial.println(buffer);

// Format as "Saturday, February 15"
clock.DateTimeToString(now, buffer, 32, "DDD, MMM DD");
Serial.println(buffer);
```

**Format Codes:**
   The following lists all the format specifiers are supported in the format string:

   | Specifier   | Output                                   | Value range       | Status   |
   |-------------|------------------------------------------|-------------------|----------|
   | `YYYY`      | the year as a 4-digit number             | (2000--2199)      | Modified |
   | `YY`        | the year as a 2-digit number             | (00--99)          |          |
   | `MM`        | the month as a 2-digit number            | (01--12)          |          |
   | `MMM`       | the abbreviated English month name       | ("Jan"--"Dec")    |          |
   | `DD`        | the day as a 2-digit number              | (01--31)          |          |
   | `DDD`       | the abbreviated English day of the week  | ("Mon"--"Sun")    |          |
   | `AP`        | either "AM" or "PM"                      | (AM/PM)           | New      |
   | `ap`        | either "am" or "pm"                      | (am/pm)           | New      |
   | `hh`        | the hour as a 2-digit number             | (00--23 / 01--12) | Modified |
   | `HH`        | the hour as a 1/2-digit number/space     | ( 0--23 /  1--12) | New      |
   | `mm`        | the minute as a 2-digit number           | (00--59)          |          |
   | `ss`        | the second as a 2-digit number           | (00--59)          |          |

   If either "__AP__" or "__ap__" is used, then the "__hh__" and "__HH__" specifiers use a 12-hour mode
   (range: 01--12). Otherwise they use a 24-hour mode (range: 00--23).

---

#### 12/24 Hour Format

```cpp
// Set 12-hour or 24-hour format
void set_Is12HourFormat(bool value);

// Check current format
bool get_Is12HourFormat();

// Get format strings
char* get_TimeFormat();   // Returns "HH:mm:ss AP" or "hh:mm:ss"
char* get_AlarmFormat();  // Returns "hh:mm AP"    or "hh:mm"
```

**Example: Toggle Time Format**
```cpp
BinaryClock& clock = BinaryClock::get_Instance();

// Switch to 12-hour format
clock.set_Is12HourFormat(true);
Serial.println(clock.get_TimeFormat());  // Prints: HH:mm:ss AP

// Switch to 24-hour format
clock.set_Is12HourFormat(false);
Serial.println(clock.get_TimeFormat());  // Prints: hh:mm:ss
```

---

#### Alarm Management

```cpp
// Set alarm time
void set_Alarm(AlarmTime value);

// Get current alarm settings
AlarmTime get_Alarm();

// Get alarm settings directly from RTC (alarm 1 or 2)
AlarmTime GetRtcAlarm(int number);

// Play the configured alarm melody
void PlayAlarm();
void PlayAlarm(AlarmTime alarm);
```

**AlarmTime Structure:**
```cpp
struct AlarmTime {
    uint8_t  number;   // Alarm number: 1 or 2
    DateTime time;     // Alarm time
    uint8_t  melody;   // Melody ID (0 = default)
    uint8_t  status;   // 0 = OFF, 1 = ON
    Repeat   freq;     // Repeat mode (Never, Hourly, Daily, Weekly(*), Monthly)
    bool     fired;    // True while alarm is active/ringing
};
// (*) Weekly uses the `DateTime::day()` i.e. `d` field of the date in `time` for 
//     day of the week (1 - 7) where 1 is the first day of the week, 7 is the last.
//     This is defined by `FIRST_WEEKDAY` in `DateTime.h` (e.g. "Mon").
```

**Example: Daily Alarm**
```cpp
BinaryClock& clock = BinaryClock::get_Instance();

void setDailyAlarm(uint8_t hour24, uint8_t minute) {
    AlarmTime alarm = clock.get_Alarm();

    alarm.number = 2;
    alarm.time = DateTime(hour24, minute, 0);
    alarm.status = 1;
    alarm.freq = AlarmTime::Daily;
    alarm.fired = false;

    clock.set_Alarm(alarm);
    Serial.printf("Alarm set for %02u:%02u\n", hour24, minute);
}
```

---

#### Display Management

```cpp
// Display binary time on LEDs
void DisplayBinaryTime(uint8_t h, uint8_t m, uint8_t s, bool use12Hr);

// Display predefined patterns
void DisplayLedPattern(LedPattern pattern);
void DisplayLedPattern(LedPattern pattern, unsigned long duration);

// Set display brightness (0-255)
void set_Brightness(byte value);
byte get_Brightness();
```

**LedPattern Enum:**
```cpp
enum class LedPattern : uint8_t {
    onColors,
    offColors,
    onText,
    offTxt,
    xAbort,
    okText,
    rainbow
    // WiFi builds add: wText, aText, pText, nText
};
```

**Example: Custom Display Patterns**
```cpp
BinaryClock& clock = BinaryClock::get_Instance();

// Show success pattern for 2 seconds
clock.DisplayLedPattern(LedPattern::okText, 2000);

// Show rainbow pattern indefinitely
clock.DisplayLedPattern(LedPattern::rainbow);

// Set brightness to 50%
clock.set_Brightness(128);
```

Note: `DisplayLedBuffer(...)` is not a public API in `BinaryClock`; use `DisplayLedPattern(...)` from application code.

---

#### Button Access

```cpp
// Get button interface references
const IBCButtonBase& get_S1TimeDec();   // Button 1 (Time Dec)
const IBCButtonBase& get_S2SaveStop();  // Button 2 (Save/Stop)
const IBCButtonBase& get_S3AlarmInc();  // Button 3 (Alarm Inc)
```

**Example: Button State Monitoring**
```cpp
BinaryClock& clock = BinaryClock::get_Instance();

void checkButtons() {
    const IBCButtonBase& s1 = clock.get_S1TimeDec();
    const IBCButtonBase& s2 = clock.get_S2SaveStop();
    const IBCButtonBase& s3 = clock.get_S3AlarmInc();

    // Raw state access is const-safe.
    if (s1.IsPressedRaw()) {
        Serial.println("S1 is currently pressed (raw).");
    }

    if (s2.IsPressedRaw()) {
        Serial.println("S2 is currently pressed (raw).");
    }

    if (s3.IsPressedRaw()) {
        Serial.println("S3 is currently pressed (raw).");
    }
}
```

Note: debounced edge detection uses `IsPressedNew()`, which is non-const; if you need it from these const accessors, cast carefully and consistently in your own wrapper code.

---

#### Melody Management

```cpp
// Play melody by ID
bool PlayMelody(size_t id);

// Register custom melody
size_t RegisterMelody(const std::vector<Note>& melody);

// Get registered melody
const std::vector<Note>& GetMelodyById(size_t id) const;

// Set alarm melody from array (only on non-STL builds)
bool SetAlarmMelody(Note* array, size_t size);
```

**Note Structure:**
```cpp
struct Note {
    unsigned tone;           // Hz (e.g., 440 for A4)
    unsigned long duration;  // Milliseconds
};
```

**Example: Custom Alarm Melody**
```cpp
BinaryClock& clock = BinaryClock::get_Instance();

// Define custom melody (C major scale)
std::vector<Note> customMelody = {
    {262U, 200UL},  // C4
    {294U, 200UL},  // D4
    {330U, 200UL},  // E4
    {349U, 200UL},  // F4
    {392U, 200UL},  // G4
    {440U, 200UL},  // A4
    {494U, 200UL},  // B4
    {523U, 400UL}   // C5
};

// Register and assign as alarm melody
size_t melodyId = clock.RegisterMelody(customMelody);
Serial.printf("Melody registered with ID: %zu\n", melodyId);

// Play immediately for testing
clock.PlayMelody(melodyId);

AlarmTime alarm = clock.get_Alarm();
alarm.melody = static_cast<uint8_t>(melodyId);
alarm.status = 1;
clock.set_Alarm(alarm);
```

For non-STL builds, use `SetAlarmMelody(...)` instead of melody registry IDs.

**Common Note Frequencies:**
```cpp
// Octave 4 (Middle octave)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494

// Octave 5
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
```

---

#### Callback Registration

```cpp
// Register time change callback (called every second)
bool RegisterTimeCallback(void (*callback)(const DateTime&));
bool UnregisterTimeCallback(void (*callback)(const DateTime&));

// Register alarm trigger callback
bool RegisterAlarmCallback(void (*callback)(const DateTime&));
bool UnregisterAlarmCallback(void (*callback)(const DateTime&));
```

**Example: Time and Alarm Callbacks**
```cpp
BinaryClock& clock = BinaryClock::get_Instance();

// Time callback - called every second
void onTimeChange(const DateTime& time) {
    static int lastMinute = -1;
    
    if (time.minute() != lastMinute) {
        lastMinute = time.minute();
        Serial.printf("Time: %02d:%02d\n", time.hour(), time.minute());
    }
}

// Alarm callback - called when alarm triggers
void onAlarmTriggered(const DateTime& time) {
    Serial.println("ALARM TRIGGERED!");
    Serial.printf("Alarm time: %02d:%02d\n", time.hour(), time.minute());
    
    // Custom alarm action (e.g., turn on lights, send notification)
}

void setup() {
    BinaryClock& clock = BinaryClock::get_Instance();
    clock.setup();
    
    // Register callbacks
    clock.RegisterTimeCallback(onTimeChange);
    clock.RegisterAlarmCallback(onAlarmTriggered);
}
```

---

#### Utility Functions

```cpp
// Get clock identifier string
const char* get_IdName();

// Check serial time mode
bool get_IsSerialTime();

// Check serial setup mode
bool get_IsSerialSetup();

// Flash LED for debugging
void FlashLed(uint8_t ledNum, uint8_t repeat = 1,
              uint8_t dutyCycle = 50, uint8_t frequency = 1,
              uint8_t onValue = CC_ON) const;
```

**Example: Debug LED Flashing**
```cpp
BinaryClock& clock = BinaryClock::get_Instance();
// Flash built-in LED 5 times, 50% duty at 2 Hz.
clock.FlashLed(LED_BUILTIN, 5, 50, 2, CC_ON);
```

---

### BCButton Class

Handles button debouncing and event detection.

**Key Features:**
- Debounced edge detection (`IsPressedNew()`)
- Stable pressed-state checks (`IsPressed()`)
- Raw pin reads (`IsPressedRaw()`)
- Configurable debounce delay

**Usage through IBCButtonBase interface:**
```cpp
const IBCButtonBase& button = clock.get_S1TimeDec();

// Const-safe raw state check
bool rawPressed = button.IsPressedRaw();

// Get timing information
unsigned long lastRead = button.get_LastReadTime();

// For debounced edge detection (non-const API), cast only if needed:
// bool pressedNow = const_cast<IBCButtonBase&>(button).IsPressedNew();
```

---

### BCMenu Class

Manages the settings menu system for configuring clock parameters.

**Menu Navigation:**
- **Button 1 (S1)**: Decrement value / Navigate up
- **Button 2 (S2)**: Confirm / Save / Next
- **Button 3 (S3)**: Increment value / Navigate down

**Menu Modes:**
- **Time Setting**: Adjust hours, minutes, seconds
- **Alarm Setting**: Configure alarm time
- **Format Setting**: Toggle 12/24 hour display
- **Brightness**: Adjust LED brightness

**Automatic Access:**
The menu system is automatically managed by `clock.loop()`. Users enter menu mode by:
1. Long press Button 2 (S2) to enter settings
2. Use Button 1/3 to adjust values
3. Press Button 2 to confirm and move to next setting
4. Long press Button 2 to save and exit

---

## Advanced Usage

### Custom Color Schemes

```cpp
BinaryClock& clock = BinaryClock::get_Instance();

void setCustomColors() {
    DateTime now = clock.get_Time();
    uint8_t hour = now.hour();
    uint8_t minute = now.minute();
    uint8_t second = now.second();
    
    // Display with custom color calculation
    clock.DisplayBinaryTime(hour, minute, second, clock.get_Is12HourFormat());
    
    // Access LED array for direct manipulation (advanced)
    // Note: This requires modification of the library to expose LED array
}
```

### Time Synchronization with RTC

```cpp
BinaryClock& clock = BinaryClock::get_Instance();

void syncWithRTC() {
    // Read directly from RTC module
    DateTime rtcTime = clock.ReadTime();
    
    // Update system time
    clock.set_Time(rtcTime);
    
    char buffer[32];
    Serial.print("Synced to RTC: ");
    Serial.println(clock.DateTimeToString(rtcTime, buffer, 32, "HH:mm:ss"));
}
```

### Alarm with Custom Actions

```cpp
BinaryClock& clock = BinaryClock::get_Instance();

void onAlarmEvent(const DateTime& alarmTime) {
    BinaryClock& localClock = BinaryClock::get_Instance();
    Serial.println("Smart Alarm Triggered!");

    // Gradually increase brightness
    for (int brightness = 0; brightness <= 255; brightness += 5) {
        localClock.set_Brightness(static_cast<byte>(brightness));
        delay(100);
    }

    // Play alarm sound
    localClock.PlayAlarm();

    // Additional actions: send notification, turn on lights, etc.
}

void setupSmartAlarm() {
    // Register function-pointer callback for alarm event.
    clock.RegisterAlarmCallback(onAlarmEvent);
}
```

### Periodic Time Display Updates

```cpp
BinaryClock& clock = BinaryClock::get_Instance();
unsigned long lastUpdate = 0;

void updateTimeDisplay() {
    unsigned long now = millis();
    
    // Update every 5 minutes
    if (now - lastUpdate >= 300000) {
        lastUpdate = now;
        
        DateTime currentTime = clock.get_Time();
        char buffer[32];
        clock.DateTimeToString(currentTime, buffer, 32, "HH:mm");
        
        Serial.print("Time Update: ");
        Serial.println(buffer);
    }
}

void loop() {
    clock.loop();
    updateTimeDisplay();
}
```

### Pattern-Based Time Indication

```cpp
void showTimePattern() {
    BinaryClock& clock = BinaryClock::get_Instance();
    DateTime now = clock.get_Time();
    
    // Show different patterns based on time
    if (now.hour() == 0 && now.minute() == 0) {
        // Midnight - special pattern
        clock.DisplayLedPattern(LedPattern::rainbow, 5000);
    } else if (now.minute() == 0) {
        // Top of hour - brief flash
        clock.DisplayLedPattern(LedPattern::okText, 500);
    }
    
    // Resume normal time display
    clock.DisplayBinaryTime(now.hour(), now.minute(), now.second(), 
                           clock.get_Is12HourFormat());
}
```

---

## Troubleshooting

### RTC Not Detected

**Symptoms:** Clock enters "Purgatory" mode with Morse code error on LED

**Solutions:**
```cpp
// Check I2C connection
#include <Wire.h>

void scanI2C() {
    Wire.begin();
    Serial.println("Scanning I2C bus...");
    
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("Device found at 0x%02X\n", addr);
        }
    }
    // DS3231 should appear at 0x68
}
```

**Checklist:**
- Verify DS3231 is connected to correct I2C pins (SDA/SCL)
- Check power supply to RTC module (3.3V or 5V depending on module)
- Ensure pull-up resistors on I2C lines (usually built-in)
- Test with Arduino Wire scanner sketch

---

### LEDs Not Working

**Symptoms:** LEDs remain off or show incorrect colors

**Solutions:**
```cpp
// Test LED connection
void testLEDs() {
    BinaryClock& clock = BinaryClock::get_Instance();
    
    // Show test patterns
    clock.DisplayLedPattern(LedPattern::rainbow, 3000);
    
    // Test individual brightness
    for (int brightness = 0; brightness <= 255; brightness += 51) {
        clock.set_Brightness(brightness);
        delay(500);
    }
}
```

**Checklist:**
- Verify NeoPixel data pin matches `NEOPIXEL_PIN` in board_select.h
- Check 5V power supply to LED strip (adequate current capacity)
- Ensure common ground between microcontroller and LED power
- Test with simple FastLED blink sketch
- Add 330Ω resistor on data line if signal integrity issues
- Add 1000µF capacitor across LED power supply

---

### Buttons Not Responding

**Symptoms:** Button presses not detected or stuck

**Solutions:**
```cpp
// Debug button states
void debugButtons() {
    BinaryClock& clock = BinaryClock::get_Instance();
    
    const IBCButtonBase& s1 = clock.get_S1TimeDec();
    const IBCButtonBase& s2 = clock.get_S2SaveStop();
    const IBCButtonBase& s3 = clock.get_S3AlarmInc();
    
    Serial.printf("S1: %d, S2: %d, S3: %d\n",
                  s1.IsPressedRaw(),
                  s2.IsPressedRaw(),
                  s3.IsPressedRaw());
}
```

**Checklist:**
- Verify button pins match configuration in board_select.h
- Check button wiring (normally open, pull-down/pull-up)
- Test button continuity with multimeter
- Adjust `DEBOUNCE_DELAY` if needed (default 50ms)
- Ensure buttons are connected to interrupt-capable pins if using interrupts

---

### Alarm Not Triggering

**Symptoms:** Alarm time reached but no sound

**Solutions:**
```cpp
// Verify alarm configuration
void checkAlarm() {
    BinaryClock& clock = BinaryClock::get_Instance();
    
    AlarmTime alarm = clock.get_Alarm();
    Serial.printf("Alarm: %02d:%02d:%02d, Status: %u, Melody: %u\n",
                  alarm.time.hour(), alarm.time.minute(), alarm.time.second(),
                  alarm.status, alarm.melody);
    
    // Force alarm test
    clock.PlayAlarm();
}
```

**Checklist:**
- Ensure `alarm.status` is 1
- Verify piezo buzzer connected to correct pin
- Test buzzer with tone() function directly
- Check if alarm time is in correct format (12/24 hour)
- Verify RTC alarm is properly configured

---

### Time Drifting

The DS3231 is a high-precision RTC with built-in temperature compensation, but it can still experience drift over time due to aging or environmental factors. The [**BinaryClockWiFi**][BinaryClockWiFi] library will synchronize time with NTP servers when WiFi is available, which can help mitigate drift issues. 

**Symptoms:** Clock loses or gains time

**Solutions:**
```cpp
// Check RTC accuracy for gross errors. 
// To check with better precision, compare the time over 1 month against a stable time source.
void monitorRTCDrift() {
    BinaryClock& clock = BinaryClock::get_Instance();
    
    DateTime start = clock.ReadTime();
    delay(86400000);  // Wait 1 day
    DateTime end = clock.ReadTime();
    
    int32_t drift = (end.unixtime() - start.unixtime()) - 86400;
    Serial.printf("Drift: %ld seconds per day\n", drift);
}
```

**Checklist:**
- Replace RTC battery if low (CR2032 typically)
- Verify RTC crystal is not damaged
- Check ambient temperature (DS3231 has built-in compensation)
- Sync with NTP if WiFi available (using BinaryClockWiFi)
- Calibrate RTC aging offset (advanced, see DS3231 datasheet)

---

### Out of Memory (Arduino UNO R3)

The Arduino UNO R3 is limited by the 32KB of Flash memory for this project. The `BinaryClock` library is designed to compile for the UNO R3 by only implementing the `IBinaryClockBase` interface, which removes some features to fit within the memory constraints. This project makes heavy use of interfaces for dependency injection, classes, modern C++ features, and a large number of functions and features that are not compatible with the UNO R3's limited resources. Compared to the original shield [**Example 11**][Example_11], on an UNO R3, the `BinaryClock` library, from a UX point of view, only adds 12 hour AM/PM mode and initial menu selection to cancel the change for **Time** and **Alarm** settings before making any changes when a button was pressed in error.

The full capability of the shield is only exposed when using an ESP32 based board, such as the [**Adafruit Metro ESP32-S3**][metro], which supports WiFi and Bluetooth connectivity, has more Flash and RAM, and supports FreeRTOS features for better performance and responsiveness. The WiFi connectivity allows the shield to synchronize time with NTP servers, make changes to the time colors and alarm melodies.

**Symptoms:** Sketch won't compile or runs erratically

**Solutions:**
```cpp
// Check memory usage
#include <AvailableMemory.h>

void checkMemory() {
    Serial.print("Free RAM: ");
    Serial.println(availableMemory());
}
```

**Optimization for UNO R3:**
- Disable serial output: `#define SERIAL_TIME false`
- Reduce melody count: smaller alarm melody arrays
- Disable test patterns on startup: `clock.setup(false)`
- Use PROGMEM for large constant arrays
- Consider upgrading to UNO R4 or ESP32

---

## Performance Considerations

### Memory Usage

| Board Type | Flash | RAM | Features |
|------------|-------|-----|----------|
| **UNO R3** | ~28 KB | ~1.8 KB | Basic (IBinaryClockBase) |
| **UNO R4** | ~35 KB | ~2.5 KB | Standard (no FreeRTOS) |
| **ESP32** | ~45 KB | ~8 KB | Full (FreeRTOS + callbacks) |

### CPU Usage

- **UNO R3**: ~80% in loop() - time display and button handling
- **UNO R4/ESP32**: ~30% in loop() - only button handling (time on FreeRTOS task)

### LED Update Rate

- **Binary time display**: 1 Hz (once per second via RTC interrupt)
- **Pattern animations**: Static up to ~30 FPS (configurable)
- **Button response**: <50ms debounce delay

### Button Debouncing

- **Default debounce**: controlled by `DEFAULT_DEBOUNCE_DELAY` (project default is commonly 75ms)
- **Per-button override**: `set_DebounceDelay(unsigned long)` on each button interface
- **Edge detection API**: use `IsPressedNew()` when you need debounced OFF->ON transitions

---

## Best Practices

### Initialization Order

```cpp
void myTimeCallback(const DateTime& now) {
    // User time callback code
}

void myAlarmCallback(const DateTime& alarmTime) {
    // User alarm callback code
}

void setup() {
    // 1. Serial first for debugging
    Serial.begin(115200);
    
    // 2. Initialize clock
    BinaryClock& clock = BinaryClock::get_Instance();
    clock.setup(true);  // Test LEDs on startup
    
    // 3. Register callbacks AFTER setup
    clock.RegisterTimeCallback(myTimeCallback);
    clock.RegisterAlarmCallback(myAlarmCallback);
    
    // 4. Set initial time/alarm
    clock.set_Time(DateTime(F(__DATE__), F(__TIME__)));
    
    // 5. Other peripherals
    // WiFi, sensors, etc.
}
```

### Power Management

```cpp
// Reduce power consumption
void enterLowPowerMode() {
    BinaryClock& clock = BinaryClock::get_Instance();
    
    // Dim LEDs
    clock.set_Brightness(32);  // ~12% brightness
    
    // Turn off display during night hours
    DateTime now = clock.get_Time();
    if (now.hour() >= 23 || now.hour() < 6) {
        clock.DisplayLedPattern(LedPattern::offColors);
    }
}
```

### Resource Management

```cpp
void cleanup() {
    BinaryClock& clock = BinaryClock::get_Instance();
    
    // Unregister callbacks
    clock.UnregisterTimeCallback(myTimeCallback);
    clock.UnregisterAlarmCallback(myAlarmCallback);
    
    // Turn off display
    clock.DisplayLedPattern(LedPattern::offColors);
    
    // Singleton persists - no explicit cleanup needed
}
```

### Error Handling

```cpp
void robustSetup() {
    BinaryClock& clock = BinaryClock::get_Instance();
    
    // Try setup with timeout
    unsigned long startTime = millis();
    clock.setup(false);
    
    // Verify RTC is working
    DateTime test = clock.ReadTime();
    if (test.year() < 2020) {
        Serial.println("WARNING: RTC time not set!");
        // Set to compile time as fallback
        clock.set_Time(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    Serial.printf("Setup completed in %lu ms\n", millis() - startTime);
}
```

---

## Examples

Complete examples are available in the project repository:

- **[Basic Binary Clock](BasicBinaryClock)** - Standard clock functionality
- **Button Test** - Test button inputs and debouncing
- **Melody Creator** - Create and test custom alarm melodies
- **RTC Sync** - Synchronize with external time source

---

## Version History

- **v0.9.4** - Current version
  - Full clock, alarm, menu, and button integration
  - FreeRTOS task support for capable boards
  - Callback system for time and alarm events
  - Extensive melody registration system
  - Support for UNO R3, UNO R4, ESP32 boards

---

## Authors

Creators the [WiFi Binary Clock software project][WiFiBinaryClock] for the **Binary Clock Shield for Arduino** hardware.

**Chris-70** (2025, 2026)  
**Chris-80** (2025)   

Creator of the [**Binary Clock Shield for Arduino**][shield] hardware and the [**Binary Clock Shield software examples**][ShieldExamples] to accompany the hardware. The [**BCMenu**][BCMenu] class maintains much of the original menu design and functionality from the original software examples.

**Marcin Saj** (2018) (https://nixietester.com)

---

## License

This library is part of the WiFi Binary Clock project.  
Licensed under [GNU General Public License v3.0][gpl_3].

See [LICENSE][license] file for details.

---

## Links

- **WiFi Binary Clock Project**: https://github.com/Chris-70/WiFiBinaryClock
- **BinaryClock Library**: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock
- **Class Diagram**: [BinaryClock_LibClassDiagram.md](BinaryClock_LibClassDiagram.md)
- **Binary Clock Shield**: https://nixietester.com/product/binary-clock-shield-for-arduino/
- **Shield GitHub**: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
- **Dependencies**:
  - [BCGlobalDefines](../BCGlobalDefines)
  - [MorseCodeLED](../MorseCodeLED)
  - [RTClibPlus](../RTClibPlus)

---

**Keep time in binary. ⏰✨**

---

[BasicBinaryClock]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/src/BinaryClock_ESP32.cpp
[BCGlobalDefines]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines/src
[BCMenu]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BCMenu.h
[boardselect]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/board_select.h
[CLASS_DIAGRAM]: ClassDiagram.md
[CLASS_DIAGRAM_GIT]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/ClassDiagram.md
[Example_11]: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino/tree/master/example/11-BinaryClockRTCInterruptAlarmButtons
[gpl_3]: https://www.gnu.org/licenses/gpl-3.0.en.html
[license]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/src/LICENSE
[metro]: https://www.adafruit.com/product/5500
[README]: README.md
[README_Git]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/README.md
[README_Project]: ../../README.md
[README_ProjectGit]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/README.md
[shield]: https://nixietester.com/product/binary-clock-shield-for-arduino/
[ShieldExamples]: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino/tree/master/example
[WiFiBinaryClock]: https://github.com/Chris-70/WiFiBinaryClock
