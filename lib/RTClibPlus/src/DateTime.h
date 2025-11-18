#pragma once
#ifndef __DATETIME_H__
#define __DATETIME_H__

#include <stdint.h>                    // Integer types: uint8_t; uint16_t; etc.
#include <wstring.h>                   // Arduino String class for __FlashStringHelper()
#include <time.h>                      // For time_t an tm types.

class TimeSpan;

//=========================================================================/
/** Constants */
#define SECONDS_PER_DAY 86400L ///< 60 * 60 * 24
#define SECONDS_FROM_1970_TO_2000                                              \
  946684800 ///< Unixtime for 2000-01-01 00:00:00, useful for initialization

#ifndef FIRST_WEEKDAY
//**************************************************************************/
///< The first day of the week: "Mon"; "Tue"; "Wed"; "Thu"; "Fri"; "Sat"; "Sun"
///  Alter this define to change the first day of the week.
#define FIRST_WEEKDAY "Mon"   
//**************************************************************************/
//
#endif
// -------------------------------------------------------------------------
// The starting day of the week isn't the same for all cultures. These MACROs
// and #defines allow you to set the first day of the week to be used in all
// calculations. Just define FIRST_WEEKDAY to the first day of the week you 
// want to use and the MACROs will take care of the rest.
// The output from dayOfTheWeek() and DateTime::toString() will reflect this 
// setting. 
// ----------------------------------------------------------------------------
// On the first week of May 2000, the day-of-the-week (Monday) number 1 (1 - 7) or 0 (0 - 6)
// matches the date number 1 (e.g. May 1st, 2000). The DS3231 RTC chips use the same alarm 
// register for day-of-week and date, only a flag indicates if it is weekly or monthly. 
// By syncing these two numbers, the alarm can be set using the same code for the numeric value.
// 
// For other starting days:
// Use  (5) May       2000, 1 for Monday    and 7 for Sunday.
// Use (10) October   2000, 1 for Sunday    and 7 for Saturday.
// Use  (1) January   2000, 1 for Saturday  and 7 for Friday.
// Use  (9) September 2000, 1 for Friday    and 7 for Thursday.
// Use  (6) June      2000, 1 for Thursday  and 7 for Wednesday.
// Use  (3) March     2000, 1 for Wednesday and 7 for Tuesday.
// Use  (2) February  2000, 1 for Tuesday   and 7 for Monday.
#define DAY_1_IS_MONDAY    5  ///< The first day of the week (1) is Monday    in May       2000
#define DAY_1_IS_SUNDAY   10  ///< The first day of the week (1) is Sunday    in October   2000
#define DAY_1_IS_SATURDAY  1  ///< The first day of the week (1) is Saturday  in January   2000
#define DAY_1_IS_FRIDAY    9  ///< The first day of the week (1) is Friday    in September 2000
#define DAY_1_IS_THURSDAY  6  ///< The first day of the week (1) is Thursday  in June      2000
#define DAY_1_IS_WEDNESDAY 3  ///< The first day of the week (1) is Wednesday in March     2000
#define DAY_1_IS_TUESDAY   2  ///< The first day of the week (1) is Tuesday   in February  2000
// Offset (order) in the names array to each day of the week name.
#define DOW_MONDAY         0  ///< Day of the week name offset value for Monday
#define DOW_SUNDAY         6  ///< Day of the week name offset value for Sunday
#define DOW_SATURDAY       5  ///< Day of the week name offset value for Saturday
#define DOW_FRIDAY         4  ///< Day of the week name offset value for Friday
#define DOW_THURSDAY       3  ///< Day of the week name offset value for Thursday
#define DOW_WEDNESDAY      2  ///< Day of the week name offset value for Wednesday
#define DOW_TUESDAY        1  ///< Day of the week name offset value for Tuesday
// ----------------------------------------------------------------------------
/// @brief MACRO to set the first day of the week for calculations.
/// @details The first day of the week is used in calculations for the DateTime::dayOfTheWeek()
///          and the DateTime::toString() methods.
/// @param DAY_NAME The name of the first day of the week, e.g. "Monday", "Sunday", etc.
#define MONTH_WEEKDAY_START(DAY_NAME) \
   (((DAY_NAME)[0] == 'M')                                                         ? DAY_1_IS_MONDAY    : \
   (((DAY_NAME)[0] == 'S') && (((DAY_NAME)[1] == 'u') || ((DAY_NAME)[1] == 'U')))  ? DAY_1_IS_SUNDAY    : \
   (((DAY_NAME)[0] == 'S') && (((DAY_NAME)[1] == 'a') || ((DAY_NAME)[1] == 'A')))  ? DAY_1_IS_SATURDAY  : \
    ((DAY_NAME)[0] == 'F')                                                         ? DAY_1_IS_FRIDAY    : \
   (((DAY_NAME)[0] == 'T') && (((DAY_NAME)[1] == 'h') || ((DAY_NAME)[1] == 'H')))  ? DAY_1_IS_THURSDAY  : \
    ((DAY_NAME)[0] == 'W')                                                         ? DAY_1_IS_WEDNESDAY : \
   (((DAY_NAME)[0] == 'T') && (((DAY_NAME)[1] == 'u') || ((DAY_NAME)[1] == 'U')))  ? DAY_1_IS_TUESDAY   : 5)  // Default to May for Monday
