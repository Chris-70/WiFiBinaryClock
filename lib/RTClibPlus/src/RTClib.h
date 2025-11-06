/**************************************************************************/
/*!
  @file     RTClib.h

  Original library by JeeLabs http://news.jeelabs.org/code/, released to the
  public domain

  License: MIT (see LICENSE)

  This is a fork of JeeLab's fantastic real time clock library for Arduino.

  For details on using this library with an RTC module like the DS1307, PCF8523,
  or DS3231, see the guide at:
  https://learn.adafruit.com/ds1307-real-time-clock-breakout-board-kit/overview

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
*/
/**************************************************************************/

#ifndef _RTCLIB_H_
#define _RTCLIB_H_

#include <Adafruit_I2CDevice.h>
#include <Arduino.h>

#include "DateTime.h"            // DateTime and TimeSpan classes.

/** DS1307 SQW pin mode settings */
enum Ds1307SqwPinMode {
  DS1307_OFF = 0x00,             // SQW/OUT PIN - Low
  DS1307_ON  = 0x80,             // SQW/OUT PIN - High
  DS1307_SquareWave1HZ   = 0x10, // 1Hz square wave
  DS1307_SquareWave4kHz  = 0x11, // 4kHz square wave
  DS1307_SquareWave8kHz  = 0x12, // 8kHz square wave
  DS1307_SquareWave32kHz = 0x13  // 32kHz square wave
};

/** DS3231 SQW pin mode settings */
enum Ds3231SqwPinMode {
  DS3231_OFF = 0x1C,            /**< Off */
  DS3231_SquareWave1Hz = 0x00,  /**<  1Hz square wave */
  DS3231_SquareWave1kHz = 0x08, /**<  1kHz square wave */
  DS3231_SquareWave4kHz = 0x10, /**<  4kHz square wave */
  DS3231_SquareWave8kHz = 0x18  /**<  8kHz square wave */
};

/** DS3231 Alarm modes for alarm 1 */
enum Ds3231Alarm1Mode {
  DS3231_A1_PerSecond = 0x0F, /**< Alarm once per second */
  DS3231_A1_Second = 0x0E,    /**< Alarm when seconds match */
  DS3231_A1_Minute = 0x0C,    /**< Alarm when minutes and seconds match */
  DS3231_A1_Hour = 0x08,      /**< Alarm when hours, minutes
                                   and seconds match */
  DS3231_A1_Date = 0x00,      /**< Alarm when date (day of month), hours,
                                   minutes and seconds match */
  DS3231_A1_Day = 0x10        /**< Alarm when day (day of week), hours,
                                   minutes and seconds match */
};
/** DS3231 Alarm modes for alarm 2 */
enum Ds3231Alarm2Mode {
  DS3231_A2_PerMinute = 0x7, /**< Alarm once per minute
                                  (whenever seconds are 0) */
  DS3231_A2_Minute = 0x6,    /**< Alarm when minutes match */
  DS3231_A2_Hour = 0x4,      /**< Alarm when hours and minutes match */
  DS3231_A2_Date = 0x0,      /**< Alarm when date (day of month), hours
                                  and minutes match */
  DS3231_A2_Day = 0x8        /**< Alarm when day (day of week), hours
                                  and minutes match */
};
/** PCF8523 INT/SQW pin mode settings */
enum Pcf8523SqwPinMode {
  PCF8523_OFF = 7,             /**< Off */
  PCF8523_SquareWave1HZ = 6,   /**< 1Hz square wave */
  PCF8523_SquareWave32HZ = 5,  /**< 32Hz square wave */
  PCF8523_SquareWave1kHz = 4,  /**< 1kHz square wave */
  PCF8523_SquareWave4kHz = 3,  /**< 4kHz square wave */
  PCF8523_SquareWave8kHz = 2,  /**< 8kHz square wave */
  PCF8523_SquareWave16kHz = 1, /**< 16kHz square wave */
  PCF8523_SquareWave32kHz = 0  /**< 32kHz square wave */
};

/** PCF8523 Timer Source Clock Frequencies for Timers A and B */
enum PCF8523TimerClockFreq {
  PCF8523_Frequency4kHz = 0,   /**< 1/4096th second = 244 microseconds,
                                    max 62.256 milliseconds */
  PCF8523_Frequency64Hz = 1,   /**< 1/64th second = 15.625 milliseconds,
                                    max 3.984375 seconds */
  PCF8523_FrequencySecond = 2, /**< 1 second, max 255 seconds = 4.25 minutes */
  PCF8523_FrequencyMinute = 3, /**< 1 minute, max 255 minutes = 4.25 hours */
  PCF8523_FrequencyHour = 4,   /**< 1 hour, max 255 hours = 10.625 days */
};

