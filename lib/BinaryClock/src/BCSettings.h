/// @file BCSettings.h
/// @brief This file contains the declaration of the `BCSettings` class
/// @details The `BCSettings` class manages the settings menu for the Binary Clock Shield.
///          It allows the user to set the time and alarm using the shield buttons.   
///          This class also handles the serial output of the settings menu.
/// @author Chris-70 (2025/09)

#pragma once
#ifndef __BCSETTINGS_H__
#define __BCSETTINGS_H__

#include "IBinaryClock.h"

namespace BinaryClockShield
   {
   /// @brief Enum class to indicate the current settings state
   enum class SettingsState : uint8_t
      {
      Inactive = 0,       ///< Settings menu is not active
      TimeSettings,       ///< Currently setting time
      AlarmSettings,      ///< Currently setting alarm
      Processing,         ///< Processing/saving settings
      Exiting             ///< Exiting settings menu
      };

   /// @brief Handles all settings menu functionality for the Binary Clock
   /// @author Chris-70 (2025/09)
   class BCSettings
      {
   public:
      /// @brief Constructor that takes a reference to the `IBinaryClock` interface
      /// @details This constructor initializes the `BCSettings` class with a reference
      ///          to an `IBinaryClock` implementation. This allows the settings class to
      ///          interact with the clock hardware.
      /// @param clockInterface Reference to the `IBinaryClock` implementation
      /// @author Chris-70 (2025/09)
      explicit BCSettings(IBinaryClock& clockInterface);

      /// @brief Destructor - Exit with a clean state.
      virtual ~BCSettings();

      /// @brief Initialize the settings class
      /// @details This is called first before using any other methods.
      ///          If called multiple times, it resets this object to the initial state.
      /// @author Chris-70 (2025/09)
      void Begin();

      //################################################################################//
      // SETTINGS
      //################################################################################//
      /*!
      @brief The method called to set the time and/or alarm from the shield
               The S1 button sets the Time, S3 sets the Alarm, S2 accepts
               the current modified value and moves to the next line.
               The S3 and S1 buttons increment/decrement the current modified value.
      @details The 'settingsMenu()' method displays the settings menu on the shield LEDs.
               The user can navigate through the menu using the S1, S2, and S3 buttons.
      @verbatim
               To enter the Alarm settings, the user presses the S3 button.
               To enter the Time  settings, the user presses the S1 button.
               The first selection (i.e. Level 1) is for the state or mode:
                     Alarm: ON; OFF; or Abort the alarm setting.
                     Time: 12 Hr; 24 Hr; or Abort the time setting.
               The second selection (i.e. Level 2) is for the hour.
               The third  selection (i.e. Level 3) is for the minute.
               The fourth selection (i.e. Level 4) is for the second (Time only)
      @endverbatim
               When the final selection is made the 'Rainbow' pattern is displayed
               to indicate to the user the changes are over and the settings are
               either being saved, indicated by the Green **✓** [✅], or the
               changes have been discarded, indicated by the Pink **X** [❌] on the shield.
      @verbatim

                            +-------------------------------+
                            |           SETTINGS            |
                +-----------+-------------------------------+
                |  BUTTONS  |    S3   |     S2    |   S1    |
      +---------+-----------+---------+-----------+---------+
      |         |           |   SET   |   ALARM   |  SET    |
      |         | Level = 0 |  ALARM  |   MELODY  |  TIME   |
      |   S     |           |         |   STOP    |         |
      +   E L   +-----------+---------+-----------+---------+
      |   T E   | Level = 1 |    +    |   SAVE    |    -    |
      |   T V   |           |         | LEVEL = 2 |         |
      +   I E   +-----------+---------+-----------+---------+
      |   N L   | Level = 2 |    +    |   SAVE    |    -    |
      |   G     |           |         | LEVEL = 3 |         |
      +   S     +-----------+---------+-----------+---------+
      |         | Level = 3 |    +    |   SAVE    |    -    |
      |         |           |         | LEVEL 0/4 |         |
      +         +-----------+---------+-----------+---------+
      |         | Level = 4 |    +    |   SAVE    |    -    |
      |         |           |         | LEVEL = 0 |         |
      +---------+-----------+---------+-----------+---------+

      @endverbatim
      When setting Time, the first option is to select the
      Display mode: 12 Hr; 24 Hr; or Abort time setting.
      When setting Alarm, the first option is to select the
      Alarm state: ON; OFF; or Abort the alarm setting.
      @verbatim

                            +-------------------------------+
                            |        SETTINGS OPTION        |
                            +---------------+---------------+
                            |   ALARM = 3   |   TIME = 1    |
      +---------------------+---------------+---------------+
      |         | Level = 1 |     3/1       | 1/1   Abort / |
      |         |           | On/Off/Abort  | 12 Hr/ 24 Hr  |
      |   S     |           |  (Row: All)   |   (Row: H)    |
      +   E L   +-----------+---------------+---------------+
      |   T E   | Level = 2 |     3/2       |     1/2       |
      |   T V   |           |     HOUR      |     HOUR      |
      |   I E   |           |   (Row: H)    |   (Row: H)    |
      +   N L   +-----------+---------------+---------------+
      |   G     | Level = 3 |     3/3       |     1/3       |
      |   S     |           |    MINUTE     |    MINUTE     |
      |         |           |   (Row: M)    |   (Row: M)    |
      +         +-----------+---------------+---------------+
      |         | Level = 4 |     N/A       |     1/4       |
      |         |           |               |    SECOND     |
      |         |           |               |   (Row: S)    |
      +---------+-----------+---------------+---------------+

      @endverbatim
      @return Current `SettingsState` enum value that signals where in the 
               processing of the settings menu we are.
      @note The binary time should be displayed only when this method returns:
            `SettingsState::Inactive` - i.e. we are not in the settings menu.
            The settings menu uses the display for setting the time/alarm 
            thus the normal time display is suspended while in the menu.
      @author Marcin Saj - From the original Binary Clock Shield for Arduino
      @author Chris-80 (2025/07)
      */
      /// @brief Main settings menu handler - call this from the main loop.
      /// @details This method handles the state machine for the settings menu.
      ///          It calls the methods that process the settings state.  
      ///          Inactive - Check for button press to set time/alarm  
      ///          Exit - Handle exit process  
      ///          Anything else - Handle the settings level processing.
      /// @returns Current settings state.
      /// @author Chris-70 (2025/09)
      SettingsState ProcessMenu();

      /// @brief Force exit from settings mode (for an emergency exit)
      /// @details This method can be called to force an exit from the settings menu.
      ///          It will reset all settings to their default values, aborting any
      ///          changes. This is not a normal exit path, it exists for the caller
      ///          to be able to force an exit of the settings menu.
      /// @author Chris-70 (2025/09)
      void ExitSettingsMode();

      /// @brief Get current settings state enumeration value
      /// @details This property returns the current state of the settings menu.
      ///          This is where we are in the settings menu processing.
      ///          When this property returns `SettingsState::Inactive` then 
      ///          we are not in the settings menu and the binary time should be displayed.
      /// @return Current settings state enum value
      /// @author Chris-70 (2025/09)
      SettingsState get_CurrentState() const { return currentState; }

      /// @brief Property pattern for the 'IsSerialSetup' flag property. 
      ///        This property controls whether the serial setup menu is displayed or not.
      /// @param value The flag to set (true to display the serial setup menu, false to disable it).
      /// @return The current flag value (true to display the serial setup menu, false to disable it).
      /// @see get_IsSerialSetup()
      /// @author Chris-80 (2025/07)
      /// @ingroup properties
      void set_IsSerialSetup(bool value) { isSerialSetup = value; }
      
      /// @copydoc set_IsSerialSetup()
      /// @see set_IsSerialSetup()
      bool get_IsSerialSetup() const { return isSerialSetup; }

      /// @brief Property pattern for the 'IsSerialTime' flag property.
      ///        This property controls whether the serial time is displayed or not.
      /// @param value The flag to set (true to display the serial time, false to disable it).
      /// @return The current flag value (true to display the serial time, false to disable it).
      /// @see get_IsSerialTime()
      /// @author Chris-80 (2025/07)
      /// @ingroup properties
      void set_IsSerialTime(bool value) { isSerialTime = value; }

      /// @copydoc set_IsSerialTime()
      /// @see set_IsSerialTime()
      bool get_IsSerialTime() const { return isSerialTime; }
    
   protected:
      /// @brief Enum class to classify the different settings types/levels in the settings menu. Type: uint8_t
      enum class SettingsType : uint8_t 
            { 
            Undefined,     ///< Error: value of 0, not in settings menu.
            TimeOptions,   ///< Time options: 12 or 24 hour mode; Cancel
            Hours,         ///< Setting the hours value.
            Minutes,       ///< Setting the minutes value.
            Seconds,       ///< Setting the seconds value (time only).
            AlarmStatus    ///< Setting the alarm status: ON; OFF; Cancel
            };

      /// @brief This method is used to get the settings type based on the options and level.
      /// @param options The current settings options, e.g. TimeOptions (1), AlarmStatus (3).
      /// @param level The current settings level, e.g. 1 - 4.
      /// @return The SettingsType enum value that corresponds to the options and level.
      /// @author Chris-70 (2025/08)
      SettingsType GetSettingsType(int options, int level) const ;

      #if SERIAL_OUTPUT
      /// @brief This method is called to format the hour for display in 12-hour or 24-hour format.
      /// @param hour24 The hour in 24-hour format (0-23).
      /// @param is12HourFormat Flag indicating if 12-hour format is requested.
      /// @param buffer The character buffer to store the formatted hour.
      /// @param size The size of the buffer.
      /// @return A pointer to the given `buffer` containing the formatted hour string.
      /// @author Chris-70 (2025/08)
      char* FormatHour(int hour24, bool is12HourFormat, char* buffer, size_t size);
      #endif
    
   private:
      // Core settings processing methods
      /// @brief This method is called to handle the main menu processing.
      /// @details This method is called from the `ProcessMenu()` method to handle
      ///          the main menu processing. This method handles the button
      ///          presses and updates the settings state accordingly.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-70 (2025/09)
      void handleMainMenu();

      /// @brief This method is called to handle the settings option processing.
      /// @details This method is called from the `ProcessMenu()` method to handle
      ///          the settings option processing. This method handles the button
      ///          presses and updates the settings state accordingly.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-70 (2025/09)
      void handleSettingsLevel();

      /// @brief This method is called to handle the exit process when exiting the settings menu.
      /// @details This method is called from the `ProcessMenu()` method to handle
      ///          the exit process when exiting the settings menu. This method handles the
      ///          saving or discarding of the settings and updates the settings state accordingly.
      /// @author Chris-70 (2025/09)
      void handleExitProcess();
      
      // Settings state management
      /// @brief This method is called when the user exceeds the current time element limits.
      ///        The value is rolled over to the next valid value, e.g. 59  -> 0, or 0 -> 59.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void setCurrentModifiedValue();

      /// @brief The method called to check the current modified value format to stay within limits,
      ///        Hours 0 - 2; Minutes 0 - 59; Seconds 0 - 59, while the user is changing them.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void checkCurrentModifiedValueFormat();

      /// @brief This method is used to save either the new time or alarm time set by the user.
      /// @details This method is called when the user has set a new time or alarm time from
      ///          the buttons on the Binary Clock Shield.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void saveCurrentModifiedValue();

      /// @brief This method displays the value as the user is changing it. Only one row is
      ///        displayed at a time while the time is being update.: Hours; Minutes; Seconds or Alarm ON/OFF.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void displayCurrentModifiedValue();
        
      // Helper methods
      // SettingsType getSettingsType(int options, int level) const;
      void resetSettingsState();
      SettingsState determineCurrentState() const;

      #if SERIAL_OUTPUT
      /// @brief Helper function to fill a string with a repeated character.
      /// @remarks This trades a bit of speed for flash memory savings by 
      ///          creating the string on the fly. If the string is local
      ///          then it's just temporary ram usage.
      /// @param ch The character to repeat.
      /// @param repeat The number of times to repeat the character.
      /// @return A String filled with the repeated character.
      /// @author Chris-70 (2025/08)
      static String fillStr(char ch, byte repeat);
      #endif

      // These methods are all part of the serial menu display and can be removed at compile time. They can be 
      // can be controlled in both software and hardware. A momentary button is used to toggle the menu on/off.
      #if SERIAL_SETUP_CODE
      /// @brief The method called to display the serial startup information.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void serialStartInfo();  // ToDo: Make somethins public to display info (BC.cpp)

      /// @brief The method called to display the serial menu for setting the time and alarm.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino;
      /// @author Chris-80 (2025/07)
      void serialSettings();

      /// @brief The method called to display the alarm settings over the serial monitor.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino;
      /// @author Chris-80 (2025/07)
      void serialAlarmInfo();

      /// @brief The method called to display the current modified value over the serial monitor.
      /// @details This method is called when the user is changing the time or alarm time.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino;
      /// @author Chris-80 (2025/07)
      void serialCurrentModifiedValue();
      #endif

      // #if SERIAL_TIME_CODE
      // /// @brief The method called to display the current time, decimal and binary, over the serial monitor.
      // /// @details While this method can still be removed at compile time, it can also be controlled, at run-time, 
      // ///          in software and hardware. This method is usually called every second so being able to control the         
      // ///          output in software and hardware, by using a switch or jumper, can start/stop the serial time display.
      // /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      // /// @author Chris-80 (2025/07)
      // void SerialTime(DateTime time, char* format) const;
      // #endif

   private:
      // Reference to the clock interface
      IBinaryClock& clock;

      // Local button references for efficiency
      const BCButton& buttonS1;  ///< Local reference to S1 button
      const BCButton& buttonS2;  ///< Local reference to S2 button
      const BCButton& buttonS3;  ///< Local reference to S3 button

      // Settings state variables
      SettingsState currentState = SettingsState::Inactive;
      int settingsOption = 0;        ///< Time = 1, Alarm = 3  
      int settingsLevel = 0;         ///< Hours = 1, Minutes = 2, Seconds / On/Off Alarm = 3
      int countButtonPressed = 0;    ///< Counter for button pressed during settings
      
      // Temporary values during settings
      DateTime tempTime;             ///< Temporary time variable used when setting time
      AlarmTime tempAlarm;           ///< Temporary Alarm used when setting alarm
      bool tempAmPm = false;         ///< Temporary flag for 12/24 Hr mode when setting time

      // Exit state management
      bool exit = false;             ///< Flag to exit settings (Finished or Abort)
      bool abort = false;            ///< Flag to abort settings, don't save
      uint8_t exitStage = 0U;        ///< Stage of exit process
      unsigned long delayTimer = 0UL; ///< Delay timer instead of using delay()
      bool continueS2 = false;       ///< Flag to resume 'buttonS2' processing after delay
      bool isSerialTime = DEFAULT_SERIAL_TIME;
      bool isSerialSetup = DEFAULT_SERIAL_SETUP;

      char buffer[64] = { 0 };       ///< Buffer for the DateTime string conversions

      #if SERIAL_SETUP_CODE
      // Setup strings for serial output that are used multiple times. Balance between flash and ram usage.
      static const String STR_SEPARATOR;              ///< Repeated separator string, generated at runtime.
      static const String STR_BARRIER;                ///< Repeated barrier string, generated at runtime.
      static const char PROGMEM STR_TIME_SETTINGS[];  ///< Repeated time settings string, stored in flash memory.
      static const char PROGMEM STR_ALARM_SETTINGS[]; ///< Repeated alarm settings string, stored in flash memory.
      static const char PROGMEM STR_CURRENT_TIME[];   ///< Repeated current time string, stored in flash memory.
      #endif
  
      };
   }

#endif