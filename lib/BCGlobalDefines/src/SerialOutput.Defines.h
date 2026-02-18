#pragma once
#ifndef __SERIALOUTPUT_DEFINES_H__
#define __SERIALOUTPUT_DEFINES_H__

#include <Streaming.h>           /// Streaming serial output with `operator<<` (https://github.com/janelia-arduino/Streaming)

/// @file SerialOutput.Defines.h
/// @brief Defines for controlling serial output statements.
/// @details This file contains the MACRO definitions to control the serial output
///          statements used for debugging, development and general output purposes. 
///          When the controlling defines are `false` the effect is that the 
///          code is removed from compilation, not just disabled. This reduces the
///          final code size. 
/// @remarks This file is included in the `BinaryClock.Defines.h` file where the
///          defines are set based on the board and other conditions.
/// @note    To include this file outside of `BinaryClock.Defines.h`, the controlling
///          defines must be set BEFORE including this file:
/// @verbatim 
///################################################################################//
/// The following defines control the output:
/// =========================================
/// #define SERIAL_OUTPUT   true  // true to enable; false to disable
/// #define DEV_CODE        true  // true to enable; false to disable
/// #define DEBUG_OUTPUT    true  // true to enable; false to disable
/// #define PRINTF_OK       true  // true to enable; false to disable
///################################################################################//
/// @endverbatim

#define FOREVER while(true)            ///< Infinite loop, e.g. used in task methods.

#ifndef PRINTF_OK
   #define PRINTF_OK       true        ///< If PRINTF_OK hasn't been defined, assume printf is available.
#endif

// ##################################################################################### //
/// These methods/functions can be redefined if the definition is placed BEFORE the 
/// #include <BinaryClock.Defines.h> statement in the source file where it is used.
/// Define any of these in the "board_select.h" file as it is included first.
/// Note: These defines are for developers who are modifying/extending the library and
///       need to tailor the `SERIAL_TIME()`; `SERIAL_TIME_STREAM()`; and `SERIAL_SETUP_STREAM()`
///       MACROs for their own code. This can be done by defining them BEFORE this file is included.
// ##################################################################################### //
#ifndef SERIAL_SETUP_TEST
   #define SERIAL_SETUP_TEST BinaryClock::get_Instance().get_IsSerialSetup()
#endif
#ifndef SERIAL_TIME_TEST
   #define SERIAL_TIME_TEST BinaryClock::get_Instance().get_IsSerialTime()
#endif
#ifndef SERIAL_TIME_FTN
   #define SERIAL_TIME_FTN  BinaryClock::get_Instance().serialTime()
#endif

//#####################################################################################//  
/// These output statements are defined as MACROs to simplify the code.
/// This avoids surrounding the code with #if `__defined__` directives. An additional
/// advantage is that the code is not compiled at all if `__defined__` is false or undefined.  
/// This code contains output statements that are used for different development and debugging
/// purposes. The output statements can be enabled/disabled by defining the appropriate
/// macros.
/// Note: These MACROs include the ';' semicolon ';' so it isn't included in the code.
///       This is required to avoid empty statements (i.e. ';') when the code is removed.
///       It is also an indicator/reminder to the developer that this code might be removed.
/// 
/// The base macros are defined first, then the higher level macros are defined that
/// use the base macros. This allows for more flexibility in controlling the output.
/// The output commands can be changed here if needed, e.g. to change from Serial to another output method.
#if DEV_CODE || DEBUG_OUTPUT || SERIAL_OUTPUT
   #define SERIAL_PRINT_MACRO(STRING) Serial.print(STRING);
   #define SERIAL_PRINTLN_MACRO(STRING) Serial.println(STRING);
   #define SERIAL_STREAM_MACRO(CMD_STRING) Serial << CMD_STRING;
   #if PRINTF_OK
      #define SERIAL_PRINTF_MACRO(FORMAT, ...) Serial.printf(FORMAT, __VA_ARGS__);
   #else
      #define SERIAL_PRINTF_MACRO(FORMAT, ...)
   #endif
#else
   // Removes the code from compilation, replaced with whitespace.
   #define SERIAL_PRINT_MACRO(STRING)
   #define SERIAL_PRINTLN_MACRO(STRING)
   #define SERIAL_STREAM_MACRO(CMD_STRING)
   #define SERIAL_PRINTF_MACRO(FORMAT, ...)
#endif

/// These output MACROs are available for all general serial output.
/// SERIAL_OUTPUT is defined true if either SERIAL_SETUP_CODE or SERIAL_TIME_CODE is true.
#if SERIAL_OUTPUT
   #define SERIAL_OUT_PRINT(STRING) SERIAL_PRINT_MACRO(STRING)
   #define SERIAL_OUT_PRINTLN(STRING) SERIAL_PRINTLN_MACRO(STRING)
   #define SERIAL_OUT_STREAM(CMD_STRING) SERIAL_STREAM_MACRO(CMD_STRING)
   #define SERIAL_OUT_PRINTF(FORMAT, ...) SERIAL_PRINTF_MACRO(FORMAT, __VA_ARGS__)
#else
   // Replace the macros with whitespace.
   #define SERIAL_OUT_PRINT(STRING)
   #define SERIAL_OUT_PRINTLN(STRING)
   #define SERIAL_OUT_STREAM(CMD_STRING)
   #define SERIAL_OUT_PRINTF(FORMAT, ...)
#endif

/// Debugging and development only output statements.
/// These output MACROs are only defined if DEV_CODE is true.
/// This allows for debugging output to be included in the code
/// only during development, but removed from the final code. 
#if DEV_CODE
   #define SERIAL_PRINT(STRING) SERIAL_PRINT_MACRO(STRING)
   #define SERIAL_PRINTLN(STRING) SERIAL_PRINTLN_MACRO(STRING)
   #define SERIAL_STREAM(CMD_STRING) SERIAL_STREAM_MACRO(CMD_STRING)
   #define SERIAL_PRINTF(FORMAT, ...) SERIAL_PRINTF_MACRO(FORMAT, __VA_ARGS__)
#else
   // Replace the macros with whitespace.
   #define SERIAL_PRINT(STRING)
   #define SERIAL_PRINTLN(STRING)
   #define SERIAL_STREAM(CMD_STRING)
   #define SERIAL_PRINTF(FORMAT, ...)
#endif

/// Temporary debug output statements controlled by the DEBUG_OUTPUT define.
/// These are defined for temporary debug statements that are never
/// included in code that is released. The `DEBUG_...` statements should
/// be removed from the code BEFORE committing to the repo.
#if DEBUG_OUTPUT
   #define DEBUG_PRINT(STRING) SERIAL_PRINT_MACRO(STRING)
   #define DEBUG_PRINTLN(STRING) SERIAL_PRINTLN_MACRO(STRING)
   #define DEBUG_STREAM(CMD_STRING) SERIAL_STREAM_MACRO(CMD_STRING)
   #define DEBUG_PRINTF(FORMAT, ...) SERIAL_PRINTF_MACRO(FORMAT, __VA_ARGS__)
#else
   // Replace the macros with whitespace.
   #define DEBUG_PRINT(STRING)
   #define DEBUG_PRINTLN(STRING)
   #define DEBUG_STREAM(CMD_STRING)
   #define DEBUG_PRINTF(FORMAT, ...)
#endif

#endif // __SERIALOUTPUT_DEFINES_H__
