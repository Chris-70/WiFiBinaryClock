// Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com
// https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
//
// Binary Clock RTC 24H with Interrupt, Alarm and Buttons Example
// This example demonstrates complete Binary Clock with Time and Alarm settings
//
// *It is recommended that the first start should be carried out with the serial terminal, 
// for better knowing the setting options. 
//
// The buttons allows you to set the time and alarm - exact hour, minute, second/alarm status.
// Alarm causes melody to play.  
// How to use piezo with the tone() command to generate notes you can find here:
// http://www.arduino.cc/en/Tutorial/Tone
//
// A falling edge at the RTC INT/SQW output causes an interrupt, 
// which is uses for regular - 1 per second - reading time from RTC and 
// checking alarm status flag 'A2F'. Since we use RTC INT/SQW output for
// regular reading current time - square wave output SQW option, 
// global interrupt flag INTCN is set to 0, this disables the interrupts from both RTC alarms.
// Referring to the documentation: when the INTCN is set to logic 0, 
// the 'A2F' bit does not initiate an interrupt signal. By turning off the interrupts from the alarms, 
// we can use the interrupt flag 'A2IE' as an info flag whether the alarm has been activated or not. 
// Check RTC datasheet page 11-13 http://bit.ly/DS3231-RTC
//
// Hardware:
// 1) Arduino Uno style board. e.g. 
//    - Arduino UNO R3                    (https://store.arduino.cc/products/arduino-uno-rev3)
//    - Arduino UNO R4 Minima             (https://store.arduino.cc/products/uno-r4-minima)
//    - Arduino UNO R4 WiFi               (https://store.arduino.cc/products/uno-r4-wifi)
//    - Adafruit Metro ESP32-S3           (https://www.adafruit.com/product/5500)
//    - Generic Wemos ESP32 D1 R32 UNO (*)
//    - Generis ESP32-S3 UNO              (uses the pinout of an ESP32-S3 DevKitC-1)
// 2) Binary Clock Shield for Arduino     (https://nixietester.com/product/binary-clock-shield-for-arduino/)
// 3) Battery CR1216/CR1220 
// 
// INT/SQW   connected to: Arduino pin   3 INT1 ; Metro ESP32-S3   3 ; ESP32_D1-R32 25  ; ESP32-S3 UNO 17
// PIEZO     connected to: Arduino pin  11  PWM ; Metro ESP32-S3  11 ; ESP32_D1-R32 23  ; ESP32-S3 UNO 11
// S3 button connected to: Arduino pin  A0      ; Metro ESP32-S3  A0 ; ESP32_D1-R32  2  ; ESP32-S3 UNO  2
// S2 button connected to: Arduino pin  A1      ; Metro ESP32-S3  A1 ; ESP32_D1-R32  4  ; ESP32-S3 UNO  1
// S1 button connected to: Arduino pin  A2      ; Metro ESP32-S3  A2 ; ESP32_D1-R32 35  ; ESP32-S3 UNO  7
// LEDs      connected to: Arduino pin  A3      ; Metro ESP32-S3  A3 ; ESP32_D1-R32 15* ; ESP32-S3 UNO  6
// RTC SDA   connected to: Arduino pin PC4  SDA ; Metro ESP32-S3  47 ; ESP32_D1-R32 36  ; ESP32-S3 UNO  8
// RTC SCL   connected to: Arduino pin PC5  SCL ; Metro ESP32-S3  48 ; ESP32_D1-R32 39  ; ESP32-S3 UNO  9
//
// * (Requires hardware board modification to use LED output pin)
//
//                        +------+       +------+       +------+       +------+       +------+
//                        |LED 16|---<---|LED 15|---<---|LED 14|---<---|LED 13|---<---|LED 12|--<-+
//                        +------+       +------+       +------+       +------+       +------+    |
//                                                                                                |
//    +--------------->-------------->-------------->-------------->-------------->---------------+
//    |
//    |    +------+       +------+       +------+       +------+       +------+       +------+
//    +----|LED 11|---<---|LED 10|---<---|LED 09|---<---|LED 08|---<---|LED 07|---<---|LED 06|--<-+
//         +------+       +------+       +------+       +------+       +------+       +------+    |
//                                                                                                |
//    +--------------->-------------->-------------->-------------->-------------->---------------+
//    |
//    |    +------+       +------+       +------+       +------+       +------+       +------+
//    +----|LED 05|---<---|LED 04|---<---|LED 03|---<---|LED 02|---<---|LED 01|---<---|LED 0 |--<-- DATA_PIN 
//         +------+       +------+       +------+       +------+       +------+       +------+
// 
// Note: (Chris-80 2025/07)
// =====
// This file has been adapted from the original Example; "11-BinaryClock-24H-RTCInterruptAlarmButtons.ino" file as published
// on the Binary Clock Shield for Arduino GitHub repository: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
// The original file was modified to be encapsulated in a class, BinaryClock. The class encapsulates all the functionality
// of the "Binary Clock Shield for Arduino by Marcin Saj." Modifications were made to support the ESP32 UNO platform and to
// allow greater flexibility by the user at runtime, such as the color selection and melodies used for the alarm.
// 
// The goal of using an ESP32 based UNO board was to allow the RTC to be connected to a NTP server over WiFi. The code
// for the WiFi connection is encapsulated in its own class, WiFiClock, which is not included in this file. It uses 
// WPS to connect to a WiFi network and stores the credentials in the ESP32 flash memory so future connections are
// made automatically without user intervention. The WiFi connection also allows the user to change the LED colors
// and melodies used for the alarm at runtime, without needing to recompile the code. 
//

