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

#include <stdint.h>                 /// Integer types: size_t; uint8_t; uint16_t; etc.

#include "DateTime.h"               /// For the `DateTime` and `TimeSpan` classes (https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/RTClibPlus)

#include "BinaryClock.Defines.h"    /// BinaryClock project-wide definitions and MACROs.
#include "BinaryClock.Structs.h"    /// Global structures and enums used by the Binary Clock project.
#include "BCButton.h"               /// Binary Clock Button class: handles all button related functionality.

#if STL_USED
   // STL classes required to be included (when using the STL):
   #include <vector> 
#endif

namespace BinaryClockShield
   {
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
      /// @note The DateTime class is defiend in the RTCLib.h header file.
      /// @see get_Time()
      /// @see ReadTime()
      /// @author Chris-80 (2025/07)
      virtual void set_Time(DateTime value) = 0;
      /// @copydoc set_Time()
      /// @return A `DateTime` object containing the current time.
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
      /// @see get_S2SaveStop()
      /// @see get_S3AlarmInc()
      /// @author Chris-70 (2025/09)
      virtual const BCButton& get_S1TimeDec() const = 0;

      /// @brief Read only property pattern to get a const reference to the `S2`
      ///        `BCButton` object used for saving a selection or stopping an alarm.
      /// @return A const reference to the `BCButton` for S2 on the shield.
      /// @see get_S1TimeDec()
      /// @see get_S3AlarmInc()
      /// @author Chris-70 (2025/09)
      virtual const BCButton& get_S2SaveStop() const = 0;

      /// @brief Read only property pattern to get a const reference to the `S3`
      ///        `BCButton` object used for setting alarm and incrementing a value.
      /// @return A const reference to the `BCButton` for S3 on the shield.
      /// @see get_S1TimeDec()
      /// @see get_S2SaveStop()
      /// @author Chris-70 (2025/09)
      virtual const BCButton& get_S3AlarmInc() const = 0;

      /// @brief Read only property pattern to get the unique identifier name of the 
      ///        Binary Clock implementation. This is implementation defined.
      /// @note  This shall return a valid `C` string, never a `nullptr`.
      /// @return A pointer to a constant character string containing the unique identifier name.
      /// @author Chris-70 (2025/10)
      virtual const char* get_IdName() const = 0;
      /// @}

      /// @brief The method to read the time from the RTC (e.g. wrapper for RTC.now()). 
      /// @details This method reads the current time from the RTC and returns it as a 
      ///          DateTime object.  A call to `get_Time()` shall return the same value.
      ///          This method may need to communicate with the RTC over an I2C or SPI bus.
      /// @return A DateTime object containing the current time read from the RTC.
      /// @see get_Time()
      /// @author Chris-70 (2025/07)
      virtual DateTime ReadTime() = 0;

      // Display operations
      /// @brief Display the given LED pattern `patternType` on the shield.
      /// @details The patterns are defined in the `LedPattern` enum.  
      /// @param patternType The LED pattern to display.
      /// @see LedPattern
      /// @author Chris-70 (2025/08)
      virtual void DisplayLedPattern(LedPattern patternType) = 0;

      /// @brief The method called to convert the time to binary and update the LEDs.
      /// @details This method converts the current time to binary and updates the LEDs 
      ///          using the color values defined in the arrays 'OnColor' and 'OffColor'
      /// @param hoursRow The value for the top, to display the hour LEDs (16-12).
      /// @param minutesRow The value for the middle, to display the minute LEDs (11-6).
      /// @param secondRow The value for the bottom, to display the second LEDs (5-0).
      /// @param use12HourMode Flag indicating whether to use 12-hour format.
      /// @see set_Brightness() for the brightness of the LEDs.
      /// @see DisplayLedPattern() for displaying the full LED buffer as defined.
      /// @author Chris-80 (2025/07)
      virtual void DisplayBinaryTime(int hours, int minutes, int seconds, bool use12Hour = false) = 0;

      /// @brief Methods to register/unregister a callback function at every second.
      /// @details The callback function will be called with the current DateTime every second.
      ///          The callback function should match the signature: `void callback(const DateTime&)`.
      /// @param callback The function to call every second with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @see UnregisterTimeCallback()
      /// @see RegisterAlarmCallback()
      /// @author Chris-70 (2025/07)
      virtual bool RegisterTimeCallback(void (*callback)(const DateTime&)) = 0;
      /// @copydoc RegisterTimeCallback()
      /// @see RegisterTimeCallback()
      /// @see UnregisterAlarmCallback()
      virtual bool UnregisterTimeCallback(void (*callback)(const DateTime&)) = 0;

      /// @brief  Methods to register/unregister a callback function for the alarm.
      ///         The callback function is called when the alarm is triggered.
      /// @details The callback function will be called with the Alarm DateTime that triggered the alarm.
      ///          The callback function should match the signature: `void callback(const DateTime&)`.
      /// @param callback The function to call when the alarm is triggered with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @see RegisterTimeCallback()
      /// @see UnregisterAlarmCallback()
      /// @author Chris-70 (2025/07)
      virtual bool RegisterAlarmCallback(void (*callback)(const DateTime&)) = 0;
      /// @copydoc RegisterAlarmCallback()
      /// @see RegisterAlarmCallback()
      /// @see UnregisterTimeCallback()
      virtual bool UnregisterAlarmCallback(void (*callback)(const DateTime&)) = 0;

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