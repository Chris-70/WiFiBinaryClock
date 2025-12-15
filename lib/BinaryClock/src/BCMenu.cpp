/// @file BCMenu.cpp
/// @brief This file contains the implementation of the `BCMenu` class
///        which manages the settings menu for the Binary Clock Shield.
/// @author Chris-70 (2025/09)

#include <Arduino.h>             /// Arduino core library. This needs to be the first include file.

#include <BinaryClock.Defines.h> /// BinaryClock project-wide definitions and MACROs.
#include "BCMenu.h"              /// Binary Clock Settings class: handles all settings and serial output.

#include <Streaming.h>           /// Streaming serial output with `operator<<` https://github.com/janelia-arduino/Streaming
#include <assert.h>              /// Catch code logic errors during development.

namespace BinaryClockShield
   {
   BCMenu::BCMenu(IBinaryClockBase& clockInterface)
         : clock(clockInterface)
         , buttonS1(clock.get_S1TimeDec())
         , buttonS2(clock.get_S2SaveStop())
         , buttonS3(clock.get_S3AlarmInc())
         , isSerialTime(clock.get_IsSerialTime())
         , isSerialSetup(clock.get_IsSerialSetup())
      {
      }

   BCMenu::~BCMenu()
      {
      // Ensure clean state when object is destroyed
      resetSettingsState();
      }      

   void BCMenu::Begin()
      { 
      resetSettingsState();
         
      #if SERIAL_OUTPUT
      Serial.begin(115200);
      delay(10);
      Serial.println(fillStr('_', 44));
      Serial.println(F("|      Software from the Chris Team        |"));
      Serial.println(F("|        (Chris-70 and Chris-80)           |"));
      Serial.println(F("|      Designed to run the fantastic:      |"));
      Serial << STR_BARRIER << endl;
      Serial.println(F("#     'Binary Clock Shield for Arduino'    #"));
      Serial << STR_BARRIER << endl;
      Serial.println(F("#      Shield created by Marcin Saj,       #"));
      Serial.println(F("#        https://nixietester.com/          #"));
      Serial.println(F("# product/binary-clock-shield-for-arduino/ #"));
      Serial << STR_BARRIER << endl;
      Serial.println(F("#  This software is licensed under the GNU #"));
      Serial.println(F("#     General Public License (GPL) v3.0    #"));
      Serial << STR_BARRIER << endl;
      Serial.println();
      #endif

      }

   //################################################################################//
   // SETTINGS
   //################################################################################//
   /// The core of the settings menu
   /// @verbatim
   ///
   ///                       +-------------------------------+
   ///                       |           SETTINGS            |
   ///           +-----------+-------------------------------+
   ///           |  BUTTONS  |    S3   |     S2    |   S1    |
   /// +---------+-----------+---------+-----------+---------+
   /// |         |           |   SET   |   ALARM   |  SET    |
   /// |         | Level = 0 |  ALARM  |   MELODY  |  TIME   |
   /// |   S     |           |         |   STOP    |         |
   /// +   E L   +-----------+---------+-----------+---------+
   /// |   T E   | Level = 1 |    +    |   SAVE    |    -    |
   /// |   T V   |           |         | LEVEL = 2 |         |
   /// +   I E   +-----------+---------+-----------+---------+
   /// |   N L   | Level = 2 |    +    |   SAVE    |    -    |
   /// |   G     |           |         | LEVEL = 3 |         |
   /// +   S     +-----------+---------+-----------+---------+
   /// |         | Level = 3 |    +    |   SAVE    |    -    |
   /// |         |           |         | LEVEL 0/4 |         |
   /// +         +-----------+---------+-----------+---------+
   /// |         | Level = 4 |    +    |   SAVE    |    -    |
   /// |         |           |         | LEVEL = 0 |         |
   /// +---------+-----------+---------+-----------+---------+
   ///
   /// @endverbatim
   /// When setting Time, the first option is to select the 
   /// Display mode: 12 Hr; 24 Hr; or Abort time setting.
   /// When setting Alarm, the first option is to select the
   /// Alarm state: ON; OFF; or Abort the alarm setting.
   /// @verbatim
   /// 
   ///                       +-------------------------------+
   ///                       |        SETTINGS OPTION        |
   ///                       +---------------+---------------+
   ///                       |   ALARM = 3   |   TIME = 1    |
   /// +---------------------+---------------+---------------+
   /// |         | Level = 1 |     3/1       | 1/1   Abort / |  
   /// |         |           | On/Off/Abort  | 12 Hr/ 24 Hr  |  
   /// |   S     |           |  (Row: All)   |   (Row: H)    |
   /// +   E L   +-----------+---------------+---------------+
   /// |   T E   | Level = 2 |     3/2       |     1/2       |
   /// |   T V   |           |     HOUR      |     HOUR      |
   /// |   I E   |           |   (Row: H)    |   (Row: H)    |
   /// +   N L   +-----------+---------------+---------------+
   /// |   G     | Level = 3 |     3/3       |     1/3       |
   /// |   S     |           |    MINUTE     |    MINUTE     |
   /// |         |           |   (Row: M)    |   (Row: M)    |
   /// +         +-----------+---------------+---------------+
   /// |         | Level = 4 |     N/A       |     1/4       |
   /// |         |           |               |    SECOND     |
   /// |         |           |               |   (Row: S)    |
   /// +---------+-----------+---------------+---------------+
   ///
   /// @endverbatim
   ///          When the final selection is made the 'Rainbow' pattern is displayed
   ///          to indicate to the user the changes are over and the settings are 
   ///          either being saved, indicated by the Green **✓**  [✅], or the 
   ///          changes have been discarded, indicated by the Pink **X** [❌] on the shield.

   /*!
      @design
      @verbatim
   /////////////////////////////////////////////////////////////////////////////////
                  settingsMenu() - DESIGN                         Chris-70 (2025/08)
                  =======================
      @endverbatim
      This is the core of the settings menu. It is called from the main loop at every
      iteration. It checks if the settings menu is active and processes button presses.
      It uses a state machine approach to navigate through the menu options and levels.
      The menu has two main options: setting the time and setting the alarm.

      Details:  
      ========  
      The nature of this method being called every iteration from loop() means that
      if something can't be processed immediately, we must be able to return to the
      point where we left off and resume the processing. The original method was
      able to process everything without the need to pause and resume. This version
      expands the UX (User eXperience) as it has added the following new features:  
         1) The ability to cancel the settings for Time and Alarm.  
         2) Support for 12 (AM/PM) and 24 hour time modes.  
      The time modes and alarm states are set initially (i.e. Level 1) where the
      choice to Abort/Cancel is provided as one of the options. If the Abort
      option is selected, the settingsLevel is set to 99 which bypasses the
      normal processing, restores the original values and displays a confirmation
      to the user before exiting the settings menu.  
      The confirmation screens are designed to provide feedback to the user about the
      changes made and to display the action being taken (i.e. Save or Abort).
      There are two screens that are displayed to the user at the end of the
      setting menu session:   
         1) A Rainbow display is shown to:  
            a) signal to the user we are exiting the settings menu.  
            b) signal the beginning of action being taken;  
         2) A display showing the user the action being taken:  
            a) Green **✓** [✅] is shown if the settings are being saved; or  
            b) Pink **X** [❌] is shown if the settings are being discarded.  
      Adding these configuration screens requires that they remain visible for a
      period of time so that the user can view them. We can't use the `delay()`
      method as it would block the main loop and prevent other tasks from running.
      Instead, we must use a non-blocking approach, by using `millis()` to
      manage the timing along with static variables, `delayTimer`, `exit`, `abort`,
      `exitStage` and `continueS2`, to handle the screen displays and to be able
      to resume the processing.  
      Selecting the Time mode, 12 (AM/PM) or 24 hour mode is done by displaying
      12 PM or 24 on the Hours (top) Row. This is immediately followed by the user
      selecting the hours which is poor from a UX perspective. The two types,
      time mode and hours, appear to be very similar to the user. There needs to
      be a clear transition between them. The Green **✓** [✅] is displayed
      briefly to mark this transition which is handed with `continueS2` variable.
      The `delayTimer` prevents any action by the user until after it has expired.
      To resume processing the Time settings (i.e. hours) after the Green **✓** [✅]
      is displayed, the `continueS2` flag is cleared (i.e. false). This allows
      the `buttonS2` processing to continue after the delay expired.  
      The `exit`, `exitStage` and `abort` variables are used when we are done with
      the `settingsMenu()` and we will resume displaying the time. This involves
      displaying two screens: the Rainbow screen; and either the Green **✓** [✅]
      or the Pink **X** [❌] before displaying the time. The `exitStage` keeps track of
      where we are in the exit process. These two flags, along with the `delayTimer`,
      handle displaying the correct screens for the specified time.
      */

   SettingsState BCMenu::ProcessMenu()
      {
      // Main menu handling
      if ((settingsOption == 0) && (settingsLevel == 0))
         {
         handleMainMenu();
         }

      // Settings level handling
      if (settingsLevel != 0)
         {
         if (exit)
            {
            handleExitProcess();
            }
         else
            {
            handleSettingsLevel();
            }
         }

      // Update and return current state
      currentState = determineCurrentState();
      return currentState;
      }

   /////////////////////////////////////////////////////////////////////////////////

   #if SERIAL_SETUP_CODE
      // Define the strings that are used multiple times throughout the serial setup code. 
      const String BCMenu::STR_SEPARATOR = BCMenu::fillStr('-', 44); // "--------------------------------------------";
      const String BCMenu::STR_BARRIER   = BCMenu::fillStr('#', 44); // "############################################";
      const char PROGMEM BCMenu::STR_TIME_SETTINGS[] = "-------------- Time Settings ---------------";
      const char PROGMEM BCMenu::STR_ALARM_SETTINGS[]= "-------------- Alarm Settings --------------";
      const char PROGMEM BCMenu::STR_CURRENT_TIME[]  = "-------- Current Time: ";
   #else
      // When SERIAL_SETUP_CODE is false, code is removed. Redefine the method calls to be whitespace only.
      // This allows the code to compile without the serial setup code, but still allows the methods 
      // to be "called" in the code without causing compilation errors (Must return void to work).
      #define serialStartInfo()
      #define serialSettings()
      #define serialAlarmInfo()
      #define serialCurrentModifiedValue()
   #endif 

   // When the SERIAL_TIME_CODE code is removed, redefine the method calls to be whitespace only
   #if SERIAL_TIME_CODE != true
      #define serialTime() 
   #endif


   void BCMenu::handleMainMenu()
      {
      // Time settings - S1 button
      if (const_cast<IBCButtonBase&>(buttonS1).IsPressedNew())
         {
         tempTime = clock.get_Time();
         settingsOption = 1;
         settingsLevel = 1;
         tempAmPm = clock.get_Is12HourFormat();
         setCurrentModifiedValue();

         #if SERIAL_SETUP_CODE
         if (get_IsSerialSetup()) { serialSettings(); }
         #endif

         displayCurrentModifiedValue();
         }

      // Alarm settings - S3 button
      if (const_cast<IBCButtonBase&>(buttonS3).IsPressedNew())
         {
         tempAlarm = clock.get_Alarm();
         settingsOption = 3;
         settingsLevel = 1;
         tempAmPm = clock.get_Is12HourFormat();
         setCurrentModifiedValue();

         #if SERIAL_SETUP_CODE
         if (get_IsSerialSetup()) { serialSettings(); }
         #endif

         displayCurrentModifiedValue();
         }
      }

   void BCMenu::handleSettingsLevel()
      {
      unsigned long curMillis = millis();

      // Decrement - S1 button
      if (const_cast<IBCButtonBase&>(buttonS1).IsPressedNew() && (curMillis > delayTimer))
         {
         countButtonPressed--;
         checkCurrentModifiedValueFormat();
         displayCurrentModifiedValue();

         #if SERIAL_SETUP_CODE
         if (get_IsSerialSetup()) { serialCurrentModifiedValue(); }
         #endif
         }

      // Increment - S3 button
      if (const_cast<IBCButtonBase&>(buttonS3).IsPressedNew() && (curMillis > delayTimer))
         {
         countButtonPressed++;
         checkCurrentModifiedValueFormat();
         displayCurrentModifiedValue();

         #if SERIAL_SETUP_CODE
         if (get_IsSerialSetup()) { serialCurrentModifiedValue(); }
         #endif
         }

      // Save - S2 button
      if ((curMillis > delayTimer) && (continueS2 || const_cast<IBCButtonBase&>(buttonS2).IsPressedNew()))
         {
         if (!continueS2)
            {
            saveCurrentModifiedValue();
            }

         // Handle special case for time mode selection
         bool displayOK = false;
         if (settingsOption == 1 && settingsLevel == 1)
            {
            displayOK = true;
            }

         if (!continueS2)
            {
            settingsLevel++;
            }

         // Check if done with settings
         if ((settingsOption == 3) && (settingsLevel > 3))
            {
            exit = true;
            if (settingsLevel < 10)
               {
               clock.set_Alarm(tempAlarm);
               }
            else
               {
               abort = true;
               }
               
            SERIAL_SETUP_STREAM(endl)        // New line for the setting displayed previously.
            serialAlarmInfo();               // Show the time and alarm status info when you exit to the main menu
            }
         else if ((settingsOption == 1) && (settingsLevel > 4))
            {
            exit = true;
            if (settingsLevel < 10)
               {
               clock.set_Is12HourFormat(tempAmPm);
               clock.set_Time(tempTime);
               }
            else
               {
               abort = true;
               }

            #if SERIAL_SETUP_CODE
            if (get_IsSerialSetup())
               {
               Serial << endl << STR_SEPARATOR << endl;
               Serial << (const __FlashStringHelper*)STR_CURRENT_TIME;
               Serial << (clock.get_Time().toString(buffer, sizeof(buffer), clock.get_TimeFormat())) << endl;
               Serial << STR_SEPARATOR << endl;
               }
            #endif
            }
         else if (displayOK)
            {
            clock.DisplayLedPattern(LedPattern::okText);
            delayTimer = curMillis + 500;
            continueS2 = true;
            }
         else
            {
            continueS2 = false;
            setCurrentModifiedValue();
            displayCurrentModifiedValue();

            #if SERIAL_SETUP_CODE
            if (get_IsSerialSetup()) { serialSettings(); }
            #endif
            }
         }
      }

   void BCMenu::handleExitProcess()
      {
      unsigned long curTimer = millis();

      if (exitStage == 0)
         {
         // Display rainbow pattern to signal start of exit process
         clock.DisplayLedPattern(LedPattern::rainbow);
         delayTimer = curTimer + 750UL;  // Display rainbow for 750ms
         exitStage++;
         }

      if ((exitStage == 1U) && (curTimer > delayTimer))
         {
         // Display success or abort confirmation
         if (abort)
            {
            clock.DisplayLedPattern(LedPattern::xAbort);  // Pink X for abort/cancel
            }
         else
            {
            clock.DisplayLedPattern(LedPattern::okText);  // Green checkmark for save
            }

         delayTimer = curTimer + 1250U;  // Display confirmation for 1.25 seconds
         exitStage++;
         }

      if ((exitStage == 2) && (curTimer > delayTimer))
         {
         // Exit process complete, reset to normal operation
         resetSettingsState();
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Depending on the options and settings level, assign to the 
   // countButtonPressed variable to be able modify the value

   void BCMenu::setCurrentModifiedValue()
      {
      // Assign current time value stored in the 'tempTime' variable for modification by the user.
      if (settingsOption == 1)
         {
         if (settingsLevel == 1)  countButtonPressed = (tempAmPm ? 2 : 1);
         if (settingsLevel == 2)  countButtonPressed = tempTime.hour();
         if (settingsLevel == 3)  countButtonPressed = tempTime.minute();
         if (settingsLevel == 4)  countButtonPressed = tempTime.second();
         }

      // Alarm time and alarm status 
      if (settingsOption == 3)
         {
         if (settingsLevel == 1)  countButtonPressed = tempAlarm.status + 1;
         if (settingsLevel == 2)  countButtonPressed = tempAlarm.time.hour();
         if (settingsLevel == 3)  countButtonPressed = tempAlarm.time.minute();
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Check current modified value format of the countButtonPressed variable

   BCMenu::SettingsType BCMenu::GetSettingsType(int options, int level) const
      {
      SettingsType type = SettingsType::Undefined;

      if (options == 1) // Time 
         {
         if (level == 1) { type = SettingsType::TimeOptions; }
         if (level == 2) { type = SettingsType::Hours; }
         if (level == 3) { type = SettingsType::Minutes; }
         if (level == 4) { type = SettingsType::Seconds; }
         }
      else if (options == 3) // Alarm
         {
         if (level == 1) { type = SettingsType::AlarmStatus; }
         if (level == 2) { type = SettingsType::Hours; }
         if (level == 3) { type = SettingsType::Minutes; }
         }

      return type;
      }

   void BCMenu::checkCurrentModifiedValueFormat()
      {
      SettingsType type = GetSettingsType(settingsOption, settingsLevel);

      switch (type)
         {
            case SettingsType::TimeOptions:
               if (countButtonPressed < 1) countButtonPressed = 3;
               if (countButtonPressed > 3) countButtonPressed = 1;
               break;

            // Value is 0 - 23; display in mode 12 / 24 done elsewhere.
            case SettingsType::Hours:
               if (countButtonPressed < 0) countButtonPressed = 23;
               if (countButtonPressed > 23) countButtonPressed = 0;
               break;

            case SettingsType::Minutes:
               if (countButtonPressed < 0) countButtonPressed = 59;
               if (countButtonPressed > 59) countButtonPressed = 0;
               break;

            case SettingsType::Seconds:
               if (countButtonPressed < 0) countButtonPressed = 59;
               if (countButtonPressed > 59) countButtonPressed = 0;
               break;

            case SettingsType::AlarmStatus:
               if (countButtonPressed < 1) countButtonPressed = 3;
               if (countButtonPressed > 3) countButtonPressed = 1;
               break;

            case SettingsType::Undefined:
               break;

            default:
               // This is a Software error. Alert the developer (Debug mode only)
               // assert(false && "BinaryClock::checkCurrentModifiedValueFormat() - Undefined settings type "  && type);
               break;
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Depending on the options and settings level, save the current modified value

   void BCMenu::saveCurrentModifiedValue()
      {
      // Save current value in the DateTime structure
      if (settingsOption == 1)
         {
         if (settingsLevel == 1)
            {
            if (countButtonPressed == 3)      // Abort/Cancel
               { settingsLevel = 99; }        // Signal user abort/cancel selected.
            else if (countButtonPressed == 2) // 12 Hour format
               { tempAmPm = true; }
            else if (countButtonPressed == 1) // 24 Hour format
               { tempAmPm = false; }

            if (settingsLevel < 10) // Except for an abort; update the time display format to the seected type 12/24 hr.
               {
               clock.set_Is12HourFormat(tempAmPm);
               }
            }
         if (settingsLevel == 2) { tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), countButtonPressed, tempTime.minute(), tempTime.second()); }
         if (settingsLevel == 3) { tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), tempTime.hour(), countButtonPressed, tempTime.second()); }
         if (settingsLevel == 4) { tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), tempTime.hour(), tempTime.minute(), countButtonPressed); }
         }

      // Alarm time and alarm status
      if (settingsOption == 3)
         {
         if (settingsLevel == 1)
            {
            if (countButtonPressed == 3) // Abort/Cancel
               {
               tempAlarm = clock.get_Alarm();   // Restore the original values
               settingsLevel = 99;              // Signal user abort/cancel selected
               }
            else
               {
               tempAlarm.status = countButtonPressed - 1;
               }
            }
         if (settingsLevel == 2) { tempAlarm.time = DateTime(tempAlarm.time.year(), tempAlarm.time.month(), tempAlarm.time.day(), countButtonPressed, tempAlarm.time.minute(), tempAlarm.time.second()); }
         if (settingsLevel == 3) { tempAlarm.time = DateTime(tempAlarm.time.year(), tempAlarm.time.month(), tempAlarm.time.day(), tempAlarm.time.hour(), countButtonPressed, tempAlarm.time.second()); }
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Display on LEDs only currently modified value

   void BCMenu::displayCurrentModifiedValue()
      {
      SettingsType type = GetSettingsType(settingsOption, settingsLevel);
      switch (type)
         {
            case SettingsType::Hours:
               clock.DisplayBinaryTime(countButtonPressed, 0, 0, tempAmPm);
               break;
            case SettingsType::Minutes:
               clock.DisplayBinaryTime(0, countButtonPressed, 0);
               break;
            case SettingsType::Seconds:
               clock.DisplayBinaryTime(0, 0, countButtonPressed);
               break;
            case SettingsType::TimeOptions:
               if (countButtonPressed == 1)
                  {
                  clock.DisplayBinaryTime(24, 0, 0, false);
                  }
               else if (countButtonPressed == 2)
                  {
                  clock.DisplayBinaryTime(12, 0, 0, true);
                  }
               else
                  {
                  clock.DisplayLedPattern(LedPattern::xAbort);
                  }
               break;
            case SettingsType::AlarmStatus:
               if (countButtonPressed == 1)
                  {
                  clock.DisplayLedPattern(LedPattern::offTxt);
                  }
               else if (countButtonPressed == 2)
                  {
                  clock.DisplayLedPattern(LedPattern::onText);
                  }
               else
                  {
                  clock.DisplayLedPattern(LedPattern::xAbort);
                  }
               break;
            case SettingsType::Undefined:
            default:
               // This is a Software error. Alert the developer (Debug mode only)
               //assert(false && (&"BinaryClock::displayCurrentModifiedValue() - Undefined settings type ") && (type == SettingsType::Undefined));
               break;
         }
      }

   
   SettingsState BCMenu::determineCurrentState() const
      {
      if (exit)
         {
         return (exitStage < 2) ? SettingsState::Exiting : SettingsState::Inactive;
         }

      if (settingsLevel == 0)
         {
         return SettingsState::Inactive;
         }

      if (settingsOption == 1)
         {
         return SettingsState::TimeSettings;
         }
      else if (settingsOption == 3)
         {
         return SettingsState::AlarmSettings;
         }

      return SettingsState::Processing;
      }

   void BCMenu::resetSettingsState()
      {
      currentState = SettingsState::Inactive;
      settingsLevel = 0;
      settingsOption = 0;
      exit = false;
      abort = false;
      exitStage = 0U;
      continueS2 = false;
      countButtonPressed = 0;
      }

   void BCMenu::ExitSettingsMode()
      {
      resetSettingsState();
      }

   #if SERIAL_SETUP_CODE
   ////////////////////////////////////////////////////////////////////////////////////
   // Show the Shield settings menu and alarm status

   void BCMenu::serialStartInfo()
      {
      Serial << STR_SEPARATOR << endl;
      Serial << fillStr('-', 11) << F(" BINARY CLOCK SHIELD ") << fillStr('-', 12) << endl;
      Serial << fillStr('-', 15) << F(" FOR ARDUINO ") << fillStr('-', 16) << endl;
      Serial << STR_SEPARATOR << endl;
      Serial << fillStr('-', 17) << F(" Options ") << fillStr('-', 18) << endl;
      Serial << F("S1 - Time Settings ") << fillStr('-', 25) << endl;
      Serial << F("S2 - Disable Alarm Melody ") << fillStr('-', 18) << endl;
      Serial << F("S3 - Alarm Settings ") << fillStr('-', 24) << endl;
      Serial << STR_SEPARATOR << endl;
      Serial << STR_SEPARATOR << endl;
      Serial << (const __FlashStringHelper*)STR_CURRENT_TIME;
      Serial << ((clock.get_Time()).toString(buffer, sizeof(buffer), clock.get_TimeFormat())) << endl;

      serialAlarmInfo();

      Serial << STR_BARRIER << endl << endl;
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show the set alarm time and current alarm status

   void BCMenu::serialAlarmInfo()
      {
      Serial << STR_SEPARATOR << endl;
      Serial << F("------ Alarm Time: ");
      Serial << ((clock.get_Alarm()).time.toString(buffer, sizeof(buffer), clock.get_AlarmFormat())) << endl;
      Serial << STR_SEPARATOR << endl;
      Serial << F("---- Alarm Status: ");
      Serial << ((clock.get_Alarm()).status == 1 ? "ON" : "OFF") << endl;
      Serial << STR_SEPARATOR << endl;
      }

      ////////////////////////////////////////////////////////////////////////////////////
   // Show alarm/time settings

   void BCMenu::serialSettings()
      {
      if (settingsOption == 1)
         {
         Serial << endl << endl;
         Serial << STR_SEPARATOR << endl;
         Serial << (const __FlashStringHelper*)STR_TIME_SETTINGS << endl;
         Serial << STR_SEPARATOR << endl;
         Serial << (const __FlashStringHelper*)STR_CURRENT_TIME;
         Serial << (tempTime.toString(buffer, sizeof(buffer), clock.get_TimeFormat())) << endl;
         Serial << STR_SEPARATOR << endl;
         }

      if (settingsOption == 3)
         {
         Serial << endl << endl;
         Serial << STR_SEPARATOR << endl;
         Serial << (const __FlashStringHelper*)STR_ALARM_SETTINGS << endl;
         Serial << STR_SEPARATOR << endl;
         serialAlarmInfo();
         }

      auto printSettingsControls = [this]()
         {
         Serial << F("S1 - Decrement ") << this->fillStr('-', 29) << endl;
         Serial << F("S2 - Save Current Settings Level ") << this->fillStr('-', 11) << endl;
         Serial << F("S3 - Increment ") << this->fillStr('-', 29) << endl;
         Serial << STR_SEPARATOR << endl;
         };

      char hourStr[6] = { 0 };
      BCMenu::SettingsType type = GetSettingsType(settingsOption, settingsLevel);
      switch (type)
         {
            case SettingsType::Hours:
               Serial << fillStr('-', 19) << F(" Hour ") << fillStr('-', 19) << endl;
               printSettingsControls();
               Serial << F("Current Hour: ") << FormatHour(countButtonPressed, tempAmPm,
                     hourStr, sizeof(hourStr)) << (" ");
               break;
            case SettingsType::Minutes:
               Serial << fillStr('-', 18) << F(" Minute ") << fillStr('-', 18) << endl;
               printSettingsControls();
               Serial << F("Current Minute: ") << countButtonPressed << (" ");
               break;
            case SettingsType::Seconds:
               Serial << fillStr('-', 18) << F(" Second ") << fillStr('-', 18) << endl;
               printSettingsControls();
               Serial << F("Current Second: ") << countButtonPressed << (" ");
               break;
            case SettingsType::AlarmStatus:
               Serial << fillStr('-', 15) << F(" ON/OFF/CANCEL ") << fillStr('-', 14) << endl;
               printSettingsControls();
               Serial << F("Alarm Status: ");
               Serial << (countButtonPressed == 2 ? "ON" : "OFF") << " ";
               Serial << (countButtonPressed == 3 ? "Cancel" : "");
               break;
            case SettingsType::TimeOptions:
               Serial << fillStr('-', 11) << F(" 12 Hr / 24 Hr / Cancel ") << fillStr('-', 9) << endl;
               printSettingsControls();
               Serial << F("Time Mode: ");
               Serial << (countButtonPressed == 2 ? "12" : "");
               Serial << (countButtonPressed == 1 ? "24" : "");
               Serial << (countButtonPressed == 3 ? "Cancel" : "");
               Serial << (" ");
               break;
            case SettingsType::Undefined:
            default:
               // This is a Software error. Alert the developer (Debug mode only)
               // assert(false && "BCMenu::displayCurrentModifiedValue() - Undefined settings type "  && type);
               break;
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show current alarm status during settings

   void BCMenu::serialCurrentModifiedValue()
      {
      if ((settingsLevel == 1) & (settingsOption == 3))
         {
         Serial << (countButtonPressed == 2 ? "ON" : "");
         Serial << (countButtonPressed == 1 ? "OFF" : "");
         Serial << (countButtonPressed == 3 ? "Cancel" : "");
         }
      else if ((settingsLevel == 1) & (settingsOption == 1))
         {
         Serial << (countButtonPressed == 2 ? "12" : "");
         Serial << (countButtonPressed == 1 ? "24" : "");
         Serial << (countButtonPressed == 3 ? "Cancel" : "");
         }
      else if (settingsLevel == 2)
         {
         char buffer[6] = { 0 };
         Serial << FormatHour(countButtonPressed, tempAmPm, buffer, sizeof(buffer));
         }
      else
         {
         Serial << countButtonPressed;
         }

      Serial << (" ");
      }

   char* BCMenu::FormatHour(int hour24, bool is12HourFormat, char* buffer, size_t size)
      {
      if ((buffer == nullptr) || (size < 6)) { return nullptr; }

      int hour = hour24 % 24;
      bool isPM = false;
      int i = 0;

      if (is12HourFormat)
         {
         hour = ((hour24 % 12) == 0 ? 12 : hour24 % 12);
         isPM = (hour24 >= 12);
         }

      if (hour < 10)
         {
         if (!is12HourFormat) { buffer[i++] = '0'; }
         buffer[i++] = '0' + hour;
         }
      else //  else (hour < 24) 
         {
         buffer[i++] = '0' + (hour / 10);
         buffer[i++] = '0' + (hour % 10);
         }

      if (is12HourFormat)
         {
         buffer[i++] = (isPM ? 'p' : 'a');
         buffer[i++] = 'm';
         }

      buffer[i] = '\0';
      return buffer;
      }
   #endif // SERIAL_SETUP_CODE
  
   #if SERIAL_OUTPUT
   String BCMenu::fillStr(char ch, byte repeat)
      {
      char buffer[MAX_BUFFER_SIZE] = { 0 };
      byte len = (repeat < sizeof(buffer) - 1) ? repeat : sizeof(buffer) - 1;
      for (byte i = 0; i < len; i++)
         { buffer[i] = ch; }
      buffer[len] = '\0';
      return String(buffer);
      }
   #endif

   } // END namespace BinaryClockShield

   