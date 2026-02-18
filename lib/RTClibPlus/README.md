# [RTClibPlus](https://github.com/Chris-70/RTClibPlus) - Modified fork

This is a fork of [Adafruit's RTClib](https://github.com/adafruit/RTClib) library v2.1.4 with modifications to: `DS3231`; `DS1307`; and `DateTime` classes. 

The full differences between this library and the original Adafruit RTClib are documented in the file: [**`ForkDifferences.md`**][ForkDifferences].

This is a summary of the modifications made to the original RTClib library:

- **12 hour mode**: The [DS3231](./src/RTC_DS3231.cpp) and [DS1307](./src/RTC_DS1307.cpp) support both 12 and 24 hour modes. The code for these RTC chips was modified to handle reading and writing in this mode as well as being able to switch modes. The support extends to the 2 alarms on the DS3231 and are kept in sync when changing modes. The `DateTime` class was also modified to fully support 12 hour mode.
- **Day of the week**: In the  [DateTime](./src/DateTime.h) class the starting weekday is changable with a recompile, it doesn't need to be fixed to Sunday or Monday. The DS RTC chips don't have the concept of a fixed starting weekday and the DateTime class now supports this. The user can compile their code for any starting weekday.
- **Century support**: The DS3231 chip supports century mode, this is now supported in the [RTC_DS3231](./src/RTC_DS3231.cpp) class. The DS1307 does not support century mode so it is not implemented in that class. The `DateTime` class now supports the century mode so dates from 2001-01-01 to 2199-12-31 can be used. The days calculations now account for the year 2100 not being a leap year.
- **`DateTime::toString()`**: The [DateTime](./src/DateTime.h) class has a new method, `toString(char* buffer, size_t size, const char *format)`, that allows the user to format the date and time in a string using a format string. This is similar to the `strftime()` function in C. The format string can include placeholders for year, month, day, hour, minute, second, and __AM/PM__. The leading zero for the hours can be replaced with a space (' ') character, using __HH__ in place of __hh__ to allow for a 12 (or 24) hour format without leading zeroes.
- **`DateTime::TIMESTAMP`**: The `enum timestampOpt` has been extended to include many more timestamp formats for use with the `DateTime::timestamp()` method. These include time in 12 hour format with AM/PM, date in various formats, and combined date and time formats in 12 and 24 hour modes.
- **Inheritance**: The `RTC_DS3231`; `RTC_DS1307`; `RTC_PCF8523`; and `RTC_PCF8563` classes now use public inheritance from the `RTC_I2C` class, e.g. `class RTC_DS3231 : public RTC_I2C`. This allows child classes to access the base class methods directly from the child classes for example to raw read and write the RTC registers. 
- **`DateTime.h`**: The `DateTime` and `TimeSpan` classes have been moved to a new header file, [DateTime.h](./src/DateTime.h). This will allow these classes to be defined where definitions for the hardware RTC chip classes are not needed. In larger projects with multiple source files it may be the case where only the associated `DateTime` classes are needed, including a `DateTime.h` header instead of `RTClib.h` makes the intent clear. In addition it reduces the size of the `RTClib.h` file which now just declare the classes related to the RTC chips. **Note:** The `RTClib.h` file includes the `DateTime.h` file so existing code that includes `RTClib.h` will still work exactly as before.

## 12 hour mode

   The 12 hour mode is supported in the DS3231 and DS1307 RTC chips. The mode can be set by calling the `setIs12HourMode(bool enable)` method on the RTC object. The current time mode on the chip can be read using the `getIs12HourTime()` method. When changing the 12 hour mode (to/from 24 hours), the hour is read from the register, converted from the current hour mode and written back to the register in the new hour mode. This method also flips the stored hour mode in the alarms hour registers. The method flips the mode rather than set it to the current mode as having a mode different from the time is an effective way to turn off an alarm. The corresponding alarm fired flag, `A1F` or `A2F`, will never be set if the hours are using different modes. The methods: `RTC_DS3231::adjust()`; `RTC_DS3231::setAlarm1()`: and `RTC_DS3231::setAlarm2()` have been overloaded to take an additional boolean parameter, `use12HourMode` to control the individual hour mode for time and each alarm.
   
   The following methods have been overloaded to allow for individual setting of the time or alarms in 12 or 24 hour modes: 
   
   - `RTC_DS3231::adjust(const DateTime &dt, bool use12HourMode)` and `RTC_DS1307::adjust(const DateTime &dt, bool use12HourMode)` - The original `adjust(const DateTime &dt)` method calls the new method with `getIs12HourMode()` as the 2<sup>nd</sup> parameter to keep the current mode.
   - `RTC_DS3231::setAlarm1(const DateTime &alarmTime, Ds3231Alarm1Mode alarm_mode, bool use12HourMode)` - The original `setAlarm1(const DateTime &alarmTime, Ds3231Alarm1Mode alarm_mode)` method calls the new method with `getIs12HourMode()` as the 3<sup>rd</sup> parameter to use the current mode.
   - `RTC_DS3231::setAlarm2(const DateTime& alarmTime, Ds3231Alarm2Mode alarm_mode, bool use12HourMode)` - The original `setAlarm2(const DateTime& alarmTime, Ds3231Alarm2Mode alarm_mode)` method calls the new method with `getIs12HourMode()` as the 3<sup>rd</sup> parameter to use the current mode.

## Day of the week

   Cultures don't all have the same starting day of the week, people have different preferences. The starting day of the week can be set by changing the `FIRST_WEEKDAY` define in the [DateTime.h](./src/DateTime.h) file. The default is "Mon" for Monday. The valid values are: "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", and "Sun". This value is used in the `DateTime::dayOfTheWeek()` method and the `DateTime::toString()` methods.

   The `DateTime::dayOfTheWeek()` method returns the day of the week as an integer value from 0 to 6 (these correspond to 1 -- 7 in the DS3231/DS1307 chips), where 0 is the first day of the week as defined by the `FIRST_WEEKDAY` define statement (e.g. `#define FIRST_WEEKDAY "Mon"`). The internal MACROs handle all the definitions based on that one `#define` statement.   

   For example:

   - If `FIRST_WEEKDAY` is set to "Mon", then Monday is 0, Tuesday is 1, Wednesday is 2, Thursday is 3, Friday is 4, Saturday is 5, and Sunday is 6. 
   - If the code is changed to `#define FIRST_WEEKDAY "Sat"` then "Saturday" is the first day of the week, so Saturday is 0, Sunday is 1, Monday is 2, Tuesday is 3, Wednesday is 4, Thursday is 5, and Friday is 6.

   The DS3231 and DS1307 chips do not have a concept of a fixed starting weekday, they just uses values from 1 to 7 (corresponding to `DateTime` values of 0 to 6). When the date is set on the chip, it uses the `DateTime::dayOfTheWeek()` method to calculate the day of the week based on the current date and the defined starting weekday. This is done by defining **`WeekdayEpoch`**, a `DateTime` object, to be the month of the year 2000 where day 1 of that month falls on the first day of the week. For example, when Monday is the first day of the week, the `WeekdayEpoch` object is set to May 1, 2000 at 00:00:00; when Saturday is the first day of the week, `WeekdayEpoch` is set to January 1, 2000 at 00:00:00. This use of a `WeekdayEpoch` makes it easy for the `DateTime::dayOfTheWeek()` method to calculate the weekday of any date from __2001__-01-01 to 2199-12-31.  

   The `DateTime::toString()` method uses the starting weekday to format the day of the week in the output string.

## Century support

   The century support is implemented in the DS3231 class. The century bit is stored in the hours register (bit 7) and is used to determine the century of the year. When reading the time from the RTC, if the century bit is set, the year is in the 2100s (i.e. 21xx), otherwise it is in the 2000s (i.e. 20xx). The _DateTime_ class now supports years from 2001 to 2199. The year 2100 is not a leap year, this is accounted for in the date calculations.

## DateTime::toString()

   - The `DateTime::toString()` method now supports the **'HH'** and **'AP'** formats for hours without leading zeroes and **AM/PM** for 12 hour mode. The **HH** format replaces the leading zero with a space (' '), in both 12 and 24 hour modes. The `HH` format can be used in place of `hh` in the format string. _For example_: to format the time as " 2:30 PM" instead of "02:30 PM", the format string would be `"HH:mm AP"`. 

   The following lists all the format specifiers are supported in the format string:

   | Specifier   | Output                                   | Value range       |
   |-------------|------------------------------------------|-------------------|
   | `YYYY`      | the year as a 4-digit number             | (2000--2199)      |
   | `YY`        | the year as a 2-digit number             | (00--99)          |
   | `MM`        | the month as a 2-digit number            | (01--12)          |
   | `MMM`       | the abbreviated English month name       | ("Jan"--"Dec")    |
   | `DD`        | the day as a 2-digit number              | (01--31)          |
   | `DDD`       | the abbreviated English day of the week  | ("Mon"--"Sun")    |
   | `AP`        | either "AM" or "PM"                      | (AM/PM)           |
   | `ap`        | either "am" or "pm"                      | (am/pm)           |
   | `hh`        | the hour as a 2-digit number             | (00--23 / 01--12) |
   | `HH`        | the hour as a 1/2-digit number/space     | ( 0--23 /  1--12) |
   | `mm`        | the minute as a 2-digit number           | (00--59)          |
   | `ss`        | the second as a 2-digit number           | (00--59)          |

   If either "__AP__" or "__ap__" is used, then the "__hh__" and "__HH__" specifiers use a 12-hour mode
   (range: 01--12). Otherwise they use a 24-hour mode (range: 00--23).

   The specifiers within _buffer_ will be overwritten with the appropriate
   values from the DateTime. Any characters not belonging to one of the
   above specifiers are left as-is.

   - The `DateTime::toString(char* buffer, size_t size, const char *format)` is a new overloaded method allows the user to format the date and time in a string using a format string inline. 
     - `buffer` is a pointer to a character array where the formatted string will be stored.
       - **Note:** The `buffer` must be pre-allocated by the user and large enough to hold the formatted string including the null terminator. The method guarentees that `buffer[size-1]` == `'\0'` after the method returns.
     - `size` is the size of the buffer to prevent overflow.
     - `format` is a C-string that specifies the format of the output string. 
       - **Note:** If `strlen(format) + 1` (for the null terminator) is greater than `size`, the output will be truncated to fit in the buffer.
      
   Thus the user can write: 
   
   ```cpp
   DateTime time(2023, 12, 31, 14, 30, 45);
   char buffer[32];
   Serial << time.toString(buffer, 31, "[HH:mm AP on DDD. MMM. DD, YYYY]");
   // Outputs: [ 2:30 PM on Sun. Dec. 31, 2023]
   ```
   instead of needing to format the buffer first:
   ```cpp
   DateTime time(2023, 12, 31, 14, 30, 45);
   char buffer[32];
   strncpy(buffer, "[HH:mm AP on DDD. MMM. DD, YYYY]", 32);
   Serial << time.toString(buffer);
   // Outputs: [ 2:30 PM on Sun. Dec. 31, 2023]
   ```
## TIMESTAMP

   The `enum timestampOpt` has been extended to include many more common timestamp formats for use with the `DateTime::timestamp()` method. These include time in 12 hour format with AM/PM (the hour has a space " " to replace the leading zero), date in various formats, and combined date and time formats in 12 and 24 hour modes. 
   
   __The list of all options are:__

   | Enum Option             | Format String             | Description                          | Sample output               | Status   |
   | ----------------------- | ------------------------- | ------------------------------------ |---------------------------- | :------: |
   | `TIMESTAMP_FULL`        | `YYYY-MM-DDThh:mm:ss`     | ISO 8601 combined date/time          | `"2023-12-31T14:30:45"`     | Original |
   | `TIMESTAMP_TIME`        | `hh:mm:ss`                | Time in 24 hour format               | `"14:30:45"`                | Original |
   | `TIMESTAMP_DATE`        | `YYYY-MM-DD`              | Date in YYYY-MM-DD format            | `"2023-12-31"`              | Original |
   | `TIMESTAMP_DATETIME`    | `YYYY-MM-DD hh:mm:ss`     | Combined date/time in 24 hour format | `"2023-12-31 14:30:45"`     | New      |
   | `TIMESTAMP_DATETIME12`  | `YYYY-MM-DD HH:mm:ss AP`  | Combined date/time in 12 hour format | `"2023-12-31  2:30:45 PM"`  | New      |
   | `TIMESTAMP_TIME12`      | `HH:mm:ss AP`             | Time in  12 hour format with AM/PM   | `" 2:30:45 PM"`             | New      |
   | `TIMESTAMP_TIME_HM`     | `hh:mm`                   | Time in  24 hour format              | `"14:30"`                   | New      |
   | `TIMESTAMP_TIME12_HM`   | `HH:mm AP`                | Time in  12 hour format with AM/PM   | `" 2:30 PM"`                | New      |
   | `TIMESTAMP_DATE_DMY`    | `DD-MM-YYYY`              | Date in DD-MM-YYYY format (Europe)   | `"31-12-2023"`              | New      |
   | `TIMESTAMP_DATE_MDY`    | `MM-DD-YYYY`              | Date in MM-DD-YYYY format (USA)      | `"12-31-2023"`              | New      |

## Inheritance

   The `RTC_DS3231`, `RTC_DS1307`, `RTC_PCF8523`, and `RTC_PCF8563` classes now use public inheritance from the `RTC_I2C` class. This allows child classes to access the base class methods directly from the child classes for example to raw read and write the RTC registers. The inheritance is done as follows:
   ```cpp
   class RTC_DS3231 : public RTC_I2C {
       // Class implementation
   };
   ```

   This change allows child classes to access the `RTC_I2C` class methods, such as `read_register()` and `write_register()`, directly.

<!-- Reference Links -->
[ForkDifferences]: ForkDifferences.md

---
# Original RTClib __README.md__ file:


# RTClib [![Build Status](https://github.com/adafruit/RTClib/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/RTClib/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/RTClib/html/index.html)

This is a fork of JeeLab's fantastic real time clock library for Arduino.

Works great with Adafruit RTC breakouts:

- [DS3231 Precision RTC](https://www.adafruit.com/product/3013) (breakout) and [Stemma QT version](https://www.adafruit.com/product/5188)
- [PCF8523 RTC](https://www.adafruit.com/product/3295)
- [DS1307 RTC](https://www.adafruit.com/product/3296)

Please note that dayOfTheWeek() ranges from 0 to 6 inclusive with 0 being 'Sunday'.

<!-- START COMPATIBILITY TABLE -->

## Compatibility

MCU                | Tested Works | Doesn't Work | Not Tested  | Notes
------------------ | :----------: | :----------: | :---------: | -----
Atmega328 @ 16MHz  |      X       |             |            |
Atmega328 @ 12MHz  |      X       |             |            |
Atmega32u4 @ 16MHz |      X       |             |            | Use SDA/SCL on pins D3 &amp; D2
Atmega32u4 @ 8MHz  |      X       |             |            | Use SDA/SCL on pins D3 &amp; D2
ESP8266            |      X       |             |            | SDA/SCL default to pins 4 &amp; 5 but any two pins can be assigned as SDA/SCL using Wire.begin(SDA,SCL)
Atmega2560 @ 16MHz |      X       |             |            | Use SDA/SCL on Pins 20 &amp; 21
ATSAM3X8E          |      X       |             |            | Use SDA1 and SCL1
ATSAM21D           |      X       |             |            |
ATtiny85 @ 16MHz   |      X       |             |            |
ATtiny85 @ 8MHz    |      X       |             |            |
Intel Curie @ 32MHz |             |             |     X       |
STM32F2            |             |             |     X       |

  * ATmega328 @ 16MHz : Arduino UNO, Adafruit Pro Trinket 5V, Adafruit Metro 328, Adafruit Metro Mini
  * ATmega328 @ 12MHz : Adafruit Pro Trinket 3V
  * ATmega32u4 @ 16MHz : Arduino Leonardo, Arduino Micro, Arduino Yun, Teensy 2.0
  * ATmega32u4 @ 8MHz : Adafruit Flora, Bluefruit Micro
  * ESP8266 : Adafruit Huzzah
  * ATmega2560 @ 16MHz : Arduino Mega
  * ATSAM3X8E : Arduino Due
  * ATSAM21D : Arduino Zero, M0 Pro
  * ATtiny85 @ 16MHz : Adafruit Trinket 5V
  * ATtiny85 @ 8MHz : Adafruit Gemma, Arduino Gemma, Adafruit Trinket 3V

<!-- END COMPATIBILITY TABLE -->
Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!

# Dependencies
 * [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO)

# Contributing

Contributions are welcome! Please read our [Code of Conduct](https://github.com/adafruit/RTClib/blob/master/code-of-conduct.md)
before contributing to help this project stay welcoming.

## Documentation and doxygen
For the detailed API documentation, see https://adafruit.github.io/RTClib/html/index.html
Documentation is produced by doxygen. Contributions should include documentation for any new code added.

Some examples of how to use doxygen can be found in these guide pages:

https://learn.adafruit.com/the-well-automated-arduino-library/doxygen

https://learn.adafruit.com/the-well-automated-arduino-library/doxygen-tips

## Code formatting and clang-format
The code should be formatted according to the [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html), which is the default of the clang-format tool.  The easiest way to ensure conformance is to [install clang-format](https://llvm.org/builds/) and run

```shell
clang-format -i <source_file>
```

See [Formatting with clang-format](https://learn.adafruit.com/the-well-automated-arduino-library/formatting-with-clang-format) for details.

Written by JeeLabs
MIT license, check license.txt for more information
All text above must be included in any redistribution

To install, use the Arduino Library Manager and search for "RTClib" and install the library.