/** PCF8523 Timer Interrupt Low Pulse Width options for Timer B only */
enum PCF8523TimerIntPulse {
  PCF8523_LowPulse3x64Hz = 0,  /**<  46.875 ms   3/64ths second */
  PCF8523_LowPulse4x64Hz = 1,  /**<  62.500 ms   4/64ths second */
  PCF8523_LowPulse5x64Hz = 2,  /**<  78.125 ms   5/64ths second */
  PCF8523_LowPulse6x64Hz = 3,  /**<  93.750 ms   6/64ths second */
  PCF8523_LowPulse8x64Hz = 4,  /**< 125.000 ms   8/64ths second */
  PCF8523_LowPulse10x64Hz = 5, /**< 156.250 ms  10/64ths second */
  PCF8523_LowPulse12x64Hz = 6, /**< 187.500 ms  12/64ths second */
  PCF8523_LowPulse14x64Hz = 7  /**< 218.750 ms  14/64ths second */
};

/** PCF8523 Offset modes for making temperature/aging/accuracy adjustments */
enum Pcf8523OffsetMode {
  PCF8523_TwoHours = 0x00, /**< Offset made every two hours */
  PCF8523_OneMinute = 0x80 /**< Offset made every minute */
};

/** PCF8563 CLKOUT pin mode settings */
enum Pcf8563SqwPinMode {
  PCF8563_SquareWaveOFF = 0x00,  /**< Off */
  PCF8563_SquareWave1Hz = 0x83,  /**< 1Hz square wave */
  PCF8563_SquareWave32Hz = 0x82, /**< 32Hz square wave */
  PCF8563_SquareWave1kHz = 0x81, /**< 1kHz square wave */
  PCF8563_SquareWave32kHz = 0x80 /**< 32kHz square wave */
};

/**************************************************************************/
/*!
    @brief  A generic I2C RTC base class. DO NOT USE DIRECTLY
*/
/**************************************************************************/
class RTC_I2C {
protected:
  /*!
      @brief  Convert a binary coded decimal value to binary. RTC stores
    time/date values as BCD.
      @param val BCD value
      @return Binary value
  */
  static uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
  /*!
      @brief  Convert a binary value to BCD format for the RTC registers
      @param val Binary value
      @return BCD value
  */
  static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }
  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
  uint8_t read_register(uint8_t reg);
  void write_register(uint8_t reg, uint8_t val);
};

/**************************************************************************/
/*!
    @brief  RTC based on the DS1307 chip connected via I2C and the Wire library
*/
/**************************************************************************/
class RTC_DS1307 : public RTC_I2C {
public:
  bool begin(TwoWire *wireInstance = &Wire);
  void adjust(const DateTime& dt, bool use12HourFormat);
  void adjust(const DateTime& dt);
  uint8_t isrunning(void);
  DateTime now();
  Ds1307SqwPinMode readSqwPinMode();
  void writeSqwPinMode(Ds1307SqwPinMode mode);
  uint8_t readnvram(uint8_t address);
  void readnvram(uint8_t *buf, uint8_t size, uint8_t address);
  void writenvram(uint8_t address, uint8_t data);
  void writenvram(uint8_t address, const uint8_t *buf, uint8_t size);
  /*!
      @brief  Convert the day of the week to a representation suitable for
              storing in the DS1307: from 1 (WeekdayEpoch.dayOfTheWeek() + 1) to 7.
      @param  d Day of the week as represented by the library: DateTime::dayOfTheWeek()
              from 0 (WeekdayEpoch.dayOfTheWeek()) to 6 (WeekdayEpoch.dayOfTheWeek() + 6).
      @return the converted value
  */
  static uint8_t dowToDS1307(uint8_t d) { return d + 1; }
  bool getIs12HourMode();
  void setIs12HourMode(bool value);
};

