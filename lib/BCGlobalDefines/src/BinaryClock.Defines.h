#pragma once
#ifndef __BINARYCLOCK_DEFINES_H__
#define __BINARYCLOCK_DEFINES_H__

// __has_include is C++17 and beyond, or an extension in some compilers.
#ifdef __has_include
   #if __has_include("board_select.h")
      #include "board_select.h"  // Include the user defined board selection file if it exists
   #endif
#else
   #warning "BinaryClock.Defines.h - Cannot check for board_select.h file, including by default."
   #include "board_select.h"     // Include the user defined board selection file, assume it exists   
#endif // __has_include
/// @file BinaryClock.Defines.h  
///###############################################################################################///  
/// @brief  System defines and MACROs, needed as part of the software to run the shield.
/// @verbatim 
///         'Binary Clock Shield for Arduino' by Marcin Saj https://nixietester.com   
///           (https://nixietester.com/products/binary-clock-shield-for-arduino/)   
///  
/// @endverbatim
/// @details This is a large and complex file that contains many defines and MACROs
///          that are used throughout the Binary Clock project. The file is designed
///          to be included in all source files that need access to these defines and MACROs.  
///          The complexity comes from the need to support multiple boards and configurations,
///          while keeping the source files understandable and maintainable.  
///          Many of the MACROs are designed to become whitespace when not included in the
///          configuration/compilation. Boards such as the UNO R3 are limited in memory
///          and resources, so any code that isn't needed must be removed to save memory.  
///          This file is very flexible to allow for simple changes to base defines to alter
///          the code that is compiled. These changes can be made outside this file, 
///          in the `board_select.h` file or another file that is included in `board_select.h`.
/// 
/// This file isn't designed to be modified by the user. Instead, the user should
/// modify/create the file:  
///                         **`board_select.h`**  
/// (https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/board_select.h)  
/// to include the define that identifies the target board for your project.
/// The `board_select.h` file is automatically included at the start of this file if it exists.
/// A sample **`board_select.h`** file is provided that also contains a template for a user's
/// custom UNO board definition. You can also add additional defines there for the project as needed.  
///-----------------------------------------------------------------------------------------------//  
/// Below are partial contents of the `board_select.h` file. The file contains a custom board template:  
///
/// The following are defines for the currently supported boards. One must be used to compile, or
/// create your own CUSTOM_UNO style board definitions for any different UNO style board you have.  
/// @verbatim
///###############################################################################################//
///
/// #define ESP32_D1_R32_UNO   // If defined, the code will use Wemos D1 R32 ESP32 UNO board definitions     (ESP32 WiFi)
/// #define METRO_ESP32_S3     // If defined, the code will use Adafruit Metro ESP32-S3 board definitions    (ESP32 WiFi)
/// #define ESP32_S3_UNO       // If defined, the code will use generic ESP32-S3 UNO board definitions       (ESP32 WiFi)
/// #define UNO_R4_WIFI        // If defined, the code will use Arduino UNO R4 WiFi board definitions        (WiFiS3)
/// #define UNO_R4_MINIMA      // If defined, the code will use Arduino UNO R4 Minima board definitions      (No WiFi)
/// #define UNO_R3             // If defined, the code will use Arduino UNO R3 (ATMEL 328) board definitions (NO WiFi)
///
///###############################################################################################//   
/// @endverbatim
/// Modify the **`board_select.h`** file to uncomment the define for your board.  
/// If your board isn't listed, create your own CUSTOM_UNO style board definitions
/// for any undefined UNO style board you have. The Binary Clock Shield pins are fixed in their
/// positions, however the name of the pins at each location may be named differently on your board.
/// You can't change the pin locations(*) you just modify/define the pin numbers for your board.
///   
/// ================================================================================   
/// @par (*) Hardware Modifications
/// Changing pin locations requires a hardware modification or an UNO development board
/// sandwiched between the UNO board and the shield. see the `README.md` file for details.   
/// (https://github.com/Chris-70/WiFiBinaryClock/blob/main/README.md#hardware-modifications)
///
/// ================================================================================   
/// @par Development Boards 
/// Binary Clock Shield Development Boards were used in the development and testing of this software.
/// They have an OLED display and additional buttons to control the software.  
/// The development boards are not part of the Binary Clock Shield, but used to develop and test the code.
/// The development boards replace/sandwich the Binary Clock Shield during development. Additional code is included
/// to support the development boards, e.g. OLED display, additional buttons, etc. during development
/// `DEVELOPMENT`; `DEV_CODE`; and `DEV_BOARD` are used to control the inclusion of the development code.
/// When compiled for the `Binary Clock Shield` the development code is usually not included.
/// The Debug Time PIN is used to print out the current time over serial monitor (if ON)
/// The debug time and setup pins are used to enable/disable the serial output at runtime.
/// without the need to change the software. The Serial Time is a switch to enable/disable 
/// the serial time display, displays while switch is ON. The Serial Setup is a momentary button
/// to toggle enable/disable the serial setup display. 
/// When the PIN values are -1 (-ve) the associated code is removed.  
///   
/// ================================================================================    
/// @par Serial Output Control
/// The SERIAL_SETUP_CODE and/or SERIAL_TIME_CODE are defined (i.e. true) in order
/// to compile the code and make them available. They can each be set in the
/// software by calling: 'void BinaryClock::set_IsSerialSetup(bool value)' and
/// 'void BinaryClock::set_IsSerialTime(bool value)' methods.
/// The SERIAL_SETUP_CODE is used to control the display of the serial menu to 
/// change the settings of the Binary Clock Shield for Time and Alarm.
/// This can be helpful to users who are just learning how to set the Binary Clock Shield
/// time and/or alarm.

