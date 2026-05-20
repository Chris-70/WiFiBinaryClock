#pragma once
#ifndef __DATETIME_H__
#define __DATETIME_H__

#include <stdint.h>                    // Integer types: uint8_t; uint16_t; etc.
#include <wstring.h>                   // Arduino String class for __FlashStringHelper()
#include <time.h>                      // For time_t an tm types.

class TimeSpan;

//=========================================================================/
/** Constants */
constexpr int32_t  SecondsPerDay         = 86400L;      ///< 60 * 60 * 24
constexpr uint32_t SecondsFrom1970to2000 = 946684800UL; ///< Unixtime for 2000-01-01 00:00:00, useful for initialization

#ifndef FIRST_WEEKDAY
//**************************************************************************/
///< The first day of the week: "Mon"; "Tue"; "Wed"; "Thu"; "Fri"; "Sat"; "Sun"
///  Alter this define to change the first day of the week.
///  Changing the value of `FIRST_WEEKDAY` is all that is needed to be done.
///  The first letter must be UPPERCASE, the other letters can be either.
#define FIRST_WEEKDAY "Mon"
//**************************************************************************/
#endif
//**************************************************************************/
// -------------------------------------------------------------------------
// The starting day of the week isn't the same for all cultures. These MACROs
// and constexpr helpers allow you to set the first day of the week to be used
// in all calculations. Just define FIRST_WEEKDAY to the first day of the week
// you want to use and the helpers will take care of the rest.
// The output from dayOfTheWeek() and DateTime::toString() will reflect this 
// setting. 
// ----------------------------------------------------------------------------
// On the first week of May 2000, the day-of-the-week (Monday) number 1 (1 - 7) 
// matches the date number 1 (e.g. May 1st, 2000). The DS3231 RTC chips use the same alarm 
// register for day-of-week and date, only a flag indicates if it is weekly or monthly. 
// By syncing these two numbers, the alarm can be set using the same code for the numeric value.
// To set an alarm to ring every Friday when the first day of the week is Monday, we
// use the date 2000-05-0, which is a Friday5. If the first day of the week is Sunday, we 
// use 2000-10-06, which is a Friday. The constant `DateTime::WeekdayEpoch` is defined 
// based on the selected first day of the week is also the first of that month.
// 
// For other starting days:
// Use  (5) May       2000, 1 for Monday    and 7 for Sunday.
// Use (10) October   2000, 1 for Sunday    and 7 for Saturday.
// Use  (1) January   2000, 1 for Saturday  and 7 for Friday.
// Use  (9) September 2000, 1 for Friday    and 7 for Thursday.
// Use  (6) June      2000, 1 for Thursday  and 7 for Wednesday.
// Use  (3) March     2000, 1 for Wednesday and 7 for Tuesday.
// Use  (2) February  2000, 1 for Tuesday   and 7 for Monday.
constexpr uint8_t Day_1_is_Monday    = 5;   ///< The first day of the week (1) is Monday    in May       2000
constexpr uint8_t Day_1_is_Sunday    = 10;  ///< The first day of the week (1) is Sunday    in October   2000
constexpr uint8_t Day_1_is_Saturday  = 1;   ///< The first day of the week (1) is Saturday  in January   2000
constexpr uint8_t Day_1_is_Friday    = 9;   ///< The first day of the week (1) is Friday    in September 2000
constexpr uint8_t Day_1_is_Thursday  = 6;   ///< The first day of the week (1) is Thursday  in June      2000
constexpr uint8_t Day_1_is_Wednesday = 3;   ///< The first day of the week (1) is Wednesday in March     2000
constexpr uint8_t Day_1_is_Tuesday   = 2;   ///< The first day of the week (1) is Tuesday   in February  2000
// Offset (order) in the names array to each day of the week name.
constexpr uint8_t IndexMonday        = 0;   ///< Day of the week name ("Mon") offset value for Monday
constexpr uint8_t IndexTuesday       = 1;   ///< Day of the week name ("Tue") offset value for Tuesday
constexpr uint8_t IndexWednesday     = 2;   ///< Day of the week name ("Wed") offset value for Wednesday
constexpr uint8_t IndexThursday      = 3;   ///< Day of the week name ("Thu") offset value for Thursday
constexpr uint8_t IndexFriday        = 4;   ///< Day of the week name ("Fri") offset value for Friday
constexpr uint8_t IndexSaturday      = 5;   ///< Day of the week name ("Sat") offset value for Saturday
constexpr uint8_t IndexSunday        = 6;   ///< Day of the week name ("Sun") offset value for Sunday
// ----------------------------------------------------------------------------
/// @brief constexpr helper to set the first day of the week for calculations.
/// @details The first day of the week is used in calculations for DateTime::dayOfTheWeek()
///          and DateTime::toString().  
/// @param dayName The name of the first day of the week, e.g. "Mon", "Sun", "Sat", etc.
constexpr uint8_t monthWeekdayStart(const char* dayName)
    {
    return ((dayName[0] == 'M')                                                   ? Day_1_is_Monday    :
           ((dayName[0] == 'S') && ((dayName[1] == 'u') || (dayName[1] == 'U')))  ? Day_1_is_Sunday    :
           ((dayName[0] == 'S') && ((dayName[1] == 'a') || (dayName[1] == 'A')))  ? Day_1_is_Saturday  :
            (dayName[0] == 'F')                                                   ? Day_1_is_Friday    :
           ((dayName[0] == 'T') && ((dayName[1] == 'h') || (dayName[1] == 'H')))  ? Day_1_is_Thursday  :
            (dayName[0] == 'W')                                                   ? Day_1_is_Wednesday :
           ((dayName[0] == 'T') && ((dayName[1] == 'u') || (dayName[1] == 'U')))  ? Day_1_is_Tuesday   : 5); // Default to May for Monday
    }
