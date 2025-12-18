/**************************************************************************/
/*!
  @file     BinaryClock.cpp

  @mainpage BinaryClock

  @section intro Introduction

   This library provides a set of functions and classes for working with the wonderful 
   [Binary Clock Shield for Arduino UNO](https://nixietester.com/products/binary-clock-shield-for-arduino/).  
   The shield was created by Marcin Saj at https://nixietester.com   
   It supports setting and reading the time, as well as configuring alarms and handling button inputs.  
   This was created as a library to isolate the functionality of the Binary Clock Shield and 
   make it easier to use in other projects and other frameworks.

   This library supports multiple UNO format boards:  
   1. **UNO\_R3** -  The original [Arduino UNO R3](https://store.arduino.cc/collections/uno/products/arduino-uno-rev3) board.
   2. **UNO\_R4\_MINIMA** - The [R4 Minima](https://store.arduino.cc/collections/uno/products/uno-r4-minima) board without WiFi.
   3. **UNO\_R4\_WIFI** - The new [Arduino UNO R4 WiFi](https://store.arduino.cc/collections/uno/products/uno-r4-wifi) board.
   4. **METRO\_ESP32\_S3** - The great [Adafruit Metro ESP32-S3](https://www.adafruit.com/product/5500) board.
   5. **ESP32\_S3\_UNO** - The generic UNO clone board with the new ESP32-S3 module.
   6. **ESP32\_D1\_R32\_UNO** - The generic Wemos D1 R32 UNO clone board with the original ESP32-32-WROOM module.
   7. **CUSTOM\_UNO** - An UNO board you define and enable in
   [`board_select.h`](https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/src/board_select.h).

   The goal of using an ESP32 based UNO board was to allow the RTC to be connected to a NTP server over WiFi.
   The original modifications to the `Example-11` file was to use a generic `Wemos-D1-R32 UNO, ESP32` board
   instead of the UNO R3. This was a challange due to some hardware limitations, see the
   [README.md](https://github.com/Chris-70/WiFiBinaryClock/blob/main/README.md#hardware-modifications) file.
   The code for the WiFi connection is encapsulated in its own class, `BinaryClockWAN`, which is not included in this file.
   It uses WPS to connect to a WiFi network and stores the credentials in the ESP32's flash memory.

  @section classes Available classes

  This library provides the following classes:

  - Interface class:
    - [**IBinaryClockBase**](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/IBinaryClockBase.h):
                        A pure interface class for the Binary Clock. This is used to define the methods and properties
                        that the BinaryClock class must implement and that other classes can count on to be available.
  - Main class:
    - [**BinaryClock**](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BinaryClock.h):
                        The main class, implements the IBinaryClockBase interface, handles all aspects of the
                        Binary Clock Shield, from display to settings and callbacks.
  - Helper classes:
    - [**BCButton**](https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/src/BCButton.h):
                        Class to encapsulate the buttons for debounce, state and wiring (CC vs CA).
    - [**BCMenu**](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BCMenu.h):
                        Class to encapsulate the settings for the Binary Clock, including time format and alarm settings.

   Custom library dependencies:
    - [**RTClibPlus**](https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/RTClibPlus) A modified fork of
                      [Adafruit's RTClib](https://github.com/adafruit/RTClib) to expand the functionality and support 12 hour mode.
    - [**BinaryClockWiFi**](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi)
                           WiFi connections, WPS, credentials with settings storage, and time syncing with an NTP server.
    - [**MorseCodeLED**](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/MorseCodeLED):
                        Display error messages over the _LED_BUILTIN_. A fun communication alternative when there is no screen.
                        
  External library dependencies:
    - [**FastLED**](https://github.com/FastLED/FastLED):                - The FastLED library for color LED animation on Arduino.
    - [**Streaming**](https://github.com/jcw/streaming):                - The Streaming library for Arduino.
    - [**Adafruit BusIO**](https://github.com/adafruit/Adafruit_BusIO): - The Adafruit BusIO library for I2C/SPI communication.

  @section license License

  This **`WiFiBinaryClock`** software, Copyright (c) 2025 Chris-70 and Chris-80, is licensed under the GNU General Public License v3.0 (**`GPL-v3.0`**).
  You may obtain a copy of the License at: **`https://www.gnu.org/licenses/gpl-3.0.en.html`** (see [**LICENSE**](https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/src/LICENSE) file).

  Original example by Marcin Saj [Binary-Clock_Shield_Example-11](https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino/blob/master/example/11-BinaryClockRTCInterruptAlarmButtons/11-BinaryClock-24H-RTCInterruptAlarmButtons.ino), (c) 2018
  also released under the GPL-v3.0 license.
*/
/**************************************************************************/  
/// @section Original_Docs Original Documentation: Example 11 - BinaryClockRTCInterruptAlarmButtons  
/// https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino/tree/master/example/11-BinaryClockRTCInterruptAlarmButtons
/// @details
/// @verbatim
/// Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com
/// https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
///
/// Binary Clock RTC 24H with Interrupt, Alarm and Buttons Example
/// This example demonstrates complete Binary Clock with Time and Alarm settings
///
/// *It is recommended that the first start should be carried out with the serial terminal, 
/// for better knowing the setting options. 
///
/// The buttons allows you to set the time and alarm - exact hour, minute, second/alarm status.
/// Alarm causes melody to play.  
/// How to use piezo with the tone() command to generate notes you can find here:
/// http://www.arduino.cc/en/Tutorial/Tone
///
/// A falling edge at the RTC INT/SQW output causes an interrupt, 
/// which is uses for regular - 1 per second - reading time from RTC and 
/// checking alarm status flag 'A2F'. Since we use RTC INT/SQW output for
/// regular reading current time - square wave output SQW option, 
/// global interrupt flag INTCN is set to 0, this disables the interrupts from both RTC alarms.
/// Referring to the documentation: when the INTCN is set to logic 0, 
/// the 'A2F' bit does not initiate an interrupt signal. By turning off the interrupts from the alarms, 
/// we can use the interrupt flag 'A2IE' as an info flag whether the alarm has been activated or not. 
/// Check RTC datasheet page 11-13 http://bit.ly/DS3231-RTC
///
/// Hardware:
/// ---------
/// - Binary Clock Shield for Arduino (https://nixietester.com/products/binary-clock-shield-for-arduino/)
/// - Arduino Uno style board;        (https://www.adafruit.com/product/5500)
/// - Battery CR1216/CR1220 
/// 
/// Pinouts for the shield:
/// -----------------------
/// INT/SQW   connected to: Arduino pin  3 INT1 ; METRO-S3  3 ; ESP32_D1-R32 UNO 25 ; ESP32-S3_UNO 17
/// PIEZO     connected to: Arduino pin 11  PWM ; METRO-S3 11 ; ESP32_D1-R32 UNO 23 ; ESP32-S3_UNO 11
/// S3 button connected to: Arduino pin A0      ; METRO-S3 A0 ; ESP32_D1-R32 UNO  2 ; ESP32-S3_UNO  2
/// S2 button connected to: Arduino pin A1      ; METRO-S3 A1 ; ESP32_D1-R32 UNO  4 ; ESP32-S3_UNO  1
/// S1 button connected to: Arduino pin A2      ; METRO-S3 A2 ; ESP32_D1-R32 UNO 35 ; ESP32-S3_UNO  7
/// LEDs      connected to: Arduino pin A3      ; METRO-S3 A3 ; ESP32_D1-R32 UNO 15 ; ESP32-S3_UNO  6
/// RTC SDA   connected to: Arduino pin 18      ; METRO-S3 18 ; ESP32_D1-R32 UNO 36 ; ESP32-S3_UNO  8
/// RTC SCL   connected to: Arduino pin 19      ; METRO-S3 19 ; ESP32_D1-R32 UNO 39 ; ESP32-S3_UNO  9
///
///                        +------+       +------+       +------+       +------+       +------+
///                        |LED 16|---<---|LED 15|---<---|LED 14|---<---|LED 13|---<---|LED 12|--<-+
///                        +------+       +------+       +------+       +------+       +------+    |
///                                                                                                |
///    +--------------->-------------->-------------->-------------->-------------->---------------+
///    |
///    |    +------+       +------+       +------+       +------+       +------+       +------+
///    +----|LED 11|---<---|LED 10|---<---|LED 09|---<---|LED 08|---<---|LED 07|---<---|LED 06|--<-+
///         +------+       +------+       +------+       +------+       +------+       +------+    |
///                                                                                                |
///    +--------------->-------------->-------------->-------------->-------------->---------------+
///    |
///    |    +------+       +------+       +------+       +------+       +------+       +------+
///    +----|LED 05|---<---|LED 04|---<---|LED 03|---<---|LED 02|---<---|LED 01|---<---|LED 0 |--<-- DATA_PIN 
///         +------+       +------+       +------+       +------+       +------+       +------+
///
/// @endverbatim

// Defines to control the assert macros.
#ifdef DEBUG
   #define __ASSERT_USE_STDERR
#else
   #define NDEBUG
#endif

#if STL_USED
   #include <vector>             // For std::vector<>
   #include <exception>          // For std::exception
   #include <functional>         // For std::function<>
#endif 

#include <Arduino.h>

#include "BinaryClock.h"         // Header file for this library 
#include <MorseCodeLED.h>        // Used in PurgatoryTask() to flash error (Total delta bytes: 0 RAM; 220 ROM on UNO R3)
#include "pitches.h"             // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody

// Include libraries
#include <FastLED.h>             // https://github.com/FastLED/FastLED
#include <RTClib.h>              // Adafruit RTC library: https://github.com/adafruit/RTClib
#include <Streaming.h>           /// Streaming serial output with `operator<<` https://github.com/janelia-arduino/Streaming