// ##################################################################################### //
/// These methods/functions can be redefined if the definition is placed BEFORE the 
/// #include <BinaryClock.Defines.h> statement in the source file where it is used.
/// Define any of these in the "board_select.h" file as it is included first.
/// Note: These defines are for developers who are modifying/extending the library and
///       need to tailor the `SERIAL_TIME()`; `SERIAL_TIME_STREAM()`; and `SERIAL_SETUP_STREAM()`
///       MACROs for their own code. This can be done by defining them BEFORE this file is included.
// ##################################################################################### //
#ifndef SERIAL_SETUP_TEST
#define SERIAL_SETUP_TEST get_IsSerialSetup()
#endif
#ifndef SERIAL_TIME_TEST
#define SERIAL_TIME_TEST get_IsSerialTime()
#endif
#ifndef SERIAL_TIME_FTN
#define SERIAL_TIME_FTN  serialTime()
#endif

// ##################################################################################### //
// These defines can be overridden by defining them BEFORE this header file is included, 
// usually in the `board_select.h` file which is included at the start of this file.
// The supported boards are defined such that these values are set UNLESS they are overridden.
// ##################################################################################### //
// #define LED_HEART           48   ///< Heartbeat LED to show working software or LED_BUILTIN output.
// #define STL_USED          true   ///< Flag to use the C++ STL library, usually true. 
// #define ESP32_WIFI        true   ///< Flag to include the ESP32 WiFi code.
// #define WIFIS3            false  ///< Flag to include the WIFIS3 code for UNO R4 WiFi.
// #define FREE_RTOS         true   ///< Flag to indicate the FreeRTOS is being used.

/// @addtogroup BoardDefines UNO Board Definitions
/// @{
///################################################################################//  
///             Defines for the different UNO sized boards                         //  
///################################################################################//  
/// @}
/// name ESP32_D1_R32_UNO
/// @ingroup BoardDefines
/// @{
/// Generic AliExpress copy of Wemos D1 R32 ESP32 based UNO board (validate against the board you receive)
/// NOTE: This requires a hardware modification to the board to use the LED on pin 15 instead of pin A3/34.
///       see the 'readme.md' file on GitHub for details: https://github.com/Chris-70/WiFiBinaryClock.
#if defined(ESP32_D1_R32_UNO)      ///< ESP32 Wemos D1 R32 UNO board definitions
   #define ESP32_UNO               ///< Define ESP32_UNO as a common base architecture for all ESP32 UNO boards

   // ESP32 UNO pin definitions
   #define RTC_INT           25   ///< Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             23   ///< The number of the Piezo pin
   #define LED_PIN           15   ///< Data pin that LEDs data will be written out. Requires board modification to use pin 15
                                  ///  You need to modify the board by removing the connector at pin 34 (A3)
                                  ///  Solder a jumper wire from PIN 15 to the LED pin (A3 location) on the shield. 
                                  ///  The Wemos ESP32 UNO PIN 34 is Read-Only and cannot be used for output.

   // Push buttons S1; S2; and S3 connected to the pin numbers equivalent to: A2, A1, A0 Arduino pins
   #define S1                35   ///< A2: S1 button: Time set & Decrement button   
   #define S2                 4   ///< A1: S2 button: Select & Confirm/Save button  
   #define S3                 2   ///< A0: S3 button: Alarm set & Increment button  

   #define ESP32_INPUT_PULLDOWN   INPUT_PULLDOWN   ///< Define for INPUT with an internal pull-down resistor

   #if DEV_BOARD
      #define DEBUG_SETUP_PIN   16   ///< Set to -1 to disable the Serial Setup display control by H/W (CC)
      #define DEBUG_TIME_PIN    27   ///< Set to -1 to disable the Serial Time display control by H/W (CA)
   #else
      #define DEBUG_SETUP_PIN   -1   ///< Set to -1 to disable the Serial Setup display control by H/W (CC)
      #define DEBUG_TIME_PIN    -1   ///< Set to -1 to disable the Serial Time display control by H/W (CA)
   #endif

   #ifndef LED_HEART
      #define LED_HEART         19   ///< Heartbeat LED to show working software and all LED_BUILTIN output.
   #endif
   
