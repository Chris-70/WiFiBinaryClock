/// @file IBinaryClock.h
/// @brief The pure interface class that defines the minimum supported features of the Binary Clock.
/// @details This interface class provides the basic methods that must be implemented by any class
///          that implements the Binary Clock functionality. This is used to decouple the implementation
///          class (e.g. BinaryClock) from any support classes that may need to interface with it
///          (e.g. BCSettings).
/// 
///          This file also contains structures and enumerations used by the Binary Clock.
///        - The AlarmTime structure is used to hold all the information related to a specific alarm,
///          including the alarm number, time, melody, status, and whether it has fired or not.     
///        - The Note structure is used to define a musical note with a frequency and duration.
///        - The LedPattern enumeration is used to define the different LED patterns supported by 
///          the Binary Clock. 
/// @author Chris-70 (2025/09)

#pragma once
#ifndef __IBINARYCLOCK_H__
#define __IBINARYCLOCK_H__

#include <Arduino.h>
#include <RTClib.h>

#include "BinaryClock.Defines.h"
#include "BCButton.h"

#if STL_USED
#include <vector> 
#endif

namespace BinaryClockShield
   {
   /// @brief The structure holds all the Alarm information used by the Binary Clock.
   /// @details This structure contains all the information related to a specific alarm, including
   ///          the alarm number, time, melody, status, and whether it has fired or not.   
   ///          While repeating the alarm based the date or day of the week instead of just daily is
   ///          supported by the DS3231 RTC, ... @todo finish.
   /// @note  The 'melody' selection has been implemented for most boards that support STL.    
   ///        The UNO_R3 will use the internal melody or one other user supplied melody.
   /// @author Chris-80 (2025/07)
   typedef struct alarmTime
      {
      enum Repeat             ///< Alarm repeation when ON. Default is Daily.
         {
         Never = 0,           ///< The alarm is turned OFF after it has fired.
         Hourly,              ///< The alarm repeats every hour at the selected minute
         Daily,               ///< The alarm repeats every day at the same time. This is the default.
         Weekly,              ///< The alarm repeats every week on the given day and time.
         Monthly,             ///< The alarm repeats every month on the given date and time.
         endTag               ///< Always the last enum value, defines the size.
         };

      uint8_t  number;        ///< The number of the alarm: 1 or 2
      DateTime time;          ///< The time of the alarm as a DateTime object
      uint8_t  melody;        ///< The melody to play when the alarm is triggered, 0 = internal melody
      uint8_t  status;        ///< Status of the alarm: 0 - inactive, 1 - active
      Repeat   freq;          ///< The alarm repeat frequency, default: Daily.
      bool     fired;         ///< The alarm has fired (e.g. alarm is 'ringing').
      void clear()            ///< Clear all data except the alarm 'number'
         {
         time = DateTime();   // 00:00:00 (2000-01-01)
         melody = 0;          // Default melody number, internal
         status = 0;          // OFF, alarm is not set.
         freq = Daily;        // The alarm repeats every day when ON.
         fired = false;       // Alarm is not ringing (OFF)
         }
      } AlarmTime;
  
   /// @brief The structure to create a note with a sound frequency and duration.
   /// @details The melody used to signal an alarm uses an array of these to create
   ///          the alarm sound.
   struct Note
      {
      unsigned tone;          ///< The tone frequency in Hz.
      unsigned long duration; ///< The duration of the tone in ms.
      };

   /// @brief Enum class to define the index to different LED patterns. Type: uint8_t
   /// @remarks The enum values correspond to the first index of the 2D `ledPatternsP` 
   ///          array of `CRGB` colors stored in flash memory.
   /// @note  The `endTAG` is equal to the number of patterns defined (7 or 8) and must
   ///        be the last entry in the enum. To reduce the use of flash memory for overhead,
   ///        all full shield patters are stored together on the 2D array. The enum acts
   ///        as the index to each pattern/color set so care must be taken to ensure
   ///        the correct pattern/color set is stored at the correct index.
   enum class LedPattern : uint8_t
         { 
         onColors = 0,  ///< The LED colors when ON (hours; minutes; seconds).
         offColors,     ///< The LED colors when OFF (usually Black; no power).
         onText,        ///< The big Green **`O`** for the On pattern.
         offTxt,        ///< The big RED sideways **`F`** for the oFF pattern.
         xAbort,       ///< The big Pink **`X`** [❌] for the abort/cancel pattern.
         okText,       ///< The big Lime **`✓`** [✅] for the okay/good pattern.
         rainbow,      ///< The colors of the rainbow on the diagnal pattern.
         #if ESP32_WIFI
         wText,         ///< The big RoyalBlue **`W`** [📶] for the WPS / WiFi pattern.
         #endif
         endTAG         ///< The end marker, also equal to the number of patterns defined (7 or 8).
         };

   /// @brief Pure abstract interface for BinaryClock functionality to reduce dependancies
   ///        and make testing easier. This follows the Interface pattern.
   /// @details We are using the Interface pattern to decouple the implementation class
   ///          (e.g. BinaryClock) from any support classes that may need to interface with it 
   ///          (e.g. BCSettings).   
   ///          The pattern we're using is similar to the early C# Interface in that all
   ///          methods and properties are public and pure abstract. No fields are defined
   ///          and all inheritance are limited to other interface classes.   
   ///          Interface classes all begin with a capital 'I' followed by the proper class 
   ///          name, also beginning with a capital letter [A-Z].
   /// @code{.cpp} Examples:
   ///          class ISomething         // Interface class for "Something"
   ///          class IImplementSometing // Interface class for "ImplementSomething"
   ///          
   ///          class ImplementSomething  // NOT an interface, just a class.
   ///          class IMplementSomething  // Interface class for "MplementSomething"
   /// @endcode
   /// @author Chris-70 (2025/09)
   class IBinaryClock
      {
   public:
      /// @brief Required virtual destructor for proper memory management and release.
      virtual ~IBinaryClock() = default;

      /// @ingroup properties
      /// @{
      /// @brief The property methods called to set/get the current 'Time' property.
      /// @param value The DateTime object containing the current time to set.
      /// @return A DateTime object containing the current time.
      /// @note The DateTime class is defiend in the RTCLib.h header file.
      /// @see get_Time()
      /// @see ReadTime()
      /// @author Chris-80 (2025/07)
      virtual void set_Time(DateTime value) = 0;
      /// @copydoc set_Time()
      /// @ingroup properties
      virtual DateTime get_Time() const = 0;

      /// @brief The property method called to set/get the current 'Alarm' property.
      /// @param value The AlarmTime structure containing the alarm time and status.
      /// @return An AlarmTime structure containing the alarm time and status.
      /// @note The AlarmTime structure contains the hour, minute, and status of the alarm
      ///       The status is 0 for inactive, 1 for active.
      ///       Hours are 0 to 23.
      /// @see get_Alarm()
      /// @see GetRtcAlarm(int)
      /// @author Chris-80 (2025/07)
      virtual void set_Alarm(AlarmTime value) = 0;
      /// @copydoc set_Alarm()
      /// @see set_Alarm()
      virtual AlarmTime get_Alarm() const = 0;

      /// @brief Property pattern for the 'Is12HourFormat' flag property.
      ///        This property controls whether the time is displayed in 
      ///        12-hour or 24-hour format.
      /// @param value The flag to set (true for 12-hour format, false for 24-hour format).
      /// @return The current flag value (true for 12-hour format, false for 24-hour format).
      /// @see get_Is12HourFormat()
      /// @author Chris-70 (2025-07)
      virtual void set_Is12HourFormat(bool value) = 0;
      /// @copydoc set_Is12HourFormat()
      /// @see set_Is12HourFormat()
      virtual bool get_Is12HourFormat() const = 0;

      /// @ingroup properties
      /// @{
      /// @brief Read only property pattern for the 'TimeFormat' string property.
      ///        This property returns the current format of the time string used for `format` in
      ///        the `DateTime::toString(char *buffer, size_t size, const char* format)` method.
      /// @returns A pointer to a constant character string containing the current time format.
      /// @see get_AlarmFormat()
      /// @author Chris-70 (2025-08)
      virtual char* const get_TimeFormat() const = 0;

      /// @brief Read only property pattern for the 'AlarmFormat' string property.
      ///        This property returns the current format of the alarm string used for `format` in
      ///        the `DateTime::toString(char *buffer, size_t size, const char* format)` method.
      /// @returns A pointer to a constant character string containing the current alarm format.
      /// @see get_TimeFormat()
      /// @author Chris-70 (2025-08)
      virtual char* const get_AlarmFormat() const = 0;

      /// @brief Read only property pattern for the 'IsSerialSetup' flag property. 
      ///        This property controls whether the serial setup menu is displayed or not.
      /// @return The current flag value (true to display the serial setup menu, false to disable it).
      /// @see get_IsSerialTime()
      /// @author Chris-80 (2025/07)
      virtual bool get_IsSerialSetup() const = 0;

      /// @brief Property pattern for the 'IsSerialTime' flag property.
      ///        This property controls whether the serial time is displayed or not.
      /// @return The current flag value (true to display the serial time, false to disable it).
      /// @see get_IsSerialSetup()
      /// @author Chris-80 (2025/07)
      virtual bool get_IsSerialTime() const = 0;

      /// @brief Read only property pattern to get a const reference to the `S1`
      ///        `BCButton` object used for setting time and decrementing a value.
      /// @return A const reference to the `BCButton` for S1 on the shield.
      /// @see get_SaveStopS2()
      /// @see get_AlarmIncS3()
      /// @author Chris-70 (2025/09)
      virtual const BCButton& get_TimeDecS1() const = 0;

      /// @brief Read only property pattern to get a const reference to the `S2`
      ///        `BCButton` object used for saving a selection or stopping an alarm.
      /// @return A const reference to the `BCButton` for S2 on the shield.
      /// @see get_TimeDecS1()
      /// @see get_AlarmIncS3()
      /// @author Chris-70 (2025/09)
      virtual const BCButton& get_SaveStopS2() const = 0;

      /// @brief Read only property pattern to get a const reference to the `S3`
      ///        `BCButton` object used for setting alarm and incrementing a value.
      /// @return A const reference to the `BCButton` for S3 on the shield.
      /// @see get_TimeDecS1()
      /// @see get_SaveStopS2()
      /// @author Chris-70 (2025/09)
      virtual const BCButton& get_AlarmIncS3() const = 0;
      /// @}

      /// @brief The method to read the time from the RTC (e.g. wrapper for RTC.now()). 
      /// @details This method reads the current time from the RTC and returns it as a 
      ///          DateTime object.  A call to `get_Time()` shall return the same value.
      ///          This method may need to communicate with the RTC over an I2C or SPI bus.
      /// @return A DateTime object containing the current time read from the RTC.
      /// @see get_Time()
      virtual DateTime ReadTime() = 0;

      // Display operations
      virtual void DisplayLedPattern(LedPattern patternType) = 0;
      virtual void DisplayBinaryTime(int hours, int minutes, int seconds, bool use12Hour = false) = 0;

      /// @brief Methods to register/unregister a callback function at every second.
      /// @param callback The function to call every second with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @see UnregisterTimeCallback()
      /// @see RegisterAlarmCallback()
      /// @author Chris-70 (2025/07)
      virtual bool RegisterTimeCallback(void (*callback)(DateTime)) = 0;
      /// @copydoc RegisterTimeCallback()
      /// @see RegisterTimeCallback()
      /// @see UnregisterAlarmCallback()
      virtual bool UnregisterTimeCallback(void (*callback)(DateTime)) = 0;

      /// @brief  Methods to register/unregister a callback function for the alarm.
      ///         The callback function is called when the alarm is triggered.
      /// @param callback The function to call when the alarm is triggered with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @see RegisterTimeCallback()
      /// @see UnregisterAlarmCallback()
      /// @author Chris-70 (2025/07)
      virtual bool RegisterAlarmCallback(void (*callback)(DateTime)) = 0;
      /// @copydoc RegisterAlarmCallback()
      /// @see RegisterAlarmCallback()
      /// @see UnregisterTimeCallback()
      virtual bool UnregisterAlarmCallback(void (*callback)(DateTime)) = 0;

      // Utility
      virtual void PlayAlarm(const AlarmTime& alarm) const = 0;

      #if STL_USED
      /// @brief Play a specific melody by its registry id.
      /// @param id The id of the melody in the melodyRegistry to play.
      /// @return True if the id was valid and melody played, false if id was invalid.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      virtual bool PlayMelody(size_t id) const = 0;

      /// @brief Register a melody in the melody registry. 
      /// @remarks ID 0 is always the default melody stored in ROM (flash memory).  
      ///          The ID can be used as the alarm melody for a given alarm.
      /// @param melody A reference to the vector of Note objects to register.
      /// @return The ID of the registered melody in the registry.
      /// @see set_Alarm()
      /// @see set_Melody()
      /// @see PlayMelody(size_t id)
      /// @see GetMelodyById()
      /// @author Chris-70 (2025/09)
      virtual size_t RegisterMelody(const std::vector<Note>& melody) = 0;

      /// @brief Get a melody from the registry by its ID (returned from `RegisterMelody()`).
      /// @param id The id of the melody in the registry.
      /// @return A reference to the melody vector, or the default melody if id is invalid.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      virtual const std::vector<Note>& GetMelodyById(size_t id) const = 0;
      #endif
      };
   }

#endif