// ----------------------------------------------------------------------------
/// @brief constexpr helper to set day-name offset to match the configured first day.
/// @details The name array is static and hardcoded; this ensures names align with selected week start.
/// @param dayName The name of the first day of the week, e.g. "Mon", "Sun", "Sat", etc.
constexpr uint8_t weekdayOffset(const char* dayName)
    {
    return ((dayName[0] == 'M')                                                   ? IndexMonday    :
           ((dayName[0] == 'S') && ((dayName[1] == 'u') || (dayName[1] == 'U')))  ? IndexSunday    :
           ((dayName[0] == 'S') && ((dayName[1] == 'a') || (dayName[1] == 'A')))  ? IndexSaturday  :
            (dayName[0] == 'F')                                                   ? IndexFriday    :
           ((dayName[0] == 'T') && ((dayName[1] == 'h') || (dayName[1] == 'H')))  ? IndexThursday  :
            (dayName[0] == 'W')                                                   ? IndexWednesday :
           ((dayName[0] == 'T') && ((dayName[1] == 'u') || (dayName[1] == 'U')))  ? IndexTuesday   : 0); // Default to Monday
    }
// ----------------------------------------------------------------------------
// These constants synchronize first day of week for both dayOfTheWeek() and toString().
constexpr uint8_t FirstWeekdayMonth = monthWeekdayStart(FIRST_WEEKDAY);  ///< First day-of-week month selector.
constexpr uint8_t WeekdayNameOffset = weekdayOffset(FIRST_WEEKDAY);      ///< First-day name offset.
// -------------------------------------------------------------------------
// The numeric day of the week, 0 - 6, based on the selected first day of the week (i.e. FIRST_WEEKDAY).
// The numeric values used by the `DateTime` class, 0 for the first day of the week, 6 for the last day of the week.
constexpr uint8_t DOW_Monday    = ((IndexMonday    + WeekdayNameOffset) % 7); ///< Monday    number based on selected first day.
constexpr uint8_t DOW_Tuesday   = ((IndexTuesday   + WeekdayNameOffset) % 7); ///< Tuesday   number based on selected first day.
constexpr uint8_t DOW_Wednesday = ((IndexWednesday + WeekdayNameOffset) % 7); ///< Wednesday number based on selected first day.
constexpr uint8_t DOW_Thursday  = ((IndexThursday  + WeekdayNameOffset) % 7); ///< Thursday  number based on selected first day.
constexpr uint8_t DOW_Friday    = ((IndexFriday    + WeekdayNameOffset) % 7); ///< Friday    number based on selected first day.
constexpr uint8_t DOW_Saturday  = ((IndexSaturday  + WeekdayNameOffset) % 7); ///< Saturday  number based on selected first day.
constexpr uint8_t DOW_Sunday    = ((IndexSunday    + WeekdayNameOffset) % 7); ///< Sunday    number based on selected first day.
// -------------------------------------------------------------------------
// The numeric day of the week for time/alarms (DS3231;DS1307), 1 - 7, based on the selected first day of the week (i.e. FIRST_WEEKDAY).
// The numeric values used by the DS3231 & DS1307 RTCs `day` registers, 1 for the first day of the week, 7 for the last day of the week.
// 
constexpr uint8_t RTC_Monday    = (DOW_Monday    + 1); ///< Monday    RTC day number based on selected first day.
constexpr uint8_t RTC_Tuesday   = (DOW_Tuesday   + 1); ///< Tuesday   RTC day number based on selected first day.
constexpr uint8_t RTC_Wednesday = (DOW_Wednesday + 1); ///< Wednesday RTC day number based on selected first day.
constexpr uint8_t RTC_Thursday  = (DOW_Thursday  + 1); ///< Thursday  RTC day number based on selected first day.
constexpr uint8_t RTC_Friday    = (DOW_Friday    + 1); ///< Friday    RTC day number based on selected first day.
constexpr uint8_t RTC_Saturday  = (DOW_Saturday  + 1); ///< Saturday  RTC day number based on selected first day.
constexpr uint8_t RTC_Sunday    = (DOW_Sunday    + 1); ///< Sunday    RTC day number based on selected first day.