/// @}
/// name METRO_ESP32_S3
/// @ingroup BoardDefines
/// @{
///================================================================================//
/// Adafruit Metro ESP32-S3 board (https://www.adafruit.com/product/5500)
///          A very capable UNO style board. See: https://learn.adafruit.com/adafruit-metro-esp32-s3
#elif defined(METRO_ESP32_S3)
   #define ESP32_UNO               // Define ESP32_UNO as a common base architecture for all ESP32 UNO boards

   // Adafruit Metro ESP32-S3 pin definitions
   #define RTC_INT            3   ///< Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             11   ///< The number of the Piezo pin
   #define LED_PIN           A3   ///< Data pin that LEDs data will be written out.

   // Push buttons S1; S2; and S3 connected to the pin numbers equivalent to: A2, A1, A0 Arduino pins
   #define S1                A2   ///< A2: S1 button: Time set & Decrement button   
   #define S2                A1   ///< A1: S2 button: Select & Confirm/Save button  
   #define S3                A0   ///< A0: S3 button: Alarm set & Increment button  

   #define ESP32_INPUT_PULLDOWN   INPUT_PULLDOWN   ///< Define for INPUT with an internal pull-down resistor

   #if DEV_BOARD
      #define DEBUG_SETUP_PIN    5   ///< Set to -1 to disable the Serial Setup display control by H/W (CC)
      #define DEBUG_TIME_PIN     6   ///< Set to -1 to disable the Serial Time display control by H/W (CA)
      #define LED_HEART         12   ///< Heartbeat LED to show working software (Dev+H/W)
   #else
      #define DEBUG_SETUP_PIN   -1   ///< Set to -1 to disable the Serial Setup display control by H/W (CC)
      #define DEBUG_TIME_PIN    -1   ///< Set to -1 to disable the Serial Time display control by H/W (CA)
   #endif
/// @}
/// name ESP32_S3_UNO
/// @ingroup BoardDefines
/// @{
///================================================================================//   
/// Generic AliExpress ESP32-S3 UNO board definitions (validate against the board you receive)
/// These boards use the ESP32-S3 DevKitC-1 pinout definitions, e.g. SDA is pin 8; SCL is pin 9; LED is pin 48, etc.
/// See: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html#hardware-reference
#elif defined(ESP32_S3_UNO)
   #define ESP32_UNO               // Define ESP32_UNO as a common base architecture for all ESP32 UNO boards

   // AliExpress ESP32-S3 UNO pin definitions
   #define RTC_INT           17   ///< Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             11   ///< The number of the Piezo pin
   #define LED_PIN            6   ///< Data pin that LEDs data will be written out.

   // Push buttons S1; S2; and S3 connected to the pin numbers equivalent to: A2, A1, A0 Arduino pins
   #define S1                 7   ///< A2: S1 button: Time set & Decrement button   
   #define S2                 1   ///< A1: S2 button: Select & Confirm/Save button  
   #define S3                 2   ///< A0: S3 button: Alarm set & Increment button  

   #define ESP32_INPUT_PULLDOWN   INPUT_PULLDOWN   // Define for INPUT with an internal pull-down resistor

   #if DEV_BOARD
      #define DEBUG_SETUP_PIN   20   // Set to -1 to disable the Serial Setup display control by H/W (CC)
      #define DEBUG_TIME_PIN     3   // Set to -1 to disable the Serial Time display control by H/W (CA)
      #define LED_HEART         48   // Heartbeat LED to show working software (Dev+H/W)
   #else
      #define DEBUG_SETUP_PIN   -1   // Set to -1 to disable the Serial Setup display control by H/W (CC)
      #define DEBUG_TIME_PIN    -1   // Set to -1 to disable the Serial Time display control by H/W (CA)
   #endif
/// @}
/// name Arduino_Uno
/// @ingroup BoardDefines
/// @{
///================================================================================//
/// Standard Arduino UNO  board definitions for:  
///    UNO R3         (https://store.arduino.cc/products/arduino-uno-rev3)  
///    UNO R4 WiFi    (https://store.arduino.cc/products/uno-r4-wifi)  
///    UNO R4 Minima  (https://store.arduino.cc/products/uno-r4-minima)   
#elif defined(UNO_R3) || defined(UNO_R4_WIFI) || defined(UNO_R4_MINIMA)
   // Arduino UNO based pin definitions (R3 & R4)
   #define RTC_INT            3     ///< Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             11     ///< The number of the Piezo pin
   #define LED_PIN           A3     ///< Data pin that LEDs data will be written out over

   // Push buttons S1; S2; and S3 connected to the: A2, A1, A0 Arduino pins
   #define S1                A2     ///< A2: S1 button: Time set & Decrement button
   #define S2                A1     ///< A1: S2 button: Select & Confirm/Save button
   #define S3                A0     ///< A0: S3 button: Alarm set & Increment button

   #define ESP32_INPUT_PULLDOWN  INPUT   ///< Define for INPUT without an internal pull-down resistor

   #define ESP32_WIFI false         ///< No ESP32 WiFi on any Arduino boards, only WIFIS3 for UNO R4 WIFI.
   #if defined(UNO_R3)
      #define FREE_RTOS    false
      #define STL_USED     false
      #define WIFIS3       false
   #else
      #define FREE_RTOS  true
      #define STL_USED   true
      #if defined(UNO_R4_WIFI)
         #define WIFIS3    false // true    ///< The UNO R4 WiFi board uses WIFIS3.h; similar to ESP32 WiFi but different, no WPS. 
      #endif
   #endif

   #if DEV_BOARD && !defined(UNO_R3)
      #define DEBUG_SETUP_PIN    5  ///< Set to -1 to disable the Serial Setup display control by H/W (CC)
      #define DEBUG_TIME_PIN     6  ///< Set to -1 to disable the Serial Time display control by H/W  (CA)
      #define LED_HEART         12  ///< Heartbeat LED to show working software (Dev+H/W)
   #else
      #define DEBUG_SETUP_PIN   -1   ///< Set to -1 to disable the Serial Setup display control by H/W (CC)
      #define DEBUG_TIME_PIN    -1   ///< Set to -1 to disable the Serial Time display control by H/W (CA)
   #endif
