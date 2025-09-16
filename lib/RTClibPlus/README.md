# RTClibPlus - Modified fork

This is a fork of [Adafruit's RTClib](https://github.com/adafruit/RTClib) library with modifications to: DS3231; DS1307; and DateTime. These modifications were made to support:

- **12 hour mode**: The [DS3231](./src/RTC_DS3231.cpp) and [DS1307](./src/RTC_DS1307.cpp) support both 12 and 24 hour modes. The code for these RTC chips was modified to handle reading and writing in this mode as well as being able to switch modes. The support extends to the 2 alarms on the DS3231 and are kept in sync when changing modes. 
- **Day of the week**: In the  [DateTime](./src/RTClib.h) class the starting weekday is changable with a recompile, it doesn't need to be fixed to Sunday. The DS RTC chips don't have the concept of a fixed starting weekday and the DateTime class now supports this.
- **Century support**: The DS3231 supports century mode, this is now supported in the [RTC_DS3231](./src/RTC_DS3231.cpp) class. The DS1307 does not support century mode so it is not implemented in that class. The DateTime class now supports the century mode so dates from 2001-01-01 to 2199-12-31 can be used. The days calculations now account for the year 2100 not being a leap year.
- **DateTime::toString()**: The [DateTime](./src/RTClib.h) class has a new method, `toString(char* buffer, size_t size, const char *format)`, that allows the user to format the date and time in a string using a format string. This is similar to the `strftime()` function in C. The format string can include placeholders for year, month, day, hour, minute, second, and AM/PM. The leading zero for the hours can be replaced with a space (' ') character, using HH in place of hh to allow for a 12 (or 24) hour format without leading zeroes.
- **Inheritance**: The `RTC_DS3231`; `RTC_DS1307`; `RTC_PCF8523`; and `RTC_PCF8563` classes now use public inheritance from the `RTC_I2C` class, e.g. `class RTC_DS3231 : public RTC_I2C`. This allows child classes to access the base class methods directly from the child classes for example to raw read and write the RTC registers. 

## 12 hour mode

   The 12 hour mode is supported in the DS3231 and DS1307 RTC chips. The mode can be set by calling the `setIs12HourMode(bool enable)` method on the RTC object. The current time mode on the chip can be read using the `getIs12HourTime()` method. When changing the 12 hour mode (to/from 24 hours), the hour is read from the register, converted from the current hour mode and written back to the register in the new hour mode. This method also flips the stored hour mode in the alarms hour registers. The method flips the mode rather than set it to the current mode as having a mode different from the time is an effective way to turn off an alarm. The corresponding alarm fired flag, A1F or A2F, will never be set if the hours are using different modes. The methods: `RTC_DS3231::adjust()`; `RTC_DS3231::setAlarm1()`: and `RTC_DS3231::setAlarm2()` have been overloaded to take an additional boolean parameter, `use12HourMode` to control the individual hour mode for time and each alarm.
   
   The following methods have been overloaded to allow for individual setting of the time or alarms in 12 or 24 hour modes: 
   
   - `RTC_DS3231::adjust(const DateTime &dt, bool use12HourMode)` and `RTC_DS1307::adjust(const DateTime &dt, bool use12HourMode)` - The original `adjust(const DateTime &dt)` method is calls this method with `getI12HourMode()` to keep the current mode.
   - `RTC_DS3231::setAlarm1(const DateTime &alarmTime, Ds3231Alarm1Mode alarm_mode, bool use12HourMode)` - The original `setAlarm1(const DateTime &alarmTime, Ds3231Alarm1Mode alarm_mode)` method is calls this method with `getIs12HourMode()` to use the current mode.
   - `RTC_DS3231::setAlarm2(const DateTime& alarmTime, Ds3231Alarm2Mode alarm_mode, bool use12HourMode)` - The original `setAlarm2(const DateTime& alarmTime, Ds3231Alarm2Mode alarm_mode)` method is calls this method with `getIs12HourMode()` to use the current mode.

## Day of the week

   The starting day of the week can be set by changing the `FIRST_WEEKDAY` define in the [RTClib.h](./src/RTClib.h) file. The default is "Mon" for Monday. The valid values are: "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", and "Sun". This value is used in the `DateTime::dayOfTheWeek()` method and the `DateTime::toString()` methods.

   The `DateTime::dayOfTheWeek()` method returns the day of the week as an integer value from 0 to 6, where 0 is the first day of the week as defined by the `FIRST_WEEKDAY` define. For example, if `FIRST_WEEKDAY` is set to "Mon", then Monday is 0, Tuesday is 1, Wednesday is 2, Thursday is 3, Friday is 4, Saturday is 5, and Sunday is 6.

   The `DateTime::toString()` method uses the starting weekday to format the day of the week in the output string.

## Century support

   The century support is implemented in the DS3231 class. The century bit is stored in the hours register (bit 7) and is used to determine the century of the year. When reading the time from the RTC, if the century bit is set, the year is in the 2100s (i.e. 21xx), otherwise it is in the 2000s (i.e. 20xx). The _DateTime_ class now supports years from 2001 to 2199. The year 2100 is not a leap year, this is accounted for in the date calculations.

## DateTime::toString()

   - The `DateTime::toString()` method now supports the 'HH' format for hours without leading zeroes, replaced with a space (' '), in 12 or 24 hour modes. 
   - The `DateTime::toString(char* buffer, size_t size, const char *format)` is a new overloaded method allows the user to format the date and time in a string using a format string inline. Thus the user can write: 
     ```cpp
     char buffer[32];
     Serial << time.toString(buffer, 31, "hh:mm AP on DDD. MMM. DD, YYYY");
     ```
     instead of needing to format the buffer first:
     ```cpp
     char buffer[32];
     strncpy(buffer, "hh:mm AP on DDD. MMM. DD, YYYY", 32);
     Serial << time.toString(buffer);
     ```

## Inheritance

   The `RTC_DS3231`, `RTC_DS1307`, `RTC_PCF8523`, and `RTC_PCF8563` classes now use public inheritance from the `RTC_I2C` class. This allows child classes to access the base class methods directly from the child classes for example to raw read and write the RTC registers. The inheritance is done as follows:
   ```cpp
   class RTC_DS3231 : public RTC_I2C {
       // Class implementation
   };
   ```

   This change allows child classes to access the `RTC_I2C` class methods, such as `read_register()` and `write_register()`, directly.

---

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