// ----------------------------------------------------------------------------
/// @brief MACRO to set the first day of the week name offset (order) to match the name.
/// @details The first day of the week name offset is used in the DateTime::toString() method.
///          The name array is static and hardcoded, this MACRO ensures that the name will
///          match the first day of the week used in calculations.
/// @param DAY_NAME The name of the first day of the week, e.g. "Monday", "Sunday", etc.
#define WEEKDAY_OFFSET(DAY_NAME) \
   (((DAY_NAME)[0] == 'M')                                                        ? DOW_MONDAY         : \
   (((DAY_NAME)[0] == 'S') && (((DAY_NAME)[1] == 'u') || ((DAY_NAME)[1] == 'U'))) ? DOW_SUNDAY         : \
   (((DAY_NAME)[0] == 'S') && (((DAY_NAME)[1] == 'a') || ((DAY_NAME)[1] == 'A'))) ? DOW_SATURDAY       : \
    ((DAY_NAME)[0] == 'F')                                                        ? DOW_FRIDAY         : \
   (((DAY_NAME)[0] == 'T') && (((DAY_NAME)[1] == 'h') || ((DAY_NAME)[1] == 'H'))) ? DOW_THURSDAY       : \
    ((DAY_NAME)[0] == 'W')                                                        ? DOW_WEDNESDAY      : \
   (((DAY_NAME)[0] == 'T') && (((DAY_NAME)[1] == 'u') || ((DAY_NAME)[1] == 'U'))) ? DOW_TUESDAY        : 0)  // Default to Monday
// ----------------------------------------------------------------------------
// These defines are used to set the first day of the week for the both the
// DateTime::dayOfTheWeek() and DateTime::toString() methods.
// They use the #define FIRST_WEEKDAY (above) to synchronize the first day of the week.
#define FIRST_WEEKDAY_MONTH MONTH_WEEKDAY_START(FIRST_WEEKDAY) ///< The first day of the week (1) is Monday in May 2000
#define WEEKDAY_NAME_OFFSET WEEKDAY_OFFSET(FIRST_WEEKDAY)      ///< First Day of the week name offset value for day names.
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
      DateTime(uint32_t t = SECONDS_FROM_1970_TO_2000);
      DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0,
               uint8_t min = 0, uint8_t sec = 0);
      DateTime(const DateTime& copy);
      DateTime(const char* date, const char* time);
      DateTime(const __FlashStringHelper* date, const __FlashStringHelper* time);
      DateTime(const char* iso8601date);
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
                   See: 'FIRST_WEEKDAY_MONTH' above.
      */
      uint8_t dayOfTheWeek() const;

      /*!
         @brief  Return the constant `WEEKDAY_NAME_OFFSET` which is defined
                  to be the offset to day of the week in the name array that
                  internally starts with "Mon" which isn't necessarily
                  the start of the week.
         @return Returns WEEKDAY_NAME_OFFSET
         @remarks The offset (order) of the day of the week name
                  based on the selected first day of the week.
                  Internally the text string is an array of 21 characters
                  with each day having 3 letters. The ((offset + dayOfTheWeek()) * 3)
                  gives the index into the array of names for the day of the week.
                  See: 'WEEKDAY_NAME_OFFSET' above.
      */
      static uint8_t dayNameOffset() { return (uint8_t)(WEEKDAY_NAME_OFFSET); }

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
      ///        in the month defined by 'FIRST_WEEKDAY_MONTH' above, in the year 2000.
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