/// @}
/// @{
///================================================================================//
/// If a custom board is defined, then the code will use the custom board definitions.
/// The custom board definitions must be defined in the "board_select.h" file.
/// This custom board is defined as: 'CUSTOM_UNO' which must be defined as true.
/// If no board is defined, then the code will not compile. This is to ensure that a board is defined.
#elif !defined(CUSTOM_UNO) || !(CUSTOM_UNO)
   #pragma message "No supported board defined. Supported boards are:"
   #pragma message "  ESP32_D1_R32_UNO  - generic Wemos D1 R32 UNO with ESP32"
   #pragma message "  METRO_ESP32_S3    - Adafruit Metro ESP32-S3 board"
   #pragma message "  ESP32_S3_UNO      - generic ESP32-S3 UNO board"
   #pragma message "  UNO_R4_WIFI       - Arduino UNO R4 WiFi board"
   #pragma message "  UNO_R4_MINIMA     - Arduino UNO R4 Minima board"
   #pragma message "  UNO_R3            - Arduino UNO R3 board"
   #pragma message "  CUSTOM_UNO        - custom board with defined pin numbers in board_select.h"
   #pragma message "Please define one of the above boards or create CUSTOM_UNO to compile the code."
   #error "Undefined board. Please define the pin numbers for your board."
   #include <NoBoardDefinition_StopCompilationNow> // Include a dummy header file to stop compilation
#endif
/// @}

//################################################################################//  
/// Defines for WiFi and FreeRTOS support based on the selected board.
/// These can be overridden by defining them in the "board_select.h" file.
#if defined(CUSTOM_UNO) && CUSTOM_UNO
   /// For a custom UNO board, assume no WiFi and no internal pullup resistors unless defined. 
   /// We will assume a minimum capability greater than an UNO R3 board unless otherwise defined.
   #ifndef ESP32_WIFI
      #define ESP32_WIFI false         ///< Assume no WiFi capability if not defined.
   #endif
   #ifndef WIFIS3
      #define WIFIS3          false    ///< Assume no WIFIS3 capability if not defined.
   #endif
   #ifndef STL_USED
      #define STL_USED        true     ///< Assume STL support if not defined.
   #endif
   #ifndef FREE_RTOS
      #define FREE_RTOS       true     ///< Assume FreeRTOS support if not defined.
   #endif
   #ifndef PRINTF_OK
      #define PRINTF_OK       true     ///< Use printf style code if STL is being used.
   #endif
   #ifndef ESP32_INPUT_PULLDOWN
      #define ESP32_INPUT_PULLDOWN  INPUT   /// Define for INPUT without an internal pull-down resistor
   #endif
#elif UNO_R3
   /// Not enough resources on the UNO R3 board to use the development board code.
   /// The UNO R3 board doesn't have the resources to include the code for an
   /// OLED display (on the development board) in addition to the code for the 
   /// Binary Clock Shield.
   #undef DEV_BOARD
   #undef DEV_CODE
   #define SERIAL_TIME_CODE   false
   #define PRINTF_OK          false
#else
   /// For all other supported boards, assume STL, FreeRTOS and WiFi support unless otherwise defined.
   #ifndef STL_USED
      #define STL_USED        true     ///< Use the STL library if not using the UNO R3 board.
   #endif
   #ifndef FREE_RTOS
      #define FREE_RTOS       true     ///< Assume all other supported boards have FreeRTOS support.
   #endif
   #ifndef PRINTF_OK
      #define PRINTF_OK       true     ///< Use printf style code if STL is being used.
   #endif
   #ifndef WIFIS3
      #define WIFIS3          false    ///< Assume no WIFIS3 capability for Arduino boards if not defined.
   #endif
   #ifndef ESP32_WIFI
      #define ESP32_WIFI      true     ///< Assume all other supported boards have WiFi capability.
   #endif
#endif // #if defined(CUSTOM_UNO) && !CUSTOM_UNO

#if ESP32_WIFI && WIFIS3
   #error "Both ESP32_WIFI and WIFIS3 cannot be true at the same time. Please check your board definitions."
   #include <MultipleWiFiDefinitions_StopCompilationNow> // Include a dummy header file to stop compilation
#endif

#ifndef WIFI
   #define WIFI (ESP32_WIFI || WIFIS3)   ///< WiFi is used if either ESP32_WIFI or WIFIS3 is true.
#endif

//################################################################################//  
/// Defines to control the inclusion/removal of development board code.
#ifndef DEV_BOARD
   #define DEV_BOARD false    ///< If DEV_BOARD hasn't been defined, don't include code for the development board
#elif DEV_BOARD
   #define DEV_CODE  true     ///< If using a development board, include the development code