/**************************************************************************/
/*!
    @brief  RTC based on the DS3231 chip connected via I2C and the Wire library
*/
/**************************************************************************/
class RTC_DS3231 : public RTC_I2C {
public:
  bool begin(TwoWire *wireInstance = &Wire);
  void adjust(const DateTime& dt);
  void adjust(const DateTime& dt, bool use12HourMode);
  uint8_t* adjust(const DateTime& dt, bool use12HourMode, uint8_t* buf);
  bool lostPower(void);
  DateTime now();
  Ds3231SqwPinMode readSqwPinMode();
  void writeSqwPinMode(Ds3231SqwPinMode mode);
  bool setAlarm1(const DateTime &dt, Ds3231Alarm1Mode alarm_mode);
  bool setAlarm1(const DateTime &dt, Ds3231Alarm1Mode alarm_mode, bool use12HourMode);
  bool setAlarm2(const DateTime &dt, Ds3231Alarm2Mode alarm_mode);
  bool setAlarm2(const DateTime &dt, Ds3231Alarm2Mode alarm_mode, bool use12HourMode);
  DateTime getAlarm1();
  DateTime getAlarm2();
  Ds3231Alarm1Mode getAlarm1Mode();
  Ds3231Alarm2Mode getAlarm2Mode();
  void disableAlarm(uint8_t alarm_num);
  void clearAlarm(uint8_t alarm_num);
  bool alarmFired(uint8_t alarm_num);
  void enable32K(void);
  void disable32K(void);
  bool isEnabled32K(void);
  float getTemperature();   // in Celsius degree, +- 0.25 degree resolution
  int getIntTemperature(); // in Celsius degree, (-128 to +127 C) no floating point
  /*!
      @brief  Convert the day of the week to a representation suitable for
              storing in the DS3231: from 1 (WeekdayEpoch.dayOfTheWeek() + 1) to 7.
      @param  d Day of the week as represented by the library:
              from 0 (WeekdayEpoch.dayOfTheWeek()) to 6 (WeekdayEpoch.dayOfTheWeek() + 6).
      @return the converted value
  */
  static uint8_t dowToDS3231(uint8_t d) { return d + 1; }
  bool getIs12HourMode();
  void setIs12HourMode(bool value);

};

/**************************************************************************/
/*!
    @brief  RTC based on the PCF8523 chip connected via I2C and the Wire library
*/
/**************************************************************************/
class RTC_PCF8523 : public RTC_I2C {
public:
  bool begin(TwoWire *wireInstance = &Wire);
  void adjust(const DateTime &dt);
  bool lostPower(void);
  bool initialized(void);
  DateTime now();
  void start(void);
  void stop(void);
  uint8_t isrunning();
  Pcf8523SqwPinMode readSqwPinMode();
  void writeSqwPinMode(Pcf8523SqwPinMode mode);
  void enableSecondTimer(void);
  void disableSecondTimer(void);
  void enableCountdownTimer(PCF8523TimerClockFreq clkFreq, uint8_t numPeriods,
                            uint8_t lowPulseWidth);
  void enableCountdownTimer(PCF8523TimerClockFreq clkFreq, uint8_t numPeriods);
  void disableCountdownTimer(void);
  void deconfigureAllTimers(void);
  void calibrate(Pcf8523OffsetMode mode, int8_t offset);
};

/**************************************************************************/
/*!
    @brief  RTC based on the PCF8563 chip connected via I2C and the Wire library
*/
/**************************************************************************/
class RTC_PCF8563 : public RTC_I2C {
public:
  bool begin(TwoWire *wireInstance = &Wire);
  bool lostPower(void);
  void adjust(const DateTime &dt);
  DateTime now();
  void start(void);
  void stop(void);
  uint8_t isrunning();
  Pcf8563SqwPinMode readSqwPinMode();
  void writeSqwPinMode(Pcf8563SqwPinMode mode);
};

/**************************************************************************/
/*!
    @brief  RTC using the internal millis() clock, has to be initialized before
   use. NOTE: this is immune to millis() rollover events.
*/
/**************************************************************************/
class RTC_Millis {
public:
  /*!
      @brief  Start the RTC
      @param dt DateTime object with the date/time to set
  */
  void begin(const DateTime &dt) { adjust(dt); }
  void adjust(const DateTime &dt);
  DateTime now();

protected:
  /*!
      Unix time from the previous call to now().

      This, together with `lastMillis`, defines the alignment between
      the `millis()` timescale and the Unix timescale. Both variables
      are updated on each call to now(), which prevents rollover issues.
  */
  uint32_t lastUnix;
  /*!
      `millis()` value corresponding `lastUnix`.

      Note that this is **not** the `millis()` value of the last call to
      now(): it's the `millis()` value corresponding to the last **full
      second** of Unix time preceding the last call to now().
  */
  uint32_t lastMillis;
};

/**************************************************************************/
/*!
    @brief  RTC using the internal micros() clock, has to be initialized before
            use. Unlike RTC_Millis, this can be tuned in order to compensate for
            the natural drift of the system clock. Note that now() has to be
            called more frequently than the micros() rollover period, which is
            approximately 71.6 minutes.
*/
/**************************************************************************/
class RTC_Micros {
public:
  /*!
      @brief  Start the RTC
      @param dt DateTime object with the date/time to set
  */
  void begin(const DateTime &dt) { adjust(dt); }
  void adjust(const DateTime &dt);
  void adjustDrift(int ppm);
  DateTime now();

protected:
  /*!
      Number of microseconds reported by `micros()` per "true"
      (calibrated) second.
  */
  uint32_t microsPerSecond = 1000000;
  /*!
      Unix time from the previous call to now().

      The timing logic is identical to RTC_Millis.
  */
  uint32_t lastUnix;
  /*!
      `micros()` value corresponding to `lastUnix`.
  */
  uint32_t lastMicros;
};

#endif // _RTCLIB_H_
