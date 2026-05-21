# RTClibPlus - Differences from Adafruit RTClib

***Current Library versions:***

### RTClibPlus Library: [![GitHub release (latest by date)](https://img.shields.io/github/v/release/Chris-70/RTClibPlus?style=flat-square)](https://github.com/Chris-70/RTClibPlus/releases)
### Adafruit RTClib: [![GitHub release (latest by date)](https://img.shields.io/github/v/release/adafruit/RTClib?style=flat-square)](https://github.com/adafruit/RTClib/releases)
___
This document provides a detailed explanation of all changes made to the original [Adafruit RTClib v2.1.4](https://github.com/adafruit/RTClib) to create [**RTClibPlus**](https://github.com/Chris-70/RTClibPlus).

## Overview

RTClibPlus is a fork of Adafruit's RTClib library with significant enhancements to support:
- Extracted `DateTime` and `Timestamp` classes from `RTClib.h` to create a new `DateTime.h` header for better modularity (`RTClib.h` includes `DateTime.h`).
- 12-hour time format (i.e. AM/PM) support in hardware (e.g. DS3231/DS1307) and the `DateTime` class.
- User-configurable starting day of the week (e.g. Mon, Sun, Sat, etc.).
- Century support for DS3231.
- Enhanced DateTime formatting with new `DateTime::toString()` capabilities.
- Extended `DateTime::timestamp()` enum options to include common formats.
- Public inheritance of the `RTC_I2C` class (e.g. for direct register access)


---

## 1. 12-Hour Mode Support

### Original Behavior
- Adafruit RTClib only supported 24-hour time format
- RTC chips (DS3231, DS1307) were always operated in 24-hour mode
- No AM/PM indicator support

### RTClibPlus Changes

#### Hardware Support
**Files Modified:** `RTClib.h`; `RTC_DS3231.cpp`; `RTC_DS1307.cpp`

**New Methods:**
```cpp
bool getIs12HourMode();           // Read current mode from RTC chip
void setIs12HourMode(bool value); // Toggle between 12/24 hour modes
```

**Overloaded Methods:**
```cpp
// DS3231 and DS1307
void adjust(const DateTime &dt, bool use12HourMode);

// DS3231 only (alarm support)
bool setAlarm1(const DateTime &dt, Ds3231Alarm1Mode alarm_mode, bool use12HourMode);
bool setAlarm2(const DateTime &dt, Ds3231Alarm2Mode alarm_mode, bool use12HourMode);
```

#### Implementation Details
- The RTC hour register (02h) bit 6 stores the 12/24-hour mode flag (as per DS3231/DS1307 datasheets).
- When mode is changed, the hour value is read, converted, and written back.
- **Alarm synchronization**: When time mode changes, both alarm hour modes are also changed.
- **Alarm disable trick**: Different hour modes between time and alarm effectively disable the alarm (the alarm fired flags `A1F`/`A2F` will never be set).

#### DateTime Class Support
**File Modified:** `RTClib.cpp`; `DateTime.h` (extracted from `RTClib.h`)

**New Methods:**
```cpp
bool    isPM() const;       // Returns true if time is PM (hour >= 12)
uint8_t twelveHour() const; // Returns hour in 12-hour format (i.e. 1 - 12).
```

**Enhanced `toString()` Support:**
- `HH` - Hour without leading zero (space-padded: " 1" to "12" or " 0" to "23")
- `hh` - Hour with leading zero ("01" to "12" or original "00" to "23")
- `AP` - AM/PM indicator ("AM" or "PM")
- `ap` - Lowercase AM/PM indicator ("am" or "pm")

**Example:**
```cpp
DateTime dt(2023, 12, 31, 14, 30, 45); // 2:30:45 PM
char buffer[32];
dt.toString(buffer, 32, "HH:mm:ss AP");
// Output: " 2:30:45 PM"
```

---

## 2. Configurable Starting Day of Week

### Original Behavior
- Fixed: Sunday (`DateTime` class) as the first day of the week; Monday for DS3231/DS1307 chips
- Not configurable for different cultural conventions

### RTClibPlus Changes

#### Compile-Time Configuration
**File Modified:** `DateTime.h` (from `RTClib.h`) 

**New Define:**
```cpp
#ifndef FIRST_WEEKDAY
#define FIRST_WEEKDAY "Mon"   // Default: Monday as first day
#endif
```

**Valid Values:** `"Mon"`, `"Tue"`, `"Wed"`, `"Thu"`, `"Fri"`, `"Sat"`, `"Sun"`

#### Implementation Mechanism
Uses a **WeekdayEpoch** concept - a reference date in year 2000 where day 1 falls on the chosen first day of week:

| First Day | Epoch Date | Math |
|-----------|------------|------|
| Monday    | May 1, 2000 | May 1, 2000 was a Monday |
| Tuesday   | Feb 1, 2000 | Feb 1, 2000 was a Tuesday |
| Wednesday | Mar 1, 2000 | Mar 1, 2000 was a Wednesday |
| Thursday  | Jun 1, 2000 | Jun 1, 2000 was a Thursday |
| Friday    | Sep 1, 2000 | Sep 1, 2000 was a Friday |
| Saturday  | Jan 1, 2000 | Jan 1, 2000 was a Saturday |
| Sunday    | Oct 1, 2000 | Oct 1, 2000 was a Sunday |

**Generated Macro:**
```cpp
#define FIRST_WEEKDAY_MONTH MONTH_WEEKDAY_START(FIRST_WEEKDAY)
// Expands to the appropriate month number (1-12)
```

**Static Member:**
```cpp
// WeekdayEpoch initialized based on FIRST_WEEKDAY_MONTH
// i.e. DateTime(2000, FIRST_WEEKDAY_MONTH, 1, 0, 0, 0)
static const DateTime WeekdayEpoch;  
```

#### dayOfTheWeek() Behavior
**Original:** Returns 0-6 where 0 == Sunday, 1 == Monday, ..., 6 == Saturday (fixed)

**RTClibPlus:** Returns 0-6 where 0 == first day defined by `FIRST_WEEKDAY`

**Example with `FIRST_WEEKDAY = "Mon"`:**
- 0 = Monday
- 1 = Tuesday
- ...
- 6 = Sunday

**Example with `FIRST_WEEKDAY = "Sat"`:**
- 0 = Saturday
- 1 = Sunday
- ...
- 6 = Friday

#### RTC Chip Synchronization
The DS3231 and DS1307 chips store day-of-week as 1-7 (not 0-6). RTClibPlus maintains this mapping so the days of the week order doesn't change:
- DateTime value `0` → RTC register value `1`
- . . .
- DateTime value `6` → RTC register value `7`  

The **Original:** mapping changed Sunday (0) to 7 (RTC), so Monday became the first day of the week and Sunday the last day of the week on the RTC chips. 
- DateTime value `0` = Sunday → RTC register value `7`
- DateTime value `1` = Monday → RTC register value `1`
- . . .
- DateTime value `6` = Saturday → RTC register value `6`

The `dayOfTheWeek()` calculation ensures the RTC chip's day-of-week stays synchronized with the configured `FIRST_WEEKDAY`.  
***NOTE:*** For the `dayOfTheWeek()` method to work correctly, the **year must be > 2000** (the weekday epoch year). For dates before __2001__, the day-of-week may be incorrect due to the way the weekday is calculated.

---

## 3. Century Support (DS3231)

### Original Behavior
- No century bit handling
- Assumed all dates in 2000-2099 range

### RTClibPlus Changes

#### DS3231 Century Bit
**File Modified:** `RTC_DS3231.cpp`

The DS3231 uses bit 7 of the month register (05h) to store the century bit:
- `0` = 2000-2099 (20xx)
- `1` = 2100-2199 (21xx)

**Implementation:**
```cpp
// Reading time
uint8_t month = read_register(0x05);
bool century = (month & 0x80) != 0;
uint16_t year = 2000 + bcd2bin(read_register(0x06));
if (century) {year += 100};  // 21xx century
```

**Writing time:**
```cpp
uint8_t month_val = bin2bcd(dt.month());
if (dt.year() >= 2100) {
    month_val |= 0x80;  // Set century bit
}
write_register(0x05, month_val);
```

#### DateTime Class Extended Range
**File Modified:** `DateTime.h` (from `RTClib.h`)

**Original Range:** 2000-2099 (implicit)

**RTClibPlus Range:** 2000-2199 (explicit)

**Year 2100 Leap Year Handling:**
The year 2100 is **not** a leap year (divisible by 100 but not by 400). The `isLeapYear()` method was updated:

```cpp
static bool isLeapYear(uint16_t year) {
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;  // 2100 is not a leap year
    return (year % 4 == 0);
}
```

**Impact on Date Calculations:**
- Day-of-week calculations now account for non-leap year 2100
- Days-since-epoch calculations handle the full 2000-2199 range
- `daysInMonth()` returns 28 for February 2100

---

## 4. Enhanced DateTime::toString()

### Original Behavior
Basic format string support with these specifiers:
- `YYYY`, `YY` - Year
- `MM`, `MMM` - Month
- `DD`, `DDD` - Day
- `hh` - Hour (always with leading zero, 24-hour only)
- `mm`, `ss` - Minute, second

### RTClibPlus Enhancements

#### New Format Specifiers

| Specifier | Output | Value Range | Notes |
|-----------|--------|-------------|-------|
| `HH` | Hour without leading zero | ( 0-23) or ( 1-12) | Space-padded instead of zero-padded |
| `AP` | AM/PM uppercase | AM or PM | Switches `hh`/`HH` to 12-hour mode |
| `ap` | AM/PM lowercase | am or pm | Switches `hh`/`HH` to 12-hour mode |

**Original `hh`:** Still supported, provides leading zeros, 12 or 24 hour modes.

#### Mode Switching
Presence of `AP` or `ap` in format string automatically switches hour display to 12-hour mode:

**Without AM/PM:**
```cpp
// 09:30:45
"hh:mm:ss" → "09:30:45"  (24-hour format)
"HH:mm:ss" → " 9:30:45"  (24-hour format, leading space for 9)
// 14:30:45
"hh:mm:ss" → "14:30:45"  (24-hour format)
"HH:mm:ss" → "14:30:45"  (24-hour format, leading space for 14)
```

**With AM/PM:**
```cpp
// 09:30:45
"hh:mm:ss AP" → "09:30:45 AM"  (12-hour format with leading zero)
"HH:mm:ss AP" → " 9:30:45 AM"  (12-hour format with leading space)
// 14:30:45
"hh:mm:ss AP" → "02:30:45 PM"  (12-hour format with leading zero)
"HH:mm:ss AP" → " 2:30:45 PM"  (12-hour format with leading space)
```

#### New Overloaded Method
**File Modified:** `DateTime.h` (from `RTClib.h`)
```cpp
      char* toString(char* buffer, size_t size, const char* format) const;
```      

**Original:**
```cpp
// 14:30:45
#define BUF_SIZE 32
char buffer[BUF_SIZE];
...
// Original `DateTime::toString()` method declaration.
char* toString(char* buffer);
...
// Required user to pre-format buffer with format string:
strncpy(buffer, "HH:mm AP", (BUF_SIZE - 1));
buffer[BUF_SIZE - 1] = '\0';
Serial.println(dt.toString(buffer));
// Outputs: " 2:30 PM" 
```

**New Overload:**
```cpp
// 14:30:45
#define BUF_SIZE 32
char buffer[BUF_SIZE];
...
// New `DateTime::toString()` method declaration with format parameter.
char* toString(char* buffer, size_t size, const char* format);
...
// User provides format directly:
Serial.println(dt.toString(buffer, BUF_SIZE, "HH:mm AP"));
// Outputs: " 2:30 PM"
```

**Parameters:**
- `buffer` - Pre-allocated character array for output
- `size` - Size of buffer (for overflow protection)
- `format` - Format specifier string

**Safety:**
- Guarantees `buffer[size-1] == '\0'` (null terminator)
- Truncates output if format string produces output longer than `size`

**Benefits:**
- Cleaner, more intuitive API
- Single-line formatting
- Inline format specification
- Buffer overflow protection

---

## 5. Extended TIMESTAMP Options

### Original Behavior
Three timestamp options in `enum timestampOpt`:

| Enum | Format String | Example Output | Description |
|------|---------------|----------------|-------------|
| TIMESTAMP_FULL | `YYYY-MM-DDThh:mm:ss` | `2023-12-31T14:30:45` | Full date/time (ISO 8601 format) |
| TIMESTAMP_TIME | `hh:mm:ss` | `14:30:45` | Time only (24-hour) |
| TIMESTAMP_DATE | `YYYY-MM-DD` | `2023-12-31` | Date only |
```cpp
// 2023/12/31 14:30:45
DateTime dt(2023, 12, 31, 14, 30, 45);

dt.timestamp(TIMESTAMP_FULL);  
// Output: "2023-12-31T14:30:45" (ISO 8601 format)

dt.timestamp(TIMESTAMP_TIME);  
// Output: "14:30:45"

dt.timestamp(TIMESTAMP_DATE);  
// Output: "2023-12-31"
```

### RTClibPlus Extensions

Seven additional timestamp options to include common formats:

| Enum | Format String | Example Output | Description |
|------|---------------|----------------|-------------|
| `TIMESTAMP_DATETIME` | `YYYY-MM-DD hh:mm:ss` | `2023-12-31 14:30:45` | Combined date/time (24-hour) TIMESTAMP_FULL without the `T` |
| `TIMESTAMP_DATETIME12` | `YYYY-MM-DD HH:mm:ss AP` | `2023-12-31  2:30:45 PM` | Combined date/time (12-hour) |
| `TIMESTAMP_TIME12` | `HH:mm:ss AP` | ` 2:30:45 PM` | Time only (12-hour) |
| `TIMESTAMP_TIME_HM` | `hh:mm` | `14:30` | Time without seconds (24-hour) |
| `TIMESTAMP_TIME12_HM` | `HH:mm AP` | ` 2:30 PM` | Time without seconds (12-hour) |
| `TIMESTAMP_DATE_DMY` | `DD-MM-YYYY` | `31-12-2023` | European date format |
| `TIMESTAMP_DATE_MDY` | `MM-DD-YYYY` | `12-31-2023` | US date format |

**Usage:**
```cpp
// 2023/12/31 14:30:45
DateTime dt(2023, 12, 31, 14, 30, 45);

dt.timestamp(TIMESTAMP_DATETIME);
// Output: "2023-12-31 14:30:45"

dt.timestamp(TIMESTAMP_DATETIME12);
// Output: "2023-12-31  2:30:45 PM"

dt.timestamp(TIMESTAMP_TIME12);  
// Output: " 2:30:45 PM"

dt.timestamp(TIMESTAMP_TIME_HM);
// Output: "14:30"

dt.timestamp(TIMESTAMP_TIME12_HM);
// Output: " 2:30 PM"

dt.timestamp(TIMESTAMP_DATE_DMY);
// Output: "31-12-2023"

dt.timestamp(TIMESTAMP_DATE_MDY);
// Output: "12-31-2023"
```

---

## 6. Public Inheritance Change

### Original Behavior
RTC classes used **private** inheritance from `RTC_I2C`:
```cpp
class RTC_DS3231 : RTC_I2C {
    // Methods like read_register() and write_register() 
    // not accessible from child classes
};
```

### RTClibPlus Changes

**Files Modified:** `RTClib.h`

All RTC classes now use **public** inheritance from `RTC_I2C`:
```cpp
class RTC_DS3231 : public RTC_I2C {
    // Base class public methods now publicly accessible
};
```

**Affected Classes:**
- `RTC_DS3231`
- `RTC_DS1307`
- `RTC_PCF8523`
- `RTC_PCF8563`

#### Benefits

**Direct Register Access:**
```cpp
RTC_DS3231 rtc;
uint8_t controlReg = rtc.read_register(0x0E);  // Now legal
rtc.write_register(0x0E, controlReg | 0x04);   // Now legal
```

**Use Cases:**
- Advanced RTC configuration
- Custom register manipulation
- Direct hardware control
- Debugging and diagnostics

**Example - RTC child class Extension:**
```cpp
// New child class example that extends RTC_DS3231
class RTCLibPlusDS3231 : public RTC_DS3231 {
public:
    uint8_t RawRead(uint8_t reg) {
        return read_register(reg);  // Accessible via public inheritance
    }
    
    void RawWrite(uint8_t reg, uint8_t value) {
        write_register(reg, value);  // Accessible via public inheritance
    }
};
```

This change enables projects to create wrapper classes that expose raw register access without modifying the core library.

---

## 7. DateTime.h Header Split

### Original Behavior
All classes in single `RTClib.h` file:
- `DateTime` class
- `TimeSpan` class
- `RTC_DS3231` class
- `RTC_DS1307` class
- `RTC_PCF8523` class
- `RTC_PCF8563` class
- `RTC_I2C` base class
- Other RTC implementations

### RTClibPlus Changes

**New File:** `DateTime.h`

**Extracted Classes:**
- `DateTime`
- `TimeSpan`
- All date/time related enums and constants

**Updated File:** `RTClib.h`
- Now includes `DateTime.h` at the top
- Contains only RTC hardware classes

#### Benefits

**Reduced Dependencies:**
```cpp
// When you only need date/time classes
#include <DateTime.h>

// Full RTC hardware support
#include <RTClib.h>  // Automatically includes DateTime.h
```

**Use Cases:**
- Components using `DateTime` and `TimeSpan` classes for calculations without RTC hardware
- Multi-file projects where only some files need RTC hardware access
- Reduced compilation dependencies
- Clearer code organization

**Backward Compatibility:**
- Existing code using `#include <RTClib.h>` continues to work unchanged
- No breaking changes to existing API
- Still requires full compilation of the RTClib classes.

---

## Summary Table

| Feature | Original RTClib | RTClibPlus | Impact |
|---------|-----------------|------------|--------|
| **12-Hour Mode** | Not supported | Full hardware & software support | Major enhancement |
| **Starting Day of Week** | Static but mixed: (a) `DateTime`: Sunday; (b) `RTC_*`: Monday | User-configurable (`FIRST_WEEKDAY`) | Cultural flexibility |
| **Century Support** | No | Yes (2001-2199) | Extended date range  (DS3231 years range) |
| **toString() Formats** | Basic | Enhanced (HH, AP/ap) | Better formatting |
| **TIMESTAMP Options** | 3 formats | 10 formats | Additional common formatting options |
| **Inheritance** | Private | Public | Direct register access |
| **File Organization** | Single header | Split headers | Better modularity |

---

## Migration Guide

### From Adafruit RTClib to RTClibPlus

**One minor breaking change**:  

Code that depends on the original day-of-week mapping where "Monday" though "Saturday" maintained the same value (i.e. 1 to 6) for both the RTC chips and the `DateTime` class and "Sunday" was the only day of the week that changed values may need adjustment.
- Day of the week order is now maintained between the `DateTime` class and the RTC `DS3231`/`DS1307` chips. 
```
   - Original code: 
      - RTC      day 7 == Sunday, day 1 == Monday, ..., day 6 == Saturday.
      - DateTime day 0 == Sunday, day 1 == Monday, ..., day 6 == Saturday.
   - RTClibPlus code:
      - RTC      day 1 == first weekday (e.g. Monday), day 7 == last weekday (e.g. Sunday).
      - DateTime day 0 == first weekday (e.g. Monday), day 6 == last weekday (e.g. Sunday).
```      
- To maintain the same day-of-week order as the original library:
   - RTC DS3231/DS1307 chips,  set `FIRST_WEEKDAY` to `"Mon"` before compilation.
   - For the `DateTime` class, set `FIRST_WEEKDAY` to `"Sun"` before compilation.      

**No other Breaking Changes:**
- All original RTClib API remains unchanged
- Existing code compiles without modification
- New features are additions, not replacements

**Enhancements:**

1. **Enable 12-Hour Mode:**
```cpp
// 2023/12/31 14:30:45
#define BUF_SIZE 32
rtc.setIs12HourMode(true);
DateTime dt = rtc.now();
char buf[BUF_SIZE];
dt.toString(buf, BUF_SIZE, "HH:mm:ss AP");
// e.g. buf value: " 2:30:45 PM"
```

2. **Change Starting Day of Week:**
```cpp
// In DateTime.h or any file/option (e.g -DFIRST_WEEKDAY="Sat") before compilation:
#define FIRST_WEEKDAY "Sat"  // Saturday is the first weekday
...
// 2023/12/31 14:30:45
uint8_t weekday = dt.dayOfTheWeek();  // Returns 1 as Saturday is 0 (2023-12-31 was a Sunday).
dt.toString(buf, BUF_SIZE, "DDD MMM DD, YYYY"); // e.g. buf value is: "Sun Dec 31, 2023"
```

3. **Use Extended Timestamps:**
```cpp
// 2023/12/31 14:30:45
dt.timestamp(TIMESTAMP_DATETIME12);
// Returns: "2023-12-31  2:30:45 PM"
```

4. **Access Registers Directly:**
```cpp
// 2023/12/31 14:30:45
uint8_t day = rtc.read_register(0x03);
// e.g. day has the value 2 (2nd day of week, "Sunday" in this case as "Saturday" is 1)
uint8_t date = rtc.read_register(0x04);
// e.g. date has the value 0x31 (BCD value for 31st day of the month)
```

---

## Repository Links

- **RTClibPlus:** https://github.com/Chris-70/RTClibPlus
- **Original Adafruit RTClib:** https://github.com/adafruit/RTClib
- **Binary Clock Project using RTClibPlus:** https://github.com/Chris-70/WiFiBinaryClock

---

## Version Information

- **Base Library:** Adafruit RTClib v2.1.4
- **Fork:** RTClibPlus [![GitHub release (latest by date)](https://img.shields.io/github/v/release/Chris-70/RTClibPlus?style=flat-square)](https://github.com/Chris-70/RTClibPlus/releases)
- [![GitHub license](https://img.shields.io/github/license/Chris-70/RTClibPlus?style=flat-square)](https://www.gnu.org/licenses/gpl-3.0.html)
(same as original)

## Credits

- **Original RTClib:** JeeLabs; Adafruit Industries
- **Enhancements:** Chris-70 (2025)
- **Binary Clock Project:** Inspired by Marcin Saj's Binary Clock Shield