#if FREE_RTOS
   #define configUSE_TASK_NOTIFICATIONS            1
   #define configTASK_NOTIFICATION_ARRAY_ENTRIES   1

   #include <TaskWrapper.h>         // Helper template methods to launch an instance method 1 time then exit task.
#endif // FREE_RTOS

#include <assert.h>                 // Catch code logic errors during development.

namespace BinaryClockShield
   {
   #define NOTE_MS(N) (1000 / N) ///< Convert note duration (1/N fractions, of a second) to milliseconds

   /// @brief Combined melody and duration notes for the alarm sound.
   /// @remarks See the links for details on creating your own melody using tone():
   /// @par  (http://www.arduino.cc/en/Tutorial/Tone)
   /// @par  (https://arduino.cc/en/Tutorial/ToneMelody)
   const Note BinaryClock::AlarmNotes[] PROGMEM =
         {
         {NOTE_A4,  NOTE_MS(2)}, {NOTE_A4,  NOTE_MS(2)}, {NOTE_A4,  NOTE_MS(2)}, {NOTE_F4,  NOTE_MS(3)},
         {NOTE_C5,  NOTE_MS(6)}, {NOTE_A4,  NOTE_MS(2)}, {NOTE_F4,  NOTE_MS(3)}, {NOTE_C5,  NOTE_MS(6)},
         {NOTE_A4,  NOTE_MS(1)}, {NOTE_E5,  NOTE_MS(2)}, {NOTE_E5,  NOTE_MS(2)}, {NOTE_E5,  NOTE_MS(2)},
         {NOTE_F5,  NOTE_MS(3)}, {NOTE_C5,  NOTE_MS(6)}, {NOTE_GS4, NOTE_MS(2)}, {NOTE_F4,  NOTE_MS(3)},
         {NOTE_C5,  NOTE_MS(6)}, {NOTE_A4,  NOTE_MS(1)}, {NOTE_A5,  NOTE_MS(2)}, {NOTE_A4,  NOTE_MS(3)},
         {NOTE_A4,  NOTE_MS(6)}, {NOTE_A5,  NOTE_MS(2)}, {NOTE_GS5, NOTE_MS(4)}, {NOTE_G5,  NOTE_MS(4)},
         {NOTE_FS5, NOTE_MS(8)}, {NOTE_F5,  NOTE_MS(8)}, {NOTE_FS5, NOTE_MS(4)}, {0,        NOTE_MS(3)},
         {NOTE_AS4, NOTE_MS(4)}, {NOTE_DS5, NOTE_MS(2)}, {NOTE_D5,  NOTE_MS(4)}, {NOTE_CS5, NOTE_MS(4)},
         {NOTE_C5,  NOTE_MS(8)}, {NOTE_B4,  NOTE_MS(8)}, {NOTE_C5,  NOTE_MS(4)}, {0,        NOTE_MS(3)},
         {NOTE_F4,  NOTE_MS(6)}, {NOTE_GS4, NOTE_MS(2)}, {NOTE_F4,  NOTE_MS(3)}, {NOTE_A4,  NOTE_MS(6)},
         {NOTE_C5,  NOTE_MS(2)}, {NOTE_A4,  NOTE_MS(3)}, {NOTE_C5,  NOTE_MS(6)}, {NOTE_E5,  NOTE_MS(1)},
         {NOTE_A5,  NOTE_MS(2)}, {NOTE_A4,  NOTE_MS(3)}, {NOTE_A4,  NOTE_MS(8)}, {NOTE_A5,  NOTE_MS(2)},
         {NOTE_GS5, NOTE_MS(4)}, {NOTE_G5,  NOTE_MS(4)}, {NOTE_FS5, NOTE_MS(8)}, {NOTE_F5,  NOTE_MS(8)},
         {NOTE_FS5, NOTE_MS(4)}, {0,        NOTE_MS(4)}, {NOTE_AS4, NOTE_MS(4)}, {NOTE_DS5, NOTE_MS(2)},
         {NOTE_D5,  NOTE_MS(4)}, {NOTE_CS5, NOTE_MS(4)}, {NOTE_C5,  NOTE_MS(8)}, {NOTE_B4,  NOTE_MS(8)},
         {NOTE_C5,  NOTE_MS(4)}, {0,        NOTE_MS(4)}, {NOTE_F4,  NOTE_MS(4)}, {NOTE_GS4, NOTE_MS(2)},
         {NOTE_F4,  NOTE_MS(3)}, {NOTE_C5,  NOTE_MS(8)}, {NOTE_A4,  NOTE_MS(2)}, {NOTE_F4,  NOTE_MS(3)},
         {NOTE_C5,  NOTE_MS(8)}, {NOTE_A4,  NOTE_MS(1)}
         };

   #undef NOTE_MS // NOTE_MS() MACRO is no longer needed.

   // Calculate the number of elements in the array
   const size_t BinaryClock::AlarmNotesSize = sizeof(BinaryClock::AlarmNotes) / sizeof(BinaryClock::AlarmNotes[0]);
   
   // Helper function templates to create an `fl::array` from PROGMEM C-style array
   // Used to convert PROGMEM array of: `onColors`; `offColors`; `onHourAm_P`; `onHourPm_P`
   template<size_t N>
   fl::array<CRGB, N> progmem2array(const CRGB* progmem_source)
      {
      fl::array<CRGB, N> arr;
      for (size_t i = 0; i < N; i++)
         {
         arr[i].r = pgm_read_byte(&progmem_source[i].r);
         arr[i].g = pgm_read_byte(&progmem_source[i].g);
         arr[i].b = pgm_read_byte(&progmem_source[i].b);
         }
      return arr;
      }

   // General helper function for const CRGB arrays - eliminates duplicate lambda code
   // This is the `const` version of `progmem2array()` template.
   // Initially used for: `onText`; `offTxt`; `xAbort`[❌]; `okText`[✅]; `rainbow`; `wText`[W]; 
   // As a RAM saving measure, `DisplayLedPattern()` now uses the PROGMEM array directly.
   template<size_t N>
   const fl::array<CRGB, N> progmem2constArray(const CRGB* progmem_source)
      { return progmem2array<N>(progmem_source); }
   
   CRGB BinaryClock::PmColor = CRGB::Indigo;       ///< Color for the PM indicator LED (e.g. Indigo).
   CRGB BinaryClock::AmColor = CRGB::DeepSkyBlue;  ///< Color for the AM indicator LED (e.g. DeepSkyBlue).

   const CRGB BinaryClock::ledPatterns_P[static_cast<uint8_t>(LedPattern::endTAG)][NUM_LEDS] PROGMEM = 
         {
         // `LedPattern::onColors':
         // `OnColor` pattern (index 0): Colors for the LEDs when ON, Seconds, Minutes and Hours
         { CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,    // Seconds (0 - 5)  
           CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green,  // Minutes (6 - 11) 
           CRGB::Blue,  CRGB::Blue,  CRGB::Blue,  CRGB::Blue,  CRGB::Blue },              // Hours   (12 - 16)

         // `LedPattern::offColors`:
         // `OffColor` pattern (index 1): Colors for the LEDs when OFF (Usually Black i.e. No Power.)
         { CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Seconds (0 - 5)
           CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,  // Minutes (6 - 11)
           CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black },             // Hours   (12 - 16)

         // `LedPattern::onText`:
         // `OnText` pattern (index 2): A big Green 'O' for On
         { CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Black, CRGB::Black,
           CRGB::Green, CRGB::Black, CRGB::Black, CRGB::Green, CRGB::Black, CRGB::Black,
           CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Black },

         // `LedPattern::offTxt`:
         // `OffTxt` pattern (index 3): A big Red sideways 'F' for oFF
         { CRGB::Red,   CRGB::Black, CRGB::Red,   CRGB::Black, CRGB::Black, CRGB::Black,
           CRGB::Red,   CRGB::Black, CRGB::Red,   CRGB::Black, CRGB::Black, CRGB::Black,
           CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red,   CRGB::Red },

         // `LedPattern::xAbort`:
         // `XAbort` pattern (index 4): A big Pink (Fuchsia) 'X' [❌] for abort/cancel
         { CRGB::Black,   CRGB::Fuchsia, CRGB::Black,   CRGB::Fuchsia, CRGB::Black,   CRGB::Black,
           CRGB::Black,   CRGB::Black,   CRGB::Fuchsia, CRGB::Black,   CRGB::Black,   CRGB::Black,
           CRGB::Black,   CRGB::Fuchsia, CRGB::Black,   CRGB::Fuchsia, CRGB::Black },

         // `LedPattern::okText`:
         // `OkText` pattern (index 5): A big Lime `✓` [✅] for okay/good                       /
         { CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Lime,  CRGB::Black, CRGB::Black, //    \/
           CRGB::Black, CRGB::Black, CRGB::Lime,  CRGB::Black, CRGB::Lime,  CRGB::Black,
           CRGB::Black, CRGB::Lime,  CRGB::Black, CRGB::Black, CRGB::Black },

         // `LedPattern::rainbow`:
         // `Rainbow` pattern (index 6): All colors of the rainbow, diagonal, over all LEDs.
         { CRGB::Violet, CRGB::Indigo, CRGB::Blue,   CRGB::Green,  CRGB::Yellow, CRGB::Orange,
           CRGB::Indigo, CRGB::Blue,   CRGB::Green,  CRGB::Yellow, CRGB::Orange, CRGB::Red,
           CRGB::Blue,   CRGB::Green,  CRGB::Yellow, CRGB::Orange, CRGB::Red }

         #if WIFI
         // `LedPattern::wText`:
         // `Wtext` pattern (index 7): A big RoyalBlue 'W' [📶] (for WPS / WiFi)
         ,{ CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::Black,
            CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue, CRGB::Black,
            CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue }

         // `LedPattern::aText`:
         // `Atext` pattern (index 8): A big Indigo 'A' [ᐋ] (for AP Access WEB page)
         ,{ CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Black,
            CRGB::Indigo, CRGB::Black,  CRGB::Indigo, CRGB::Black,  CRGB::Black,  CRGB::Black,
            CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo }

         // `LedPattern::pText`:
         // `Ptext` pattern (index 9): A big Orange 'P' [ᐳ] (for Phone app)
         ,{ CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Black,  CRGB::Black,  CRGB::Black,
            CRGB::Orange, CRGB::Black,  CRGB::Orange, CRGB::Black,  CRGB::Black,  CRGB::Black,
            CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange }
         #endif
         };

   const CRGB BinaryClock::hourColors_P[][NUM_HOUR_LEDS] PROGMEM = 
         {
         // onHourAm_P colors (index 0): Hours AM (LEDS 12 - 16)
         { CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue }, 
         // onHourPm_P colors (index 0): Hours PM (LEDS 12 - 16)
         { CRGB::Indigo,      CRGB::Indigo,      CRGB::Indigo,      CRGB::Indigo,      CRGB::Indigo }
         };    

   const CRGB* BinaryClock::onColor_P  = ledPatterns_P[static_cast<uint8_t>(LedPattern::onColors)];
   const CRGB* BinaryClock::offColor_P = ledPatterns_P[static_cast<uint8_t>(LedPattern::offColors)];

   const uint8_t BinaryClock::ledPatternCount = (sizeof(ledPatterns_P) / sizeof(ledPatterns_P[0]));

   const CRGB* BinaryClock::onHourAm_P = hourColors_P[0];
   const CRGB* BinaryClock::onHourPm_P = hourColors_P[1];
   const CRGB* BinaryClock::onHour24_P = &onColor_P[HOUR_LED_OFFSET]; // Pointer to the standard ON colors for Hour.

   // Initialize fl::arrays using the helper function 
   fl::array<CRGB, NUM_HOUR_LEDS> BinaryClock::OnHourAM = progmem2array<NUM_HOUR_LEDS>(onHourAm_P);
   fl::array<CRGB, NUM_HOUR_LEDS> BinaryClock::OnHourPM = progmem2array<NUM_HOUR_LEDS>(onHourPm_P);
   fl::array<CRGB, NUM_HOUR_LEDS> BinaryClock::onHour24 = progmem2array<NUM_HOUR_LEDS>(onColor_P + HOUR_LED_OFFSET);
   fl::array<CRGB, NUM_LEDS>      BinaryClock::OnColor  = progmem2array<NUM_LEDS>(onColor_P);
   fl::array<CRGB, NUM_LEDS>      BinaryClock::OffColor = progmem2array<NUM_LEDS>(offColor_P);

   /// @brief 2D table array to map the `AlarmTime::Repeat` enumerations with
   ///        the corresponding enumeration for Alarm1 and Alarm2.
   /// @details The alarms each have different enumeration values for the
   ///          alarm repetations so this array provides a way to map a common
   ///          repeat enumeration with the different alarms on the hardware.
   /// note The `Repeat::endTag` must be the last value as it is used to define the array size.
   const uint8_t BinaryClock::repeatModeTable[static_cast<uint8_t>(AlarmTime::Repeat::endTag)][2] =
         {
         {static_cast<uint8_t>(Ds3231Alarm1Mode::DS3231_A1_Hour),   static_cast<uint8_t>(Ds3231Alarm2Mode::DS3231_A2_Hour)},
         {static_cast<uint8_t>(Ds3231Alarm1Mode::DS3231_A1_Minute), static_cast<uint8_t>(Ds3231Alarm2Mode::DS3231_A2_Minute)},
         {static_cast<uint8_t>(Ds3231Alarm1Mode::DS3231_A1_Hour),   static_cast<uint8_t>(Ds3231Alarm2Mode::DS3231_A2_Hour)},
         {static_cast<uint8_t>(Ds3231Alarm1Mode::DS3231_A1_Day),    static_cast<uint8_t>(Ds3231Alarm2Mode::DS3231_A2_Day)},
         {static_cast<uint8_t>(Ds3231Alarm1Mode::DS3231_A1_Date),   static_cast<uint8_t>(Ds3231Alarm2Mode::DS3231_A2_Date)},
         };

   // Note: On the Wemos D1-R32 UNO boards, the builtin LED is GPIO 02, this is also the S3 button pin (A0).
   //       Setting this pin HIGH (ON) for more than 'bounceDelay' (e.g. 75) ms will trigger the alarm setup
   //       routine to be called. The alarm setup requires the user to go through the setup steps to exit 
   //       which can only happen while the LED is LOW (OFF). These boards need LED_HEART to be defined.

   #if defined(LED_HEART) && LED_HEART >= 0
   uint8_t BinaryClock::HeartbeatLED = LED_HEART;
   #else
   uint8_t BinaryClock::HeartbeatLED = LED_BUILTIN;
   #endif

   // When the SERIAL_TIME_CODE code is removed, redefine the method calls to be whitespace only
   #if SERIAL_TIME_CODE != true
      #define serialTime() 
   #endif

   #if DEV_CODE
   const char* weekdays[7] = 
         {
         [0]  = "Monday", 
         [1]  = "Tuesday", 
         [2]  = "Wednesday", 
         [3]  = "Thursday", 
         [4]  = "Friday", 
         [5]  = "Saturday", 
         [6]  = "Sunday"
         };

   const char* nibbles[16] =
         {
         [0]  = "0000", [1]  = "0001", [2]  = "0010", [3]  = "0011",
         [4]  = "0100", [5]  = "0101", [6]  = "0110", [7]  = "0111",
         [8]  = "1000", [9]  = "1001", [10] = "1010", [11] = "1011",
         [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
         };

   String ToBinary(uint8_t byte)   // *** DEBUG ***
      {
      char byteStr[10];
      memmove(byteStr, nibbles[byte >> 4], 4);
      byteStr[4] = ' ';
      memmove(byteStr + 5, nibbles[byte & 0x0F], 4);
      byteStr[9] = '\0'; // Null-terminate the string

      return String(byteStr);
      }
   
   void BinaryClock::DisplayAllRegisters()
      {
      char buffer[32];
      // Register names for the DS3231 RTC chip.
      const char* regNames[] = 
            { 
            [0]  = "Seconds",
            [1]  = "Minutes",
            [2]  = "Hours",
            [3]  = "Day",
            [4]  = "Date",
            [5]  = "Month",
            [6]  = "Year",
            [7]  = "Alarm 1 Seconds",
            [8]  = "Alarm 1 Minutes",
            [9]  = "Alarm 1 Hours",
            [10] = "Alarm 1 Day/Date",
            [11] = "Alarm 2 Minutes",
            [12] = "Alarm 2 Hours",
            [13] = "Alarm 2 Day/Date",
            [14] = "Control",
            [15] = "Control/Status",
            [16] = "Aging Offset",
            [17] = "MSB Temp",
            [18] = "LSB Temp"
            }; 

      char byteStr[10];
      // Lambda function to convert a byte to a binary string representation.
      // The string is the upper nibble, a space, the lower nibble. This is the
      // best way to display a byte as anyone who knows binary and is worth
      // their salt can instantly understand a binary nibble. It is up to the 
      // caller to deliniate between bytes, e.g. by using a comma and a space.
      auto binByteStr = [&byteStr](uint8_t byte)
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
         snprintf(buffer, sizeof(buffer), "  [0x%02X] 0x%02X; %s, ", 
                  i, regs[i], binByteStr(regs[i])); 
         Serial << buffer << regNames[i] << endl;
         }

      Serial << endl;
      }
   #else 
      #define DisplayAllRegisters() // Turn calls in to whitespace.
   #endif

   //################################################################################//
   // SETUP
   //################################################################################//

   void BinaryClock::setup(bool testLeds)
      {
      #if SERIAL_OUTPUT
      Serial.begin(115200);
      delay(10);
      #endif

      pinMode(HeartbeatLED, OUTPUT);
      digitalWrite(HeartbeatLED, LOW);

      static_assert(sizeof(AlarmNotes) / sizeof(Note) > 0, "AlarmNotes array must not be empty");

      bool s2Pressed = buttonS2.IsPressed();
      if (s2Pressed)       // User override check, display all the LED test patterns on the shield.
         { testLeds = true; }

      SERIAL_STREAM("Display LED test patterns on the shield: " << (testLeds? "YES" : "NO") << "; S2 Button was: " 
             << (s2Pressed? "Pressed" : "OFF") << "; Value: " << buttonS2.get_Value() << " OnValue: is: " 
             << buttonS2.get_OnValue() << endl)   // *** DEBUG ***

      if (SetupRTC())
         {
         testLeds = testLeds || RTC.lostPower();
         SetupFastLED(testLeds | true);   // *** DEBUG *** " | true"
         SetupAlarm();

         // Show the serial output, display the initial info.
         if (get_IsSerialSetup()) 
            { settings.Begin(); } 
         }
      else
         {
         // Send this to Purgatory, we're dead.
         PurgatoryTask("No RTC found.");
         }

      #if FREE_RTOS
      TaskHandle_t timeHandle = CreateInstanceTask<BinaryClock, void*>
            ( this
            , &BinaryClock::TimeTask
            , "TimeTask"
            , 3096
            , tskIDLE_PRIORITY + 1
            , nullptr);

      if (timeHandle == nullptr)
         {
         SERIAL_OUT_PRINTLN("Failed to create the 'TimeTask', unable to continue.")
         PurgatoryTask("Time Task failed", false);
         }

      set_TimeDispatchHandle(timeHandle);

      TaskHandle_t callbackHandle = CreateInstanceTask<BinaryClock, void*>
            ( this
            , &BinaryClock::CallbackTask
            , "CallbackTask"
            , 3096
            , tskIDLE_PRIORITY + 1
            , nullptr);

      if (callbackHandle == nullptr)
         {
         SERIAL_OUT_PRINTLN("Failed to create the 'CallbackTask', unable to continue.")
         PurgatoryTask("Callback Task failed", false);
         }

      set_CallbackTaskHandle(callbackHandle);
      #endif // FREE_RTOS

      isAmBlack = (AmColor == CRGB::Black);
      isPmBlack = (PmColor == CRGB::Black);
      switchColors = (isAmBlack || isPmBlack) && get_Is12HourFormat();

      delay(150); // Wait to stabilize after setup

      SERIAL_STREAM("Time: " << time.timestamp(get_Is12HourFormat() ? DateTime::TIMESTAMP_TIME12 : DateTime::TIMESTAMP_TIME)
            << endl << "Date:  " << time.timestamp(DateTime::TIMESTAMP_DATE) << " (" << weekdays[(time.dayOfTheWeek() + time.dayNameOffset()) % 7] 
            << ")" << endl)   // *** DEBUG ***
      }

   //################################################################################//
   // MAIN LOOP
   //################################################################################//

   void BinaryClock::loop()
      {
      yield();  // Give WiFi time

      if (TimeDispatch())
         {
         SettingsState settingsState = settings.ProcessMenu();

         // Only display time when not in settings
         if (settingsState == SettingsState::Inactive)
            {
            #if FREE_RTOS
            static bool firstTime = true;  // *** DEBUG ***
            if (firstTime) // *** DEBUG ***
               {
               vTaskDelay(15000); // *** DEBUG ***
               firstTime = false;
               }
            #endif
            DisplayBinaryTime(time.hour(), time.minute(), time.second(), get_Is12HourFormat());
            SERIAL_TIME()

            // Check if the alarm has gone off
            if (Alarm2.fired)
               {
               PlayAlarm();
               set_CallbackAlarmTriggered(true);
               Alarm2.fired = false;
               }
            }

         CallbackDispatch();
         yield();
         }
      else
         {
         // Process settings even when time hasn't updated
         settings.ProcessMenu();
         }
      
      #if HARDWARE_DEBUG
      CheckHardwareDebugPin();
      #endif
      }

   //################################################################################//
   // CLASS CONSTRUCTOR / DESTRUCTOR
   //################################################################################//

   BinaryClock::BinaryClock() 
         : rtcInterruptWasCalled(false)
         , callbackAlarmTriggered(false)
         , callbackTimeTriggered(false)
         , onColors(OnColor)
         , offColors(OffColor)
         , onHourPM(OnHourPM)
         , onHourAM(OnHourAM)
         , buttonS1(S1, CC_ON)
         , buttonS2(S2, CC_ON)
         , buttonS3(S3, CC_ON)
         #if HW_DEBUG_SETUP
         , buttonDebugSetup(DEBUG_SETUP_PIN, CC_ON)
         #endif
         #if HW_DEBUG_TIME
         , buttonDebugTime(DEBUG_TIME_PIN, CA_ON)
         #endif
         , settings(*this)
      {
      #if STL_USED   // For boards with enough memory to include Standard Template Libraries.
      currentMelody = 0;               // Use the default melody from PROGMEM
      initializeDefaultMelody();       // Create the default melody from PROGMEM array
      RegisterMelody(defaultMelody);   // Register the default melody as the first entry (index 0)
      #else
      isDefaultMelody = true;          // Using the melody stored in PROGMEM
      alarmNotes      = nullptr;       // No user supplied melody in RAM.
      alarmNotesSize  = 0;
      #endif

      // The compiler doesn't like the initialization of structs/classes at declaration, do it here.
      // UNO error: "sorry, unimplemented: non-trivial designated initializers not supported"
      memset(leds, 0, sizeof(leds)); // Clear the LED array
      memset(binaryArray, 0, sizeof(binaryArray)); // Clear the binary array
      // Copy the  hour portion of the `OnColor` array to the `onHour24` array to initialize it.
      memmove(onHour24.data(), OnColor.data() + HOUR_ROW_OFFSET, sizeof(onHour24));

      Alarm1.number = ALARM_1;
      Alarm1.clear();
      Alarm2.number = ALARM_2;
      Alarm2.clear();
      // END UNO required

      BCButton::set_BounceDelay(DEFAULT_DEBOUNCE_DELAY);
 
      buttonS1.Initialize();
      buttonS2.Initialize();
      buttonS3.Initialize();
      
      #if HW_DEBUG_SETUP
      buttonDebugSetup.Initialize();
      #endif
      #if HW_DEBUG_TIME
      buttonDebugTime.Initialize();
      #endif

      #if HW_DEBUG_TIME
      // Set the 'isSerialTime' to true if the hardware Time button is ON 
      // This is necessary if the button is actually a switch or is hardwired
      if (buttonDebugTime.IsPressedRaw())
         {
         // If the software has enabled IsSerialTime, give control to the software,
         // otherwise just turn on the flag. 
         if (isSerialTime)
            { set_IsSerialTime(isSerialTime); }
         else
            { isSerialTime = true; }
         } 
      #endif

      // Initialize the serial output properties to follow this initial value.
      // Any changes to these properties will be pushed to settings as well.
      settings.set_IsSerialTime(isSerialTime); 
      settings.set_IsSerialSetup(isSerialSetup);

      time = DateTime(70, 1, 1, 10, 4, 10);  // An 'X' [❌] if RTC fails.
      // This is an important check as we are using the enum value as an index in the array.
      static_assert((uint8_t)LedPattern::endTAG == BinaryClock::ledPatternCount, "LedPattern enum and ledPatterns_P array size mismatch");
      }

   BinaryClock::~BinaryClock()
      {
      // Detach the interrupt from the RTC INT pin
      detachInterrupt(digitalPinToInterrupt(RTC_INT));

      // Note: Not sure if the RTC 1 Hz H/W output should be stopped (C-70 2025/09)
      // // Disable 1 Hz square wave RTC SQW output
      // RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_OFF);;

      // Turn off the LEDs.
      FastLED.setBrightness(0);
      FastLED.clear(true);
      
      // Clean up button states
      buttonS1.Reset();
      buttonS2.Reset();
      buttonS3.Reset();

      #if HW_DEBUG_SETUP
      buttonDebugSetup.Reset();
      #endif
      #if HW_DEBUG_TIME
      buttonDebugTime.Reset();
      #endif
  
      // Stop any active alarm/tone
      noTone(PIEZO);
  
      // Clean up callback registrations
      timeCallback = nullptr;
      alarmCallback = nullptr;
      callbackTimeEnabled = false;
      callbackAlarmEnabled = false;
  
      // Reset interrupt flags
      set_RTCinterruptWasCalled(false);
      set_CallbackTimeTriggered(false);
      set_CallbackAlarmTriggered(false);
      }

   //#####################################################################//
   //#              Initialize the RTC library                           #//
   //#####################################################################//

   bool BinaryClock::SetupRTC()
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

      SERIAL_STREAM("Time from RTC: " << time.timestamp(get_Is12HourFormat()? DateTime::TIMESTAMP_TIME12 : DateTime::TIMESTAMP_TIME) 
            << " internal date: " << time.timestamp(DateTime::TIMESTAMP_DATE) << endl)   // *** DEBUG ***

      return rtcValid;
      }

   //#####################################################################//
   //#    Setup the Alarms from the RTC library and copy them locally    #//
   //#####################################################################//

   #define DAY_SECONDS 86400     // 24 * 3600
   #define MAX_ALARM_DELTA 300   // 5 minutes in seconds

   void BinaryClock::SetupAlarm()
      {
      if (rtcValid)
         {
         // Get the alarms stored in the RTC memory.
         DateTime alarm1time = RTC.getAlarm1();
         DateTime alarm2time = RTC.getAlarm2();
         bool alarm1valid = alarm1time.isTimeValid();
         bool alarm2valid = alarm2time.isTimeValid();

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

         // For each of the alarms, get the alarm mode (i.e. repeat frequency) and
         // convert them to the `AlarmTime::Repeat` value, then save them.
         Ds3231Alarm1Mode mode1 = RTC.getAlarm1Mode();
         Ds3231Alarm2Mode mode2 = RTC.getAlarm2Mode();
         int index1 = -1;
         int index2 = -1;
         for (uint8_t i= 0; i < REPEAT_MODE_ROW_COUNT; i++)
            {
            if (index1 < 0 && repeatModeTable[i][0] == mode1)
               { index1 = i; }
            if (index2 < 0 && repeatModeTable[i][1] == mode2)
               { index2 = i; }

            if (index1 >= 0 && index2 >= 0)
               { break; }
            }

         if (index1 >= 0 && index1 < (int)(AlarmTime::Repeat::endTag))
            { Alarm1.freq = (AlarmTime::Repeat)(repeatModeTable[index1][0]); }
         if (index2 >= 0 && index2 < (int)(AlarmTime::Repeat::endTag))
            { Alarm2.freq = (AlarmTime::Repeat)(repeatModeTable[index2][1]); }

         RTC.disableAlarm(Alarm1.number); // Disable alarm 1, not yet supported.

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

         SERIAL_STREAM("Alarm1: " <<  Alarm1.time.timestamp(DateTime::TIMESTAMP_TIME) << " (" << alarm1time.timestamp(DateTime::TIMESTAMP_TIME) << 
               (Alarm1.time.isValid() ? " Valid) " : " Bad Time) ") << (Alarm1.status > 0 ? " ON; " : " OFF; ") << alarm1delta <<
               (alarm1inrange ? " In Range; " : " Continue; ") << (Alarm1.fired ? " Alarm Fired " : " No Alarm ") << endl)   // *** DEBUG ***

         SERIAL_STREAM("Alarm2: " << Alarm2.time.timestamp(DateTime::TIMESTAMP_TIME) << " (" << alarm2time.timestamp(DateTime::TIMESTAMP_TIME) <<
               (Alarm2.time.isValid() ? " Valid) " : " Bad Time) ") << (Alarm2.status > 0 ? " ON; " : " OFF; ") << alarm2delta <<
               (alarm2inrange? " In Range; " : " Continue; ") << (Alarm2.fired? " Alarm Fired " : " No Alarm ") << endl)   // *** DEBUG ***

         // Clear the alarm 'Fired' flags on the RTC to catch a new alarm.
         RTC.clearAlarm(Alarm1.number);
         RTC.clearAlarm(Alarm2.number);
         }
      }

   //#####################################################################//
   //#            Initialize the FastLED library                         #//
   //#####################################################################//

   void BinaryClock::splashScreen(bool testLEDs)
      {
      int frequency = 3;
      DisplayLedPattern(LedPattern::rainbow);      // Turn on all LEDS showing a rainbow of colors.
      FlashLed(HeartbeatLED, 2, 25, frequency);    // Acts as a delay(2000/3) and does something.
      // Display the LED test patterns for the user.
      if (testLEDs)
         {
         DisplayLedPattern(LedPattern::onColors);
         FlashLed(HeartbeatLED, 3, 75, frequency);       // Acts as a delay(3000/3) and does something.
         DisplayLedPattern(LedPattern::onText);
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(4000/3) and does something.
         DisplayLedPattern(LedPattern::offTxt);
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(4000/3) and does something.
         DisplayLedPattern(LedPattern::xAbort);
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(4000/3) and does something.
         DisplayLedPattern(LedPattern::okText);
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(4000/3) and does something.
         #if WIFI
         DisplayLedPattern(LedPattern::wText); 
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(2000/3) and does something.
         DisplayLedPattern(LedPattern::aText); 
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(2000/3) and does something.
         DisplayLedPattern(LedPattern::pText); 
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(2000/3) and does something.
         #endif
         frequency = 2;
         }

      // Display the rainbow pattern over all pixels to show everything working.
      DisplayLedPattern(LedPattern::rainbow);   // Turn on all LEDS showing a rainbow of colors.
      FlashLed(HeartbeatLED, 5, 25, frequency); // Acts as a delay(5000/2) and does something.
      // DisplayLedPattern(LedPattern::offColors);
      // FlashLed(HeartbeatLED, 1, 25, frequency); // Acts as a delay(1000/2) and does something.
      };


   void BinaryClock::SetupFastLED(bool testLEDs)
      {
      // Set the `OnHhour` to the default color (24 hour mode; PM; or always when AmColor isn't Black)
      // The `OnColor` for the hours row is saved to the `OnHourPM` for use with the `OnHourAM` values.
      // These hour colors are copied to save them when when the OnHourAM colors are in use as they 
      // overwrite the hour LEDs. This only happens when the AM indicator is Black, otherwise it's unused.
      // Given the target is an UNO board, a little bit bashing between between the same array types
      // should be fine as the C++ standard states the data is contiguous.
      memmove(OnHourPM.data(), (OnColor.data() + NUM_SECOND_LEDS + NUM_MINUTE_LEDS), sizeof(OnHourPM));
      
      // Turn off the display, start with a blank display.
      FastLED.setBrightness(0);
      FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, TOTAL_LEDS);
      FastLED.clearData();
      FastLED.show();
      delay(50);
      
      FastLED.setCorrection(TypicalSMD5050);
      // Limit to 450mA at 5V of power draw total for all LEDs
      FastLED.setMaxPowerInVoltsAndMilliamps(5, 450);
      FastLED.setBrightness(brightness);


      #if FREE_RTOS
      // Create splash screen task with error handling to allow setup to continue.
      bool taskCreated = CreateInstanceTask<BinaryClock, bool>(
            this,                         // Instance pointer
            &BinaryClock::splashScreen,   // Method pointer
            "LEDSplashTask",              // Task name
            testLEDs                      // Argument
            );

      if (taskCreated)
         {
         SERIAL_STREAM("[" << millis() << "] Splash screen task created successfully" << endl)
         }
      else
         {
         SERIAL_PRINTLN("ERROR: Failed to create splash screen task!")
         // Fall back to direct execution with a limited screen display.
         splashScreen(false);
         }
      #else
      splashScreen(testLEDs);
      #endif
      }

   void BinaryClock::FlashLed(uint8_t ledNum, uint8_t repeat, uint8_t dutyCycle, uint8_t frequency)
      {
      // Validate/correct the inputs.
      if (dutyCycle > 100) { dutyCycle = 100; }
      if (frequency <  1)  { frequency = 1; }
      if (frequency > 25)  { frequency = 25; }

      uint32_t onTime  =  (dutyCycle * 10) / (frequency);         // On  time in ms.
      uint32_t offTime = ((100 - dutyCycle) * 10) / (frequency);  // Off time in ms.
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

   void BinaryClock::RTCinterrupt()
      {
      set_RTCinterruptWasCalled(true);

      #if FREE_RTOS
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xTaskNotifyFromISR(get_TimeDispatchHandle(), 0, eNoAction, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      #endif
      // Set the trigger flag IFF the "callback time is enabled" flag is set.
      set_CallbackTimeTriggered(callbackTimeEnabled);
      }

   void BinaryClock::set_Time(DateTime value)
      {
      #if DEV_CODE
      DateTime::timestampOpt timestampFormat = (get_Is12HourFormat()? DateTime::TIMESTAMP_DATETIME12 : DateTime::TIMESTAMP_DATETIME);
      #endif
      // Check if the RTC is valid and the new time is valid, we don't care about the date.
      // Once we have a valid DateTime object, adjust the time on the RTC in the current mode.
      // We read the current time on the RTC and set the local `time` to the value  the
      // RTC has. Caller can check for errors by comparing given `value` to get_Time().
      if (rtcValid && value.isTimeValid())
         {
         SERIAL_STREAM(">>> Set time to: " << value.timestamp(timestampFormat) << "; from: " << time.timestamp(timestampFormat) << endl)   // *** DEBUG ***

         // If the year is 2000, set it to 2001 so that the DayOfWeek() calculation works correctly
         // This would indicate that only the time was being set.
         if (value.year() == 2000) 
            { value = DateTime(2001, value.month(), value.day(), value.hour(), value.minute(), value.second()); }

         time = RTC.now();
         if (time != value)
            { 
            RTC.adjust(value, get_Is12HourFormat()); 
            time = ReadTime();
            SERIAL_STREAM(">>> RTC time adjusted to: " << time.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl)   // *** DEBUG ***
            }
         else
            { SERIAL_STREAM("     RTC has the same time: " << time.timestamp(timestampFormat) << ". Nothing to do." << endl) }  // *** DEBUG ***
         }
      else
         { SERIAL_STREAM("*** Invalid RTC / time. RTC Valid? " << (rtcValid ? "True, " : "False, ") << value.timestamp(timestampFormat) << endl) } // *** DEBUG ***
      }

   // DateTime BinaryClock::get_Time() const
   //    {  return time; }

   void BinaryClock::set_Alarm(AlarmTime value)
      {
      // Exit on bad input or missing RTC hardware.
      if (value.number < ALARM_1 || value.number > ALARM_2 || !rtcValid || !value.time.isValid()) { return; }

      // Set the alarm time and status in the RTC
      if (value.status >= 0)
         {
         // // NOTE: (Chris-70, 2025/07/05)
         // // The current version of Adafruit's RTCLib (2.1.4) does not allow setting the
         // // Alarm interrupt registers A1IE and A2IE when the INTCN bit is set to 0.
         // // This means we must disable the 1Hz SQ Wave, set the bits and then re-enable the SQ Wave.
         // // This shouldn't impact the LED display but it might generate an additional interrupt.
         // RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_OFF);

         // Set the alarm to sound at the same time, 'hour:minute', each day.
         if (value.number == ALARM_1)
            { 
            Ds3231Alarm1Mode mode = (Ds3231Alarm1Mode)(repeatModeTable[0][(uint8_t)(value.freq)]);

            if (RTC.setAlarm1(value.time, mode))
               { Alarm1 = value; }
            }
         else if (value.number == ALARM_2)
            { 
            Ds3231Alarm2Mode mode = (Ds3231Alarm2Mode)(repeatModeTable[1][(uint8_t)(value.freq)]);

            if (RTC.setAlarm2(value.time, mode))
               { Alarm2 = value; }
            }

         RTC.clearAlarm(value.number); // Clear the Alarm Trigger flag.
         // RTC.writeSqwPinMode(Ds3231SqwPinMode::DS3231_SquareWave1Hz);

         // We saved the alarm time in the RTC, now turn it off IFF the status is OFF
         if (value.status == 0)
            { RTC.disableAlarm(value.number); }
         }
      else { ; } // Ignore bad (-ve) input status
      }

   AlarmTime BinaryClock::GetRtcAlarm(int number)
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

   char* BinaryClock::DateTimeToString(DateTime time, char* buffer, size_t size, const char* format)
      {
      if (buffer == nullptr || size == 0) { return nullptr; } // Return null if buffer is null

      strncpy(buffer, format, size);
      return time.toString(buffer);
      }

   bool BinaryClock::get_IsSerialSetup() const
      {
      #if SERIAL_SETUP_CODE
      return isSerialSetup;
      #else
      return false;
      #endif
      }

   void BinaryClock::set_IsSerialSetup(bool value)
      {
      #if SERIAL_SETUP_CODE
      isSerialSetup = value;
      settings.set_IsSerialSetup(value);
      #endif
      }

   bool BinaryClock::get_IsSerialTime() const
      {
      #if SERIAL_TIME_CODE
      return isSerialTime;
      #else
      return false;
      #endif
      }

   void BinaryClock::set_IsSerialTime(bool value)
      {
      #if SERIAL_TIME_CODE
         #if HW_DEBUG_TIME
         // Pause any hardware control, software has the control.
         buttonDebugTime.Reset();
         #endif
      isSerialTime = value;
      settings.set_IsSerialTime(value);
      #endif
      }

   void BinaryClock::set_OnColors(const fl::array<CRGB, NUM_LEDS>& value)
      { onColors = value; }

   const fl::array<CRGB, NUM_LEDS>& BinaryClock::get_OnColors() const
      { return onColors; }

   void BinaryClock::set_OffColors(const fl::array<CRGB, NUM_LEDS>& value)
      { offColors = value; }

   const fl::array<CRGB, NUM_LEDS>& BinaryClock::get_OffColors() const
      { return offColors; }

   void BinaryClock::set_OnHourPM(const fl::array<CRGB, NUM_HOUR_LEDS>& value)
      { onHourPM = value; }

   const fl::array<CRGB, NUM_HOUR_LEDS>& BinaryClock::get_OnHourPM() const
      { return onHourPM; }

   void BinaryClock::set_OnHourAM(const fl::array<CRGB, NUM_HOUR_LEDS>& value)
      { onHourAM = value; }

   const fl::array<CRGB, NUM_HOUR_LEDS>& BinaryClock::get_OnHourAM() const
      { return onHourAM; }

   void BinaryClock::set_AmColor(CRGB value)
      { 
      if (value != AmColor)
         {
         AmColor = value;
         if (value == CRGB::Black)  
            { 
            isAmBlack = true; 
            switchColors = get_Is12HourFormat();
            }
         }
      }

   CRGB BinaryClock::get_AmColor() const
      { return AmColor; }

   void BinaryClock::set_PmColor(CRGB value)
      { PmColor = value; }

   CRGB BinaryClock::get_PmColor() const
      { return PmColor; }

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
      set_TimeFormat( value? timeFormat12  : timeFormat24);
      set_AlarmFormat(value? alarmFormat12 : alarmFormat24);
      RTC.setIs12HourMode(value); // Set the RTC to 12/24 hour mode
      #if DEV_CODE
      SERIAL_STREAM(endl << "Is AM/PM? " << (value? "True" : "False") << "; Formats in use: " << TimeFormat << "; " 
            << AlarmFormat << "; " << time.toString(buffer, sizeof(buffer), get_TimeFormat()) << endl) // *** DEBUG ***
      #endif

      curHourColor = HourColor::Hour24;
      if (value)
         { 
         if (time.hour() < 12)
            { curHourColor = (isAmBlack? HourColor::Am : HourColor::Hour24); }
         else
            { curHourColor = (isPmBlack? HourColor::Pm : HourColor::Hour24); }
         }
      else
         {  }
      
      // Only switch colors when the AM or PM Indicator color is Black.
      switchColors = isAmBlack || isPmBlack;
      }

   bool BinaryClock::get_Is12HourFormat() const
      { return amPmMode; }

   #if HW_DEBUG_TIME
   void BinaryClock::set_DebugOffDelay(unsigned long value)
      { debugDelay = value; }

   unsigned long BinaryClock::get_DebugOffDelay() const
      { return debugDelay; }
   #endif

   void BinaryClock::set_DebounceDelay(unsigned long value)
      { debounceDelay = value; }

   unsigned long BinaryClock::get_DebounceDelay() const
      { return debounceDelay; }
   
   #if STL_USED
   void BinaryClock::set_Melody(size_t value)
      {
      // Validate the index against the registry size
      if (value < melodyRegistry.size())
         {
         currentMelody = value;
         }
      // If invalid index, currentMelody remains unchanged
      }
   
   size_t BinaryClock::get_Melody() const
      {
      return currentMelody;
      }
   
   const std::vector<Note>& BinaryClock::get_CurrentMelody() const
      {
      return GetMelodyById(currentMelody);
      }
   #endif

   const fl::array<CRGB, NUM_HOUR_LEDS>& BinaryClock::getCurHourColors()
      {
      if (switchColors)
         {
         if (curHourColor == HourColor::Am && isAmBlack)
            {
            // Switch to AM colors
            onHour = OnHourAM;
            }
         else if (curHourColor == HourColor::Pm && isPmBlack)
            {
            // Switch to PM colors
            onHour = OnHourPM;
            }
         else // i.e. if (curHourColor == HourColor::Hour24)
            {
            // Switch to 24 hour colors
            onHour = onHour24;
            }

         switchColors = false; // Reset the switch flag
         }

      return onHour;
      }

   bool BinaryClock::registerCallback(void (*callbackFtn)(const DateTime&), void (*&callback)(const DateTime&), bool& cbFlag)
      {
      bool result = false;

      // Only register if there isn't already a callback registered and the provided callback is valid.
      if (callback == nullptr && callbackFtn != nullptr)
         {
         callback = callbackFtn;
         cbFlag = true;
         result = true;
         }

      return result;
      }

   bool BinaryClock::unregisterCallback(void(*callbackFtn)(const DateTime&), void(*&callback)(const DateTime&), bool& cbFlag)
      {
      bool result = false;

      if (callback == callbackFtn)
         {
         callback = nullptr;
         cbFlag = false;
         result = true; // Successfully unregistered
         }

      return result; 
      }

   #define TIMETASK_DELAY_MS  100      ///< The minimum time between time task calls.
   bool BinaryClock::TimeDispatch(uint32_t notificationFlags)
      {
      bool result = false;

      if (get_RTCinterruptWasCalled() || (notificationFlags & TIME_TRIGGER))
         {
         static unsigned long curTime  = 0UL;
         static unsigned long lastTime = 0UL;

         //////////////////////////////////////
         curTime = millis();
         if ((lastTime + TIMETASK_DELAY_MS) > curTime)
            { return result; }
         else
            { lastTime = curTime; }
         //////////////////////////////////////

         uint8_t prevHour = time.hour();
         time = RTC.now();

         /// @brief Lambda to check if an alarm was triggered, returns the result.
         /// @details If the alarm has fired, the alarm fired flag on the RTC 
         ///          (i.e. A1F or A2F)
         /// @param alarm The `AlarmTime` instance to check if it has fired.
         /// @returns  Flag indicating if the `alarm` had fired.
         auto checkAlarm = [&](AlarmTime& alarm)
               {
               if (alarm.number < 1 || alarm.number > 2) { return false; }
               
               if ((alarm.status > 0) && RTC.alarmFired(alarm.number))
                  {
                  alarm.fired = true;           // Set the flag, the alarm went off (e.g. ringing).
                  RTC.clearAlarm(alarm.number); // Clear the alarm flag for next alarm trigger.
                  // If this was a one-shot alarm, turn it off.
                  if (alarm.freq == AlarmTime::Repeat::Never)
                     {
                     RTC.disableAlarm(alarm.number);
                     alarm.status = 0; // Inactive
                     }
                  }
               else
                  { alarm.fired = false; }

               return alarm.fired;
               };

         #if FREE_RTOS
         notificationFlags |= TIME_TRIGGER;  // Set the time trigger flag in case we got here from a wait timeout.
         if (checkAlarm(Alarm1)) 
            { 
            notificationFlags |= ALARM1_TRIGGER; 
            set_CallbackAlarmTriggered(true);
            }
         if (checkAlarm(Alarm2)) 
            { 
            notificationFlags |= ALARM2_TRIGGER; 
            set_CallbackAlarmTriggered(true);
            }

         // Notify the callback task with the flags.
         xTaskNotify(get_CallbackTaskHandle(), notificationFlags, eSetBits);
         #else
         set_CallbackAlarmTriggered(checkAlarm(Alarm1) || checkAlarm(Alarm2)); // Set the alarm callback flag
         #endif

         uint8_t hour = time.hour();
         HourColor ampmColor = (hour < 12)? HourColor::Am : HourColor::Pm;
         // Check if we need to switch the hour colors, i.e. from PM to AM or AM to PM.
         if (((prevHour == 23) && (hour == 0)) || ((prevHour == 11) && (hour == 12)))
            {
            switchColors = true; // Signal a color switch is neede~        d
            curHourColor = (get_Is12HourFormat() ? ampmColor : HourColor::Hour24);
            }

         set_RTCinterruptWasCalled(false);
         result = true;
         }  // get_RTCinterruptWasCalled()

      return result;
      } // TimeDispatch()

   #if FREE_RTOS
   void BinaryClock::TimeTask(void*)
      {
      uint32_t notificationValue;
      FOREVER
         {
         BaseType_t notifyResult = xTaskNotifyWait ( TIME_TRIGGER | EXIT_TRIGGER
                                                   , 0x0000
                                                   , &notificationValue
                                                   , pdMS_TO_TICKS(TIMETASK_DELAY_MS));

         if (notifyResult == pdTRUE)
            {
            if (notificationValue & EXIT_TRIGGER)
               { break; }
            if (notificationValue & TIME_TRIGGER)
               { set_CallbackTimeTriggered(true); }

            TimeDispatch(notificationValue);
            }
         if (get_RTCinterruptWasCalled())
            { TimeDispatch(); }

         // vTaskDelay to prevent busy waiting
         vTaskDelay(pdMS_TO_TICKS(50));
         }
      }

   void BinaryClock::CallbackTask(void*)
      {
      uint32_t notificationValue;
      FOREVER
         {
         BaseType_t notifyResult = xTaskNotifyWait ( ALL_TRIGGERS
                                                   , ALL_TRIGGERS
                                                   , &notificationValue
                                                   , pdMS_TO_TICKS(CB_MAX_WAIT_MS));

         if (notifyResult == pdTRUE)
            {
            if (notificationValue & EXIT_TRIGGER)
               { break; }
            if (notificationValue & TIME_TRIGGER)
               { set_CallbackTimeTriggered(true); }
            if (notificationValue & ALARMS_TRIGGER)
               { set_CallbackAlarmTriggered(true); }

            CallbackDispatch();
            }
         else if (notifyResult == pdFALSE)
            {
            if (get_CallbackTimeTriggered() || get_CallbackAlarmTriggered())
               { CallbackDispatch(); }
            }
         }
      }
   #endif

   void BinaryClock::CallbackDispatch()
      {
      if (callbackTimeEnabled && get_CallbackTimeTriggered() && timeCallback != nullptr)
         {
         set_CallbackTimeTriggered(false);
         CallbackFtn(get_Time(), timeCallback);
         }

      if (callbackAlarmEnabled && get_CallbackAlarmTriggered() && alarmCallback != nullptr)
         {
         set_CallbackAlarmTriggered(false);
         CallbackFtn(get_Alarm().time, alarmCallback);
         }
      }

   void BinaryClock::CallbackFtn(DateTime time, void(*callback)(const DateTime&))
      {
      #if STL_USED
      // Protect ourselves against come unknow callback function.
      try
         {
      #endif

         if (callback != nullptr)
            {
            callback(time);
            }

      #if STL_USED
         }
      catch (std::exception& e)
         {
         SERIAL_OUT_STREAM("BinaryClock::CallbackFtn() - Caught exception: '" << e.what() << "' at " << time.timestamp(DateTime::TIMESTAMP_DATETIME) << endl)
         }
      catch (...)
         {
         SERIAL_OUT_STREAM("BinaryClock::CallbackFtn() - Caught an unknow exception at " << time.timestamp(DateTime::TIMESTAMP_DATETIME) << endl)
         }
      #endif
      }

   void BinaryClock::PurgatoryTask(const char* message, bool rtcFault)
      {
      // This is where failure comes to die.
      FastLED.clear(true); // Clear the LEDs.

      #ifdef ESP32_D1_R32_UNO
      // We are in Purgatory, button S3 isn't used, flash the builtin LED.
      // To leave Purgatory the RTC must appear, then we reboot and
      // IO02 is defined as the S3 button again and BUILTIN_LED is redefined.
      HeartbeatLED = 2;
      #else
      HeartbeatLED = LED_BUILTIN;
      #endif
      pinMode(HeartbeatLED, OUTPUT);

      SERIAL_OUT_PRINTLN("")
      SERIAL_OUT_PRINTLN(F("Failure: Unable to continue."))
      if (message != nullptr)
         {
         SERIAL_OUT_STREAM(F("Message: ") << message << endl << endl)
         }
      SERIAL_OUT_PRINTLN(F("    CQD - Entering Purgatory..."))

      // =================================================================
      // -.-. --.- -..  -.-. --.- -..  -.-. --.- -..  -.-. --.- -..  
      // Flash the LED to signal the failure.
      // This is just a little fun, flash the failure in Morse code.
      // On an UNO R3, adding this bit of fun costs 0 bytes of additional
      // RAM and only 220 bytes ROM (flash). Given that I need to signal a
      // catastrophic failure, such as no shield, without looking like a 
      // blinky sketch, this is a good (and educational) option.
      // 
      // Warning:
      // ========
      // Never use S.O.S. outside an actual life emergency.
      //
      // I got schooled by SAR, and now so have you.
      // So we can use a 100 year old alternative: 
      // CQD (-.-. --.- -..) it expands to: Come Quick Distress(*)
      // CQD was used before SOS and is a good alternative for us.
      // So, flash the message: 
      //       CQD NO RTC  -  Come Quick Distress NO Real Time Clock.
      //       -.-. --.- -..  -. ---   .-. - -.-.
      // We can't get out of purgatory without a Real Time Clock.
      // ---
      // (*) Its actual meaning is: "General Call To Any Station - Distress"
      //     CQ is shorthand for: General Call To Any Station. 
      //     D  is shorthand for: Distress in this context.
      MorseCodeLED morseCode(HeartbeatLED);
      morseCode.Begin();

      // Flash CQD NO RTC (Come Quick Distress NO Real Time Clock) to signal the failure.
      SERIAL_OUT_STREAM(F("  C    Q    D     N  O     R   T C ") << endl
                     << F(" [-.-. --.- -..   -. ---   .-. - -.-.] ") << endl
                     << F("(Come Quick Distress NO Real Time Clock)") << endl << endl)

      FOREVER
         {
         #ifdef UNO_R3
         morseCode.Flash_CQD_NO_RTC();
         #else
         morseCode.FlashString("CQD");
         delay(750);
         morseCode.FlashString(message);
         #endif
         delay(1950);

         // If RTC missing was the error, check if it gets connected.
         //    e.g. shield wasn't attached at start.
         // Otherwise we'll spend eternity in Purgatory.
         if (rtcFault && RTC.begin())
            {
            resetBoard(); // Reset the board if RTC is available
            }
         }
      }

   const CRGB* BinaryClock::patternLookup(LedPattern patternType)
      { return (patternType < LedPattern::endTAG ? ledPatterns_P[(uint8_t)(patternType)] : nullptr); }

   void BinaryClock::DisplayLedPattern(LedPattern patternType)
      {
      const CRGB* pattern = patternLookup(patternType);
      // Copy the pattern to the FastLED display array and display.
      // The pattern is based on the display LEDS size/layout.
      // We need to translate this to the physical layoutof the LEDs
      // on the clock. In the normal case these two are identical, in
      // other cases we need to remap the pattern to the physical layout.
      // If we are displaying on a larger LED matrix, e.g. 8x8, then we
      // need to map the pattern to the smaller clock display, row by row.
      if (pattern != nullptr)
         {
         // The number of display LEDs in each row
         int displayLeds[NUM_ROWS] 
               = { NUM_SECOND_LEDS  // 6
                 , NUM_MINUTE_LEDS  // 6
                 , NUM_HOUR_LEDS};  // 5
         int displayOffset = 0;
         // The physical LED offsets for each row..
         int physicalLeds[NUM_ROWS] 
               = { SECOND_ROW_OFFSET
                 , MINUTE_ROW_OFFSET
                 , HOUR_ROW_OFFSET};
         for (int j = 0; j < NUM_ROWS; j++)
            {
            for (int i = 0; i < displayLeds[j]; i++)
               {
               leds[physicalLeds[j] + i].r = pgm_read_byte(&pattern[displayOffset + i].r);
               leds[physicalLeds[j] + i].g = pgm_read_byte(&pattern[displayOffset + i].g);
               leds[physicalLeds[j] + i].b = pgm_read_byte(&pattern[displayOffset + i].b);
               }
            // Offset to the first LED in the next row.
            displayOffset += displayLeds[j];
            }
         FastLED.show();
         }
      }
   
   void BinaryClock::DisplayLedBuffer(const fl::array<CRGB, TOTAL_LEDS>& ledBuffer)
      {
      if (ledBuffer.empty()) { return; }

      // Copy the LED buffer to the FastLED display array and display
      memmove(leds, ledBuffer.data(), sizeof(CRGB) * ledBuffer.size());
      FastLED.show();
      }

   ////////////////////////////////////////////////////////////////////////////////////
   // Convert values from DEC to BIN format and display

   void BinaryClock::DisplayBinaryTime(int hoursRow, int minutesRow, int secondsRow, bool use12HourMode)
      {
      // Serial << "[" << millis() << "] DisplayBinaryTime() start" << endl;
      #if SERIAL_TIME_CODE
         // If SERIAL_TIME_CODE is true, we need to keep track of the binary representation of the time
      #define SET_LEDS(led_num, display_num, value, bitmask, on_color, off_color) \
                  leds[led_num] = (binaryArray[display_num] = ((value) & (bitmask))) ? on_color : off_color;
      #else
      #define SET_LEDS(led_num, display_num, value, bitmask, on_color, off_color) \
                  leds[led_num] = ((value) & (bitmask)) ? on_color : off_color;
      #endif
      // Use local variables for the calculations
      uint8_t hourBits, minuteBits, secondBits;
      // Use the (6) bit masks to test for the bits values.
      static const uint8_t bitMasks_P[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };

      if (use12HourMode)
         {
         hourBits = hoursRow % 12;
         if (hourBits == 0)
            {
            hourBits = 12;
            }
       // Display the indicator for AM or PM.
         leds[(HOUR_ROW_OFFSET + NUM_HOUR_LEDS) - 1] = (hoursRow >= 12) ? PmColor : AmColor;
         }
      else
         {
         hourBits = hoursRow & HOUR_MASK_24; // 5 bits for 24-hour
         }

      minuteBits = minutesRow & MINUTE_MASK; // 6 bits
      secondBits = secondsRow & SECOND_MASK; // 6 bits

      uint8_t ledIndex;
      uint8_t displayIndex;
      const fl::array<CRGB, NUM_HOUR_LEDS>& onColorsHour = getCurHourColors();
      // Hours (LEDs 12-15/16, skip LED 16 if in 12-hour mode)
      for (uint8_t i = 0; i < (use12HourMode ? NUM_HOUR_LEDS - 1 : NUM_HOUR_LEDS); i++)
         {
         ledIndex = HOUR_ROW_OFFSET + i;
         displayIndex = HOUR_LED_OFFSET + i;
         SET_LEDS(ledIndex, displayIndex, hourBits, bitMasks_P[i], onColorsHour[i], offColors[displayIndex]);
         }

      // Minutes (LEDs 6-11)
      for (uint8_t i = 0; i < NUM_MINUTE_LEDS; i++)
         {
         ledIndex = MINUTE_ROW_OFFSET + i;
         displayIndex = MINUTE_LED_OFFSET + i;
         SET_LEDS(ledIndex, displayIndex, minuteBits, bitMasks_P[i], onColors[displayIndex], offColors[displayIndex]);
         }

      // Seconds (LEDs 0-5)
      for (uint8_t i = 0; i < NUM_SECOND_LEDS; i++)
         {
         ledIndex = SECOND_ROW_OFFSET + i;
         displayIndex = SECOND_LED_OFFSET + i;
         SET_LEDS(ledIndex, displayIndex, secondBits, bitMasks_P[i], onColors[displayIndex], offColors[displayIndex]);
         }

      FastLED.show();
      }

   #undef SET_LEDS   // Undefine the MACRO, it isn't needed anymore.

   //################################################################################//
   // MELODY ALARM
   //################################################################################//

   #if STL_USED
   // Add this private method to initialize the default melody:
   void BinaryClock::initializeDefaultMelody()
      {
      defaultMelody.reserve(AlarmNotesSize);

      // Copy the default melody from the PROGMEM array
      for (size_t i = 0; i < AlarmNotesSize; i++)
         {
         Note note;
         // Read the Note from PROGMEM to the local Note.
         memcpy_P(&note, &AlarmNotes[i], sizeof(Note));
         // Build the melody by copying the local note to the end of the vector.
         defaultMelody.insert(defaultMelody.end(), note);
         }

      RegisterMelody(defaultMelody);
      }
   #endif

   ////////////////////////////////////////////////////////////////////////////////////
   // During playing the alarm melody, time display function is disabled

   /// @brief Helper method to play a single note and handle user interruption (i.e. STOP button pressed).
   /// @details Plays a single note and monitors the __S2__ button waiting for the user
   ///          to press it and stop the alarm sounding. If the button is pressed, the
   ///          playing is stopped and the function returns false to signal to the caller
   ///          that the melody playing should stop.
   /// @remarks This inline helper method exists to remove the shared code in the
   ///          `PlayMelody(const std::vector<Note>& melody)` method defined with `STL_USED` and 
   ///          `PlayAlarm(const AlarmTime& alarm)` method defined otherwise.
   /// @returns True if the note played successfully, false if stopped by user.
   inline bool playNote(const Note& note, const BCButton& buttonS2)
      {
      unsigned long millis_time_now = millis();
      
      // Create the tone with the note frequency and duration for the current note
      tone(PIEZO, note.tone, note.duration);

      // To distinguish the notes, set a minimum time between them ((1+1/4+1/16 = 21/16) * duration).
      int pauseBetweenNotes = note.duration + (note.duration >> 2) + (note.duration >> 4);

      // Pause between notes and check for button press to stop the melody.
      while (millis() < millis_time_now + pauseBetweenNotes)
         {
         // Stop alarm melody and go to main menu
         if (const_cast<BCButton&>(buttonS2).IsPressedNew())
            {
            SERIAL_OUT_STREAM("Melody Stopped by User - Button press." << endl)

            // Stop the tone playing
            noTone(PIEZO);

            // Exit we are done. Escape to main menu
            return false;
            }
         }

      // Stop the tone playing between notes
      noTone(PIEZO);

      return true;
      }

   #if STL_USED
   void BinaryClock::PlayAlarm(const AlarmTime& alarm) const
      {
      // Play the current melody using the currentMelody index
      PlayMelody(alarm.melody);
      }

   size_t BinaryClock::RegisterMelody(const std::vector<Note>& melody)
      {
      // Add melody reference to registry
      melodyRegistry.emplace_back(std::cref(melody));

      // Return the index (0-based)
      return melodyRegistry.size() - 1;
      }

   const std::vector<Note>& BinaryClock::GetMelodyById(size_t index) const
      {
      if (index < melodyRegistry.size())
         {
         return melodyRegistry[index].get();
         }

      // Return default melody if index is invalid
      return defaultMelody;
      }

   bool BinaryClock::PlayMelody(size_t id) const
      {
      // Validate index and get melody reference
      if (id < melodyRegistry.size())
         {
         const std::vector<Note>& melody = melodyRegistry[id].get();
         PlayMelody(melody);
         return true;
         }

      return false; // Invalid index
      }

   void BinaryClock::PlayMelody(const std::vector<Note>& melody) const
      {
      if (melody.empty()) { return; }

      unsigned long millis_time_now = 0;
      unsigned long noteDuration;

      for (int i = 0; i < alarmRepeatMax; i++)
         {
         for (size_t thisNote = 0; thisNote < melody.size(); thisNote++)
            {
            if (!playNote(const_cast<Note&>(melody[thisNote]), buttonS2)) 
               { return; } // Exit if user stopped the melody
            }
         }
      }
   #else
   void BinaryClock::PlayAlarm(const AlarmTime& alarm) const
      {
      // We are on an UNO R3 (or some other RAM/ROM constraind board) without STL.
      // Play the alarm melody directly from flash ROM to avoid making a local copy, 
      // or play the user supplied alarm melody from RAM.
      size_t melodySize = isDefaultMelody? AlarmNotesSize : alarmNotesSize;

      for (int i = 0; i < alarmRepeatMax; i++)
         {
         Note currentNote;
         for (size_t thisNote = 0; thisNote < melodySize; thisNote++)
            {
            // Read the note from PROGMEM or from the user given melody.
            // This saves us from needing to copy the melody to RAM.
            if (isDefaultMelody) 
               { memcpy_P(&currentNote, &AlarmNotes[thisNote], sizeof(Note)); }
            else
               { currentNote = alarmNotes[thisNote]; }

            if (!playNote(currentNote, buttonS2)) 
               { return; } // Exit if user stopped the melody
            }
         }
      }

   bool BinaryClock::SetAlarmMelody(Note* melodyArray, size_t melodySize)
      {
      bool result = false;
      if (melodyArray == nullptr || melodySize == 0)
         {
         alarmNotes = melodyArray;
         alarmNotesSize = melodySize;
         isDefaultMelody = false;
         result = true;
         }
      else
         {
         alarmNotes = nullptr;
         alarmNotesSize = 0;
         isDefaultMelody = true;
         }

      return result;
      }
   #endif

   DateTime BinaryClock::ReadTime() 
      { return RTC.now(); }

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
         Serial << F("Time: ") <<  get_Time().toString(buffer, sizeof(buffer), get_TimeFormat()) << F("  Binary: ");

         for (int i = NUM_LEDS - 1; i >= 0; i--)
            {
            if (i == (HOUR_LED_OFFSET - 1) || i == (MINUTE_LED_OFFSET - 1)) Serial << (" "); // Insert a space between hours - minutes, and minutes - seconds.
            Serial << (binaryArray[i] ? "1" : "0"); // Print 1 or 0 for each LED
            }
         Serial << endl;
         }
      }
   #endif 

   ////////////////////////////////////////////////////////////////////////////////////
   #if HARDWARE_DEBUG
   void BinaryClock::CheckHardwareDebugPin()
      {
      #if HW_DEBUG_SETUP
      static bool isLocalSetupOn = DEFAULT_SERIAL_SETUP;

      if (buttonDebugSetup.IsPressedNew()) 
         {
         set_IsSerialSetup(!isLocalSetupOn);
         isLocalSetupOn = !isLocalSetupOn;         
         Serial << F("Serial Menu is: ") << (get_IsSerialSetup() ? "ON" : "OFF") << endl;
         }
      #endif

      #if HW_DEBUG_TIME
      // For the first reading, setup values so isButtonOnNew() returns false;
      // Design: Initial priority to S/W configuration values so we set the
      // state and lastSate to reflect the current PIN value forcing the user to
      // change the state of the switch before we have H/W control.
      if (buttonDebugTime.get_IsFirstRead())
         {
         buttonDebugTime.ClearPressedNew();
         }

      // Check if the hardware debug pin is set, if not turn off the serial time output 
      // after 'debugDelay' msec has passed. If the initial H/W value is ON but the S/W
      // value is OFF, it will remain OFF until the H/W cycles OFF then ON.
      if (buttonDebugTime.IsPressedNew()) 
         {
         isSerialTime = true; // Set the serial time flag to true
         Serial << F(" Serial Time is: ON") << endl; // Debug: 
         }
      else if ((buttonDebugTime.get_LastReadTime() > 0) && (isSerialTime && !buttonDebugTime.IsPressed()) && ((millis() - buttonDebugTime.get_LastReadTime()) > get_DebugOffDelay()))
         {
         isSerialTime = false; // Reset the serial time flag
         Serial << F(" Serial Time is: OFF") << endl;
         }
      #endif
      }
   #endif
   }  // END OF NAMESPACE BinaryClockShield && BINARY CLOCK CLASS code
   //################################################################################//