#pragma once
#ifndef _BINARY_CLOCK_RTC_24_ALARM_BUTTONS_
#define _BINARY_CLOCK_RTC_24_ALARM_BUTTONS_

#include <Arduino.h>             // Arduino core library

#include <FastLED.h>             // https://github.com/FastLED/FastLED
#include <RTClib.h>              // Adafruit RTC library: https://github.com/adafruit/RTClib
#include <Streaming.h>           // https://github.com/janelia-arduino/Streaming                            

#include "BinaryClock.Defines.h" // Include the Binary Clock definitions file
#include "pitches.h"             // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody

namespace BinaryClockShield
   {
   /// @brief The structure holds all the Alarm information used by the Binary Clock.
   /// @note  The 'melody' selection has not been implemented, it will always use the internal melody
   /// @author Chris-80 (2025/07)
   typedef struct alarmTime
      {
      int      number;        // The number of the alarm: 1 or 2
      DateTime time;          // The time of the alarm as a DateTime object
      int      melody;        // The melody to play when the alarm is triggered, 0 = internal melody
      int      status;        // Status of the alarm: 0 - inactive, 1 - active
      bool     fired;         // The alarm has fired (e.g. alarm is 'ringing').
      void clear()
         {
         time = DateTime();   // 00:00:00 (2000-01-01)
         melody = 0;          // Internal melody
         status = 0;          // OFF, alarm is not set.
         fired = false;       // Alarm is not ringing (OFF)
         }
      } AlarmTime;

   /// @brief The structure that holds the state of a button.
   ///        It contains the pin number, current state, last read state, last read time
   ///        the onValue (the value when the button is pressed) for CC or CA connections.
   ///        The isPressed() method returns true if the button is currently pressed, false otherwise.
   /// @author Chris-80 (2025/07)
   typedef struct buttonState
      {
      uint8_t pin;                     // The button pin number: e.g. S1, S2, S3, etc.
      int state;                       // The current state of the button: LOW or HIGH
      int lastRead;                    // The last read state of the button: LOW or HIGH
      unsigned long lastReadTime;      // The last time the button was read
      unsigned long lastDebounceTime;  // The last debounce time in milliseconds
      int onValue;                     // The button value when pressed, HIGH (CC) or LOW (CA)
      bool isPressed() const           // True if the button is currently pressed, false otherwise
         { return (state == onValue); }  
      } ButtonState;

   /// @brief The class that extends the RTCLib's RTC_DS3231 class to add raw read/write methods
   /// @note  This class is a hack for now as the RTCLib does not provide a way to read/write raw registers.
   /// @author Chris-80 (2025/07)
   class RTCLibPlusDS3231 : public RTC_DS3231
      {
   public:
      /// @brief Wrapper for the 'RTC_I2C::read_register' method to read a register from the DS3231 RTC.
      /// @details This method reads a single byte from the specified register of the DS3231
      /// @param reg The DS3231 register number to read.
      /// @return The register value that was read.
      uint8_t RawRead(uint8_t reg);

      /// @brief Wrapper for the 'RTC_I2C::write_register' method to write a value to a register in the DS3231 RTC.
      /// @details This method writes a single byte to the specified register of the DS3231
      /// @param reg The DS3231 register number to write to.
      /// @param value The value to write to the register.
      void RawWrite(uint8_t reg, uint8_t value);
      };

   /// @brief The BinaryClock class encapsulates the functionality of the Binary Clock Shield for Arduino.
   ///        It provides all the methods needed to initialize, set the date/time, set the alarm, and 
   ///        change the brightness and LED colors in addition to handling the LED display.
   /// @details The class has public methods to get/set the time, alarm, alarm melody, and brightness.
   ///          It also has methods to handle the settings menu on a serial display. Refactoring the 
   ///          original example code into a class was done to pair it with another class that handles
   ///          the WiFi connection and allows the user to change the LED colors and melodies used for
   ///          the alarm at runtime, without needing to recompile the code. The original code and shield design
   ///          targeted an Arduino UNO board, this is designed for an ESP32 based UNO board, such as 
   ///          the Wemos D1 R32 UNO or the new ESP32-S3 UNO. The idea is to connect to a NTP server over WiFi.
   /// @remarks This class is designed to be used with the excellent 'Binary Clock Shield for Arduino' by 
   ///          Marcin Saj (available from: https://nixietester.com), original source code: 
   ///          https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
   ///          This class uses the Adafruit RTCLib library for the RTC functionality 
   ///          (https://github.com/adafruit/RTClib) in place of the original DS3232RTC library by 
   ///          Jack Christensen (https://github.com/JChristensen/DS3232RTC) used in the original code.
   ///          One big reason for this is the inclusion of the classes 'DateTime' and 'TimeSpan'
   ///          which closely resemble the C# classes. The implementation of DateTime keeps the
   ///          time in individual bytes for Year; Month; Day; Hours; Minutes; and Seconds which is
   ///          close to the format that the RTC uses.
   /// @author  Chris-80 (2025/07)
   class BinaryClock
      {
   public:
      /// @brief The method called to initialize the Binary Clock.
      ///        This has the same functionality of the Arduino setup() method.
      ///        Call this method before using the BinaryClock class.
      /// @param testLeds Flag - Show the test patterns at startup.
      void setup(bool testLeds);
      void setup() { setup(false); };

      /// @brief The method called to run the Binary Clock.
      ///        This has the same functionality of the Arduino loop() method.
      ///        Call this method in the main loop of your program or run this
      ///        method in a separate thread that just loops forever.
      void loop();

      //################################################################################//
      // SETTINGS
      //################################################################################//
      //
      //                       +-------------------------------+
      //                       |           SETTINGS            |
      //           +-----------+-------------------------------+
      //           |  BUTTONS  |    S3   |     S2    |   S1    |
      // +---------+-----------+---------+-----------+---------+
      // |         |           |         |   ALARM   |         |
      // |         |     0     |  ALARM  |   MELODY  |  TIME   |
      // |   S     |           |         |   STOP    |         |
      // +   E L   +-----------+---------+-----------+---------+
      // |   T E   | ROW H = 1 |    +    |   SAVE    |    -    |
      // |   T V   |           |         | LEVEL = 2 |         |
      // +   I E   +-----------+---------+-----------+---------+
      // |   N L   | ROW M = 2 |    +    |   SAVE    |    -    |
      // |   G     |           |         | LEVEL = 3 |         |
      // +   S     +-----------+---------+-----------+---------+
      // |         | ROW S = 3 |    +    |   SAVE    |    -    |
      // |         |           |         | LEVEL = 0 |         |
      // +---------+-----------+---------+-----------+---------+
      //
      //                       +-------------------------------+
      //                       |        SETTINGS OPTION        |
      //                       +---------------+---------------+
      //                       |   ALARM = 3   |   TIME = 1    |
      // +---------------------+---------------+---------------+
      // |   S     | ROW H = 1 |     3/1       |     1/1       |
      // |   E L   |           |     HOUR      |     HOUR      |
      // +   T E   +-----------+---------------+---------------+
      // |   T V   | ROW M = 2 |     3/2       |     1/2       |
      // |   I E   |           |    MINUTE     |    MINUTE     |
      // +   N L   +-----------+---------------+---------------+
      // |   G     | ROW S = 3 |     3/3       |     1/3       |
      // |   S     |           | ALARM STATUS  |    SECOND     |
      // +---------+-----------+---------------+---------------+
      //
      // The core of the settings menu

      /// @brief The method called to set the time and/or alarm from the shield
      ///        The S1 button sets the Time, S3 sets the Alarm, S2 accepts
      ///        the current modified value and moves to the next line.
      ///        The S1 and S3 buttons increment/decrement the current modified value.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino
      /// @author Chris-80 (2025/07)
      void settingsMenu();

      /// @brief The method called to play the alarm melody.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino
      void playAlarm();

      //#################################################################################//
      // Public PROPERTIES
      //#################################################################################//

      /// @brief The method called to set the current 'Time' property.
      /// @param value The DateTime object containing the current time to set.
      /// @note The DateTime class is defiend in the RTCLib.h header file.
      /// @author Chris-80 (2025/07)
      void set_Time(DateTime &value);

      /// @brief The method called to get the current 'Time' property.
      /// @return A DateTime object containing the current time.
      /// @note The DateTime class is defiend in the RTCLib.h header file.
      /// @author Chris-80 (2025/07)
      DateTime get_Time();

      /// @brief The method called to set the current 'Alarm' property.
      /// @param alarmTime The AlarmTime structure containing the alarm time and status.
      /// @note The AlarmTime structure contains the hour, minute, and status of the alarm
      ///       The status is 0 for inactive, 1 for active.
      ///       Hours are 0 to 23.
      /// @author Chris-80 (2025/07)
      void set_Alarm(AlarmTime &value);

      /// @brief The method called to get the default 'Alarm' property
      /// @return An AlarmTime structure containing the alarm time and status.
      /// @author Chris-80 (2025/07)
      AlarmTime get_Alarm() { return GetAlarm(ALARM_2); }

      /// @brief The method called to get the 'AlarmTime' for alarm 'number'
      /// @param number The alarm number: 1 or 2. Alarm 2 is the default alarm.
      /// @return An AlarmTime structure containing the alarm time and status.
      /// @design This method was included as a workaround to allow the user to get alarm 1
      ///         without breaking the property pattern for the Alarm, so no '_' after get....
      /// @author Chris-80 (2025/07)
      AlarmTime GetAlarm(int number);

      /// @brief Property pattern for the LED 'Brightness' property.
      ///        This property controls the brightness of the LEDs, 0-255, 20-30 is normal
      /// @author Chris-80 (2025/07)
      void set_Brightness(byte value);
      byte get_Brightness();

      /// @brief Property pattern for the 'isSerialSetup' flag property.
      ///        This property controls whether the serial setup menu is displayed or not.
      /// @author Chris-80 (2025/07)
      void set_isSerialSetup(bool value);
      bool get_isSerialSetup() const;

      /// @brief Property pattern for the 'isSerialTime' flag property.
      ///        This property controls whether the serial time is displayed or not.
      /// @author Chris-80 (2025/07)
      void set_isSerialTime(bool value);
      bool get_isSerialTime() const ;

      /// @brief Property pattern for the 'DebugOffDelay' property. This controls how fast 
      ///        the serial time monitor is turned off after the debug pin goes OFF.
      /// @author Chris-80 (2025/07)
      void set_DebugOffDelay(unsigned long value);
      unsigned long get_DebugOffDelay() const;

      static int BuiltinLED;

      //#################################################################################//
      // Public METHODS
      //#################################################################################//

      /// @brief Methods to register/unregister a callback function at every second.
      /// @param callback The function to call every second with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @author Chris-70 (2025/07)
      bool registerTimeCallback(void (*callback)(DateTime));
      bool unregisterTimeCallback(void (*callback)(DateTime));

      /// @brief  Methods to register/unregister a callback function for the alarm.
      ///         The callback function is called when the alarm is triggered.
      /// @param callback The function to call when the alarm is triggered with the current DateTime.
      /// @return Flag: true - success; false - failure (e.g. if the callback is null).
      /// @author Chris-70 (2025/07)
      bool registerAlarmCallback(void (*callback)(DateTime));
      bool unregisterAlarmCallback(void (*callback)(DateTime));

      /// @brief Method to change the alarm melody with a melody and note duration arrays.
      /// @param melodyArray The array of melody notes to play, each note is a frequency in Hz.
      /// @param melodySize The size (i.e. number of notes) of the melody array.
      /// @param noteDurationArray The array of note durations in milliseconds, each note duration corresponds to a note in the melody.
      /// @param noteDurationSize The size of the note duration array, must match the melodySize.
      /// @return Flag: true - success, false - failure (e.g. if the arrays are null or sizes do not match).
      /// @author Chris-70 (2025/07)
      bool SetAlarmMelody(unsigned *melodyArray, size_t melodySize, unsigned long *noteDurationArray, size_t noteDurationSize)
         {
         bool result = false;
         if (melodyArray != nullptr && noteDurationArray != nullptr && melodySize > 0 && noteDurationSize == melodySize)
            {
            this->melodyAlarm = melodyArray;
            this->melodySize = melodySize;
            this->noteDurations = noteDurationArray;
            this->noteDurationsSize = noteDurationSize;
            result = true;
            }

         return result;
         }

      /// @brief Method to convert a DateTime value to a string inline. This method takes the format as a parameter
      ///        and copies it to the buffer before calling DateTime.toString() and returning the result.
      /// @note  The DateTime.toString(char *buffer) method reads the buffer parameter to extract the 
      ///        formatting string before performing the conversion, placing the result back in the buffer and
      ///        returning a pointer to the same buffer. This forces the developer to litter their code with 
      ///        lines to initialize the buffer with the formatting string. This method changes that, you just 
      ///        pass the format string as a parameter and it is copied to the buffer before calling DateTime.toString().
      /// @param time The DateTime object to convert to a string.
      /// @param buffer The buffer to store the resulting string. Size the buffer to hold the resulting string, the
      ///               DateTime.toString() methods is vulnerable to buffer overrun.
      /// @param size The size of the buffer, not including the null '\0' terminator.
      /// @param format The format string to use for the conversion, default is "YYYY/MM/DD hh:mm:ss".
      /// @author Chris-80 (2025/07)
      char* DateTimeToString(DateTime time, char* buffer, size_t size, const char* format = "YYYY/MM/DD hh:mm:ss") const
         {
         if (buffer == nullptr || size == 0)
            return nullptr; // Return null if buffer is null

         strncpy(buffer, format, size);
         return time.toString(buffer);
         }

      /// @brief Singleton pattern to ensure only one instance of BinaryClock
      /// @author Chris-80 (2025/07)
      static BinaryClock& get_Instance()
         {
         static BinaryClock instance; // Guaranteed to be destroyed, instantiated on first use
         return instance;
         }

      /// @brief Method to flash the 'ledNum' ON/OFF for ~(1 sec / frequency). with an ON/OFF 'dutyCycle' 0-100
      /// @param ledNum The output pin number to flash ON/OFF (HIGH/LOW).
      /// @param repeat The number of times to flash ON/OFF, each takes ~(1 sec / frequency).{1}
      /// @param dutyCycle The percentage of time to keep the LED ON (HIGH), 0 - 100.       {50}
      /// @param frequency The number of times per second to flash ON/OFF (Hz) 1 - 25.       {1}
      /// @note The LED must be wired CC, where the LED is on when the pin goes HIGH.
      ///       The LED is in the OFF (LOW) state when this method returns.
      /// @remarks A duty cycle outside of the range 10 - 90 or a frequency > 10 will not 
      ///          appear to be flashing. Use a duty cycle between 25 - 75 and a frequency between 1 - 5
      void FlashLed (uint8_t ledNum, uint8_t repeat = 1, uint8_t dutyCycle = 50, uint8_t frequency = 1);

   protected:
      /// @brief Default Constructor for the BinaryClock class. This initializes the 
      ///        button states, settings options, and brightness. It assigns the 
      ///        melody and note durations arrays to the class members.
      BinaryClock(); 

      /// @brief Destructor for the BinaryClock class. This destructor is 
      ///        empty as there is no dynamic memory allocation in this class.
      virtual ~BinaryClock();

      // Singleton pattern - Disable these constructors and assignment operators.
      BinaryClock (const BinaryClock&) = delete;            // Disable copy constructor
      BinaryClock& operator=(const BinaryClock&) = delete;  // Disable assignment operator
      BinaryClock (BinaryClock&&) = delete;                 // Disable move constructor
      BinaryClock& operator=(BinaryClock&&) = delete;       // Disable move assignment operator

      /// @brief This method is to isolate the code needed to initialize the Buttons.
      ///        The 'ButtonState.onValue' determines the type: INPUT_PULLUP/DOWN.
      /// @author Chris-70 (2025/07)
      void initializeButtons();

      /// @brief This method is to isolate the code needed to setup for the RTC.
      /// @author Chris-80 (2025/07)
      bool setupRTC();

      /// @brief This method is to isolate the code needed to setup the alarm.
      /// @author Chris-80 (2025/07)
      void setupAlarm();

      /// @brief This method is to isolate the code needed to setup the FastLED library.
      /// @param testLEDs - Flag: Display the LED test patterns.
      /// @author Chris-80 (2025/07)
      void setupFastLED(bool testLEDs);

      /// @brief Property: 'DebounceDelay' time (ms) for the buttons. Initially set to  DEFAULT_DEBOUNCE_DELAY.
      /// @author Chris-70 (2025/07)
      void set_DebounceDelay(unsigned long value);
      unsigned long get_DebounceDelay() const;

      /// @brief This method runs the task to handle the RTC time and alarm. It waits for the 
      ///        1 Hz RTC Interrupt, calls the 'timeDispatch()' method to read the RTC time and
      ///        check if the alarm has fired. 
      /// @note  This method isn't used on boards that don't run FreeRTOS, they just call the
      ///        'timeDispatch()' method from within the 'loop()' method.
      void timeTask();
      
      /// @brief This method handles the reading of the time from the RTC and checks if the 
      ///        alarm has been triggered (when set). 
      /// @returns bool - Flag indicating the interrupt had fired and time was read from the RTC.
      /// @design - This method exists to be called by boards that don't have FreeRTOS.
      ///           Instead of executing the code in 'timeTask()' the code is encompassed in 
      ///           this method so that it can be called from within the 'loop()' method.
      bool timeDispatch();

      // ################################################################################
      // ORIGINAL METHODS - 
      // ################################################################################

      /// @brief The method called when the RTC generates an interrupt on the 'RTC_INT' pin every second.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino
      void RTCinterrupt();

      /// @brief The method called to read the alarm time status (ON/OFF), for the default alarm, from the RTC.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void getAlarmTimeAndStatus();

      /// @brief The method called to write the alarm time status (ON/OFF), for the default alarm, from the RTC.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void setAlarmTimeAndStatus();

      /// @brief The method called to convert the time to binary and update the LEDs.
      /// @param hourRow The value for the top, to display the hour LEDs (16-12).
      /// @param minuteRow The value for the middle, to display the minute LEDs (11-6).
      /// @param secondRow The value for the bottom, to display the second LEDs (5-0).
      /// @details This method converts the current time to binary and updates the LEDs 
      ///          using the color values defined in the arrays 'OnColors' and 'OffColor'
      /// @see FastLED.setBrightness() for the brightness of the LEDs.
      /// @see displayLedBuffer() for displaying the full LED buffer as defined.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void convertDecToBinaryAndDisplay(int hoursRow, int minutesRow, int secondsRow);

      /// @brief The method called to display the LED buffer on the LEDs.
      /// @param ledBuffer The buffer containing the LED colors to display.
      /// @param size The size of the LED buffer from 'ledBuffer' to the end, e.g. 17.
      /// @details This method just copies the given 'ledBuffer' contents directly to the 
      // /         FastLED buffer and displays it.
      void displayLedBuffer(const CRGB* ledBuffer, int size);

      /// @brief The method called to set the time on the RTC from the value in the time field.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void setNewTime();

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

      #if SERIAL_TIME_CODE
      /// @brief The method called to display the current time, decimal and binary, over the serial monitor.
      /// @details While this method can still be removed at compile time, it can also be controlled, at run-time, 
      ///          in software and hardware. This method is called every second so being able to control the         
      ///          output in software and hardware, by using a switch or jumper, can start/stop the serial time display.
      /// @remarks This code was modified from the original Binary Clock Shield for Arduino by Marcin Saj.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      void serialTime();
      #endif

      // These methods are all part of the serial menu display and can be removed at compile time. They can be 
      // can be controlled in both software and hardware. A momentary button is used to toggle the menu on/off.
      #if SERIAL_SETUP_CODE
      /// @brief The method called to display the serial startup information.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      void serialStartInfo();

      /// @brief The method called to display the serial menu for setting the time and alarm.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino;
      void serialSettings();

      /// @brief The method called to display the alarm settings over the serial monitor.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino;
      void serialAlarmInfo();

      /// @brief The method called to display the current modified value over the serial monitor.
      /// @details This method is called when the user is changing the time or alarm time.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino;
      void serialCurrentModifiedValue();
      #endif

      /// @brief Method to check if the button was pressed ON from OFF since the last call.
      /// @param button - The ButtonState structure containing the button state, pin, type, etc.
      /// @return True if the button is pressed ON from OFF, false otherwise (button OFF or button ON and no change).
      /// @note The method returns false if the button is ON but has not changed state since the last read.
      ///       Check the ButtonState::isPressed() property to see if the button is currently pressed or not. 
      /// @details This method handles buttons that are wired either CC and CA where the concept of ON or PRESSED is
      ///          defined by the way it is wired. As a result, the first time it called on a button, it will
      ///          behave as if it had just transitioned from its opposite state to its current state. This fixes a
      ///          bug with switches (or jumpers) that are wired ON at startup.
      /// @remarks This code was adapted and modified from the methods: checkS1(); checkS2; and checkS3() in the 
      ///          original Binary Clock Shield for Arduino by Marcin Saj.
      /// @author Marcin Saj - From the original Binary Clock Shield for Arduino; 
      /// @author Chris-80 (2025/07)
      bool isButtonOnNew(ButtonState& button);

      #if HARDWARE_DEBUG
      /// @brief This method is called to check the hardware debug buttons/switches and set the serial output level.
      /// @author Chris-80 (2025/07)
      void checkHardwareDebugPin();

      /// @brief This method is called to service the user callback function with the associated time.
      /// @param triggerFlag The flag that indicates if the callback was fired.
      /// @param time The associated DateTime object to pass to the callback function (e.g. alarm time / current time).
      /// @param callback The user callback function to call with the associated DateTime.
      /// @details This method is called when the RTC 1 Hz signal is triggered (time) or the alarm has triggered.
      void callbackFtn(volatile bool &triggerFlag, DateTime time, void (*callback)(DateTime));

      /// @brief This method is called to dispatch the callback functions for the alarm and time.
      /// @details This method calls the 'callbackFtn()' when the associated trigger is set and
      ///          the user has registered a callback function for the trigger.
      void callbackDispatch();

      /// @brief This method is called to run the callback task in a separate thread.
      /// @details This method is called in a separate thread on UNO boards that run FreeRTOS.
      ///          This task just calls 'callbackDispatch()' and briefly pauses execution in a loop.
      void callbackTask();

      /// @brief This method is called when the BinaryClock has died. It signals S.O.S. on the builtin led forever.
      ///        It turns off the the LEDs on the shield and goes in a loop forever signaling SOS on the builtin LED.
      ///        This is called for a catastrophic failure such as missing/failed RTC chip.
      void purgatoryTask(const char* message = nullptr);
      #endif

   public:         
      static CRGB OnColor [NUM_LEDS];     // Colors for the LEDs when ON
      static CRGB OffColor[NUM_LEDS];     // Colors for the LEDs when OFF
      static CRGB PmColor;                // Color for the PM indicator LED, e.g. Gold.
      static CRGB AmColor;                // Color for the AM indicator (Usually Black/OFF).

   protected:
      RTCLibPlusDS3231 RTC;               // Create RTC object using Adafruit RTCLib library

      // The UNO Compiler doesn't support this C++ style object initialization, move it to the constructor.
      AlarmTime Alarm1; // = { .number = ALARM_1, .melody = 0, .status = 0 };  // DS3232 alarm, includes seconds in alarm.
      AlarmTime Alarm2; // = { .number = ALARM_2, .melody = 0, .status = 0 };  // Default alarm, seconds set at 00.

      volatile bool RTCinterruptWasCalled;   // Flag: The RTC interrupt was triggere
      volatile bool CallbackAlarmTriggered;  // Flag: The 'Alarm' callback needs to be called.
      volatile bool CallbackTimeTriggered;   // Flag: The 'Time'  callback needs to be called.

      // Note durations: 4 = quarter note, 8 = eighth note, etc.:
      // Some notes durations have been changed (1, 3, 6) to make them sound better
      static const unsigned long NoteDurations[] PROGMEM; // Note durations array, unsigned long array (64 bits)
      static const unsigned      MelodyAlarm[] PROGMEM;   // Melody for alarm, unsigned integer array (32 bits)
      static const size_t        MelodySize; // Size of the melody array
      static const size_t        NoteDurationsSize; // Size of the note durations array

      static CRGB DrawBuffer[NUM_LEDS];   // Buffer for drawing the LEDs, used for static symbol display.
      static const CRGB OnText[NUM_LEDS]; // A big       Green 'O'   (for On)
      static const CRGB OffTxt[NUM_LEDS]; // A sideways  Red   'F'   (for oFF)
      static const CRGB XAbort[NUM_LEDS]; // A big       Pink  'X'   (for Abort/Cancel)
      static const CRGB OkText[NUM_LEDS]; // A big       Lime  tick  (for Okay/Good)

   private:
      // These variables are initially set to the internal static melody and note durations arrays
      // They can be changed to use different melodies and note durations in the ESP32 flash memory.
      unsigned      *melodyAlarm;         // Pointer to the melody array
      int            melodySize;          // Size of the melody array
      unsigned long *noteDurations;       // Pointer to the note durations array
      int            noteDurationsSize;   // Size of the note durations array

      CRGB leds[NUM_LEDS];                // Array of LED colors to display the current time
      bool binaryArray[NUM_LEDS];         // Serial Debug: Array for binary representation of time

      // Define a MACRO to declare the buttons and initialize the 'ButtonState' values.
      // A macro to reduce cut-n-paste errors so initialization is always correct based on 
      //         if the button input pin is pulled LOW (CC) or HIGH (CA) in the OFF state.
      // The 'onValue' is set to the value that the button pin sees when pressed, 
      //      HIGH (i.e. CC_ON, button connects to VCC when pressed) or 
      //      LOW  (i.e. CA_ON, button connects to ground when pressed).
      // 'NAME' - The suffix to add to 'button' to create the ButtonState object.
      // 'PIN' - The pin number that the button is connected to.
      // 'TYPE_CC_CA' - 'CC' when the OFF state is LOW  (pulled HIGH when pressed), 
      //              - 'CA' when the OFF state is HIGH (pulled LOW  when pressed)..
      #define DECLARE_BUTTON(NAME, PIN, TYPE_CC_CA) \
            ButtonState button##NAME = { .pin = PIN, .state = TYPE_CC_CA##_OFF, .lastRead = TYPE_CC_CA##_OFF, \
                                         .lastReadTime = 0UL, .lastDebounceTime = 0UL, .onValue = TYPE_CC_CA##_ON }

      // The 3 buttons used to control the Binary Clock Shield menu for setting the time and alarm.
      DECLARE_BUTTON(S1, S1, CC);         // Declare 'ButtonState buttonS1'
      DECLARE_BUTTON(S2, S2, CC);         // Declare 'ButtonState buttonS2'
      DECLARE_BUTTON(S3, S3, CC);         // Declare 'ButtonState buttonS3'
            
      #if HW_DEBUG_SETUP
      DECLARE_BUTTON(DebugSetup, DEBUG_SETUP_PIN, CC);   // Declare 'ButtonState buttonDebugSetup'
      #endif

      #if HW_DEBUG_TIME
      DECLARE_BUTTON(DebugTime, DEBUG_TIME_PIN, CA);     // Declare 'ButtonState buttonDebugTime'
      #endif
      #undef DECLARE_BUTTON   // Undefine, we only needed it here to write the declarations without errors.

      DateTime time;                         // Current time from the RTC, updated every second.
      DateTime tempTime;                     // Temporary time variable used when setting the time.
      AlarmTime tempAlarm;                   // Temporary Alarm used when setting the alarm.
      int countButtonPressed;                // Counter for button pressed during time/alarm settings
      bool callbackAlarmEnabled = false;     // Flag: The 'Alarm' callback is enabled (i.e. is not nullptr) or not.
      bool callbackTimeEnabled  = false;     // Flag: The 'Time'  callback is enabled (i.e. is not nullptr) or not.
      bool rtcValid             = false;     // Flag: The RTC was found and initialized.
      void (*alarmCallback)(DateTime) = nullptr; // Callback function for alarm triggers.
      void (*timeCallback)(DateTime)  = nullptr; // Callback function for time trigger (1 Hz frequency).

      unsigned long debounceDelay = DEFAULT_DEBOUNCE_DELAY; // The debounce time for a button press.
      bool pixelsPresent = false;            // Flag: Indicates if the shield is attached (or just a dev. board).

      // Variables that store the current settings option
      int settingsOption = 0;               // Time = 1, Alarm = 3  
      int settingsLevel = 0;                // Hours = 1, Minutes = 2, Seconds / On/Off Alarm = 3

      int alarmRepeatMax = DEFAULT_ALARM_REPEAT;   // Maximum alarm repeat count
      int alarmRepeatCount = 0;                    // Current alarm repeat count
      byte brightness = DEFAULT_BRIGHTNESS;        // Brightness of the LEDs, 0-255 (20 - 60 is a good range).

      char buffer[64] = { 0 };                     // Buffer for the DateTime string conversions

      bool isSerialSetup = (SERIAL_SETUP_CODE) && (DEFAULT_SERIAL_SETUP); // Serial setup flag
      bool isSerialTime  = (SERIAL_TIME_CODE)  && (DEFAULT_SERIAL_TIME);  // Serial time flag   

      // Time to wait after serial time button goes off before stopping the serial output.
      // Set to a long delay if using a momentary button, keep short for a switch. This
      // allows a button to be pressed, released and you still get output for 'debugDelay' ms.
      unsigned long debugDelay = DEFAULT_DEBUG_OFF_DELAY; 
      };
   }
#endif
