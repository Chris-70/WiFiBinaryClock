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
// Arduino Uno, Binary Clock Shield for Arduino
// Battery CR1216/CR1220 
// INT/SQW   connected to Arduino pin 3  INT1 ESP32_D1-R32 UNO 25
// PIEZO     connected to Arduino pin 11 PWM  ESP32_D1-R32 UNO 23
// S3 button connected to Arduino pin A0      ESP32_D1-R32 UNO  2
// S2 button connected to Arduino pin A1      ESP32_D1-R32 UNO  4
// S1 button connected to Arduino pin A2      ESP32_D1-R32 UNO 35
// LEDs      connected to Arduino pin A3      ESP32_D1-R32 UNO 34
// RTC SDA   connected to Arduino pin A4      ESP32_D1-R32 UNO 36
// RTC SCL   connected to Arduino pin A5      ESP32_D1-R32 UNO 39
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

#include "BinaryClock.h"

// Include libraries
#include <FastLED.h>            // https://github.com/FastLED/FastLED
#include <RTClib.h>             // Adafruit RTC library: https://github.com/adafruit/RTClib
#include <Streaming.h>          // https://github.com/janelia-arduino/Streaming                            
#include "pitches.h"            // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody

namespace BinaryClockShield
   {
   // This is a 'Binary Clock Shield for Arduino' by Marcin Saj https://nixietester.com
   //

   // Notes in the melody: (70)
   const unsigned BinaryClock::MelodyAlarm[] PROGMEM =
         {
         NOTE_A4,  NOTE_A4,  NOTE_A4,  NOTE_F4,  NOTE_C5,  NOTE_A4,  NOTE_F4,  NOTE_C5,
         NOTE_A4,  NOTE_E5,  NOTE_E5,  NOTE_E5,  NOTE_F5,  NOTE_C5,  NOTE_GS4, NOTE_F4,
         NOTE_C5,  NOTE_A4,  NOTE_A5,  NOTE_A4,  NOTE_A4,  NOTE_A5,  NOTE_GS5, NOTE_G5,
         NOTE_FS5, NOTE_F5,  NOTE_FS5, 0,        NOTE_AS4, NOTE_DS5, NOTE_D5,  NOTE_CS5,
         NOTE_C5,  NOTE_B4,  NOTE_C5,  0,        NOTE_F4,  NOTE_GS4, NOTE_F4,  NOTE_A4,
         NOTE_C5,  NOTE_A4,  NOTE_C5,  NOTE_E5,  NOTE_A5,  NOTE_A4,  NOTE_A4,  NOTE_A5,
         NOTE_GS5, NOTE_G5,  NOTE_FS5, NOTE_F5,  NOTE_FS5, 0,        NOTE_AS4, NOTE_DS5,
         NOTE_D5,  NOTE_CS5, NOTE_C5,  NOTE_B4,  NOTE_C5,  0,        NOTE_F4,  NOTE_GS4,
         NOTE_F4,  NOTE_C5,  NOTE_A4,  NOTE_F4,  NOTE_C5,  NOTE_A4,
         };

   #define NOTE_MS(N) (1000 / N) // Convert note duration to milliseconds
   // Note durations: 4 = quarter note, 8 = eighth note, etc.:
   // Some notes durations have been changed (1, 3, 6) to make them sound better
   const unsigned long BinaryClock::NoteDurations[] PROGMEM =
         {
         NOTE_MS(2), NOTE_MS(2), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6),
         NOTE_MS(1), NOTE_MS(2), NOTE_MS(2), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6), NOTE_MS(2), NOTE_MS(3),
         NOTE_MS(6), NOTE_MS(1), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6), NOTE_MS(2), NOTE_MS(4), NOTE_MS(4),
         NOTE_MS(8), NOTE_MS(8), NOTE_MS(4), NOTE_MS(3), NOTE_MS(4), NOTE_MS(2), NOTE_MS(4), NOTE_MS(4),
         NOTE_MS(8), NOTE_MS(8), NOTE_MS(4), NOTE_MS(3), NOTE_MS(6), NOTE_MS(2), NOTE_MS(3), NOTE_MS(6),
         NOTE_MS(2), NOTE_MS(3), NOTE_MS(6), NOTE_MS(1), NOTE_MS(2), NOTE_MS(3), NOTE_MS(8), NOTE_MS(2),
         NOTE_MS(4), NOTE_MS(4), NOTE_MS(8), NOTE_MS(8), NOTE_MS(4), NOTE_MS(4), NOTE_MS(4), NOTE_MS(2),
         NOTE_MS(4), NOTE_MS(4), NOTE_MS(8), NOTE_MS(8), NOTE_MS(4), NOTE_MS(4), NOTE_MS(4), NOTE_MS(2),
         NOTE_MS(3), NOTE_MS(8), NOTE_MS(2), NOTE_MS(3), NOTE_MS(8), NOTE_MS(1),
         };
   #undef NOTE_MS

   // Calculate the number of elements in each array, then we validate they are the same size to catch definition errors.
   const size_t BinaryClock::MelodySize = sizeof(BinaryClock::MelodyAlarm) / sizeof(BinaryClock::MelodyAlarm[0]); 
   const size_t BinaryClock::NoteDurationsSize = sizeof(BinaryClock::NoteDurations) / sizeof(BinaryClock::NoteDurations[0]); 
   
   // Default: Colors for the LEDs when ON, Seconds, Minutes and Hours
   CRGB BinaryClock::OnColor[NUM_LEDS] =
         {
         CRGB::DarkRed,   CRGB::DarkRed,   CRGB::DarkRed,   CRGB::DarkRed,   CRGB::Red,   CRGB::Red,    // Seconds (0 - 5)  
         CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::Green, CRGB::Green,  // Minutes (6 - 11) 
         CRGB::DarkBlue,  CRGB::DarkBlue,  CRGB::DarkBlue,  CRGB::DarkBlue,  CRGB::Blue                 // Hours   (12 - 16)
         };

   // Default: Colors for the LEDs when OFF (Usually Black or No Power (i.e. OFF), Seconds, Minutes and Hours)
   //    Note: Using any color other than Black means the LEDS will be consuming power at all times.
   CRGB BinaryClock::OffColor[NUM_LEDS] =
         {
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Seconds (0 - 5)
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Minutes (6 - 11)
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black                // Hours   (12 - 16)
         };

   // When the SERIAL_SETUP_CODE code is removed, redefine the method calls to be whitespace only
   // This allows the code to compile without the serial setup code, but still allows the methods 
   // to be "called" in the code without causing compilation errors (Must return void to work).
   #if SERIAL_SETUP_CODE != true
   #define serialStartInfo()
   #define serialSettings()
   #define serialAlarmInfo()
   #define serialCurrentModifiedValue()
   #endif 

   // When the SERIAL_TIME_CODE code is remove, redefine the method calls to be whitespace only
   #if SERIAL_TIME_CODE != true
   #define serialTime() 
   #endif

   //################################################################################//
   // SETUP
   //################################################################################//

   void BinaryClock::setup()
      {
      #if SERIAL_SETUP_CODE || SERIAL_TIME_CODE
      Serial.begin(115200);
      delay(10);
      Serial.println(F("Binary Clock Shield for Arduino\n  by Marcin Saj https://nixietester.com"));
      #endif

      assert(melodySize == noteDurationsSize);  // Ensure the melody and note durations arrays are the same size

      if (setupRTC())
         {
         setupAlarm();
         setupFastLED();

         // Delay the initial startup, show the serial output or just a small delay.
         if (isSerialSetup) 
            { serialStartInfo(); }
         else 
            { delay(3000); } 
         }
      else
         {
         // Send this to Purgatory, we're dead.
         purgatoryTask("No RTC found.");
         }
      }

   void BinaryClock::purgatoryTask(const char* message)
      {
      // This is where failure comes to die.
      pinMode(LED_BUILTIN, OUTPUT); 
      Serial.begin(115200);
      Serial.println("Failure: Unable to continue.");
      if (message != nullptr)
         {
         Serial << "Message: " << message << "\n";
         }  
      Serial.println("Entering Purgatory...");

      FOREVER
         {
         // Flash SOS to signal failure.
         int ditLength = 100;
         int delayDit = ditLength;
         for (int j = 0; j < 3; j++)
            {
            delayDit = (j == 1? 3 * ditLength : ditLength);
            for (int i = 0; i < 3; i++ )
               {
               digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED on
               delay(delayDit);
               digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on
               delay(delayDit);
               }
            }
         delay(750);
         }
      }

   //################################################################################//
   // MAIN LOOP
   //################################################################################//

   void BinaryClock::loop()
      {
      if (RTCinterruptWasCalled)
         {
         time = get_Time();
         RTCinterruptWasCalled = false;         // Clear the interrupt flag

         if (settingsOption == 0)               // Display time but not during time/alarm settings
            {
            convertDecToBinaryAndDisplay(time.hour(), time.minute(), time.second());
            
            #if SERIAL_TIME_CODE
            if (get_isSerialTime()) { serialTime(); }
            #endif

            if (RTC.alarmFired(Alarm2.number) & (Alarm2.status > 0))
               {
               playAlarm();
               CallbackAlarmTriggered = true; // Set the alarm callback flag

               #if SERIAL_TIME_CODE
               if (get_isSerialTime()) { Serial << "   ALARM!\n"; }
               #endif  
               }
            }
         }

      settingsMenu();   // Check if the settings menu is active and handle button presses

      #if HARDWARE_DEBUG
      checkHardwareDebugPin();
      #endif
      }

   //#####################################################################//
   //#              Initialize the RTC library                           #//
   //#####################################################################//

   bool BinaryClock::setupRTC()
      {
      rtcValid = RTC.begin();

      if (rtcValid)
         {
         // Configure an interrupt on the falling edge from the RTC INT/SQW output
         pinMode(RTC_INT, INPUT_PULLUP);
         // Attach the interrupt to the member RTCinterrupt function using a lambda
         attachInterrupt(
               digitalPinToInterrupt(RTC_INT),
               []() { get_Instance().RTCinterrupt(); },
               FALLING); // FALLING; 1 Hz is driven LOW

         // Enable 1 Hz square wave RTC SQW output
         RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_SquareWave1Hz);
         }

      return rtcValid;
      }

   //#####################################################################//
   //#            Initialize the RTC library and set up the RTC          #//
   //#####################################################################//

   void BinaryClock::setupAlarm()
      {
      if (rtcValid)
         {
         // Get the alarms stored in the RTC memory.
         Alarm1.time = RTC.getAlarm1();
         Alarm2.time = RTC.getAlarm2();

         // Clear the alarm status flags 'A1F' and 'A2F' after reboot
         uint8_t control;
         control = RTC.RawRead(DS3231_CONTROL);

         // Design: Alarm 1 and Alarm 2 status/control reflect the values in the RTC so
         //         we will reflect their stored values as the RTC is battery backed.
         // Future: These alarm values should be kept in the EEPROM to persist across reboots
         Alarm1.status = (control & DS3231_ALARM1_STATUS_MASK) ? 1 : 0; // Alarm 1 status
         Alarm2.status = (control & DS3231_ALARM2_STATUS_MASK) ? 1 : 0; // Alarm 2 status

         getAlarmTimeAndStatus();
         }
      }

   //#####################################################################//
   //#            Initialize the FastLED library                         #//
   //#####################################################################//

   void BinaryClock::setupFastLED()
      {
      FastLED.setBrightness(0);
      FastLED.clear(true);
      
      // Limit my draw to 450mA at 5V of power draw
      FastLED.setMaxPowerInVoltsAndMilliamps(5, 450);

      // Initialize LEDs
      FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
      delay(50);
      FastLED.setBrightness(brightness);
      }

   void BinaryClock::callbackTask()
      {
      FOREVER
         {
         if (callbackTimeEnabled && CallbackTimeTriggered)
            { callbackFtn(CallbackTimeTriggered, time, timeCallback); }

         if (callbackAlarmEnabled && CallbackAlarmTriggered)
            { callbackFtn(CallbackAlarmTriggered, get_Alarm().time, alarmCallback); }

         // vTaskDelay to prevent busy waiting
         // vTaskDelay(pdMS_TO_TICKS(50));
         delay(150); // *** DEBUG ***
         }
      }

   void BinaryClock::callbackFtn(volatile bool& triggerFlag, DateTime time, void(*callback)(DateTime))
      {
      if (triggerFlag) // If the time callback was triggered
         {
         triggerFlag = false; // Reset the flag
         if (callback != nullptr)   // If the callback is set
            {
            callback(time);         // Call the time callback with the current time
            }
         }
      }

   //################################################################################//
   // RTC LIBRARY PLUS - EXTENDED FUNCTIONALITY                                     #//
   //################################################################################//

   uint8_t  RTCLibPlusDS3231::RawRead(uint8_t reg)
      {
      return read_register(reg);  // Call the base class method to read a register
      }

   void RTCLibPlusDS3231::RawWrite(uint8_t reg, uint8_t value)
      {
      write_register(reg, value);  // Call the base class method to write a register
      }

   //################################################################################//
   // BINARY CLOCK METHODS                                                          #//
   //################################################################################//

   BinaryClock::BinaryClock() :
         countButtonPressed(0),
         settingsOption(0),
         settingsLevel(0),
         brightness(DEFAULT_BRIGHTNESS),
         RTCinterruptWasCalled(false), 
         CallbackAlarmTriggered(false),
         CallbackTimeTriggered(false)
      {
      melodyAlarm = (unsigned *)MelodyAlarm;          // Assign the melody array to the pointer
      melodySize = MelodySize;                        // Assign the size of the melody array
      noteDurations = (unsigned long *)NoteDurations; // Assign the note durations array to the pointer
      noteDurationsSize = NoteDurationsSize;          // Assign the size of the note

      #pragma region Required code, UNO error: "sorry, unimplemented: non-trivial designated initializers not supported"
      memset(leds, 0, sizeof(leds)); // Clear the LED array
      memset(binaryArray, 0, sizeof(binaryArray)); // Clear the binary array

      Alarm1.number = ALARM_1; 
      Alarm1.melody = 0; 
      Alarm1.status = 0;
      Alarm2.number = ALARM_2;
      Alarm2.melody = 0;
      Alarm2.status = 0;
      #pragma endregion

      // Setup the button pin as input and based on connection internal pullup/down.
      initializeButtons();

      #if HW_DEBUG_TIME
      // Set the 'isSerialTime' to true if the hardware Time button is ON 
      // This is necessary if the button is actually a switch or is hardwired
      if (digitalRead(buttonDebugTime.pin) == buttonDebugTime.onValue) 
         {
         isSerialTime = true;                   // Enable serial time
         }
      #endif
      }
   
   BinaryClock::~BinaryClock()
      {
      // Detach the interrupt from the RTC INT pin
      detachInterrupt(digitalPinToInterrupt(RTC_INT)); 

      // Disable 1 Hz square wave RTC SQW output
      RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_OFF);;

      // Turn off the LEDs.
      FastLED.setBrightness(0);
      FastLED.clear(true);  
      }

   void BinaryClock::initializeButtons()
      {
      // Define a MACRO to write the button setup, reduce potential cut-n-paste errors.
      #define INITIALIZE_BUTTON(BUTTON_OBJ) \
            pinMode((BUTTON_OBJ).pin, (HIGH == (BUTTON_OBJ).onValue ? ESP32_INPUT_PULLDOWN : INPUT_PULLUP))

      // Set the button pins as input with pull-up/down based on the wiring (i.e. onValue)
      INITIALIZE_BUTTON(buttonS1);
      INITIALIZE_BUTTON(buttonS2);
      INITIALIZE_BUTTON(buttonS3);

      #if HW_DEBUG_SETUP
      INITIALIZE_BUTTON(buttonDebugSetup);
      #endif

      #if HW_DEBUG_TIME
      INITIALIZE_BUTTON(buttonDebugTime);
      #endif

      #undef INITIALIZE_BUTTON // Undefine the MACRO, only used here
      }

   void BinaryClock::RTCinterrupt()
      {
      RTCinterruptWasCalled = true;
      // Set the trigger flag IFF the callback time is enabled flag is set.
      CallbackTimeTriggered = callbackTimeEnabled;
      }

   void BinaryClock::set_Time(DateTime &value)
      {
      RTC.adjust(value);    // Set the time in the RTC
      }

   DateTime BinaryClock::get_Time()
      {
      return RTC.now();
      }

   void BinaryClock::set_Alarm(AlarmTime& value)
      {
      if (value.number < ALARM_1 || value.number > ALARM_2) { return; }

      // Set the alarm time and status in the RTC
      if (value.status > 0)
         {
         // NOTE: (Chris-70, 2025/07/05)
         // The current version of Adafruit's RTCLib (2.1.4) does not allow setting the
         // Alarm interrupt registers A1IE and A2IE when the INTCN bit is set to 0.
         // This means we must disable the 1Hz SQ Wave, set the bits and then re-enable the SQ Wave.
         // This shouldn't impact the LED display but it might generate an additional interrupt.
         RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_OFF);

         // If the alarm status is +ve, set the alarm to sound at 'hour:minute' each day.
         if (value.number == ALARM_1)
            {
            RTC.setAlarm1(value.time, Ds3231Alarm1Mode::DS3231_A1_Hour);
            }
         else if (value.number == ALARM_2)
            {  
            RTC.setAlarm2(value.time, Ds3231Alarm2Mode::DS3231_A2_Hour);
            }

         RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_SquareWave1Hz);
         }
      else if (value.status == 0)
         {
         RTC.disableAlarm(value.number);
         }
      else { ; } // Ignore bad input status
      }

   AlarmTime BinaryClock::GetAlarm(int number)
      {
      AlarmTime result;
      if (number == ALARM_1)
         {
         Alarm1.time = RTC.getAlarm1();
         Alarm1.status = RTC.RawRead(DS3231_CONTROL) & DS3231_ALARM1_STATUS_MASK;
         result = Alarm1;
         }
      else if (number == ALARM_2) 
         {
         Alarm2.time = RTC.getAlarm2();
         Alarm2.status = (RTC.RawRead(DS3231_CONTROL) & DS3231_ALARM2_STATUS_MASK) >> 1;
         result = Alarm2;
         }

      return result;
      }

   bool BinaryClock::get_isSerialSetup() const
      {
      #if SERIAL_SETUP_CODE
      return isSerialSetup;
      #else
      return false;
      #endif
      }

   void BinaryClock::set_isSerialSetup(bool value)
      {
      #if SERIAL_SETUP_CODE
      isSerialSetup = value;
      #endif
      }

   bool BinaryClock::get_isSerialTime() const
      {
      #if SERIAL_TIME_CODE
      return isSerialTime;
      #else
      return false;
      #endif
      }

   void BinaryClock::set_isSerialTime(bool value)
      {
      #if SERIAL_TIME_CODE
      isSerialTime = value;
      #endif
      }

   void BinaryClock::set_Brightness(byte value)
      {
      brightness = value;
      FastLED.setBrightness(brightness); // Set the brightness of the LEDs
      }

   byte BinaryClock::get_Brightness() 
      {
      brightness = FastLED.getBrightness();
      return brightness;
      }

   void BinaryClock::set_DebugOffDelay(unsigned long value)
      {
      debugDelay = value;
      }

   unsigned long BinaryClock::get_DebugOffDelay() const
      {
      return debugDelay;
      }

   void BinaryClock::set_DebounceDelay(unsigned long value)
      {
      debounceDelay = value;
      }

   unsigned long BinaryClock::get_DebounceDelay() const
      {
      return debounceDelay;
      }
   
   bool BinaryClock::registerTimeCallback(void (*callback)(DateTime))
      {
      if (timeCallback == nullptr) // Only register if not already registered
         {
         timeCallback = callback;
         callbackTimeEnabled = true;
         return true; // Successfully registered
         }

         return false; // Callback already registered
      }

   bool BinaryClock::unregisterTimeCallback(void (*callback)(DateTime)) 
      {
      callbackTimeEnabled = false;
      if (timeCallback == callback) // Only unregister if it matches the registered callback
         {
         timeCallback = nullptr;
         return true; // Successfully unregistered
         }

      return false; // Callback not found
      }

   bool BinaryClock::registerAlarmCallback(void (*callback)(DateTime)) 
      {
      if (alarmCallback == nullptr) // Only register if not already registered
         {
         alarmCallback = callback;
         callbackAlarmEnabled = true;
         return true; // Successfully registered
         }

      return false; // Callback already registered
      }

   bool BinaryClock::unregisterAlarmCallback(void (*callback)(DateTime)) 
      {
      callbackAlarmEnabled = false;
      if (alarmCallback == callback) // Only unregister if it matches the registered callback
         {
         alarmCallback = nullptr;
         return true; // Successfully unregistered
         }

      return false; // Callback not found
      }

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
   
   ////////////////////////////////////////////////////////////////////////////////////
   /// This function handles the settings menu for the Binary Clock.
   /// It allows the user to set the time and alarm, modify the current value, and
   /// save the modified value. The settings menu is navigated using the buttons S1, S2, and S3.

   void BinaryClock::settingsMenu()
      {
      // Main menu
      if ((settingsOption == 0) & (settingsLevel == 0))
         {
         // Time settings
         if (isButtonOnNew(buttonS1))
            {
            DateTime temp;
            temp = RTC.now();              // Read time from RTC 
            // t = temp.secondstime();                 // Convert to time_t format
            settingsOption = 1;                     // Set time option settings
            settingsLevel = 1;                      // Set hour level settings
            setCurrentModifiedValue();              // Assign hours for modify +/- 
            if (isSerialSetup) { serialSettings(); }// Use serial monitor for showing settings
            displayCurrentModifiedValue();          // Display current hour on LEDs
            }

         // Alarm settings
         if (isButtonOnNew(buttonS3))
            {
            getAlarmTimeAndStatus();                // Read alarm time and status from RTC
            AlarmTime temp;
            temp = get_Alarm();           // Get the default alarm time and status

            settingsOption = 3;                     // Set Alarm time option settings
            settingsLevel = 1;                      // Set hour level settings
            setCurrentModifiedValue();              // Assign hours for modify +/-
            if (isSerialSetup) { serialSettings(); }// Use serial monitor for showing settings
            displayCurrentModifiedValue();          // Display current alarm hour on LEDs 
            }
         }

      // Any settings option level except main menu
      if (settingsLevel != 0)
         {
         // Decrement - if the buttonS1 was just pressed
         if (isButtonOnNew(buttonS1))
            {
            countButtonPressed--;                  // Decrement current value e.g. hour, minute, second, alarm status
            checkCurrentModifiedValueFormat();     // Check if the value has exceeded the range e.g minute = 60 and correct
            displayCurrentModifiedValue();         // Display current modified value on LEDs 
            if (isSerialSetup) { serialCurrentModifiedValue(); }  // Use serial monitor for showing settings
            }

         // Increment - if the buttonS3 was just pressed
         if (isButtonOnNew(buttonS3)) 
            {
            countButtonPressed++;                  // Increment current value e.g. hour, minute, second, alarm status
            checkCurrentModifiedValueFormat();     // Check if the value has exceeded the range e.g minute = 60 and correct
            displayCurrentModifiedValue();         // Display current modified value on LEDs  
            if (isSerialSetup) { serialCurrentModifiedValue(); }  // Use serial monitor for showing settings
            }

         // Save if buttonS2 was just pressed
         if (isButtonOnNew(buttonS2))
            {
            saveCurrentModifiedValue();            // Save current value e.g. hour, minute, second, alarm status     
            settingsLevel++;                       // Go to next settings level - hour => minute => second / alarm status 

            if (settingsLevel > 3)                 // If escape from settings then return to main menu
               {
               if (settingsOption == 1)            // If you were in the process of setting the time:   
                  {
                  setNewTime();                    // Save time to the RTC
                  #if SERIAL_TIME_CODE
                  if (isSerialSetup)
                     {
                     Serial << F("\n-------------------------------------") << endl;
                     Serial << F("---- Current Time: ");
                     Serial << (DateTimeToString(RTC.now(), buffer, sizeof(buffer), "hh:mm:ss")) << endl;
                     Serial << F("-------------------------------------") << endl;
                     }
                  #endif
                  }

               if (settingsOption == 3)            // If you were in the process of setting the alarm: 
                  {
                  setAlarmTimeAndStatus();         // Save time and alarm status to the RTC    
                  #if SERIAL_TIME_CODE
                  if (isSerialSetup)
                     {
                     Serial << endl;               // New line for the setting display above.
                     }
                  #endif

                  serialAlarmInfo();               // Show the time and alarm status info when you exit to the main menu
                  }

               settingsLevel = 0;                  // Escape to main menu  
               settingsOption = 0;                 // Escape to main menu
               }
            else                                   // If you do not go to the main menu yet
               {
               checkCurrentModifiedValueFormat();  // Check if the value has exceeded the range e.g minute = 60 and correct                  
               setCurrentModifiedValue();          // Assign next variable for modify +/- hour => minute => second / alarm status
               displayCurrentModifiedValue();      // Display current modified value on LEDs                
               if (isSerialSetup) { serialSettings(); }  // Use serial monitor for showing settings
               }
            }
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Depending on the options and settings level, assign to the 
   // countButtonPressed variable to be able modify the value

   void BinaryClock::setCurrentModifiedValue()
      {
      // Assign current time value stored in the 'time' variable for modification by the user.
      if (settingsOption == 1)
         {
         if (settingsLevel == 1)  countButtonPressed = time.hour();
         if (settingsLevel == 2)  countButtonPressed = time.minute();
         if (settingsLevel == 3)  countButtonPressed = time.second();
         }

      // Alarm time and alarm status 
      if (settingsOption == 3)
         {
         if (settingsLevel == 1)  countButtonPressed = Alarm2.time.hour();
         if (settingsLevel == 2)  countButtonPressed = Alarm2.time.minute();
         if (settingsLevel == 3)  countButtonPressed = Alarm2.status + 1;
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Check current modified value format of the countButtonPressed variable

   void BinaryClock::checkCurrentModifiedValueFormat()
      {
      // Hours 0-23       
      if (settingsLevel == 1)
         {
         if (countButtonPressed < 0) countButtonPressed = 23;
         if (countButtonPressed > 23) countButtonPressed = 0;
         }

      // Minutes 0-59
      if (settingsLevel == 2)
         {
         if (countButtonPressed < 0) countButtonPressed = 59;
         if (countButtonPressed > 59) countButtonPressed = 0;
         }

      // Seconds & Alarm status
      if (settingsLevel == 3)
         {
         // Seconds 0-59
         if (settingsOption == 1)
            {
            if (countButtonPressed < 0) countButtonPressed = 59;
            if (countButtonPressed > 59) countButtonPressed = 0;
            }

         // Alarm status 1/2
         if (settingsOption == 3)
            {
            if (countButtonPressed < 1) countButtonPressed = 2;
            if (countButtonPressed > 2) countButtonPressed = 1;
            }
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Depending on the options and settings level, save the current modified value

   void BinaryClock::saveCurrentModifiedValue()
      {
      // Save current value in the tmElements_t tm structure
      if (settingsOption == 1)
         {
         if (settingsLevel == 1) time = DateTime(time.year(), time.month(), time.day(), countButtonPressed, time.minute(), time.second());
         if (settingsLevel == 2) time = DateTime(time.year(), time.month(), time.day(), time.hour(), countButtonPressed, time.second());
         if (settingsLevel == 3) time = DateTime(time.year(), time.month(), time.day(), time.hour(), time.minute(), countButtonPressed);
         }

      // Alarm time and alarm status
      if (settingsOption == 3)
         {
         if (settingsLevel == 1) Alarm2.time = DateTime(Alarm2.time.year(), Alarm2.time.month(), Alarm2.time.day(), countButtonPressed, Alarm2.time.minute(), Alarm2.time.second());
         if (settingsLevel == 2) Alarm2.time = DateTime(Alarm2.time.year(), Alarm2.time.month(), Alarm2.time.day(), Alarm2.time.hour(), countButtonPressed, Alarm2.time.second());
         if (settingsLevel == 3) Alarm2.status = countButtonPressed - 1; // Convert 1/2 to 0/1 for the alarm status
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Display on LEDs only currently modified value
   // convertDecToBinaryAndDisplay(hours row, minutes row, seconds row)

   void BinaryClock::displayCurrentModifiedValue()
      {
      if (settingsLevel == 1) { convertDecToBinaryAndDisplay(countButtonPressed, 0, 0); } // Hours
      if (settingsLevel == 2) { convertDecToBinaryAndDisplay(0, countButtonPressed, 0); } // Minutes
      if (settingsLevel == 3) { convertDecToBinaryAndDisplay(0, 0, countButtonPressed); } // Seconds
      }

   //################################################################################//
   // ALARM HANDLING
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Get alarm time and convert it from BCD to DEC format
   // Get alarm status active/inactive

   void BinaryClock::getAlarmTimeAndStatus() 
      {
      Alarm2 = get_Alarm(); // Get the default alarm time and status
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Set alarm time and status

   void BinaryClock::setAlarmTimeAndStatus()
      {
      set_Alarm(Alarm2); // Set the second alarm time and status in the RTC
      }

   //################################################################################//
   // RTC TIME & BINARY FORMAT
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Write time to RTC

   void BinaryClock::setNewTime() { set_Time(time); }

   ////////////////////////////////////////////////////////////////////////////////////
   // Convert values from DEC to BIN format and display

   void BinaryClock::convertDecToBinaryAndDisplay(int hoursRow, int minutesRow, int secondsRow)
      {
      // A fast and efficient way to convert decimal to binary format and to set the individual LED colors
      // The original code ran in a hard-coded loop extracting on bit at a time from the decimal value
      // and setting the corresponding LED color based on whether the bit was 0 or 1
      // This code just does a bitwise AND and tests if the bit was non-zero, then sets the corresponding LED color.
      // I removed the loops as the number of LEDs is fixed in the hardware and doesn't need to be generic.
      // This results in a faster execution time and less code complexity, easier to maintain.
      // Using a MACRO to handle SERIAL_TIME_CODE compiles where the serial output is used, and 'binaryArray[]' is required.
      // Performing the assignment to 'binaryArray[]' then testing the result for non-zero to assign the colour
      #if SERIAL_TIME_CODE
         // If SERIAL_TIME_CODE is defined, we need to keep track of the binary representation of the time
         #define SET_LEDS(led_num, value, bitmask, on_color, off_color) \
               leds[led_num] = (binaryArray[led_num] = ((value) & (bitmask))) ? on_color : off_color;
      #else
         #define SET_LEDS(led_num, value, bitmask, on_color, off_color) \
               leds[led_num] = ((value) & (bitmask)) ? on_color : off_color;
      #endif
      
      // H - upper row LEDs where the hour is displayed, left most bit, the MSB, is led 16, LSB is LED 12
      SET_LEDS(16, hoursRow,   0b00010000, OnColor[16], OffColor[16]); // LED 16 - bit[4] for hour 16 (msb)
      SET_LEDS(15, hoursRow,   0b00001000, OnColor[15], OffColor[15]); // LED 15 - bit[3] for hour  8      
      SET_LEDS(14, hoursRow,   0b00000100, OnColor[14], OffColor[14]); // LED 14 - bit[2] for hour  4      
      SET_LEDS(13, hoursRow,   0b00000010, OnColor[13], OffColor[13]); // LED 13 - bit[1] for hour  2      
      SET_LEDS(12, hoursRow,   0b00000001, OnColor[12], OffColor[12]); // LED 12 - bit[0] for hour  1 (lsb)

      // M - middle row LEDs where the minute is displayed, left most bit, the MSB, is led 12, LSB is LED 6
      SET_LEDS(11, minutesRow, 0b00100000, OnColor[11], OffColor[11]); // LED 11 - bit[5] for minute 32 (msb)
      SET_LEDS(10, minutesRow, 0b00010000, OnColor[10], OffColor[10]); // LED 10 - bit[4] for minute 16      
      SET_LEDS( 9, minutesRow, 0b00001000, OnColor[ 9], OffColor[ 9]); // LED  9 - bit[3] for minute  8      
      SET_LEDS( 8, minutesRow, 0b00000100, OnColor[ 8], OffColor[ 8]); // LED  8 - bit[2] for minute  4      
      SET_LEDS( 7, minutesRow, 0b00000010, OnColor[ 7], OffColor[ 7]); // LED  7 - bit[1] for minute  2      
      SET_LEDS( 6, minutesRow, 0b00000001, OnColor[ 6], OffColor[ 6]); // LED  6 - bit[0] for minute  1 (lsb)

      // S - bottom row LEDs where the second is displayed, left most bit, the MSB, is led 5, LSB is LED 0
      SET_LEDS( 5, secondsRow, 0b00100000, OnColor[ 5], OffColor[ 5]); // LED  5 - bit[5] for second 32 (msb)
      SET_LEDS( 4, secondsRow, 0b00010000, OnColor[ 4], OffColor[ 4]); // LED  4 - bit[4] for second 16
      SET_LEDS( 3, secondsRow, 0b00001000, OnColor[ 3], OffColor[ 3]); // LED  3 - bit[3] for second  8
      SET_LEDS( 2, secondsRow, 0b00000100, OnColor[ 2], OffColor[ 2]); // LED  2 - bit[2] for second  4
      SET_LEDS( 1, secondsRow, 0b00000010, OnColor[ 1], OffColor[ 1]); // LED  1 - bit[1] for second  2
      SET_LEDS( 0, secondsRow, 0b00000001, OnColor[ 0], OffColor[ 0]); // LED  0 - bit[0] for second  1 (lsb)

      FastLED.show();
      #undef SET_LEDS
      }

   //################################################################################//
   // MELODY ALARM
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // During playing the alarm melody, time display function is disabled

   void BinaryClock::playAlarm()
      {
      unsigned long millis_time_now = 0;
      unsigned long noteDuration;

      for (int i = 0; i < alarmRepeatMax; i++)
         {
         for (int thisNote = 0; thisNote < noteDurationsSize; thisNote++)  // Changed from: allNotes
            {
            noteDuration = noteDurations[thisNote];
            // Create the tone with the note frequency and duration for the current note
            tone(PIEZO, melodyAlarm[thisNote], noteDuration);

            // To distinguish the notes, set a minimum time between them.
            // The note's duration + 30% seems to work well; using 31.25% to remove floating point math (25% + 6.25%).
            int pauseBetweenNotes = noteDuration + (noteDuration >> 2) + (noteDuration >> 4); // 1.3125 instead of 1.30;

            // Millis time start, time 0 for this note pause
            millis_time_now = millis();

            // Pause between notes
            while (millis() < millis_time_now + pauseBetweenNotes)
               {
               // Stop alarm melody and go to main menu
               if (isButtonOnNew(buttonS2))
                  {
                  // Prepare for escape to main menu
                  settingsLevel = 0;
                  settingsOption = 0;

                  // Stop the tone playing
                  noTone(PIEZO);

                  // Exit we are done. Escape to main menu
                  return;
                  }
               }

            // Stop the tone playing between notes
            noTone(PIEZO);
            }
         }
      }

   //################################################################################//
   // CHECK BUTTONS
   //################################################################################//

   bool BinaryClock::isButtonOnNew(ButtonState &button)
      {
      bool result = false;

      // Read the state of the push button into a local variable:
      int currentread = digitalRead(button.pin);
      unsigned long currentReadTime = millis();

      // For the first read of the button (i.e. lastReadTime == 0) we will treat this
      // as a new button state and force the code to proceed as if this was a state change.
      if (button.lastReadTime == 0)
         {
         button.state = !currentread;
         button.lastRead = currentread;
         }

      // Check to see if you just pressed the button
      // (i.e. the input went from LOW to HIGH), and you've waited long enough
      // since the last press to ignore any noise:

      // Check if button changed, due to noise or pressing:
      if (currentread != button.lastRead)
         {
         // Reset the debouncing timer
         button.lastDebounceTime = currentReadTime;
         }

      // Whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:
      if ((currentReadTime - button.lastDebounceTime) > debounceDelay)
         {
         // If the button state has changed:
         if (currentread != button.state)
            {
            button.state = currentread;
            button.lastReadTime = currentReadTime; // Save the time of the last state change

            // Return 1 only if the new button state is HIGH
            if (button.state == button.onValue)
               {
               result = true;
               }
            }
         }

      // Save button state. Next time through the loop, it'll be the lastread:
      button.lastRead = currentread;
      return result;
      }

   //################################################################################//
   // SERIAL INFO
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Print Time in Decimal & Binary formats

   #if SERIAL_TIME_CODE
   void BinaryClock::serialTime()
      {
      static unsigned long lastCall = 0;
      unsigned long curCall = millis();
      if (curCall - lastCall > 750) // prevent multiple displays within ~1 sec.
         {
         lastCall = curCall;
         Serial << F("Time: ") << time.toString(buffer, sizeof(buffer), "hh:mm:ss  ") << F("Binary: ");

         for (int i = NUM_LEDS - 1; i >= 0; i--)
            {
            if (i == 11 || i == 5) Serial << (" ");
            Serial << (binaryArray[i] ? "1" : "0"); // Print 1 or 0 for each LED
            }
         Serial << endl;
         }
      }
   #endif 

   #if SERIAL_SETUP_CODE
   ////////////////////////////////////////////////////////////////////////////////////
   // Show the Shield settings menu and alarm status

   void BinaryClock::serialStartInfo()
      {
      Serial << F("-------------------------------------") << endl;
      Serial << F("------- BINARY CLOCK SHIELD ---------") << endl;
      Serial << F("----------- FOR ARDUINO -------------") << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << F("------------- Options ---------------") << endl;
      Serial << F("S1 - Time Settings ------------------") << endl;
      Serial << F("S2 - Disable Alarm Melody -----------") << endl;
      Serial << F("S3 - Alarm Settings -----------------") << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << F("---- Current Time: ");
      Serial << (DateTimeToString(RTC.now(), buffer, sizeof(buffer), "hh:mm:ss")) << endl;

      serialAlarmInfo();

      Serial << F("#####################################") << endl;
      Serial << F("---- STARTING WITHIN 10 SECONDS -----") << endl;

      // Show progress bar: #37 * 270ms = ~10s delay
      for (int i = 0; i < 37; i++)
         {
         delay(270);
         Serial << ("#");
         }

      Serial << endl << endl;
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show alarm settings

   void BinaryClock::serialSettings()
      {
      if (settingsOption == 1)
         {
         Serial << endl << endl;
         Serial << F("-------------------------------------") << endl;
         Serial << F("---------- Time Settings ------------") << endl;
         Serial << F("-------------------------------------") << endl;
         Serial << F("---- Current Time: ");
         Serial << (DateTimeToString(RTC.now(), buffer, sizeof(buffer), "hh:mm:ss")) << endl;
         Serial << F("-------------------------------------") << endl;
         }

      if (settingsOption == 3)
         {
         Serial << endl << endl;
         Serial << F("-------------------------------------") << endl;
         Serial << F("---------- Alarm Settings -----------") << endl;
         Serial << F("-------------------------------------") << endl;
         serialAlarmInfo();
         }

      if (settingsLevel == 1) Serial << F("--------------- Hour ----------------") << endl;
      if (settingsLevel == 2) Serial << F("-------------- Minute ---------------") << endl;
      if (settingsLevel == 3)
         {
         if (settingsOption == 1) Serial << F("-------------- Second ---------------") << endl;
         if (settingsOption == 3) Serial << F("-------------- ON/OFF ---------------") << endl;
         }

      Serial << F("S1 - Decrement ----------------------") << endl;
      Serial << F("S3 - Increment ----------------------") << endl;
      Serial << F("S2 - Save Current Settings Level ----") << endl;
      Serial << F("-------------------------------------") << endl;

      if (settingsLevel == 1) Serial << F("Current Hour: ") << countButtonPressed << (" ");
      if (settingsLevel == 2) Serial << F("Current Minute: ") << countButtonPressed << (" ");
      if (settingsLevel == 3)
         {
         if (settingsOption == 1) Serial << F("Current Second: ") << countButtonPressed << (" ");
         if (settingsOption == 3)
            {
            Serial << F("Alarm Status: ");
            Serial << (countButtonPressed == 2 ? "ON" : "");
            Serial << (countButtonPressed == 1 ? "OFF" : "");
            Serial << (" ");
            }
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show the set alarm time and current alarm status

   void BinaryClock::serialAlarmInfo()
      {
      Serial << F("-------------------------------------") << endl;
      Serial << F("------ Alarm Time: ");
      Serial << (DateTimeToString(Alarm2.time, buffer, sizeof(buffer), "hh:mm")) << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << F("---- Alarm Status: ");
      Serial << (Alarm2.status ? "ON" : "OFF") << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << endl;
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show current alarm status during settings

   void BinaryClock::serialCurrentModifiedValue()
      {
      if ((settingsLevel == 3) & (settingsOption == 3))
         {
         Serial << (countButtonPressed == 2 ? "ON" : "");
         Serial << (countButtonPressed == 1 ? "OFF" : "");
         }
      else
         {
         Serial << countButtonPressed;
         }

      Serial << (" ");
      }
   #endif // SERIAL_SETUP_CODE
    
   #if HARDWARE_DEBUG
   void BinaryClock::checkHardwareDebugPin()
      {
      #if HW_DEBUG_SETUP
      static bool isSerialSetupOn = DEFAULT_SERIAL_SETUP;

      // For the first reading, setup values so isButtonOnNew() returns false;
      // Design: Initial priority to S/W configuration values. This will
      // force the user the press the button AFTER the initial start.
      if (buttonDebugSetup.lastReadTime == 0) 
         {
         buttonDebugSetup.lastReadTime = buttonDebugSetup.lastDebounceTime = millis();
         buttonDebugSetup.lastRead = buttonDebugSetup.state = !buttonDebugSetup.onValue;
         }

      if (isButtonOnNew(buttonDebugSetup)) 
         {
         set_isSerialSetup(!isSerialSetupOn);
         isSerialSetupOn = !isSerialSetupOn;         
         Serial << F("Serial Menu is: ") << (isSerialSetupOn ? "ON" : "OFF") << endl;
         }
      #endif

      #if HW_DEBUG_TIME
      // For the first reading, setup values so isButtonOnNew() returns false;
      // Design: Initial priority to S/W configuration values so we set the
      // state and lastSate to reflect the current PIN value forcin the user to
      // change the state of the switch before we have H/W control.
      if (buttonDebugTime.lastReadTime == 0)
         {
         buttonDebugTime.lastReadTime = millis();
         buttonDebugTime.lastRead = buttonDebugTime.state = digitalRead(buttonDebugTime.pin);
         }

      // Check if the hardware debug pin is set, if not turn off the serial time output 
      // after 'debugDelay' msec has passed. If the initial H/W value is ON but the S/W
      // value is OFF, it will remain OOF until the H/W cycles OFF then ON.
      if (isButtonOnNew(buttonDebugTime)) 
         {
         set_isSerialTime(true); // Set the serial time flag to true
         Serial << F(" Serial Time is: ON") << endl; // Debug: 
         }
      else if (isSerialTime && !buttonDebugTime.isPressed() && ((millis() - buttonDebugTime.lastReadTime) > get_DebugOffDelay()))
         {
         set_isSerialTime(false); // Reset the serial time flag
         Serial << F(" Serial Time is: OFF") << endl;
         }
      #endif
      }
   #endif
   }  // END OF NAMESPACE BinaryClockShield && BINARY CLOCK CLASS code
   //################################################################################//
   