enum class DoW
   {
   Monday    = DOW_Monday,
   Tuesday   = DOW_Tuesday,
   Wednesday = DOW_Wednesday,
   Thursday  = DOW_Thursday,
   Friday    = DOW_Friday,
   Saturday  = DOW_Saturday,
   Sunday    = DOW_Sunday
   };
//=========================================================================/

/**************************************************************************/
/*!
    @brief  Simple general-purpose date/time class (no TZ / DST / leap
            seconds).

    This class stores date and time information in a broken-down form, as a
    tuple (year, month, day, hour, minute, second). The day of the week is
    not stored, but computed on request. The class has no notion of time
    zones, daylight saving time, or
    [leap seconds](http://en.wikipedia.org/wiki/Leap_second): time is stored
    in whatever time zone the user chooses to use.

    The class supports dates in the range from 1 Jan 2000 to 31 Dec 2199
    inclusive.
*/
/**************************************************************************/
class DateTime
   {
public:
   DateTime(uint32_t unix_t = SecondsFrom1970to2000);
   explicit DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour24 = 0,
                     uint8_t min = 0, uint8_t sec = 0);
   explicit DateTime(uint8_t hour24, uint8_t min, uint8_t sec = 0)
            : DateTime(DateTimeEpoch.yOff, DateTimeEpoch.m, DateTimeEpoch.d, hour24, min, sec) {} 
   explicit DateTime(uint8_t day, uint8_t hour24, uint8_t min, uint8_t sec)
            : DateTime(DateTimeEpoch.yOff, DateTimeEpoch.m, day, hour24, min, sec) {}
   explicit DateTime(DoW weekday, uint8_t hour24 = 0, uint8_t min = 0, uint8_t sec = 0)
            : DateTime(WeekdayEpoch.yOff, WeekdayEpoch.m, static_cast<uint8_t>(weekday), hour24, min, sec) {}
   // explicit DateTime(time_t unix_t) : DateTime(static_cast<uint32_t>(unix_t)) {}
   DateTime(const DateTime &date, const DateTime &time)
            : DateTime(date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second()) {}
   DateTime(const DateTime &copy)
            : DateTime(copy.year(), copy.month(), copy.day(), copy.hour(), copy.minute(), copy.second()) {}
   DateTime(const char* date, const char* time);
   DateTime(const __FlashStringHelper* date, const __FlashStringHelper* time);
   explicit DateTime(const char* iso8601date);
   DateTime(struct tm& tmTime);

   bool isValid() const;
   bool isTimeValid() const;
   bool isDateValid() const;

   char* toString(char* buffer) const;
   char* toString(char* buffer, size_t size, const char* format) const;

   /*!
      @brief  Return the year.
      @return Year (range: 2000--2199).
   */
   uint16_t year() const { return 2000U + yOff; }
   /*!
      @brief  Return the month.
      @return Month number (1--12).
   */
   uint8_t month() const { return m; }
   /*!
      @brief  Return the day of the month.
      @return Day of the month (1--31).
   */
   uint8_t day() const { return d; }
   /*!
      @brief  Return the hour
      @return Hour (0--23).
   */
   uint8_t hour() const { return hh; }
   /*!
      @brief  Return the hour in 12-hour format.
      @return Hour (1--12).
   */
   uint8_t twelveHour() const;
   /*!
         @brief  Return whether the time is PM.
         @return 0 if the time is AM, 1 if it's PM.
   */
   uint8_t isPM() const { return hh >= 12; }
   /*!
      @brief  Return the minute.
      @return Minute (0--59).
   */
   uint8_t minute() const { return mm; }
   /*!
      @brief  Return the second.
      @return Second (0--59).
   */
   uint8_t second() const { return ss; }

   /*!
      @brief  Return the day of the week.
      @return Day of the week (0--6).
      @remarks 0 = the day of the week of WeekdayEpoch (see above),
               1 = the next day, ..., 6 = the day before the day of
                  WeekdayEpoch.
               E.g. if WeekdayEpoch is a Monday, then 0 = Monday,
               See: 'FirstWeekdayMonth' above.
   */
   uint8_t dayOfTheWeek() const;

   /*!
      @brief  Return the constant `WeekdayNameOffset` which is defined
               to be the offset to day of the week in the name array that
               internally starts with "Mon" which isn't necessarily
               the start of the week.
      @return Returns WeekdayNameOffset based on the starting day of the week.
      @remarks The offset (order) of the day of the week name
               based on the selected first day of the week.
               Internally the text string is an array of 21 characters
               with each day having 3 letters. The ((offset + dayOfTheWeek()) * 3)
               gives the index into the array of names for the day of the week.
               See: 'WeekdayNameOffset' above.
   */
   static uint8_t dayNameOffset() { return WeekdayNameOffset; }

   /* 32-bit time as seconds since 2000-01-01. */
   uint32_t secondstime() const;

   /* 32-bit time as seconds since 1970-01-01. */
   uint32_t unixtime(void) const;

   /*!
         Format of the ISO 8601 timestamp generated by `timestamp()`.
         Also some common time and date formats that are provided.
         Each option corresponds to a `toString()` format as follows:
   */
   enum timestampOpt
      {
      TIMESTAMP_FULL,     //!< `YYYY-MM-DDThh:mm:ss`
      TIMESTAMP_TIME,     //!< `hh:mm:ss`
      TIMESTAMP_DATE,     //!< `YYYY-MM-DD`
      // Non-ISO but common formats
      TIMESTAMP_DATETIME,  //!< `YYYY-MM-DD hh:mm:ss`
      TIMESTAMP_DATETIME12,//!< `YYYY-MM-DD HH:mm:ss AM/PM`
      TIMESTAMP_TIME12,    //!< `HH:mm:ss AM/PM`
      TIMESTAMP_TIME_HM,   //!< `hh:mm`
      TIMESTAMP_TIME12_HM, //!< `HH:mm AM/PM`
      TIMESTAMP_DATE_DMY,  //!< `DD-MM-YYYY`
      TIMESTAMP_DATE_MDY,  //!< `MM-DD-YYYY`
      };
   String timestamp(timestampOpt opt = TIMESTAMP_FULL) const;

   DateTime operator+(const TimeSpan& span) const;
   DateTime operator-(const TimeSpan& span) const;
   TimeSpan operator-(const DateTime& right) const;
   bool operator<(const DateTime& right) const;

   /*!
      @brief  Test if one DateTime is greater (later) than another.
      @warning if one or both DateTime objects are invalid, returned value is
               meaningless
      @see use `isValid()` method to check if DateTime object is valid
      @param right DateTime object to compare
      @return True if the left DateTime is later than the right one,
              false otherwise
   */
   bool operator>(const DateTime& right) const { return right < *this; }

   /*!
      @brief  Test if one DateTime is less (earlier) than or equal to another
      @warning if one or both DateTime objects are invalid, returned value is
               meaningless
      @see use `isValid()` method to check if DateTime object is valid
      @param right DateTime object to compare
      @return True if the left DateTime is earlier than or equal to the
                 right one, false otherwise
   */
   bool operator<=(const DateTime& right) const { return !(*this > right); }

   /*!
      @brief  Test if one DateTime is greater (later) than or equal to another
      @warning if one or both DateTime objects are invalid, returned value is
               meaningless
      @see use `isValid()` method to check if DateTime object is valid
      @param right DateTime object to compare
      @return True if the left DateTime is later than or equal to the right
              one, false otherwise
   */
   bool operator>=(const DateTime& right) const { return !(*this < right); }
   bool operator==(const DateTime& right) const;

   /*!
      @brief  Test if two DateTime objects are not equal.
      @warning if one or both DateTime objects are invalid, returned value is
               meaningless
      @see use `isValid()` method to check if DateTime object is valid
      @param right DateTime object to compare
      @return True if the two objects are not equal, false if they are
   */
   bool operator!=(const DateTime& right) const { return !(*this == right); }

   /// @brief The epoch for the day of the week calculations, which is
   ///        the first day of the month that is also the first day of the week 
   ///        in the month defined by 'FirstWeekdayMonth' above, in the year 2000.
   static const DateTime WeekdayEpoch;

   /// @brief The epoch for the DateTime class, which is 1 Jan 2000, 00:00:00.
   static const DateTime DateTimeEpoch;