#endif
#ifndef DEV_CODE
   #define DEV_CODE false     ///< If DEV_CODE hasn't been defined, don't include the development code
#endif

#if DEV_CODE
   #ifndef SERIAL_SETUP_CODE
      #define SERIAL_SETUP_CODE     true  ///< If using a development board/code, include the Serial Setup code
   #endif
   #ifndef SERIAL_TIME_CODE
      #define SERIAL_TIME_CODE      true  ///< If using a development board/code, include the Serial Time  code
   #endif
#endif
/// Unless we have a development board, remove all hardware development board related items.
/// This ensures that no development board code is included when not using a development board.
#if !DEV_BOARD
   #undef DEBUG_SETUP_PIN
   #undef DEBUG_TIME_PIN
   #define DEBUG_SETUP_PIN   -1   ///< No development board, so no H/W debug setup
   #define DEBUG_TIME_PIN    -1   ///< No development board, so no H/W debug time
#endif

/// This determines if the menu and/or time are also displayed on the serial monitor.
/// - If SERIAL_SETUP_CODE is defined, code to display the serial menu is included in the project.
/// - If SERIAL_TIME_CODE  is defined, code to display the serial time, every second, is included in the project.
#ifndef SERIAL_SETUP_CODE
   #define SERIAL_SETUP_CODE    true   ///< If (true) - serial setup code included, (false) - code removed
#endif
#ifndef SERIAL_TIME_CODE
   #define SERIAL_TIME_CODE     true   ///< If (true) - serial time  code included, (false) - code removed
#endif
#ifndef SERIAL_OUTPUT
   #define SERIAL_OUTPUT (SERIAL_SETUP_CODE || SERIAL_TIME_CODE) ///< If (true) - Allow serial output messages.
#endif

/// Defines for the hardware development board to control software output from the hardware.
/// This controls the inclusion/removal of the code to support hardware buttons/switches to also control the serial output.
/// The serial output can always be controlled in software if the SERIAL_xxxx_CODE is defined (true).
#define HW_DEBUG_SETUP ((DEBUG_SETUP_PIN >= 0) && (SERIAL_SETUP_CODE))  ///< Include code to support H/W to control setup display
#define HW_DEBUG_TIME  ((DEBUG_TIME_PIN  >= 0) && (SERIAL_TIME_CODE))   ///< Include code to support H/W to control time  display
#define HARDWARE_DEBUG (HW_DEBUG_SETUP ||  HW_DEBUG_TIME)
#define DEVELOPMENT    (DEV_BOARD || DEV_CODE) 

//#####################################################################################//  

#include "SerialOutput.Defines.h"      // For all the serial output macros.

//#####################################################################################//  
/// Wrapper macro for single serial print statements to check the
/// `IsSerialSetup` properety flag for printing menu items and eleminates the
/// need to surrond the statements with `#if SERIAL_SETUP_CODE...#endif`
/// directives. Don't use for multiple statements as it unnecessarly
/// adds an `if ()` statement to each line, use conditional compilation `#if ...` instead.
#if SERIAL_SETUP_CODE
   #define SERIAL_SETUP_STREAM(CMD_STRING) \
         if (SERIAL_SETUP_TEST) { SERIAL_STREAM_MACRO(CMD_STRING) }
#else
   // Replace the macro with whitespace.
   #define SERIAL_SETUP_STREAM(CMD_STRING)
#endif

/// Wrapper macro for single serial print statements to check the
/// `IsSerialTime` properety flag for printing and eleminates the
/// need to surrond the statements with `#if SERIAL_TIME_CODE...#endif`
/// directives. Don't use for multiple statements as it unnecessarly
/// adds an `if ()` statement to each line
/// For multiple lines use conditional compilation `#if ... #endif` instead.
#if SERIAL_TIME_CODE
   #define SERIAL_TIME() \
         if (SERIAL_TIME_TEST) { SERIAL_PRINT("SerialTime() - ") SERIAL_TIME_FTN; }
   #define SERIAL_TIME_STREAM(CMD_STRING) \
         if (SERIAL_TIME_TEST) { SERIAL_STREAM_MACRO(CMD_STRING) }
#else
   // Replace the macros with whitespace.
   #define SERIAL_TIME()
   #define SERIAL_TIME_STREAM(CMD_STRING)
#endif

#if FREE_RTOS
   #define BINARYCLOCK_DELAY_MS(ms) vTaskDelay(ms / portTICK_PERIOD_MS)
   #define SEC_TO_TICKS(seconds) pdMS_TO_TICKS((TickType_t)((seconds)*1000U))
#else
   #define BINARYCLOCK_DELAY_MS(ms) delay(ms)
#endif

//#####################################################################################//  
/// General defines for the Binary Clock Shield display layout.
// The display layout of the LEDs on the shield, one row each.
#ifndef DISPLAY_LAYOUT
   #define DISPLAY_LAYOUT    ///< Define the display layout of the Binary Clock Shield
   #define NUM_HOUR_LEDS         5         ///< The LEDs on the top row of the shield.
   #define NUM_MINUTE_LEDS       6         ///< The LEDs on the middle row of the shield.
   #define NUM_SECOND_LEDS       6         ///< The LEDs on the bottom row of the shield.
