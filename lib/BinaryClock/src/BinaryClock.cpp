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
   [README.md](https://github.com/Chris-70/WiFiBinaryClock/blob/main/README.md#hardware-modifications-for-the-wemos-d1-r32-uno) file.
   The code for the WiFi connection is encapsulated in its own class, `BinaryClockWiFi`, which is not included in this file.
   It uses WPS to connect to a WiFi network and stores the credentials in the ESP32's flash memory.

  @section classes Available classes

  This library provides the following classes:

  - Interface class:
    - [**IBinaryClock**](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/IBinaryClock.h):
                        A pure interface class for the Binary Clock. This is used to define the methods and properties
                        that the BinaryClock class must implement and that other classes can count on to be available.
  - Main class:
    - [**BinaryClock**](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BinaryClock.h):
                        The main class, implements the IBinaryClock interface, handles all aspects of the
                        Binary Clock Shield, from display to settings and callbacks.
  - Helper classes:
    - [**BCButton**](https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/src/BCButton.h):
                        Class to encapsulate the buttons for debounce, state and wiring (CC vs CA).
    - [**BCSettings**](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BCSettings.h):
                        Class to encapsulate the settings for the Binary Clock, including time format and alarm settings.
  - Custom library dependencies:
    - **RTLlibPlus**       A modified fork of [Adafruit's RTLlib](https://github.com/adafruit/RTClib) to expand the functionality and support 12 hour mode.
    - **BinaryClockWiFi**  WiFi connection and syncing with an NTP server.
    - [**MorseCodeLED**](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/MorseCodeLED/src/MorseCodeLED.h):
                        Display error messages over the _LED_BUILTIN_. A fun communication alternative when there is no screen.
  - External library dependencies:
    - [**FastLED**](https://github.com/FastLED/FastLED):                - The FastLED library for colored LED animation on Arduino.
    - [**Streaming**](https://github.com/jcw/streaming):                - The Streaming library for Arduino.
    - [**Adafruit BusIO**](https://github.com/adafruit/Adafruit_BusIO): - The Adafruit BusIO library for I2C/SPI communication.

  @section license License

  This **`WiFiBinaryClock`** software, Copyright (c) 2025 Chris-70 and Chris-80, is licensed under the GNU General Public License v3.0 (**`GPL-v3.0`**).
  You may obtain a copy of the License at: **`https://www.gnu.org/licenses/gpl-3.0.en.html`** (see [**LICENSE**](https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/src/LICENSE) file).

  Original example by Marcin Saj [Binary-Clock_Shield_Example-11](https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino/blob/master/example/11-BinaryClockRTCInterruptAlarmButtons/11-BinaryClock-24H-RTCInterruptAlarmButtons.ino), (c) 2018
  released under the GPL-v3.0 license.
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
   #include <vector>
   #include <functional>
#endif 

#include <Arduino.h>

#include "BinaryClock.h"         // Header file for this library 
#include "MorseCodeLED.h"        // Used in PurgatoryTask() to flash error (Total delta bytes: 0 RAM; 220 ROM on UNO R3)
#include "pitches.h"             // Need to create the pitches.h library: https://arduino.cc/en/Tutorial/ToneMelody

// Include libraries
#include <FastLED.h>             // https://github.com/FastLED/FastLED
#include <RTClib.h>              // Adafruit RTC library: https://github.com/adafruit/RTClib
#include <Streaming.h>           // https://github.com/janelia-arduino/Streaming                            

#include <assert.h>              // Catch code logic errors during development.

namespace BinaryClockShield
   {
   #define NOTE_MS(N) (1000 / N) ///< Convert note duration to milliseconds

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
   // Used to convert PROGMEM array of: `onColors`; `offColors`; `onHourAmP`; `onHourP`
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
   // Initially used for: `onText`; `offTxt`; `xAbort`; `okText`; `rainbow`; `wText`; 
   // As a RAM saving measure, `DisplayLedPattern()` now uses the PROGMEM array directly.
   template<size_t N>
   const fl::array<CRGB, N> progmem2constArray(const CRGB* progmem_source)
      { return progmem2array<N>(progmem_source); }
   
   CRGB BinaryClock::PmColor = CRGB::Indigo;       ///< Color for the PM indicator LED (e.g. Indigo).
   CRGB BinaryClock::AmColor = CRGB::DeepSkyBlue;  ///< Color for the AM indicator LED (e.g. DeepSkyBlue).

   const CRGB BinaryClock::ledPatternsP[static_cast<uint8_t>(LedPattern::endTAG)][NUM_LEDS] PROGMEM = 
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

         #if ESP32_WIFI
         // `LedPattern::wText`:
         // `Wtext` pattern (index 7): A big RoyalBlue 'W' [📶] (for WPS / WiFi)
         ,{ CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::RoyalBlue, CRGB::Black,
            CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue, CRGB::Black,
            CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue, CRGB::Black,     CRGB::RoyalBlue }
         #endif
         };

   const CRGB BinaryClock::hourColorsP[][NUM_HOUR_LEDS] PROGMEM = 
         {
         // onHourAmP colors (index 0): Hours AM (LEDS 12 - 16)
         { CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue }, 
         // onHourP colors (index 0): Hours PM and 24 (LEDS 12 - 16)
         { CRGB::Blue,        CRGB::Blue,        CRGB::Blue,        CRGB::Blue,        CRGB::Blue }
         };    

   const CRGB* BinaryClock::onColorP  = ledPatternsP[static_cast<uint8_t>(LedPattern::onColors)];
   const CRGB* BinaryClock::offColorP = ledPatternsP[static_cast<uint8_t>(LedPattern::offColors)];

   const uint8_t BinaryClock::ledPatternCount = (sizeof(ledPatternsP) / sizeof(ledPatternsP[0]));

   const CRGB* BinaryClock::onHourAmP = hourColorsP[0];
   const CRGB* BinaryClock::onHourP   = hourColorsP[1];

   // Initialize fl::arrays using the helper function 
   fl::array<CRGB, NUM_HOUR_LEDS> BinaryClock::OnHourAM = progmem2array<NUM_HOUR_LEDS>(onHourAmP);
   fl::array<CRGB, NUM_HOUR_LEDS> BinaryClock::OnHour   = progmem2array<NUM_HOUR_LEDS>(onHourP);
   fl::array<CRGB, NUM_LEDS>      BinaryClock::OnColor  = progmem2array<NUM_LEDS>(onColorP);
   fl::array<CRGB, NUM_LEDS>      BinaryClock::OffColor = progmem2array<NUM_LEDS>(offColorP);

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
   String ToBinary(uint8_t byte)   // *** DEBUG ***
      {
      const char* nibbles[16] =
         {
         [0]  = "0000", [1]  = "0001", [2]  = "0010", [3]  = "0011",
         [4]  = "0100", [5]  = "0101", [6]  = "0110", [7]  = "0111",
         [8]  = "1000", [9]  = "1001", [10] = "1010", [11] = "1011",
         [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
         };

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
      const char* nibbles[16] = {
         [0]  = "0000", [1]  = "0001", [2]  = "0010", [3]  = "0011",
         [4]  = "0100", [5]  = "0101", [6]  = "0110", [7]  = "0111",
         [8]  = "1000", [9]  = "1001", [10] = "1010", [11] = "1011",
         [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
         };

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
         SetupFastLED(testLeds);
         SetupAlarm();

         // Show the serial output, display the initial info.
         if (isSerialSetup) 
            { settings.Begin(); } 
         }
      else
         {
         // Send this to Purgatory, we're dead.
         PurgatoryTask("No RTC found.");
         }

      isAmBlack = (AmColor == CRGB::Black);
      switchColors = isAmBlack && get_Is12HourFormat();

      delay(150); // Wait to stabilize after setup
      }

   //################################################################################//
   // MAIN LOOP
   //################################################################################//

   void BinaryClock::loop()
      {
      if (TimeDispatch())
         {
         // CHANGED: Use settings state instead of settingsMenu()
         SettingsState settingsState = settings.ProcessMenu();
         
         if (settingsState == SettingsState::Inactive)
            {
            // Only display time when not in settings
            DisplayBinaryTime(time.hour(), time.minute(), time.second(), get_Is12HourFormat());
            SERIAL_TIME()
            
            // Check if the alarm has gone off
            if (Alarm2.fired)
               {
               PlayAlarm();
               CallbackAlarmTriggered = true;
               Alarm2.fired = false;
               }
            }
            
         CallbackDispatch();
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

   BinaryClock::BinaryClock() :
         RTCinterruptWasCalled(false),
         CallbackAlarmTriggered(false),
         CallbackTimeTriggered(false),
         onColors(OnColor),
         offColors(OffColor),
         onHour(OnHour),
         onHourAM(OnHourAM),
         buttonS1(S1, CC_ON),
         buttonS2(S2, CC_ON),
         buttonS3(S3, CC_ON),
         #if HW_DEBUG_SETUP
         buttonDebugSetup(DEBUG_SETUP_PIN, CC_ON),
         #endif
         #if HW_DEBUG_TIME
         buttonDebugTime(DEBUG_TIME_PIN, CA_ON),
         #endif
         settings(*this)
      {
      #if STL_USED
      currentMelody = 0;               // Use the default melody from PROGMEM
      initializeDefaultMelody();       // Create the default melody from PROGMEM array
      RegisterMelody(defaultMelody);   // Register the default melody as the first entry (index 0)
      #else
      isDefaultMelody = true;          // Using the meldy stored in POGNMEM
      alarmNotes      = nullptr;       // No user supplied melody in RAM.
      alarmNotesSize  = 0;
      #endif

      // The compiler doesn't like the initialization of structs/classe at declatration, do it here.
      // UNO error: "sorry, unimplemented: non-trivial designated initializers not supported"
      memset(leds, 0, sizeof(leds)); // Clear the LED array
      memset(binaryArray, 0, sizeof(binaryArray)); // Clear the binary array

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
         { isSerialTime = true; }                  // Enable serial time
      #endif

      // Initialize the serial output properties to follow this initial value.
      // Any changes to these properties will be pushed to settings as well.
      settings.set_IsSerialTime(isSerialTime);
      settings.set_IsSerialSetup(isSerialSetup);

      time = DateTime(70, 1, 1, 10, 4, 10);  // An 'X' [❌] if RTC fails.
      // This is an important check as we are using the enum value as an index in the array.
      static_assert((uint8_t)LedPattern::endTAG == BinaryClock::ledPatternCount, "LedPattern enum and ledPatternsP array size mismatch");
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
      RTCinterruptWasCalled = false;
      CallbackTimeTriggered = false;
      CallbackAlarmTriggered = false;
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
         
         // tempAlarm = Alarm2; // Save a copy of Alarm2. Used for setting the alarm time from the menu.
         }
      }

   //#####################################################################//
   //#            Initialize the FastLED library                         #//
   //#####################################################################//

   void BinaryClock::SetupFastLED(bool testLEDs)
      {
      // Set the `OnHhour` to the default color (24 hour mode; PM; or always when AmColor isn't Black)
      // The `OnColor` for the hours row is saved to the `OnHour` for use with the `OnHourAM` values.
      // These hour colors are copied to save them when when the OnHourAM colors are in use as they 
      // overwrite the hour LEDs. This only happens when the AM indicator is Black, otherwise it's unused.
      // Given the target is an UNO board, a little bit bashing between between the same array types
      // should be fine as the C++ standard states the data is contiguous.
      memmove(OnHour.data(), (OnColor.data() + NUM_SECOND_LEDS + NUM_MINUTE_LEDS), sizeof(OnHour));
      
      FastLED.setBrightness(0);
      FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
      FastLED.clearData();
      FastLED.show();
      delay(50);
      
      int frequency = 3;
      FastLED.setCorrection(TypicalSMD5050);
      // Limit to 450mA at 5V of power draw total for all LEDs
      FastLED.setMaxPowerInVoltsAndMilliamps(5, 450);
      FastLED.setBrightness(brightness);
      DisplayLedPattern(LedPattern::rainbow);      // Turn on all LEDS showing a rainbow of colors.
      FlashLed(HeartbeatLED, 2, 25, frequency); // Acts as a delay(2000/2) and does something.
      // Display the LED test patterns for the user.
      if (testLEDs)
         {
         DisplayLedPattern(LedPattern::onColors);
         FlashLed(HeartbeatLED, 3, 75, frequency);       // Acts as a delay(3000/2) and does something.
         DisplayLedPattern(LedPattern::onText);
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(4000/3) and does something.
         DisplayLedPattern(LedPattern::offTxt);
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(4000/3) and does something.
         DisplayLedPattern(LedPattern::xAbort);
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(4000/3) and does something.
         DisplayLedPattern(LedPattern::okText);
         FlashLed(HeartbeatLED, 4, 50, frequency);      // Acts as a delay(4000/3) and does something.
         #if ESP32_WIFI
         DisplayLedPattern(LedPattern::wText); 
         FlashLed(HeartbeatLED, 2, 50, frequency);      // Acts as a delay(4000/3) and does something.
         #endif
         frequency = 2;
         }

      // Display the rainbow pattern over all pixels to show everything working.
      DisplayLedPattern(LedPattern::rainbow);      // Turn on all LEDS showing a rainbow of colors.
      FlashLed(HeartbeatLED, 5, 25, frequency); // Acts as a delay(2000/2) and does something.
      DisplayLedPattern(LedPattern::offColors);
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

   void BinaryClock::RTCinterrupt()
      {
      RTCinterruptWasCalled = true;
      // Set the trigger flag IFF the callback time is enabled flag is set.
      CallbackTimeTriggered = callbackTimeEnabled;
      }

   void BinaryClock::set_Time(DateTime value)
      {
      // Check if the RTC is valid and the new time is valid, we don't care about the date.
      // Once we have a valid DateTime object, adjust the time on the RTC in the current mode.
      // We read the current time on the RTC and set the local `time` to the value  the
      // RTC has. Caller can check for errors by comparing given `value` to get_Time().
      if (rtcValid && value.isTimeValid())
         {
         SERIAL_STREAM(endl << ">>> Set time to: " << value.timestamp() << "; from: " << time.timestamp() << endl)   // *** DEBUG ***

         if (!value.isDateValid())  // Set a valid date if needed, January 1st, 2001.
            { value = DateTime(2001, 1, 1, value.hour(), value.minute(), value.second()); } 

         // If the year is 2000, set it to 2001 so that the DayOfWeek() calculation works correctly
         if (value.year() == 2000) 
            { value = DateTime(2001, value.month(), value.day(), value.hour(), value.minute(), value.second()); }

         RTC.adjust(value, get_Is12HourFormat()); // Set the time in the RTC
         time = ReadTime();
         }
      else
         { SERIAL_STREAM("*** Invalid RTC / time. " << value.timestamp() << endl) } // *** DEBUG ***
      }

   DateTime BinaryClock::get_Time() const
      {  return time; }

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

   void BinaryClock::set_OnHour(const fl::array<CRGB, NUM_HOUR_LEDS>& value)
      { onHour = value; }

   const fl::array<CRGB, NUM_HOUR_LEDS>& BinaryClock::get_OnHour() const
      { return onHour; }

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
      TimeFormat = value? timeFormat12 : timeFormat24;
      AlarmFormat = value? alarmFormat12 : alarmFormat24;
      RTC.setIs12HourMode(value); // Set the RTC to 12/24 hour mode
      #if DEV_CODE
      SERIAL_STREAM(endl << "Is AM/PM? " << (value? "True" : "False") << "; Formats in use: " << TimeFormat << "; " 
            << AlarmFormat << "; " << time.toString(buffer, sizeof(buffer), TimeFormat) << endl) // *** DEBUG ***
      #endif

      if (value)
         { curHourColor = (time.hour() < 12)? HourColor::Am : HourColor::Pm; }
      else
         { curHourColor = HourColor::Hour24; }
      
      // Only switch colors when the AM Indicator color is Black.
      switchColors = isAmBlack;
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

   bool BinaryClock::RegisterTimeCallback(void (*callback)(DateTime))
      {
      if (timeCallback == nullptr) // Only register if not already registered
         {
         timeCallback = callback;
         callbackTimeEnabled = true;
         return true; // Successfully registered
         }

         return false; // Callback already registered
      }

   bool BinaryClock::UnregisterTimeCallback(void (*callback)(DateTime)) 
      {
      callbackTimeEnabled = false;
      if (timeCallback == callback) // Only unregister if it matches the registered callback
         {
         timeCallback = nullptr;
         return true; // Successfully unregistered
         }

      return false; // Callback not found
      }

   bool BinaryClock::RegisterAlarmCallback(void (*callback)(DateTime)) 
      {
      if (alarmCallback == nullptr) // Only register if not already registered
         {
         alarmCallback = callback;
         callbackAlarmEnabled = true;
         return true; // Successfully registered
         }

      return false; // Callback already registered
      }

   bool BinaryClock::UnregisterAlarmCallback(void (*callback)(DateTime)) 
      {
      callbackAlarmEnabled = false;
      if (alarmCallback == callback) // Only unregister if it matches the registered callback
         {
         alarmCallback = nullptr;
         return true; // Successfully unregistered
         }

      return false; // Callback not found
      }

   bool BinaryClock::TimeDispatch()
      {
      bool result = false;

      if (RTCinterruptWasCalled)
         {
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
                     alarm.status = 0;
                     }
                  }
               else
                  { alarm.fired = false; }

               return alarm.fired;
               };

         CallbackAlarmTriggered = checkAlarm(Alarm2); // Set the alarm callback flag

         uint8_t hour = time.hour();
         HourColor ampmColor = (hour < 12)? HourColor::Am : HourColor::Pm;
         if (isAmBlack && curHourColor != HourColor::Hour24 && curHourColor != ampmColor)
            {
            switchColors = true; // Signal a color switch is needed
            curHourColor = ampmColor;
            }

         RTCinterruptWasCalled = false;
         result = true;
         }

      return result;
      }

   #if FREE_RTOS
   void BinaryClock::TimeTask()
      {
      FOREVER
         {
         TimeDispatch();

         // vTaskDelay to prevent busy waiting
         vTaskDelay(pdMS_TO_TICKS(50));
         }
      }

   void BinaryClock::CallbackTask()
      {
      FOREVER
         {
         CallbackDispatch();

         // vTaskDelay to prevent busy waiting
         vTaskDelay(pdMS_TO_TICKS(50));
         }
      }
   #endif

   void BinaryClock::CallbackDispatch()
      {
      if (callbackTimeEnabled && CallbackTimeTriggered)
         {
         CallbackFtn(CallbackTimeTriggered, get_Time(), timeCallback);
         }

      if (callbackAlarmEnabled && CallbackAlarmTriggered)
         {
         CallbackFtn(CallbackAlarmTriggered, get_Alarm().time, alarmCallback);
         }
      }

   void BinaryClock::CallbackFtn(volatile bool& triggerFlag, DateTime time, void(*callback)(DateTime))
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

   void BinaryClock::PurgatoryTask(const char* message)
      {
      // This is where failure comes to die.
      FastLED.clear(true); // Clear the LEDs.
      #ifdef ESP32_D1_R32_UNO
      HeartbeatLED = 2;
      #else
      HeartbeatLED = LED_BUILTIN;
      #endif
      pinMode(HeartbeatLED, OUTPUT);

      #if SERIAL_OUTPUT
      Serial.println(F("Failure: Unable to continue."));
      if (message != nullptr)
         {
         Serial << F("Message: ") << message << endl << endl;
         }
      Serial.println(F("    CQD - Entering Purgatory..."));
      #endif

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
      // Never use S.O.S. outside an actual emergency.
      //
      // I got schooled by SAR, and now so have you.
      // So we can use a 100 year old alternative: 
      // CQD (-.-. --.- -..) it expands to: Come Quick Distress
      // CQD was used before SOS and is a good alternative for us.
      // So, flash the message: 
      //       CQD NO RTC  -  Come Quick Distress NO Real Time Clock.
      //       -.-. --.- -..  -. ---   .-. - -.-.
      // We can't get out of purgatory without a Real Time Clock
      //
      MorseCodeLED morseCode(HeartbeatLED);
      morseCode.Begin();

      #ifndef UNO_R3
      morseCode.FlashString("CQD");
      delay(1250);
      morseCode.FlashString(message);
      delay(1750);
      #endif

      // Flash CQD NO RTC (Come Quick Distress NO Real Time Clock) to signal the failure.
      SERIAL_OUT_STREAM(F("  C    Q    D     N  O     R   T C ") << endl
                     << F(" [-.-. --.- -..   -. ---   .-. - -.-.] ") << endl
                     << F("(Come Quick Distress NO Real Time Clock)") << endl << endl)

      FOREVER
         {
         morseCode.Flash_CQD_NO_RTC();        
         delay(1950);

         if (RTC.begin())
            {
            resetBoard(); // Reset the board if RTC is available
            }
         }
      }

   const CRGB* BinaryClock::patternLookup(LedPattern patternType)
      { return (patternType < LedPattern::endTAG ? ledPatternsP[(uint8_t)(patternType)] : nullptr); }

   void BinaryClock::DisplayLedPattern(LedPattern patternType)
      {
      const CRGB* pattern = patternLookup(patternType);
      if (pattern != nullptr)
         {
         // memmove(leds, pattern, sizeof(CRGB) * NUM_LEDS);
         for (int i = 0; i < NUM_LEDS; i++)
            {
            leds[i].r = pgm_read_byte(&pattern[i].r);
            leds[i].g = pgm_read_byte(&pattern[i].g);
            leds[i].b = pgm_read_byte(&pattern[i].b);
            }
         FastLED.show();
         }
      }
   
   void BinaryClock::displayLedBuffer(const fl::array<CRGB, NUM_LEDS>& ledBuffer)
      {
      if (ledBuffer.empty()) { return; }

      // Copy the LED buffer to the FastLED display array and display
      memmove(leds, ledBuffer.data(), sizeof(CRGB) * ledBuffer.size());
      FastLED.show();
      }

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
   // Convert values from DEC to BIN format and display

   void BinaryClock::DisplayBinaryTime(int hoursRow, int minutesRow, int secondsRow, bool use12HourMode)
      {
      #if SERIAL_TIME_CODE
         // If SERIAL_TIME_CODE is true, we need to keep track of the binary representation of the time
         #define SET_LEDS(led_num, value, bitmask, on_color, off_color) \
               leds[led_num] = (binaryArray[led_num] = ((value) & (bitmask))) ? on_color : off_color;
      #else
         #define SET_LEDS(led_num, value, bitmask, on_color, off_color) \
               leds[led_num] = ((value) & (bitmask)) ? on_color : off_color;
      #endif
      // Use local variables for the calculations
      uint8_t hourBits, minuteBits, secondBits;
      // Use the (6) bit masks to test for the bits values.
      static const uint8_t bitMasks_P[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
      
      if (use12HourMode)
         {
         hourBits = hoursRow % 12;
         if (hourBits == 0) 
            { hourBits = 12; }
         leds[16] = (hoursRow >= 12) ? PmColor : AmColor;
         hourBits &= 0x0F; // Only need lower 4 bits for 12-hour
         }
      else
         {
         hourBits = hoursRow & 0x1F; // 5 bits for 24-hour
         }
      
      minuteBits = minutesRow & 0x3F; // 6 bits
      secondBits = secondsRow & 0x3F; // 6 bits
      
      // Hours (LEDs 12-15/16, skip LED 16 if in 12-hour mode)
      for (uint8_t i = 0; i < (use12HourMode ? 4 : 5); i++)
         {
         uint8_t ledIndex = HOUR_LED_OFFSET + i;
         SET_LEDS(ledIndex, hourBits, bitMasks_P[i], onColors[ledIndex], offColors[ledIndex]);
         }
      
      // Minutes (LEDs 6-11)
      for (uint8_t i = 0; i < 6; i++)
         {
         uint8_t ledIndex = MINUTE_LED_OFFSET + i;
         SET_LEDS(ledIndex, minuteBits, bitMasks_P[i], onColors[ledIndex], offColors[ledIndex]);
         }
      
      // Seconds (LEDs 0-5)
      for (uint8_t i = 0; i < 6; i++)
         {
         SET_LEDS(i, secondBits, bitMasks_P[i], onColors[i], offColors[i]);
         }
      
      FastLED.show();
      }

   //################################################################################//
   // MELODY ALARM
   //################################################################################//

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
         Serial << F("Time: ") <<  get_Time().toString(buffer, sizeof(buffer), TimeFormat) << F("  Binary: ");

         for (int i = NUM_LEDS - 1; i >= 0; i--)
            {
            if (i == 11 || i == 5) Serial << (" ");
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
      if (buttonDebugTime.get_IsFirstRead() ) // && buttonDebugTime.IsPressedRaw())
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