protected:
   uint8_t yOff; ///< Year offset from 2000 (0-199)
   uint8_t m;    ///< Month 1-12
   uint8_t d;    ///< Day 1-31
   uint8_t hh;   ///< Hours 0-23
   uint8_t mm;   ///< Minutes 0-59
   uint8_t ss;   ///< Seconds 0-59
   };

   /**************************************************************************/
   /*!
       @brief  Timespan which can represent changes in time with seconds accuracy.
   */
   /**************************************************************************/
class TimeSpan
   {
public:
   TimeSpan(int32_t seconds = 0);
   TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds);
   TimeSpan(const TimeSpan& copy);

   /*!
      @brief  Number of days in the TimeSpan
            e.g. 4
      @return int16_t days
   */
   int16_t days() const { return _seconds / 86400L; }
   /*!
      @brief  Number of hours in the TimeSpan
            This is not the total hours, it includes the days
            e.g. 4 days, 3 hours - NOT 99 hours
      @return int8_t hours
   */
   int8_t hours() const { return _seconds / 3600 % 24; }
   /*!
      @brief  Number of minutes in the TimeSpan
            This is not the total minutes, it includes days/hours
            e.g. 4 days, 3 hours, 27 minutes
      @return int8_t minutes
   */
   int8_t minutes() const { return _seconds / 60 % 60; }
   /*!
      @brief  Number of seconds in the TimeSpan
            This is not the total seconds, it includes the days/hours/minutes
            e.g. 4 days, 3 hours, 27 minutes, 7 seconds
      @return int8_t seconds
   */
   int8_t seconds() const { return _seconds % 60; }
   /*!
      @brief  Total number of seconds in the TimeSpan, e.g. 358027
      @return int32_t seconds
   */
   int32_t totalseconds() const { return _seconds; }

   TimeSpan operator+(const TimeSpan& right) const;
   TimeSpan operator-(const TimeSpan& right) const;

protected:
   int32_t _seconds; ///< Actual TimeSpan value is stored as seconds
   };

#endif // __DATETIME_H__