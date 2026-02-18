/// @file BCMenu.h
/// @brief This file contains the declaration of the `BCMenu` class for the menu.
/// @details The `BCMenu` class manages the settings menu for the Binary Clock Shield.
///          It allows the user to set the time and alarm using the shield buttons.   
///          The `IBinaryClockBase` interface is used to interact with the clock hardware.  
///          The settings menu is a state machine that processes button presses.  
///          This class also handles the serial output of the settings menu.
/// @author Chris-70 (2025/09)

#pragma once
#ifndef __BC_MENU_H__
#define __BC_MENU_H__

#include <IBinaryClockBase.h>

#if TESTING    ///< Changes needed for unit testing of this code.
   #define TEST_VIRTUAL   virtual      ///< Virtul methods for unit testing ony.
   #define TEST_PROTECTED protected:   ///< Access specifier for unit testing ony.
#else
   #define TEST_VIRTUAL                ///< Virtual methods  only for unit testing, removed otherwise.
   #define TEST_PROTECTED              ///< Access specifier only for unit testing, removed otherwise.
#endif

namespace BinaryClockShield
   {
   /// @brief Enum class to indicate the current settings state.
   /// @details This enum class is used to indicate the current state of the settings menu.
   ///          It is used in the `ProcessMenu()` method to determine where we are in the 
   ///          settings menu processing. 
   enum class SettingsState : uint8_t
      {
      Inactive = 0,       ///< Settings menu is not active
      TimeSettings,       ///< Currently setting time
      AlarmSettings,      ///< Currently setting alarm
      Processing,         ///< Processing/saving settings
      Exiting             ///< Exiting settings menu
      };

   /// @brief Handles all settings menu functionality for the Binary Clock Shield.
   /// @details This class manages the settings menu and serial output of the menu.
   ///          It allows the user to set the time and alarm using the shield buttons.
   ///          The settings menu is a state machine that processes button presses
   ///          and updates the settings state accordingly. This class is designed
   ///          to be called from the main loop of the program every few ms.  
   /// @par IBinaryClockBase Interface
   ///          This class requires an implementation of the `IBinaryClockBase` interface
   ///          to interact with the clock hardware. This is passed in the constructor.  
   ///          The `IBinaryClockBase` interface provides methods to get and set the time, alarm,
   ///          12/24 hour mode and display the menu items on the shield.
   /// @par Menu Navigation
   ///          The alarm settings have 3 stages: 
   ///          1. Alarm ON; Alarm OFF; Cancel.
   ///          2. Set Hour (0-23 or 1-12 AM/PM).
   ///          3. Set Minute (0-59).  
   ///          The time settings have 4 stages:   
   ///          1. 12 Hour mode; 24 Hour mode; Cancel.
   ///          2. Set Hour (0-23 or 1-12 AM/PM).
   ///          3. Set Minute (0-59).
   ///          4. Set Second (0-59).
   /// @par Button Functions
   ///          The following button functions are available for navigating the settings menu:
   ///          - S1: Time set/Decrement value
   ///          - S2: Stop alarm/Save current value and move to next level
   ///          - S3: Alarm set/Increment value
   /// @par Display
   ///          The current setting being modified is displayed on the shield LEDs.   
   ///          The normal binary time display is suspended while in the settings menu.   
   ///          When exiting the settings menu, the display shows a Green **✓** [✅] if 
   ///          the settings were saved, or a Pink **X** [❌] if the settings were discarded.  
   ///          The 12 hour mode is displayed as `12` (LED[14]+LED[15]) with an `Indigo` `PM` 
   ///          indicator (LED[16]),  
   ///          The 24 hour mode is displayed as `24` (LED[15]+LED[16]).  
   ///          A green **✓** [✅] is displayed after mode selection before setting the time.
   /// @author Chris-70 (2025/09)
   class BCMenu
      {
   public:
      /// @brief Constructor that takes a reference to the `IBinaryClockBase` interface
      /// @details This constructor initializes the `BCMenu` class with a reference
      ///          to an `IBinaryClockBase` implementation. This allows the settings class to
      ///          interact with the clock hardware.
      /// @param clockInterface Reference to the `IBinaryClockBase` implementation
      /// @author Chris-70 (2025/09)
      explicit BCMenu(IBinaryClockBase& clockInterface);

      /// @brief Destructor - Exit with a clean state.
      virtual ~BCMenu();

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
      @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino;
      @author Chris-70 (2025/09)
      */
      SettingsState ProcessMenu();

      /// @brief Force exit from settings mode (for an emergency exit)
      /// @details This method can be called to force an exit from the settings menu.
      ///          It will reset all settings to their default values, aborting any
      ///          changes. This is not a normal exit path, it exists for the caller
      ///          to be able to force an exit of the settings menu.
      /// @author Chris-70 (2025/09)
      void ExitSettingsMode();

      #if SERIAL_SETUP_CODE
      /// @brief The method called to display the serial startup information.
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino;
      /// @author Chris-80 (2025/07)
      void SerialStartInfo();  // ToDo: Make something public to display info (BC.cpp)

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
      #endif

      #if SERIAL_TIME_CODE
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
      #endif

      /// @brief Get current settings state enumeration value
      /// @details This property returns the current state of the settings menu.
      ///          This is where we are in the settings menu processing.
      ///          When this property returns `SettingsState::Inactive` then 
      ///          we are not in the settings menu and the binary time should be displayed.
      /// @return Current settings state enum value
      /// @author Chris-70 (2025/09)
      SettingsState get_CurrentState() const { return currentState; }
    
   protected:
      /// @brief Enum class to classify the different settings types/levels in the settings menu. Type: uint8_t
      /// @details This enum class is used to classify the different settings types/levels in the settings menu.
      ///          It is used in the `GetSettingsType(int options, int level)` method to determine the current
      ///          settings type based on the options and level.
      /// @author Chris-70 (2025/08)
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
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino;
      /// @author Chris-70 (2025/09)
      void handleMainMenu();

      /// @brief This method is called to handle the settings option processing.
      /// @details This method is called from the `ProcessMenu()` method to handle
      ///          the settings option processing. This method handles the button
      ///          presses and updates the settings state accordingly.
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino; 
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
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void setCurrentModifiedValue();

      /// @brief The method called to check the current modified value format to stay within limits,
      ///        Hours 0 - 2; Minutes 0 - 59; Seconds 0 - 59, while the user is changing them.
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void checkCurrentModifiedValueFormat();

      /// @brief This method is used to save either the new time or alarm time set by the user.
      /// @details This method is called when the user has set a new time or alarm time from
      ///          the buttons on the Binary Clock Shield.
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void saveCurrentModifiedValue();

      /// @brief This method displays the value as the user is changing it. Only one row is
      ///        displayed at a time while the time is being update.: Hours; Minutes; Seconds or Alarm ON/OFF.
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void displayCurrentModifiedValue();
        
      ////////////////////
      // Helper methods //
      ////////////////////

      /// @brief This method resets the settings state and associated fields to their initial values.
      /// @details This has the effect of clearing the menu state to start over.
      /// @see SettingsState
      /// @author Chris-70 (2025/09)
      void resetSettingsState();

      /// @brief This method determines the current settings state based on the internal variables.
      /// @return The current `SettingsState` enum value.
      /// @see SettingsState
      /// @author Chris-70 (2025/09)
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
      /// @brief The method called to display the serial menu for setting the time and alarm.
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino;
      /// @author Chris-80 (2025/07)
      void serialSettings();

      /// @brief The method called to display the alarm settings over the serial monitor.
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino;
      /// @author Chris-80 (2025/07)
      void serialAlarmInfo();

      /// @brief The method called to display the current modified value over the serial monitor.
      /// @details This method is called when the user is changing the time or alarm time.
      /// @author Marcin Saj (2018) - From the original Binary Clock Shield for Arduino;
      /// @author Chris-80 (2025/07)
      void serialCurrentModifiedValue();
      #endif

   private:
   TEST_PROTECTED
      // Reference to the clock interface
      IBinaryClockBase& clock;

      // Local button references for efficiency
      const IBCButtonBase& buttonS1;  ///< Local reference to S1 button
      const IBCButtonBase& buttonS2;  ///< Local reference to S2 button
      const IBCButtonBase& buttonS3;  ///< Local reference to S3 button

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
      #if SERIAL_TIME_CODE
      bool isSerialTime;   ///< Flag to indicate if serial time output is enabled, default value to be set in constructor.
      #endif
      #if SERIAL_SETUP_CODE
      bool isSerialSetup;  ///< Flag to indicate if serial setup is enabled, default value to be set in constructor.
      #endif

      char buffer[64] = { 0 };       ///< Buffer for the DateTime string conversions

      #if SERIAL_OUTPUT
      // Setup strings for serial output that are used multiple times. Balance between flash and ram usage.
      static const String strSeparator;              ///< Repeated separator string, generated at runtime.
      static const String strBarrier;                ///< Repeated barrier string, generated at runtime.
      static const String strCurrentTime;           ///< Repeated current time string, generated at runtime. cutt
      #endif
  
      };
   }

   #undef TEST_VIRTUAL     // Remove the test virtual specifier define
   #undef TEST_PROTECTED   // Remove the test protected specifier define 

#endif // __BC_MENU_H__