#endif
#ifndef NUM_LEDS
   #define NUM_LEDS (NUM_HOUR_LEDS + NUM_MINUTE_LEDS + NUM_SECOND_LEDS)
#endif
#ifndef DISPLAY_OFFSETS
   #define HOUR_LEDS_OFFSET      (NUM_SECOND_LEDS + NUM_MINUTE_LEDS)
   #define MINUTE_LEDS_OFFSET    (NUM_SECOND_LEDS)
   #define SECOND_LEDS_OFFSET    0
#endif
#define NUM_ROWS                 3         ///< The number of LED time display rows on the shield.
// Define the number of LEDs in each row, can be overridden if needed.
// This allows for different LED configurations if needed.
// If the rows have more LEDs than needed, the extra LEDs will be unused (OFF).
#ifndef PHYSICAL_LAYOUT
   #define HOUR_ROW_LEDS         NUM_HOUR_LEDS    ///< The number of hour LEDs in the hour row.
   #define MINUTE_ROW_LEDS       NUM_MINUTE_LEDS  ///< The number of minute LEDs in the minute row.
   #define SECOND_ROW_LEDS       NUM_SECOND_LEDS  ///< The number of second LEDs in the second row.
#endif
#ifndef PHYSICAL_OFFSETS
   #define HOUR_ROW_OFFSET       (SECOND_ROW_LEDS + MINUTE_ROW_LEDS)
   #define MINUTE_ROW_OFFSET     (SECOND_ROW_LEDS)
   #define SECOND_ROW_OFFSET     0
#endif
// This is to account for the physical layout beyond the 3 rows for the time display.
// This is used in the `FastLED` setup for the total buffer size. For example to
// display the time on an 8x8 matrix, the total LEDs would be 64, even though only
// 17 LEDs are used for the time display and each row is 8 LEDs long.
#ifndef TOTAL_LEDS
   #define TOTAL_LEDS     (HOUR_ROW_LEDS + MINUTE_ROW_LEDS + SECOND_ROW_LEDS)
#endif
// Masks for the binary display of the time components.
#define HOUR_MASK_24      0x1F         ///< Mask for the 24 hour format (5 bits)
#define HOUR_MASK_12      0x0F         ///< Mask for the 12 hour format (4 bits)
#define MINUTE_MASK       0x3F         ///< Mask for the minutes (6 bits)
#define SECOND_MASK       0x3F         ///< Mask for the seconds (6 bits)

#define LED_TYPE           WS2812B     ///< Datasheet: http://bit.ly/LED-WS2812B
#define COLOR_ORDER          GRB       ///< For color ordering use this sketch: http://bit.ly/RGBCalibrate   

#define ALARM_1                    1   ///< Alarm 1. available on the RTC DS3231, adds seconds.
#define ALARM_2                    2   ///< Alarm 2, the default alarm used by the shield.

#define AMPM_MODE               true   ///< AM/PM mode flag value
#define HR24_MODE              false   ///< 24 Hour mode flag value
#define MAX_BUFFER_SIZE           64   ///< Maximum size of temporary buffers.
#define MAX_DISPLAY_PAUSE      60000   ///< Maximum display pause time in ms (1 minute).

//#####################################################################################//  
//              DEFAULT values for the Binary Clock Shield settings.
// These can be overridden by defining them BEFORE this header file is included,
//#####################################################################################//  

/// - The DEFAULT_SERIAL_SETUP and DEFAULT_SERIAL_TIME values are used to determine if the serial 
///                 Setup and/or Time messages are displayed initially or not.
///                 use the public methods 'set_IsSerialSetup()' and 'set_IsSerialTime()' to
///                 enable/disable the serial output at runtime. H/W buttons, if defined, 
///                 can also be used to enable/disable the serial output at runtime.
#ifndef DEFAULT_SERIAL_SETUP
   #define DEFAULT_SERIAL_SETUP    true   ///< Initial serial setup display value (e.g. allow the serial setup to be displayed).
#endif
#ifndef DEFAULT_SERIAL_TIME
   #define DEFAULT_SERIAL_TIME    false   ///< Initial serial time display value  (e.g. no continious serial time display at startup).
#endif

/// The delay, in ms, is set to a high value when using a momentary button so the button can be release quickly and the user will
/// still see the serial output. When using a switch the delay can be short as the user won't need to keep pressing a button.
#ifndef DEFAULT_DEBUG_OFF_DELAY
   #define DEFAULT_DEBUG_OFF_DELAY 3000 
#endif

#ifndef DEFAULT_DEBOUNCE_DELAY
   #define DEFAULT_DEBOUNCE_DELAY    75   ///< The default debounce delay in milliseconds for the buttons
#endif
#ifndef DEFAULT_BRIGHTNESS
   #define DEFAULT_BRIGHTNESS        30   ///< The best tested LEDs brightness range: 20-60
#endif
#ifndef DEFAULT_ALARM_REPEAT
   #define DEFAULT_ALARM_REPEAT       3   ///< How many times to play the melody alarm
#endif

#ifndef DEFAULT_TIME_MODE
   #define DEFAULT_TIME_MODE  AMPM_MODE  ///< Default time mode when not defined.
