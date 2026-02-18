# RTClibPlus - Differences from Adafruit RTClib

This document provides a detailed explanation of all changes made to the original [Adafruit RTClib v2.1.4](https://github.com/adafruit/RTClib) to create **RTClibPlus**.

## Overview

RTClibPlus is a fork of Adafruit's RTClib library with significant enhancements to support:
- 12-hour time format (AM/PM) support in hardware and DateTime
- User-configurable starting day of the week
- Century support for DS3231
- Enhanced DateTime formatting with new `toString()` capabilities
- Extended timestamp format options
- Public inheritance for direct register access

---

## 1. 12-Hour Mode Support

### Original Behavior
- Adafruit RTClib only supported 24-hour time format
- RTC chips (DS3231, DS1307) were always operated in 24-hour mode
- No AM/PM indicator support

### RTClibPlus Changes

#### Hardware Support
**Files Modified:** `RTC_DS3231.cpp`, `RTC_DS1307.cpp`

**New Methods:**
```cpp
bool getIs12HourMode();          // Read current mode from RTC chip
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
- The RTC hour register bit 6 stores the 12/24-hour mode flag
- When mode is changed, the hour value is read, converted, and written back
- **Alarm synchronization**: When time mode changes, both alarm hour modes are also changed
- **Alarm disable trick**: Different hour modes between time and alarm effectively disable the alarm (the alarm fired flags `A1F`/`A2F` will never be set)

#### DateTime Class Support
**File Modified:** `DateTime.h`

**New Methods:**
```cpp
bool isPM() const;     // Returns true if time is PM in 12-hour mode
uint8_t hour12() const; // Returns hour in 12-hour format (1-12)
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
- Fixed Sunday (DateTime) as the first day of the week; Monday for DS3231/DS1307 chips
- Not configurable for different cultural conventions

### RTClibPlus Changes

#### Compile-Time Configuration
**File Modified:** `DateTime.h`

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
static const DateTime WeekdayEpoch;  // Initialized based on FIRST_WEEKDAY_MONTH
```

#### dayOfTheWeek() Behavior
**Original:** Returns 0-6 where 0 = Sunday, 1 = Monday, ..., 6 = Saturday (fixed)

**RTClibPlus:** Returns 0-6 where 0 = first day defined by `FIRST_WEEKDAY`

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
The DS3231 and DS1307 chips store day-of-week as 1-7 (not 0-6). RTClibPlus maintains this mapping:
- DateTime value `0` → RTC register value `1`
- DateTime value `6` → RTC register value `7`

The `dayOfTheWeek()` calculation ensures the RTC chip's day-of-week stays synchronized with the configured `FIRST_WEEKDAY`.

---

## 3. Century Support (DS3231)

### Original Behavior
- No century bit handling
- Assumed all dates in 2000-2099 range

### RTClibPlus Changes

#### DS3231 Century Bit
**File Modified:** `RTC_DS3231.cpp`

The DS3231 uses bit 7 of the month register to store the century bit:
- `0` = 2000-2099 (20xx)
- `1` = 2100-2199 (21xx)

**Implementation:**
```cpp
// Reading time
uint8_t month = read_register(0x05);
bool century = (month & 0x80) != 0;
uint16_t year = 2000 + bcd2bin(read_register(0x06));
if (century) year += 100;  // 21xx century
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
**File Modified:** `DateTime.h`

**Original Range:** 2000-2099 (implicit)

**RTClibPlus Range:** 2001-2199 (explicit)

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
- Day-of-week calculations now account for non-leap 2100
- Days-since-epoch calculations handle the full 2001-2199 range
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

**Original `hh`:** Still supported, provides leading zeros

#### Mode Switching
Presence of `AP` or `ap` in format string automatically switches hour display to 12-hour mode:

**Without AM/PM:**
```cpp
"hh:mm:ss" → "14:30:45"  (24-hour format)
"HH:mm:ss" → "14:30:45"  (24-hour format, no leading space for 14)
```

**With AM/PM:**
```cpp
"hh:mm:ss AP" → "02:30:45 PM"  (12-hour format with leading zero)
"HH:mm:ss AP" → " 2:30:45 PM"  (12-hour format with space padding)
```

#### New Overloaded Method
**File Modified:** `DateTime.h`

**Original:**
```cpp
char* toString(char* buffer);
// Required user to pre-format buffer with format string:
strncpy(buffer, "HH:mm AP", 32);
Serial.println(dt.toString(buffer));
```

**New Overload:**
```cpp
char* toString(char* buffer, size_t size, const char* format);
// User provides format directly:
Serial.println(dt.toString(buffer, 32, "HH:mm AP"));
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
```cpp
TIMESTAMP_FULL,  // "YYYY-MM-DDThh:mm:ss" (ISO 8601)
TIMESTAMP_TIME,  // "hh:mm:ss"
TIMESTAMP_DATE   // "YYYY-MM-DD"
```

### RTClibPlus Extensions

Seven additional timestamp format options:

| Enum | Format String | Example Output | Description |
|------|---------------|----------------|-------------|
| `TIMESTAMP_DATETIME` | `YYYY-MM-DD hh:mm:ss` | `2023-12-31 14:30:45` | Combined date/time (24-hour) |
| `TIMESTAMP_DATETIME12` | `YYYY-MM-DD HH:mm:ss AP` | `2023-12-31  2:30:45 PM` | Combined date/time (12-hour) |
| `TIMESTAMP_TIME12` | `HH:mm:ss AP` | ` 2:30:45 PM` | Time only (12-hour) |
| `TIMESTAMP_TIME_HM` | `hh:mm` | `14:30` | Time without seconds (24-hour) |
| `TIMESTAMP_TIME12_HM` | `HH:mm AP` | ` 2:30 PM` | Time without seconds (12-hour) |
| `TIMESTAMP_DATE_DMY` | `DD-MM-YYYY` | `31-12-2023` | European date format |
| `TIMESTAMP_DATE_MDY` | `MM-DD-YYYY` | `12-31-2023` | US date format |

**Usage:**
```cpp
DateTime dt(2023, 12, 31, 14, 30, 45);
char buffer[32];

dt.timestamp(buffer, 32, TIMESTAMP_DATETIME12);
// Output: "2023-12-31  2:30:45 PM"

dt.timestamp(buffer, 32, TIMESTAMP_DATE_DMY);
// Output: "31-12-2023"
```

---

## 6. Public Inheritance Change

### Original Behavior
RTC classes used **protected** inheritance from `RTC_I2C`:
```cpp
class RTC_DS3231 : protected RTC_I2C {
    // Methods like read_register() and write_register() 
    // not accessible from child classes
};
```

### RTClibPlus Changes

**Files Modified:** `RTClib.h`

All RTC classes now use **public** inheritance:
```cpp
class RTC_DS3231 : public RTC_I2C {
    // Base class methods now publicly accessible
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

**Example - RTC Extension:**
```cpp
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
- Projects using `DateTime` for calculations without RTC hardware
- Multi-file projects where only some files need RTC hardware access
- Reduced compilation dependencies
- Clearer code organization

**Backward Compatibility:**
- Existing code using `#include <RTClib.h>` continues to work unchanged
- No breaking changes to existing API

**File Size:**
- `RTClib.h` reduced from ~500 lines to ~300 lines
- `DateTime.h` contains ~400 lines
- Clearer separation of concerns

---

## Summary Table

| Feature | Original RTClib | RTClibPlus | Impact |
|---------|-----------------|------------|--------|
| **12-Hour Mode** | Not supported | Full hardware & software support | Major enhancement |
| **Starting Day of Week** | Fixed (Sunday) | User-configurable | Cultural flexibility |
| **Century Support** | No | Yes (2001-2199) | Extended date range |
| **toString() Formats** | Basic | Enhanced (HH, AP/ap) | Better formatting |
| **TIMESTAMP Options** | 3 formats | 10 formats | More flexibility |
| **Inheritance** | Protected | Public | Direct register access |
| **File Organization** | Single header | Split headers | Better modularity |
| **Year 2100 Handling** | Assumed leap year | Correctly non-leap | Accurate calculations |

---

## Migration Guide

### From Adafruit RTClib to RTClibPlus

**No Breaking Changes:**
- All original RTClib API remains unchanged
- Existing code compiles without modification
- New features are additions, not replacements

**Optional Enhancements:**

1. **Enable 12-Hour Mode:**
```cpp
rtc.setIs12HourMode(true);
DateTime now = rtc.now();
char buf[32];
now.toString(buf, 32, "HH:mm:ss AP");
```

2. **Change Starting Day of Week:**
```cpp
// In DateTime.h before compilation:
#define FIRST_WEEKDAY "Sat"  // Saturday first
```

3. **Use Extended Timestamps:**
```cpp
char buf[32];
dt.timestamp(buf, 32, TIMESTAMP_DATETIME12);
```

4. **Access Registers Directly:**
```cpp
uint8_t status = rtc.read_register(0x0F);
```

---

## Repository Links

- **RTClibPlus:** https://github.com/Chris-70/RTClibPlus
- **Original Adafruit RTClib:** https://github.com/adafruit/RTClib
- **Binary Clock Project using RTClibPlus:** https://github.com/Chris-70/WiFiBinaryClock

---

## Version Information

- **Base Library:** Adafruit RTClib v2.1.4
- **Fork:** RTClibPlus (2025)
- **Author of Modifications:** Chris-70
- **License:** MIT (same as original)

## Credits

- **Original RTClib:** JeeLabs, Adafruit Industries
- **Enhancements:** Chris-70 (2025)
- **Binary Clock Project:** Inspired by Marcin Saj's Binary Clock Shield
