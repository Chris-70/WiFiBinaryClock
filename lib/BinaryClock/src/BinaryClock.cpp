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
// ---------
// - Binary Clock Shield for Arduino (https://nixietester.com/products/binary-clock-shield-for-arduino/)
// - Arduino Uno style board;        (https://www.adafruit.com/product/5500)
// - Battery CR1216/CR1220 
// 
// Pinouts for the shield:
// -----------------------
// INT/SQW   connected to: Arduino pin  3 INT1 ; METRO-S3  3 ; ESP32_D1-R32 UNO 25 ; ESP32-S3_UNO 17
// PIEZO     connected to: Arduino pin 11  PWM ; METRO-S3 11 ; ESP32_D1-R32 UNO 23 ; ESP32-S3_UNO 11
// S3 button connected to: Arduino pin A0      ; METRO-S3 A0 ; ESP32_D1-R32 UNO  2 ; ESP32-S3_UNO  2
// S2 button connected to: Arduino pin A1      ; METRO-S3 A1 ; ESP32_D1-R32 UNO  4 ; ESP32-S3_UNO  1
// S1 button connected to: Arduino pin A2      ; METRO-S3 A2 ; ESP32_D1-R32 UNO 35 ; ESP32-S3_UNO  7
// LEDs      connected to: Arduino pin A3      ; METRO-S3 A3 ; ESP32_D1-R32 UNO 15 ; ESP32-S3_UNO  6
// RTC SDA   connected to: Arduino pin 18      ; METRO-S3 18 ; ESP32_D1-R32 UNO 36 ; ESP32-S3_UNO  8
// RTC SCL   connected to: Arduino pin 19      ; METRO-S3 19 ; ESP32_D1-R32 UNO 39 ; ESP32-S3_UNO  9
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
#include "MorseCodeLED.h"

// Include libraries
#include <FastLED.h>            // https://github.com/FastLED/FastLED
#include <RTClib.h>             // Adafruit RTC library: https://github.com/adafruit/RTClib
#include <Streaming.h>          // https://github.com/janelia-arduino/Streaming                            
#include "pitches.h"            // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody

namespace BinaryClockShield
   {
   // Software for the 'Binary Clock Shield for Arduino' by Marcin Saj https://nixietester.com
   //

   /// @brief Notes (70) to play in the melody for the alarm sound.
   /// @remarks See the links for details on creating your own melody using tone():
   /// @paragraph  (http://www.arduino.cc/en/Tutorial/Tone)
   /// @paragraph  (https://arduino.cc/en/Tutorial/ToneMelody)
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
   /// @brief Note durations: 4 = quarter note, 8 = eighth note, etc.:
   /// @remarks Some notes durations have been changed (1, 3, 6) to make them sound better
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
   
   /// @brief Default: Colors for the LEDs when ON, Seconds, Minutes and Hours
   /// @details The default colors are Hours: Blue; Minutes: Green; and Seconds: Red with the lower nibble a
   ///          darker shade than the upper (half) nibble.
   CRGB BinaryClock::OnColor[NUM_LEDS] =
         {
         CRGB::DarkRed,   CRGB::DarkRed,   CRGB::DarkRed,   CRGB::DarkRed,   CRGB::Red,   CRGB::Red,    // Seconds (0 - 5)  
         CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::Green, CRGB::Green,  // Minutes (6 - 11) 
         CRGB::DarkBlue,  CRGB::DarkBlue,  CRGB::DarkBlue,  CRGB::DarkBlue,  CRGB::Blue                 // Hours   (12 - 16)
         };

   /// @brief Default: Colors for the LEDs Seconds, Minutes and Hours, when OFF (Usually Black i.e. No Power.)
   /// @note  Using any color other than Black means the LEDs will be consuming power at all times.
   CRGB BinaryClock::OffColor[NUM_LEDS] =
         {
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Seconds (0 - 5)
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Minutes (6 - 11)
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black                // Hours   (12 - 16)
         };

   CRGB BinaryClock::PmColor = CRGB::Indigo; // Color for the PM indicator LED, distinct from other time colors (e.g. 0x4000A0)
   CRGB BinaryClock::AmColor = CRGB::DarkSlateBlue;  // Black;  // Color for the AM indicator (Usually Black/OFF.)

   /// @brief The screen shaped in an 'O' for 'On' when setting the alarm to ON in the alarm menu.
   /// @see OffTxt
   /// @see XAbort
   const CRGB BinaryClock::OnText[NUM_LEDS] = 
         {  // A big Green 'O' for On
         CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Black, CRGB::Black,
         CRGB::Green, CRGB::Black, CRGB::Black, CRGB::Green, CRGB::Black, CRGB::Black,
         CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Black
         };

   /// @brief The screen shaped in a RED 'F' for 'Off' when setting the alarm to OFF in the alarm menu.
   /// @see OnText
   /// @see XAbort
   const CRGB BinaryClock::OffTxt[NUM_LEDS] = 
         {  // A big Red 'F' for oFF
         CRGB::Red,   CRGB::Black, CRGB::Red,   CRGB::Black, CRGB::Black, CRGB::Black,
         CRGB::Red,   CRGB::Black, CRGB::Red,   CRGB::Black, CRGB::Black, CRGB::Black,
         CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red
         };

   /// @brief The screen shaped in a big Pink (Fuchsia) 'X' [❌] for abort/cancel when setting the alarm or time.
   /// @remarks This is used to cancel the Time and Alarm settings and exit without making any changes.
   ///          This is also displayed, after the Rainbow (saving/exit) screen to signal nothing saved.
   /// @see OnText
   /// @see OffTxt
   /// @see OkText
   const CRGB BinaryClock::XAbort[NUM_LEDS] = 
         {  // A big Pink (Fuchsia) 'X' [❌] for abort/cancel
         CRGB::Black,   CRGB::Fuchsia, CRGB::Black,   CRGB::Fuchsia, CRGB::Black,   CRGB::Black,
         CRGB::Black,   CRGB::Black,   CRGB::Fuchsia, CRGB::Black,   CRGB::Black,   CRGB::Black,
         CRGB::Black,   CRGB::Fuchsia, CRGB::Black,   CRGB::Fuchsia, CRGB::Black
         };

   /// @brief The screen shaped in a big Lime 'check mark' [✅] for okay/good when saving settings.
   /// @remarks This is used to signal that the settings have been saved successfully.
   /// @see XAbort
   /// @see Rainbow
   const CRGB BinaryClock::OkText[NUM_LEDS] = 
         {                                    // A big Lime 'check mark' [✅] for okay/good     / 
         CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Lime,  CRGB::Black, CRGB::Black, //     \/
         CRGB::Black, CRGB::Black, CRGB::Lime,  CRGB::Black, CRGB::Lime,  CRGB::Black,
         CRGB::Black, CRGB::Lime,  CRGB::Black, CRGB::Black, CRGB::Black
         };

   #ifdef ESP32_WIFI
   /// @brief The screen shaped in a big Blue 'W' [📶] for WPS / WiFi
   /// @remarks This is used to signal that the WiFi needs to setup (e.g. WPS)
   const CRGB BinaryClock::W_Text[NUM_LEDS] = 
         {                                    // A big RoyalBlue 'W' [📶] (for WPS / WiFi)
         CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::Black,
         CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue, CRGB::Black,
         CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue              
         };
   #endif

   /// @brief The screen shaped in a big Rainbow of colors across all LEDs. 
   /// @details This is displayed after the Time or Alarm settings has ended and the 
   ///          program is saving/restoring the settings. This is followed by either
   ///          the 'check mark' [✅] for settings saved or the 'X' [❌] for no changes.
   /// @see OkText
   /// @see XAbort
   const CRGB BinaryClock::Rainbow[NUM_LEDS] = 
         {  // All colors of the rainbow over all LEDs.
         CRGB::Violet, CRGB::Indigo, CRGB::Blue,   CRGB::Green,  CRGB::Yellow, CRGB::Orange,
         CRGB::Indigo, CRGB::Blue,   CRGB::Green,  CRGB::Yellow, CRGB::Orange, CRGB::Red,
         CRGB::Blue,   CRGB::Green,  CRGB::Yellow, CRGB::Orange, CRGB::Red
         };

   // Note: On the Wemos D1-R32 UNO boards, the builtin LED is GPIO 2, this is also the S3 button pin (A0).
   //       Setting this pin HIGH (ON) for more than 'bounceDelay' (e.g. 75) ms will trigger the alarm setup
   //       routine to be called, requiring the user to go through the alarm setup steps to exit 
   //       which can only happen while the LED is LOW (OFF).
   #if defined(LED_HEART) && LED_HEART >= 0
   uint8_t BinaryClock::HeartbeatLED = LED_HEART;
   #else
   uint8_t BinaryClock::HeartbeatLED = LED_BUILTIN;
   #endif

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

   #if DEV_CODE
   void BinaryClock::DisplayAllRegisters()
      {
      char buffer[32];
      const char* nibbles[16] = {
         [0]  = "0000", [1]  = "0001", [2]  = "0010", [3]  = "0011",
         [4]  = "0100", [5]  = "0101", [6]  = "0110", [7]  = "0111",
         [8]  = "1000", [9]  = "1001", [10] = "1010", [11] = "1011",
         [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
         };

      char byteStr[10];
      // Lambda function to convert a byte to a binary string representation.
      // The string is the upper nibble, a space, the lower nibble. This is the
      // best way to display a byte as anyone who knows binary and is worth
      // their salt can instantly understand a binary nibble. It is up to the 
      // caller to deliniate between bytes, e.g. by using a comma and a space.
      auto binByteStr = [&nibbles, &byteStr](uint8_t byte)
         {
         memmove(byteStr, nibbles[byte >> 4], 4);
         byteStr[4] = ' ';
         memmove(byteStr + 5, nibbles[byte & 0x0F], 4);
         byteStr[9] = '\0'; // Null-terminate the string

         return byteStr;
         };

      const size_t RegSize = 0x13;

      uint8_t regs[RegSize] = { 0 };
      Serial << F("RTC Registers:\n");
      for (size_t i = 0; i < RegSize; i++)
         {
         regs[i] = RTC.RawRead(i);
         // Print register number, value in hex, binary and decimal
         snprintf(buffer, sizeof(buffer), "Reg[0x%02X] 0x%02X; %s (%3d)\n", 
                  i, regs[i], binByteStr(regs[i]), (int)regs[i]); 
         Serial << buffer;
         }

      Serial << endl;
      }
   #endif

   void BinaryClock::setup(bool testLeds)
      {
      #if SERIAL_OUTPUT
      Serial.begin(115200);
      delay(10);
      Serial.println(F("____________________________________________"));
      Serial.println(F("|      Software from the Chris Team        |"));
      Serial.println(F("|        (Chris-70 and Chris-80)           |"));
      Serial.println(F("|      Designed to run the fantastic:      |"));
      Serial.println(F("############################################"));
      Serial.println(F("#     'Binary Clock Shield for Arduino'    #"));
      Serial.println(F("############################################"));
      Serial.println(F("#      Shield created by Marcin Saj,       #"));
      Serial.println(F("#        https://nixietester.com/          #"));
      Serial.println(F("# product/binary-clock-shield-for-arduino/ #"));
      Serial.println(F("############################################"));
      Serial.println(F("#  This software is licensed under the GNU #"));
      Serial.println(F("#     General Public License (GPL) v3.0    #"));
      Serial.println(F("############################################"));
      Serial.println();
      #endif

      pinMode(HeartbeatLED, OUTPUT);
      digitalWrite(HeartbeatLED, LOW);

      assert(melodySize == noteDurationsSize);  // Ensure the melody and note durations arrays are the same size
      bool s2Pressed = buttonS2.read();
      if (s2Pressed)       // User override check, display the LED test patterns on the shield.
         { testLeds = true; }

      #if DEV_BOARD
      Serial << "Display LED test patterns on the shield: " << (testLeds? "YES" : "NO") << "; S2 Button was: " 
             << (s2Pressed? "Pressed" : "OFF") << "; Value: " << buttonS2.value() << " OnValue: is: " 
             << buttonS2.onValue << endl;   // *** DEBUG ***
      #endif

      if (setupRTC())
         {
         testLeds = testLeds || RTC.lostPower();
         setupFastLED(testLeds);
         setupAlarm();

         // Show the serial output, display the initial info.
         if (isSerialSetup) 
            { serialStartInfo(); }
         }
      else
         {
         // Send this to Purgatory, we're dead.
         purgatoryTask("No RTC found.");
         }

      delay(150); // Wait to stabilize after setup
      }

   //################################################################################//
   // MAIN LOOP
   //################################################################################//

   void BinaryClock::loop()
      {
      if (timeDispatch())                       // Check if time has been updated (from RTC)
         {
         if (settingsOption == 0)               // Display the time but not during time/alarm settings
            {
            // Display the current time in binary format on the LEDs in the selected time format.
            convertDecToBinaryAndDisplay(time.hour(), time.minute(), time.second(), get_Is12HourFormat());
            
            #if SERIAL_TIME_CODE
            if (get_isSerialTime()) { serialTime(); }
            #endif

            // Check if the alarm has gone off, play the alarm sounds.
            if (Alarm2.fired)
               {
               playAlarm();
               CallbackAlarmTriggered = true;   // Set the alarm callback flag
               Alarm2.fired = false;            // Clear the fired flag, we've processed the alarm.

               #if SERIAL_TIME_CODE
               if (get_isSerialTime()) { Serial << "   ALARM!\n"; }
               #endif  
               }
            }

         callbackDispatch(); // Service the user callback functions as needed.
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

         bool mode12 = RTC.getIs12HourMode();
         // Set the 12/24 hour format based on RTC setting
         if (get_Is12HourFormat() != mode12)
            { set_Is12HourFormat(mode12); }

         time = RTC.now();
         }

      return rtcValid;
      }

   //#####################################################################//
   //#            Initialize the RTC library and set up the RTC          #//
   //#####################################################################//

   #define DAY_SECONDS 86400     // 24 * 3600
   #define MAX_ALARM_DELTA 300   // 5 minutes in seconds

   void BinaryClock::setupAlarm()
      {
      if (rtcValid)
         {
         // Get the alarms stored in the RTC memory.
         DateTime alarm1time = RTC.getAlarm1();
         DateTime alarm2time = RTC.getAlarm2();
         bool alarm1valid = alarm1time.isValid();
         bool alarm2valid = alarm2time.isValid();

         // Validate the alarms and clear them if they aren't valid.
         if (alarm1valid) 
            { Alarm1.time = alarm1time; }
         else
            { 
            Alarm1.clear(); 
            set_Alarm(Alarm1);
            }

         if (alarm2valid)
            { Alarm2.time = alarm2time; }
         else
            { 
            Alarm2.clear(); 
            set_Alarm(Alarm2);
            }

         // Clear the alarm status flags 'A1F' and 'A2F' after reboot
         uint8_t control;
         uint8_t status;
         control = RTC.RawRead(DS3231_CONTROL);
         status = RTC.RawRead(DS3231_STATUSREG);

         // Having just started, we need to know if an alarm has just fired 
         // (i.e. < MAX_ALARM_DELTA seconds ago) so we can set the alarm
         // fired flag correctly. The RTC alarm fired flag will always be set if 
         // the shield has been off for over 24 hrs (for daily alarms) or if
         // the alarm happened while the shield was off.
         long alarm1seconds = (Alarm1.time.secondstime() % (DAY_SECONDS));
         long alarm2seconds = (Alarm2.time.secondstime() % (DAY_SECONDS));
         long timeDaySeconds = RTC.now().secondstime() % (DAY_SECONDS);
         long alarm1delta = timeDaySeconds - alarm1seconds;
         long alarm2delta = timeDaySeconds - alarm2seconds;
         bool alarm1inrange = (alarm1delta > 0L) && (alarm1delta < MAX_ALARM_DELTA);
         bool alarm2inrange = (alarm2delta > 0L) && (alarm2delta < MAX_ALARM_DELTA);

         // Design: Alarm 1 and Alarm 2 status/control reflect the values in the RTC so
         //         we will reflect their stored values as the RTC is battery backed.
         //         Control A1IE and A2IE will match the status value for alarms 1 & 2.
         //         Status  A1F  and A2F  will match the fired flag   for alarms 1 & 2.
         //         The A1IE and A2IE bits are being used as ON/OFF for our 'AlarmTime' objects.
         //         They can't trigger an interrupt as we are using that pin for the 1 Hz
         //         interrupt. We test the A1F and A2F status bits for an alarm triggered/fired.
         // Future: These alarm values could be kept in the EEPROM to persist across reboots if
         //         we can. Will need a solution to work for all boards. DS3231M has no user storage.
         Alarm1.status = (control & DS3231_ALARM1_STATUS_MASK) ? 1 : 0; // Alarm 1 status ON/OFF
         Alarm2.status = (control & DS3231_ALARM2_STATUS_MASK) ? 1 : 0; // Alarm 2 status ON/OFF
         Alarm1.fired = (status & DS3231_ALARM1_FLAG_MASK) ? (Alarm1.status == 1) && (alarm1inrange) : false; // Alarm 'ringing'
         Alarm2.fired = (status & DS3231_ALARM2_FLAG_MASK) ? (Alarm2.status == 1) && (alarm2inrange) : false; // Alarm 'ringing'

         #if DEV_CODE
         Serial << "Alarm1: " <<  Alarm1.time.timestamp(DateTime::TIMESTAMP_TIME) << " (" << alarm1time.timestamp(DateTime::TIMESTAMP_TIME) <<   // *** DEBUG ***
               (Alarm1.time.isValid() ? " Valid) " : " Bad Time) ") << (Alarm1.status > 0 ? " ON; " : " OFF; ") << alarm1delta <<
               (alarm1inrange? " In Range; " : " Continue; ") << (Alarm1.fired? " Alarm Fired " : " No Alarm ") << endl;

         Serial << "Alarm2: " << Alarm2.time.timestamp(DateTime::TIMESTAMP_TIME) << " (" << alarm2time.timestamp(DateTime::TIMESTAMP_TIME) <<  // *** DEBUG ***
               (Alarm2.time.isValid() ? " Valid) " : " Bad Time) ") << (Alarm2.status > 0 ? " ON; " : " OFF; ") << alarm2delta <<
               (alarm2inrange? " In Range; " : " Continue; ") << (Alarm2.fired? " Alarm Fired " : " No Alarm ") << endl;
         #endif

         // Clear the alarm 'Fired' flags on the RTC to catch a new alarm.
         RTC.clearAlarm(Alarm1.number);
         RTC.clearAlarm(Alarm2.number);
         
         tempAlarm = Alarm2; // Save a copy of Alarm2. Used for setting the alarm time from the menu.
         }
      }

   //#####################################################################//
   //#            Initialize the FastLED library                         #//
   //#####################################################################//

   void BinaryClock::setupFastLED(bool testLEDs)
      {
      FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
      delay(50);
      FastLED.setBrightness(0);
      FastLED.clearData();
      
      // Limit to 450mA at 5V of power draw total for all LEDs
      FastLED.setMaxPowerInVoltsAndMilliamps(5, 450);
      FastLED.setBrightness(brightness);
      int frequency = 3;
      // Display the LED test patterns for the user.
      if (testLEDs)
         {
         displayLedBuffer(OnColor, NUM_LEDS);
         FlashLed(HeartbeatLED, 3, 75, 2);      // Acts as a delay(3000/2) and does something.
         displayLedBuffer(OnText, NUM_LEDS);
         FlashLed(HeartbeatLED, 4, 50, 4);      // Acts as a delay(4000/3) and does something.
         displayLedBuffer(OffTxt, NUM_LEDS);
         FlashLed(HeartbeatLED, 4, 50, 4);      // Acts as a delay(4000/3) and does something.
         displayLedBuffer(XAbort, NUM_LEDS);
         FlashLed(HeartbeatLED, 4, 50, 4);      // Acts as a delay(4000/3) and does something.
         displayLedBuffer(OkText, NUM_LEDS);
         FlashLed(HeartbeatLED, 4, 50, 3);      // Acts as a delay(4000/3) and does something.
         displayLedBuffer(W_Text, NUM_LEDS); 
         FlashLed(HeartbeatLED, 4, 50, 2);      // Acts as a delay(4000/3) and does something.
         frequency = 2;
         }

      // Display the rainbow pattern over all pixels to show everything working.
      displayLedBuffer(Rainbow, NUM_LEDS);      // Turn on all LEDS showing a rainbow of colors.
      FlashLed(HeartbeatLED, 5, 25, frequency); // Acts as a delay(2000/2) and does something.
      displayLedBuffer(OffColor, NUM_LEDS);
      FlashLed(HeartbeatLED, 1, 25, frequency); // Acts as a delay(1000/2) and does something.
      }

   void BinaryClock::FlashLed(uint8_t ledNum, uint8_t repeat, uint8_t dutyCycle, uint8_t frequency)
      {
      // Validate/correct the inputs.
      if (dutyCycle > 100) { dutyCycle = 100; }
      if (frequency <  1)  { frequency = 1; }
      if (frequency > 25)  { frequency = 25; }

      uint32_t onTime  = (dutyCycle * 10) / (frequency);
      uint32_t offTime = ((100 - dutyCycle) * 10) / (frequency);
      for (unsigned i = 0; i < repeat; i++)
         {
         digitalWrite(ledNum, HIGH);
         delay(onTime);
         digitalWrite(ledNum, LOW);
         delay(offTime);
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
         RTCinterruptWasCalled(false), 
         CallbackAlarmTriggered(false),
         CallbackTimeTriggered(false),
         brightness(DEFAULT_BRIGHTNESS)
      {
      melodyAlarm = (unsigned *)MelodyAlarm;          // Assign the melody array to the pointer
      melodySize = MelodySize;                        // Assign the size of the melody array
      noteDurations = (unsigned long *)NoteDurations; // Assign the note durations array to the pointer
      noteDurationsSize = NoteDurationsSize;          // Assign the size of the note

      // Required code, UNO error: "sorry, unimplemented: non-trivial designated initializers not supported"
      memset(leds, 0, sizeof(leds)); // Clear the LED array
      memset(binaryArray, 0, sizeof(binaryArray)); // Clear the binary array

      Alarm1.number = ALARM_1; 
      Alarm1.clear();
      Alarm2.number = ALARM_2;
      Alarm2.clear();
      // END UNO required

      // Setup the button pin as input and based on connection internal pullup/down.
      initializeButtons();

      #if HW_DEBUG_TIME
      // Set the 'isSerialTime' to true if the hardware Time button is ON 
      // This is necessary if the button is actually a switch or is hardwired
      if (buttonDebugTime.isPressed())
         { isSerialTime = true; }                  // Enable serial time
      #endif

      time = DateTime(70, 1, 1, 10, 4, 10);  // An 'X' [❌] if RTC fails.
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

      #if DEV_BOARD
      INITIALIZE_BUTTON(buttonDOut); // *** DEBUG ***
      #endif

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
      if (rtcValid && value.isValid())
         { 
         // If the year is 2000, set it to 2001 so that the DayOfWeek() calculation works correctly
         if (value.year() == 2000) 
            { value = DateTime(2001, value.month(), value.day(), value.hour(), value.minute(), value.second()); }
         RTC.adjust(value, get_Is12HourFormat()); // Set the time in the RTC
         }

      time = value;
      }

   DateTime BinaryClock::get_Time() const
      {  return time; }

   void BinaryClock::set_Alarm(AlarmTime& value)
      {
      // Exit on bad input or missing RTC hardware.
      if (value.number < ALARM_1 || value.number > ALARM_2 || !rtcValid || !value.time.isValid()) { return; }

      // Set the alarm time and status in the RTC
      if (value.status >= 0)
         {
         // NOTE: (Chris-70, 2025/07/05)
         // The current version of Adafruit's RTCLib (2.1.4) does not allow setting the
         // Alarm interrupt registers A1IE and A2IE when the INTCN bit is set to 0.
         // This means we must disable the 1Hz SQ Wave, set the bits and then re-enable the SQ Wave.
         // This shouldn't impact the LED display but it might generate an additional interrupt.
         RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_OFF);

         // Set the alarm to sound at the same time, 'hour:minute', each day.
         if (value.number == ALARM_1)
            { RTC.setAlarm1(value.time, Ds3231Alarm1Mode::DS3231_A1_Hour); }
         else if (value.number == ALARM_2)
            { RTC.setAlarm2(value.time, Ds3231Alarm2Mode::DS3231_A2_Hour); }

         RTC.clearAlarm(value.number); // Clear the Alarm Trigger flag.
         RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_SquareWave1Hz);

         // We saved the alarm time in the RTC, now turn it off IFF the status is OFF
         if (value.status == 0)
            { RTC.disableAlarm(value.number); }
         }
      else { ; } // Ignore bad (-ve) input status
      }

   AlarmTime BinaryClock::GetAlarm(int number)
      {
      AlarmTime result;
      if (number == ALARM_1)
         {
         if (rtcValid)
            {
            Alarm1.time = RTC.getAlarm1();
            Alarm1.status = RTC.RawRead(DS3231_CONTROL) & DS3231_ALARM1_STATUS_MASK;
            }               

            result = Alarm1;
         }
      else if (number == ALARM_2) 
         {
         if (rtcValid)
            {
            Alarm2.time = RTC.getAlarm2();
            Alarm2.status = (RTC.RawRead(DS3231_CONTROL) & DS3231_ALARM2_STATUS_MASK) >> 1;
            }

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
         #if HW_DEBUG_TIME
         // Pause any hardware control, software has the control.
         buttonDebugTime.lastReadTime = 0UL; 
         buttonDebugTime.lastRead = buttonDebugTime.state = buttonDebugTime.value();
         #endif
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

   void BinaryClock::set_Is12HourFormat(bool value)
      {
      amPmMode = value;
      timeFormat = value? timeFormat12 : timeFormat24;
      alarmFormat = value? alarmFormat12 : alarmFormat24;
      RTC.setIs12HourMode(value); // Set the RTC to 12/24 hour mode
      #if DEV_CODE
      Serial << endl << "Is AM/PM? " << (value? "True" : "False") << "; Formats in use: " << timeFormat << "; " << alarmFormat << endl; // *** DEBUG ***
      #endif
      }

   bool BinaryClock::get_Is12HourFormat() const
      {
      return amPmMode;
      }

   #if HW_DEBUG_TIME
   void BinaryClock::set_DebugOffDelay(unsigned long value)
      {
      debugDelay = value;
      }

   unsigned long BinaryClock::get_DebugOffDelay() const
      {
      return debugDelay;
      }
   #endif

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

   #if FREE_RTOS
   void BinaryClock::timeTask()
      {
      FOREVER
         {
         timeDispatch();

         // vTaskDelay to prevent busy waiting
         vTaskDelay(pdMS_TO_TICKS(50));
         }
      }
   #endif

   bool BinaryClock::timeDispatch()
      {
      bool result = false;

      if (RTCinterruptWasCalled)
         {
         time = RTC.now();

         if ((Alarm2.status > 0) && RTC.alarmFired(Alarm2.number))
            {
            Alarm2.fired = true;           // Set the flag, the alarm went off (e.g. ringing).
            CallbackAlarmTriggered = true; // Set the alarm callback flag
            RTC.clearAlarm(Alarm2.number); // Clear the alarm flag for next alarm trigger.
            }

         RTCinterruptWasCalled = false;
         result = true;
         }

      return result;
      }

   #if FREE_RTOS
   void BinaryClock::callbackTask()
      {
      FOREVER
         {
         callbackDispatch();

         // vTaskDelay to prevent busy waiting
         vTaskDelay(pdMS_TO_TICKS(50));
         }
      }
   #endif

   void BinaryClock::callbackDispatch()
      {
      if (callbackTimeEnabled && CallbackTimeTriggered)
         {
         callbackFtn(CallbackTimeTriggered, get_Time(), timeCallback);
         }

      if (callbackAlarmEnabled && CallbackAlarmTriggered)
         {
         callbackFtn(CallbackAlarmTriggered, get_Alarm().time, alarmCallback);
         }
      }

   void BinaryClock::callbackFtn(volatile bool& triggerFlag, DateTime time, void(*callback)(DateTime))
      {
      if (triggerFlag) // If the flag signals a callback was triggered
         {
         triggerFlag = false;       // Reset the flag first
         if (callback != nullptr)   // If the callback function is set/registered
            {
            callback(time);         // Call the callback function with the given time
            }
         }
      }

   void BinaryClock::purgatoryTask(const char* message)
      {
      // This is where failure comes to die.
      FastLED.clear(true); // Clear the LEDs.

      pinMode(HeartbeatLED, OUTPUT);
      #if SERIAL_OUTPUT
      Serial.begin(115200);
      Serial.println(F("Failure: Unable to continue."));
      if (message != nullptr)
         {
         Serial << F("Message: ") << message << endl << endl;
         }
      Serial.println(F("    CQD - Entering Purgatory..."));
      #endif

      // -.-. --.- -..  -.-. --.- -..  -.-. --.- -..  -.-. --.- -..  
      // Flash the LED to signal the failure.
      // This is just a little fun, flash the failure in Morse code.
      // Never use S.O.S. outside an actual emergency, so use the 
      // alternate: CQD (-.-. --.- -..) it expands to: Come Quick Distress.
      // CQD was used before SOS and is a good alternative for us.
      // So, flash the message: 
      //       CQD NO RTC  -  Come Quick Distress NO Real Time Clock.
      //       -.-. --.- -..  -. ---   .-. - -.-.
      // We can't get out of purgatory without a Real Time Clock
      //
      MorseCodeLED morseCode(HeartbeatLED);
      morseCode.begin();

      #ifndef UNO_R3
      morseCode.flashString("CQD");
      delay(1250);
      morseCode.flashString(message);
      delay(1750);
      #endif

      bool done = false;   // Flag to display the serial message only once.
      // Flash CQD NO RTC (Come Quick Distress NO Real Time Clock) to signal the failure.
      if (!done)
         {
         Serial << "  C    Q    D     N  O     R   T C " << endl
            << " [-.-. --.- -..   -. ---   .-. - -.-.] " << endl
            << "(Come Quick Distress NO Real Time Clock)" << endl;
         }
      if (!done) { Serial << endl; }
      done = true;

      FOREVER
         {
         morseCode.flash_CQD_NO_RTC();        
         delay(1950);

         if (RTC.begin())
            {
            resetBoard(); // Reset the board if RTC is available
            }
         }
      }

   void BinaryClock::displayLedBuffer(const CRGB* ledBuffer, int size)
      {
      if (ledBuffer == nullptr || size < NUM_LEDS) { return; }

      // Copy the LED buffer to the FastLED display array and display
      memmove(leds, ledBuffer, sizeof(CRGB) * size);
      FastLED.show();
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
   ///          either being saved, indicated by the Green  [✅], or the 
   ///          changes have been discarded, indicated by the Pink 'X' [❌] on the shield.

   ////////////////////////////////////////////////////////////////////////////////////
   ///               settingsMenu() - DESIGN                         Chris-70 (2025/08)
   ///               =======================
   /// This is the core of the settings menu. It is called from the main loop at every 
   /// iteration. It checks if the settings menu is active and processes button presses.
   /// It uses a state machine approach to navigate through the menu options and levels.
   /// The menu has two main options: setting the time and setting the alarm.
   /// 
   /// Details:
   /// ========
   /// The nature of this method being called every iteration from loop() means that 
   ///   if something can't be processed immediately, we must be able to return to the
   ///   point where we left off and resume the processing. The original method was
   ///   able to process everything without the need to pause and resume. This version
   ///   expands the UX (User eXperience) as it has added the following new features:
   ///      1) The ability to cancel the settings for Time and Alarm.
   ///      2) Support for 12 (AM/PM) and 24 hour time modes.
   /// The time modes and alarm states are set initially (i.e. Level 1) where the 
   ///   choice to Abort/Cancel is provided as one of the options. If the Abort
   ///   option is selected, the settingsLevel is set to 99 which bypasses the
   ///   normal processing, restores the original values and displays a confirmation
   ///   to the user before exiting the settings menu.
   /// The confirmation screens are designed to provide feedback to the user about the
   ///   changes made and to display the action being taken (Save or Abort). 
   ///   There are two screens that are displayed to the user at the end of the 
   ///   setting menu session:
   ///      1) A Rainbow display is shown to: 
   ///         a) signal to the user we are exiting the settings menu.
   ///         b) signal the beginning of action being taken; 
   ///      2) A display showing the user the action being taken:
   ///         a) Green Check mark is shown if the settings are being saved; or 
   ///         b) Pink 'X' [❌] is shown if the settings are being discarded.
   /// Adding these configuration screens requires that they remain visible for a 
   ///   period of time so that the user can view them. We can't use the 'delay()'
   ///   method as it would block the main loop and prevent other tasks from running. 
   ///   Instead, we must use a non-blocking approach, by using 'millis()' to 
   ///   manage the timing along with static variables, 'delayTimer', 'exit', 'abort',
   ///   'exitStage' and 'continueS2', to handle the screen displays and to be able 
   ///   to resume the processing.
   /// Selecting the Time mode, 12 (AM/PM) or 24 hour mode is done by displaying 
   ///   12 PM or 24 on the Hours (top) Row. This is immediately followed by the user 
   ///   selecting the hours which is poor from a UX perspective. The two types, 
   ///   time mode and hours, appear to be very similar to the user. There needs to 
   ///   be a clear transition between them. The Green check mark [✅] is displayed
   ///   briefly to mark this transition which is handed with 'continueS2' variable.
   /// The 'delayTimer' prevents any action by the user until after it has expired.
   ///   To resume processing the Time settings (i.e. hours) after the Green check 
   ///   mark is dispayed, the 'continueS2' flag is set to true. This allows the
   ///   buttonS2 processing to continue after the delay.
   /// The 'exit', 'exitStage' and 'abort' variables are used when we are done with 
   ///   the 'settingsMenu()' and we will resume displaying the time. This involves 
   ///   displaying two screens: the Rainbow screen; and either the Green check mark [✅]
   ///   or the Pink 'X' [❌] before displaying the time. The 'exitStage' keeps track of
   ///   where we are in the exit process. These two flags, along with the 'delayTimer',
   ///   handle displaying the correct screens for the specified time.
   ///

   void BinaryClock::settingsMenu()
      {
      // Main menu, check the Time or Alarm setting buttons was pressed.
      // This only happens when the time is being displayed, otherwise the
      // S3 and S1 buttons are used to increment/decrement the current selection.
      if ((settingsOption == 0) & (settingsLevel == 0))
         {
         // Time settings
         if (isButtonOnNew(buttonS1))               // Check if Time button, S1, was just pressed.
            {
            tempTime = time;                        // Read time from the current value
            settingsOption = 1;                     // Set time option settings 
            settingsLevel = 1;                      // Set time options level value (12/24)
            tempAmPm = get_Is12HourFormat();           // Get the current 12/24 Hr. mode
            setCurrentModifiedValue();              // Assign options to modify +/- 
            if (isSerialSetup) { serialSettings(); }// Use serial monitor for showing settings
            displayCurrentModifiedValue();          // Display current hour on LEDs
            }

         // Alarm settings
         if (isButtonOnNew(buttonS3))               // Check if Alarm button, S3, was just pressed.
            {
            tempAlarm = get_Alarm();                // Get the default alarm time and status
            settingsOption = 3;                     // Set Alarm time option settings
            settingsLevel = 1;                      // Set hour level settings
            tempAmPm = get_Is12HourFormat();        // Get the current 12/24 Hr. mode
            timeFormat = (tempAmPm ? timeFormat12 : timeFormat24); // Set the time format for displaying the alarm
            setCurrentModifiedValue();              // Assign hours to modify +/-
            if (isSerialSetup) { serialSettings(); }// Use serial monitor for showing settings
            displayCurrentModifiedValue();          // Display current alarm hour on LEDs 
            }
         }

      // Any settings option level except main menu (i.e. Alarm or Time)
      if (settingsLevel != 0)
         {
         unsigned long curMillis = millis(); // Current ms (since startup, max ~28 days).
         static bool exit = false;       // Flag to exit settings (Finished or Abort)
         static bool abort = false;      // Flag to abort settings, don't save.
         static uint8_t exitStage = 0U;  // Stage of exit process (Needed for user info/display)
         static unsigned long delayTimer = 0UL; // Delay timer instead of using delay().
         static bool continueS2 = false; // Flag to resume 'buttonS2' processing after a delay.
         bool displayOK = false;         // Flag to display check mark [✅] after 12/24 hr. mode selection

         // Decrement - if the buttonS1 was just pressed
         if (isButtonOnNew(buttonS1) && (curMillis > delayTimer))
            {
            countButtonPressed--;               // Decrement current value e.g. hour, minute, second, alarm status
            checkCurrentModifiedValueFormat();  // Check if the value has exceeded the range e.g minute = 60 and correct
            displayCurrentModifiedValue();      // Display current modified value on LEDs 
            if (isSerialSetup) { serialCurrentModifiedValue(); }  // Use serial monitor for showing settings
            }

         // Increment - if the buttonS3 was just pressed
         if (isButtonOnNew(buttonS3) && (curMillis > delayTimer)) 
            {
            countButtonPressed++;               // Increment current value e.g. hour, minute, second, alarm status
            checkCurrentModifiedValueFormat();  // Check if the value has exceeded the range e.g minute = 60 and correct
            displayCurrentModifiedValue();      // Display current modified value on LEDs  
            if (isSerialSetup) { serialCurrentModifiedValue(); }  // Use serial monitor for showing settings
            }

         // Save if buttonS2 was just pressed (or continue the processing after a delay)
         if ((curMillis > delayTimer) && (continueS2 || isButtonOnNew(buttonS2)))
            {
            if (!continueS2)                    // If the button S2 was just presse (i.e. not resume S2).
               { saveCurrentModifiedValue(); }  // Save current value e.g. hour, minute, second, alarm status or Cancel    

            // During time setting, display a check mark [✅] after 12/24 hour mode selected to 
            // indicate to the user that level was done. Now on to setting Hours. 
            // This is due to using the Hours Row to display the 12/24 hour mode selection. 
            // Abort/Cancel changes the settings level to 99 so this is bypassed, it will process the abort instead.
            if (settingsOption == 1 && settingsLevel == 1)
               { displayOK = true; }            // Flag to display the Green check mark [✅]

            // This sequence position is important in this state machine, the level needs to be incremented
            // after the value was saved and after we checked if we need to display a check mark [✅] once the user
            // has selected to save the selected time mode. This must be done as part of the processing of the
            // physical save button press never during a resume of the S2 processing.
            if (!continueS2)                    // If the button S2 was just pressed (i.e. not resume S2).
               { settingsLevel++; }             // Go to next settings level: abort or => hour => minute => second

            // Check if we are done with the Alarm settings (Alarm levels are 1 - 3, 100 == Abort)
            if ((settingsOption == 3) && (settingsLevel > 3)) 
               {
               exit = true;
               // Save the new alarm if the user didn't abort, abort is level 100.
               if (settingsLevel < 10) 
                  { 
                  Alarm2 = tempAlarm;
                  set_Alarm(Alarm2);            // Save the alarm to the RTC.
                  }
               else { abort = true; }           // Level was too large (e.g. 100) signaling an abort.

               #if SERIAL_SETUP_CODE
               if (isSerialSetup)
                  {
                  Serial << endl;               // New line for the setting displayed previously.
                  }
               #endif

               serialAlarmInfo();               // Show the time and alarm status info when you exit to the main menu
               }
            // Check if we are done with the Time settings (Time levels are 1 - 4, 100 == Abort)
            else if ((settingsOption == 1) && (settingsLevel > 4)) 
               {
               exit = true;
               // Save the new time to the RTC if the user didn't abort.
               if (settingsLevel < 10) 
                  { 
                  // Order is important here, we need to update the time format first.
                  // When setting the time on the RTC, we save it in the selected format.
                  set_Is12HourFormat(tempAmPm);
                  set_Time(tempTime);
                  time = ReadTime();
                  }
               else
                  { 
                  abort = true;                 // User selected abort.
                  time = ReadTime();            // Update the current time.
                  timeFormat = get_Is12HourFormat() ? timeFormat12 : timeFormat24;
                  }

               #if SERIAL_SETUP_CODE
               if (isSerialSetup)
                  {
                  Serial << F("\n-------------------------------------") << endl;
                  Serial << F("---- Current Time: ");
                  Serial << (DateTimeToString(time, buffer, sizeof(buffer), timeFormat)) << endl;
                  Serial << F("-------------------------------------") << endl;
                  }
               #endif
               }
            // This interrupts the buttonS2 processing to display a check mark [✅] after
            // the user selects 12 or 24 hour time display mode to signal the
            // transition to setting the time. Required for a better UX as the 
            // time mode selection (12/24) is too similar to the hour setting.
            else if (displayOK)
               {
               displayLedBuffer(OkText, NUM_LEDS);
               delayTimer = curMillis + 500;    // Delay further updates for ~0.5 seconds.
               displayOK = false;               // Clear the flag
               continueS2 = true;               // Set the flag to resume buttonS2 processing after the delay.
               }
            else                                // Continue processing, more settings level(s) remain.
               { 
               continueS2 = false;              // Clear the flag, we have arrived to finish S2. 
               setCurrentModifiedValue();       // Set the value for the next level.
               displayCurrentModifiedValue();   // Display next level value on the LEDs                
               if (isSerialSetup) 
                   { serialSettings(); }        // Use serial monitor for showing settings
               }
            } // END S2

         // This is the exit process that occurs over several stages.
         // Stage 0: Display the rainbow LEDs to signal the end.
         // Stage 1: Display the Pink 'X' [❌] for an abort or the check mark [✅] for a save.
         // Stage 2: Clean up and exit back to the main menu (displaying the time).
         if (exit)
            {
            unsigned long curTimer = millis();
            if (exitStage == 0)
               {
               displayLedBuffer(Rainbow, NUM_LEDS);
               delayTimer = curTimer + 750UL;
               exitStage++;
               }
            
            if ((exitStage == 1U) && (curTimer > delayTimer))
               {
               if (abort)
                  { 
                  displayLedBuffer(XAbort, NUM_LEDS);  // Show Abort message on LEDs
                  #if SERIAL_SETUP_CODE
                  if (get_isSerialSetup())
                     { Serial << F("Aborted; Nothing Saved.") << endl; }
                  #endif
                  }
               else
                  { 
                  displayLedBuffer(OkText, NUM_LEDS);  // Show OK message on LEDs
                  #if SERIAL_SETUP_CODE
                  if (get_isSerialSetup())
                     { Serial << F("Changes Were Saved.") << endl; }
                  #endif
                  }

               delayTimer = curTimer + 1250U;   
               exitStage++;   
               }

            if ((exitStage == 2) && (curTimer > delayTimer))
               {
               settingsLevel = 0;                  // Escape to main menu (Displaying the time)
               settingsOption = 0;                 // Escape to main menu
               exit = false;                       // Reset the flags for the next time.
               abort = false;
               exitStage = 0U;                     // Reset thestabge io its initial value.
               }
            }  // END if (exit)
         } // END SettingsLevel > 0
      } // END settingsMenu()

   ////////////////////////////////////////////////////////////////////////////////////
   // Depending on the options and settings level, assign to the 
   // countButtonPressed variable to be able modify the value

   void BinaryClock::setCurrentModifiedValue()
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

   BinaryClock::SettingsType BinaryClock::GetSettingsType(int options, int level)
      {
      SettingsType type = SettingsType::Undefined;

      if (options == 1) // Time 
         {
         if (level == 1)  { type = SettingsType::TimeOptions; }
         if (level == 2)  { type = SettingsType::Hours; }
         if (level == 3)  { type = SettingsType::Minutes; }
         if (level == 4)  { type = SettingsType::Seconds; }
         }
      else if (options == 3) // Alarm
         {
         if (level == 1)  { type = SettingsType::AlarmStatus; }
         if (level == 2)  { type = SettingsType::Hours; }
         if (level == 3)  { type = SettingsType::Minutes; }
         }

      return type;
      }

   void BinaryClock::checkCurrentModifiedValueFormat()
      {
      enum { OffTxt, OnText, XAbort, OkText }; // Text for the LEDs

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
            assert(false && "BinaryClock::checkCurrentModifiedValueFormat() - Undefined settings type "  && type);
            break;
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Depending on the options and settings level, save the current modified value

   void BinaryClock::saveCurrentModifiedValue()
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
               { timeFormat = (tempAmPm ? timeFormat12 : timeFormat24); }
            }
         if (settingsLevel == 2) { tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), countButtonPressed, tempTime.minute(),  tempTime.second());  }
         if (settingsLevel == 3) { tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), tempTime.hour(),    countButtonPressed, tempTime.second());  }
         if (settingsLevel == 4) { tempTime = DateTime(tempTime.year(), tempTime.month(), tempTime.day(), tempTime.hour(),    tempTime.minute(),  countButtonPressed); }
         }

      // Alarm time and alarm status
      if (settingsOption == 3)
         {
         if (settingsLevel == 1)
            {
            if (countButtonPressed == 3) // Abort/Cancel
               {
               tempAlarm = Alarm2;       // Restore the original values
               settingsLevel = 99;       // Signal user abort/cancel selected
               }   
            else
               {
               tempAlarm.status = countButtonPressed - 1;
               }
            }
         if (settingsLevel == 2) { tempAlarm.time = DateTime(tempAlarm.time.year(), tempAlarm.time.month(), tempAlarm.time.day(), countButtonPressed,    tempAlarm.time.minute(), tempAlarm.time.second()); }
         if (settingsLevel == 3) { tempAlarm.time = DateTime(tempAlarm.time.year(), tempAlarm.time.month(), tempAlarm.time.day(), tempAlarm.time.hour(), countButtonPressed,      tempAlarm.time.second()); }
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Display on LEDs only currently modified value
   // convertDecToBinaryAndDisplay(hours row, minutes row, seconds row)

   void BinaryClock::displayCurrentModifiedValue()
      {
      SettingsType type = GetSettingsType(settingsOption, settingsLevel);
      switch(type)
         {
         case SettingsType::Hours:
            convertDecToBinaryAndDisplay(countButtonPressed, 0, 0, tempAmPm);
            break;
         case SettingsType::Minutes:
            convertDecToBinaryAndDisplay(0, countButtonPressed, 0);
            break;
         case SettingsType::Seconds:
            convertDecToBinaryAndDisplay(0, 0, countButtonPressed);
            break;
         case SettingsType::TimeOptions:
            switch (countButtonPressed)
               {
               case 1:
                  convertDecToBinaryAndDisplay(24, 0, 0, false);
                  break;
               case 2:
                  convertDecToBinaryAndDisplay(12, 0, 0, true);
                  break;
               case 3:
                  displayLedBuffer(XAbort, NUM_LEDS);
                  break;
               }
            break;
         case SettingsType::AlarmStatus:
            switch (countButtonPressed)
               {
               case 1:
                  displayLedBuffer(OffTxt, NUM_LEDS);
                  break;
               case 2:
                  displayLedBuffer(OnText, NUM_LEDS);
                  break;
               case 3:
                  displayLedBuffer(XAbort, NUM_LEDS);
                  break;
               }
            break;
         case SettingsType::Undefined:
         default:
            // This is a Software error. Alert the developer (Debug mode only)
            assert(false && "BinaryClock::displayCurrentModifiedValue() - Undefined settings type "  && type);
            break;
         }
      }

   //################################################################################//
   // RTC TIME & BINARY FORMAT
   //################################################################################//

   ////////////////////////////////////////////////////////////////////////////////////
   // Write time to RTC

   void BinaryClock::setNewTime() { set_Time(time); }

   ////////////////////////////////////////////////////////////////////////////////////
   // Convert values from DEC to BIN format and display

   void BinaryClock::convertDecToBinaryAndDisplay(int hoursRow, int minutesRow, int secondsRow, bool use12HourMode)
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
   int displayHour = hoursRow;
   if (use12HourMode)
      {
      displayHour = hoursRow % 12;
      if (displayHour == 0) { displayHour = 12; } // Display 12 instead of 0.
      leds[16] = (hoursRow >= 12) ? PmColor : AmColor; // LED 16 - AM/PM Indicator
      }
   else
      {
      SET_LEDS(16, displayHour, 0b00010000, OnColor[16], OffColor[16]); // LED 16 - bit[4] for hour 16 (msb)
      }
   SET_LEDS(15, displayHour, 0b00001000, OnColor[15], OffColor[15]); // LED 15 - bit[3] for hour  8      
   SET_LEDS(14, displayHour, 0b00000100, OnColor[14], OffColor[14]); // LED 14 - bit[2] for hour  4      
   SET_LEDS(13, displayHour, 0b00000010, OnColor[13], OffColor[13]); // LED 13 - bit[1] for hour  2      
   SET_LEDS(12, displayHour, 0b00000001, OnColor[12], OffColor[12]); // LED 12 - bit[0] for hour  1 (lsb)

   // M - middle row LEDs where the minute is displayed, left most bit, the MSB, is led 12, LSB is LED 6
   SET_LEDS(11, minutesRow,  0b00100000, OnColor[11], OffColor[11]); // LED 11 - bit[5] for minute 32 (msb)
   SET_LEDS(10, minutesRow,  0b00010000, OnColor[10], OffColor[10]); // LED 10 - bit[4] for minute 16      
   SET_LEDS( 9, minutesRow,  0b00001000, OnColor[ 9], OffColor[ 9]); // LED  9 - bit[3] for minute  8      
   SET_LEDS( 8, minutesRow,  0b00000100, OnColor[ 8], OffColor[ 8]); // LED  8 - bit[2] for minute  4      
   SET_LEDS( 7, minutesRow,  0b00000010, OnColor[ 7], OffColor[ 7]); // LED  7 - bit[1] for minute  2      
   SET_LEDS( 6, minutesRow,  0b00000001, OnColor[ 6], OffColor[ 6]); // LED  6 - bit[0] for minute  1 (lsb)

   // S - bottom row LEDs where the second is displayed, left most bit, the MSB, is led 5, LSB is LED 0
   SET_LEDS( 5, secondsRow,  0b00100000, OnColor[ 5], OffColor[ 5]); // LED  5 - bit[5] for second 32 (msb)
   SET_LEDS( 4, secondsRow,  0b00010000, OnColor[ 4], OffColor[ 4]); // LED  4 - bit[4] for second 16
   SET_LEDS( 3, secondsRow,  0b00001000, OnColor[ 3], OffColor[ 3]); // LED  3 - bit[3] for second  8
   SET_LEDS( 2, secondsRow,  0b00000100, OnColor[ 2], OffColor[ 2]); // LED  2 - bit[2] for second  4
   SET_LEDS( 1, secondsRow,  0b00000010, OnColor[ 1], OffColor[ 1]); // LED  1 - bit[1] for second  2
   SET_LEDS( 0, secondsRow,  0b00000001, OnColor[ 0], OffColor[ 0]); // LED  0 - bit[0] for second  1 (lsb)

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

   DateTime BinaryClock::ReadTime() 
      {
      return RTC.now();
      }

   //################################################################################//
   // CHECK BUTTONS
   //################################################################################//

   bool BinaryClock::isButtonOnNew(ButtonState &button)
      {
      bool result = false;
      // Read the current physical state of the push button into a local variable:
      int currentread = button.value();
      unsigned long currentReadTime = millis();

      // For the first read of the button (i.e. lastReadTime == 0) we will treat this
      // as a new button state and force the code to proceed as if this was a state change.
      if ((button.lastReadTime == 0) && (currentread == button.onValue))
         {
         button.state = !currentread;
         button.lastRead = currentread;
         }

      // Check to see if you just pressed the button
      // (e.g. the input went from OFF to ON)
      // Reset the debounce time to eliminate noise.
      if (currentread != button.lastRead)
         {
         // Reset the debouncing timer
         button.lastDebounceTime = currentReadTime;
         }

      // Whatever the reading is at, and it's been there for longer than the debounce
      // delay, take the value as the actual current state:
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
      // prevent multiple displays within ~1 sec or so less any overhead.
      // 950, large enough to work, small enough to meet the 1 Hz interrupt.
      if (curCall - lastCall > 950) 
         {
         lastCall = curCall;
         Serial << F("Time: ") <<  get_Time().toString(buffer, sizeof(buffer), timeFormat) << F("  Binary: ");

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
      Serial << (DateTimeToString(RTC.now(), buffer, sizeof(buffer), timeFormat)) << endl;

      serialAlarmInfo();

      Serial << F("#####################################"); // << endl;
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
         Serial << (DateTimeToString(tempTime, buffer, sizeof(buffer), timeFormat)) << endl;
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
      
      auto printSettingsControls = []() 
         {
         Serial << F("S1 - Decrement ----------------------") << endl;
         Serial << F("S2 - Save Current Settings Level ----") << endl;
         Serial << F("S3 - Increment ----------------------") << endl;
         Serial << F("-------------------------------------") << endl;
         };

      char hourStr[6] = { 0 };
      BinaryClock::SettingsType type = GetSettingsType(settingsOption, settingsLevel);
      switch(type)
         {
         case SettingsType::Hours:
            Serial << F("--------------- Hour ----------------") << endl;
            printSettingsControls();
            Serial << F("Current Hour: ") << formatHour(countButtonPressed, tempAmPm, 
                  hourStr, sizeof(hourStr)) << (" ");
            break;
         case SettingsType::Minutes:
            Serial << F("-------------- Minute ---------------") << endl;
            printSettingsControls();
            Serial << F("Current Minute: ") << countButtonPressed << (" ");
            break;
         case SettingsType::Seconds:
            Serial << F("-------------- Second ---------------") << endl;
            printSettingsControls();
            Serial << F("Current Second: ") << countButtonPressed << (" ");
            break;
         case SettingsType::AlarmStatus:
            Serial << F("----------- ON/OFF/CANCEL -----------") << endl;
            printSettingsControls();
            Serial << F("Alarm Status: ");
            Serial << (countButtonPressed == 2 ? "ON" : "");
            Serial << (countButtonPressed == 1 ? "OFF" : "");
            Serial << (countButtonPressed == 3 ? "Cancel" : "");
            Serial << (" ");
            break;
         case SettingsType::TimeOptions:
            Serial << F("------- 12 Hr / 24 Hr / Cancel ------") << endl;
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
            assert(false && "BinaryClock::displayCurrentModifiedValue() - Undefined settings type "  && type);
            break;
            }
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show the set alarm time and current alarm status

   void BinaryClock::serialAlarmInfo()
      {
      Serial << F("-------------------------------------") << endl;
      Serial << F("------ Alarm Time: ");
      Serial << (DateTimeToString(Alarm2.time, buffer, sizeof(buffer), alarmFormat)) << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << F("---- Alarm Status: ");
      Serial << (Alarm2.status == 1 ? "ON" : "OFF") << endl;
      Serial << F("-------------------------------------") << endl;
      Serial << endl;
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Show current alarm status during settings

   void BinaryClock::serialCurrentModifiedValue()
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
         char buffer[6] = { 0 } ;
         Serial << formatHour(countButtonPressed, tempAmPm, buffer, sizeof(buffer));
         }
      else
         {
         Serial << countButtonPressed;
         }

      Serial << (" ");
      }

   char* BinaryClock::formatHour(int hour24, bool is12HourFormat, char* buffer, size_t size)
      {
      if ((buffer == nullptr) || (size < 6)) { return nullptr; }

      int hour = hour24;
      bool isPM = false;
      int i = 0;

      if (is12HourFormat)
         {
         if (hour == 0)
            { // midnight
            isPM = false;
            hour = 12;
            }
         else if (hour == 12)
            { // noon
            isPM = true;
            hour = 12;
            }
         else if (hour < 12)
            { // morning
            isPM = false;
            }
         else
            { // 1 o'clock or after
            isPM = true;
            hour -= 12;
            }
         }

      if (hour < 10) 
         { 
         if (!is12HourFormat) { buffer[i++] = '0'; }
         buffer[i++] = '0' + hour; 
         }
      else if (hour < 24) 
         { 
         buffer[i++] = '0' + (hour / 10); 
         buffer[i++] = '0' + (hour % 10); 
         }
      else 
         { 
         buffer[i++] = '9'; 
         buffer[i++] = '9'; 
         buffer[i++] = 'E';
         buffer[i++] = 'r';
         buffer[i++] = 'r';
         is12HourFormat = false;
         } 

      if (is12HourFormat)
         {
         // buffer[i++] = ' ';
         buffer[i++] = (isPM ? 'p' : 'a');
         buffer[i++] = 'm';
         }

      buffer[i] = '\0';
      return buffer;
      }
   #endif // SERIAL_SETUP_CODE
    
   ////////////////////////////////////////////////////////////////////////////////////
   #if HARDWARE_DEBUG
   void BinaryClock::checkHardwareDebugPin()
      {
      #if HW_DEBUG_SETUP
      static bool isLocalSetupOn = DEFAULT_SERIAL_SETUP;

      if (isButtonOnNew(buttonDebugSetup)) 
         {
         set_isSerialSetup(!isLocalSetupOn);
         isLocalSetupOn = !isLocalSetupOn;         
         Serial << F("Serial Menu is: ") << (get_isSerialSetup() ? "ON" : "OFF") << endl;
         }
      #endif

      #if HW_DEBUG_TIME
      // For the first reading, setup values so isButtonOnNew() returns false;
      // Design: Initial priority to S/W configuration values so we set the
      // state and lastSate to reflect the current PIN value forcing the user to
      // change the state of the switch before we have H/W control.
      int curTimeButton = digitalRead(buttonDebugTime.pin);
      if ((buttonDebugTime.lastReadTime == 0) && (buttonDebugTime.isPressed()))
         {
         buttonDebugTime.lastReadTime = millis();
         buttonDebugTime.lastRead = buttonDebugTime.state = curTimeButton;
         }

      // Check if the hardware debug pin is set, if not turn off the serial time output 
      // after 'debugDelay' msec has passed. If the initial H/W value is ON but the S/W
      // value is OFF, it will remain OFF until the H/W cycles OFF then ON.
      if (isButtonOnNew(buttonDebugTime)) 
         {
         isSerialTime = true; // Set the serial time flag to true
         Serial << F(" Serial Time is: ON") << endl; // Debug: 
         }
      else if ((buttonDebugTime.lastReadTime > 0) && (isSerialTime && !buttonDebugTime.isPressed()) && ((millis() - buttonDebugTime.lastReadTime) > get_DebugOffDelay()))
         {
         isSerialTime = false; // Reset the serial time flag
         Serial << F(" Serial Time is: OFF") << endl;
         }
      #endif
      }
   #endif
   }  // END OF NAMESPACE BinaryClockShield && BINARY CLOCK CLASS code
   //################################################################################//
   