#endif
#ifndef DEFAULT_TIME_FORMAT
   #define DEFAULT_TIME_FORMAT   "HH:mm:ss AP"  ///< Default time  format when not defined.
#endif
#ifndef DEFAULT_ALARM_FORMAT
   #define DEFAULT_ALARM_FORMAT  "HH:mm AP"     ///< Default alarm format when not defined.
#endif
//#####################################################################################//  

/// @addtogroup DefinesDS3231 Defines for the DS3231 RTC registers and bit masks
/// @{
//#####################################################################################//
// Bit numbers for DS3231 RTC registers and register numbers
// ----------------------------------------------------------------------------------- //
// The DS3231 & DS3232 are very similar, so we can use the same code for both.
// The #defines that start with DS3232_ are for the DS3232 chip only and have the
// bit values set to 0 on the DS3231 chip. The OSF bit on the DS3231 is a status bit
// to signal the oscillator has stopped and can only be cleared, while on the DS3232 
// it is a control bit to enable/disable the oscillator. The DS3232 has a selectable
// temperature conversion rate, while the DS3231 has a fixed 10 seconds conversion rate.
// on battery. On VCC, both chips have a 1 second conversion rate.
// From: (https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231.pdf)
//       (https://www.analog.com/media/en/technical-documentation/data-sheets/DS1307.pdf)
//=====================================================================================//
/// @name DS3231 RTC register numbers:
#define DS3231_TIME                 0x00  ///< Time register start (0x00 - 0x06)
#define DS3231_SECONDS              0x00  ///< Seconds register address for DS3231
#define DS3231_MINUTES              0x01  ///< Minutes register address for DS3231
#define DS3231_HOUR                 0x02  ///< Hour register address for DS3231
#define DS3231_DAY                  0x03  ///< Day register address for DS3231
#define DS3231_DATE                 0x04  ///< Date register address for DS3231
#define DS3231_MONTH                0x05  ///< Month register address for DS3231
#define DS3231_YEAR                 0x06  ///< Year register address for DS3231
#define DS3231_ALARM1               0x07  ///< Alarm 1 register start (0x07 - 0x0A)
#define DS3231_ALARM1_SECONDS       0x07  ///< Alarm 1 seconds register address for DS3231
#define DS3231_ALARM1_MINUTES       0x08  ///< Alarm 1 minutes register address for DS3231
#define DS3231_ALARM1_HOUR          0x09  ///< Alarm 1 hour register address for DS3231
#define DS3231_ALARM1_DAY_DATE      0x0A  ///< Alarm 1 day/date register address for DS3231
#define DS3231_ALARM2               0x0B  ///< Alarm 2 register start (0x0B - 0x0E)
#define DS3231_ALARM2_MINUTES       0x0B  ///< Alarm 2 minutes register address for DS3231
#define DS3231_ALARM2_HOUR          0x0C  ///< Alarm 2 hour register address for DS3231
#define DS3231_ALARM2_DAY_DATE      0x0D  ///< Alarm 2 day/date register address for DS3231
#define DS3231_CONTROL              0x0E  ///< Control register
#define DS3231_STATUSREG            0x0F  ///< Status register
#define DS3231_AGING_OFFSET         0x10  ///< Aging offset register
#define DS3231_TEMPERATUREREG       0x11  ///< Temperature register start (0x11 MSB) - 0x12 (LSB)); scaled(2) 10 bit 2's complement
#define DS3231_TEMPERATURE_MSB      0x11  ///< Temperature MSB register, integer part (-128->+127 C), integer  8 bit 2's complement,
#define DS3231_TEMPERATURE_LSB      0x12  ///< Temperature LSB register, Fractional part (bits 7, 6): 0.00; 0.25; 0.50; 0.75;
/// @name DS3231 Bit Numbers and Masks:
#define DS3231_CONTROL_A1IE_MASK    0x01  ///< Bit 0: Alarm 1 Interrupt Enable bit in control register
#define DS3231_CONTROL_A2IE_MASK    0x02  ///< Bit 1: Alarm 2 Interrupt Enable bit in control register
#define DS3231_CONTROL_INTCN_MASK   0x04  ///< Bit 2: Interrupt Control bit in control register
#define DS3231_CONTROL_RS1_MASK     0x08  ///< Bit 3: Rate Select bit 1 in control register (1 Hz; 1K)
#define DS3231_CONTROL_RS2_MASK     0x10  ///< Bit 4: Rate Select bit 2 in control register (4K  ; 8K)
#define DS3231_CONTROL_RATE_MASK    0x18  ///< Bits 3-4: Rate Select bits in control register (1 Hz; 1K; 4K; 8K)
#define DS3231_CONTROL_CONV_MASK    0x20  ///< Bit 5: Force Temperature Conversion (STATUS BSY flag must be 0)
#define DS3231_CONTROL_BBSQW_MASK   0x40  ///< Bit 6: Battery Backed Square Wave bit in control register
#define DS3231_CONTROL_EOSC_MASK    0x80  ///< Bit 7: Enable Oscillator bit in control register
#define DS3231_CONTROL_SQWMODE_MASK 0x1C  ///< Bits 2-4: Square Wave output mode bits in control register
#define DS3231_STATUS_A1F_MASK      0x01  ///< Bit 0: Alarm 1 Flag bit in status register
#define DS3231_STATUS_A2F_MASK      0x02  ///< Bit 1: Alarm 2 Flag bit in status register
#define DS3231_STATUS_BSY_MASK      0x04  ///< Bit 2: Busy Flag bit, temperature conversion (TCXO), in status register
#define DS3231_STATUS_EN32KHZ_MASK  0x08  ///< Bit 3: Enable 32kHz output bit in status register
#define DS3232_STATUS_CRATE0_MASK   0x10  ///< Bit 4: DS3232: Conversion Rate Select bit 0 (64,  256) in status register
#define DS3232_STATUS_CRATE1_MASK   0x20  ///< Bit 5: DS3232: Conversion Rate Select bit 1 (128, 512) in status register
#define DS3232_STATUS_BB32KHZ_MASK  0x40  ///< Bit 6: DS3232: Battery Backed 32kHz output bit in status register
#define DS3231_STATUS_OSF_MASK      0x80  ///< Bit 7: DS3231: Oscillator Stopped Flag bit in status register
#define DS3232_STATUS_EOSF_MASK     0x80  ///< Bit 7: DS3232: Enable Oscillator bit in status register
#define DS3231_ALARM1_STATUS_MASK   0x01  ///< Bit 1: Alarm 1 status Flag mask in control register
#define DS3231_ALARM2_STATUS_MASK   0x02  ///< Bit 2: Alarm 2 status Flag mask in control register
#define DS3231_ALARM1_FLAG_MASK     0x01  ///< Bit 1: Alarm 1 alarm triggered Mask in status register
#define DS3231_ALARM2_FLAG_MASK     0x02  ///< Bit 2: Alarm 2 alarm triggered Mask in status register
#define DS3231_ALARM1_DAY_DATE_MASK 0x80  ///< Bit 7: Alarm 1 day/date Flag bit in alarm register DAY/Date (0x0A)
#define DS3231_ALARM2_DAY_DATE_MASK 0x80  ///< Bit 7: Alarm 2 day/date Flag bit in alarm register DAY/Date (0x0D)
#define DS3231_ALARM1_A1M1_MASK     0x80  ///< Bit 7: Alarm 1 A1M1 Flag bit in alarm 1 register Seconds (0x07)
#define DS3231_ALARM1_A1M2_MASK     0x80  ///< Bit 7: Alarm 1 A1M2 Flag bit in alarm 1 register Minutes (0x08)
#define DS3231_ALARM1_A1M3_MASK     0x80  ///< Bit 7: Alarm 1 A1M3 Flag bit in alarm 1 register Hours (0x09)
#define DS3231_ALARM1_A1M4_MASK     0x80  ///< Bit 7: Alarm 1 A1M4 Flag bit in alarm 1 register Day (0x0A)
#define DS3231_ALARM1_A2M2_MASK     0x80  ///< Bit 7: Alarm 2 A2M2 Flag bit in alarm 2 register Minutes (0x0B)
#define DS3231_ALARM1_A2M3_MASK     0x80  ///< Bit 7: Alarm 2 A2M3 Flag bit in alarm 2 register Hours (0x0C)
#define DS3231_ALARM1_A2M4_MASK     0x80  ///< Bit 7: Alarm 2 A2M4 Flag bit in alarm 2 register Day (0x0D)
#define DS3231_CENTURY_MASK         0x80  ///< Bit 7: Century bit in month register (0x05)
#define DS3231_TEMP_LSB_MASK        0xC0  ///< Bit 7-6: Temperature LSB mask for DS3231, fractional part.
/// @name Time reading masks for most DS chips:
#define DS_SECONDS_MASK             0x7F  ///< Mask for seconds registers (0x00; 0x07)
#define DS_MINUTES_MASK             0x7F  ///< Mask for minutes registers (0x01; 0x08; 0x0B)
#define DS_HOUR_REG_MASK            0x7F  ///< Mask for all hour bits reg.(0x02; 0x09; 0x0C)
#define DS_HOUR_12_24_MASK          0x40  ///< Bit 6: 12/24 hour mode bit in hour registers
#define DS_HOUR_PM_MASK             0x20  ///< Bit 5: PM bit in hour registers (0x02; 0x09; 0x0C)
#define DS_HOUR24_MASK              0x3F  ///< Mask for 24 hours value    (e.g. 0x02; 0x09; 0x0C) - 24 hour mode
#define DS_HOUR12_MASK              0x1F  ///< Mask for 12 hours value    (e.g. 0x02; 0x09; 0x0C) - 12 hour mode
#define DS_DAY_MASK                 0x07  ///< Mask for day registers     (e.g. 0x03; 0x0A; 0x0D)
#define DS_DATE_MASK                0x3F  ///< Mask for date registers    (e.g. 0x04; 0x0A; 0x0D)
#define DS_MONTH_MASK               0x1F  ///< Mask for month registers   (e.g. 0x05)
#define DS_YEAR_MASK                0xFF  ///< Mask for year registers    (e.g. 0x06)
/// @}
#endif // _BinaryClock_Defines_h_