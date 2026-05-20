#pragma once
#ifndef __BINARYCLOCK_PROJECT_CONFIG_H__
#define __BINARYCLOCK_PROJECT_CONFIG_H__

/// @file BinaryClock.ProjectConfig.h
/// @brief Shared user project-configuration file include used by multiple libraries.
/// @details
/// - Defines `BINARY_CLOCK_LIB` for builds that include BCGlobalDefines.
/// - Optionally includes `board_select.h` when present so user overrides and
///   board selection are consistently available across libraries.
/// - This file is a wrapper for the `board_select.h` file  `board_select.h`which contains 
///   all the user configuration selections and overrides for the project.
///
/// @par Required by `board_select.h` file
/// The `board_select.h` file requires the following include and defines to be set for
/// the file to compile on its own. The `board_select.h` file needs the most basic 
/// defines to be able to define the pin numbers using the pin names (e.g. A1, A2, etc.) 
/// and the basic definitions of ON and OFF for the buttons depending on how they are wired.
/// The shield has all 3 buttons wired as Common Cathode (CC), so they are all ON when 
/// the pin reads HIGH. If you are using a different shield where the buttons are wired 
/// differently, you can change the S1_ON, S2_ON, and S3_ON defines, in `board_select.h`
/// to match your wiring.
#include <stdint.h>                    /// Integer types: size_t; uint8_t; uint16_t; etc.
#include <pins_arduino.h>              /// Standard Arduino Pin definitions (e.g. A1, A2, etc.).

#ifndef LOW
   #define LOW                    0u   ///< Digital LOW value if not already defined, remove after use to avoid redefinition warnings
   #define UNDEF_LOW            true   ///< Flag to indicate LOW was undefined and is now defined, used to undefine later to avoid conflicts with other libraries.
#endif
#ifndef HIGH
   #define HIGH                   1u   ///< Digital HIGH value if not already defined, remove after use to avoid redefinition warnings
   #define UNDEF_HIGH           true   ///< Flag to indicate HIGH was undefined and is now defined, used to undefine later to avoid conflicts with other libraries.  
#endif

// Constants for Common Anode (CA) and Common Cathode (CC) button wiring
//    A CA wired button is connected HIGH and pulled LOW when pressed.
//    A CC wired button is connected LOW and pulled HIGH when pressed.
#ifndef CA_ON
   #define CA_ON                 LOW   ///< The value when ON  for CA connections
#endif
#ifndef CC_ON
   #define CC_ON                HIGH   ///< The value when ON  for CC connections
#endif
#ifndef CA_OFF
   #define CA_OFF               HIGH   ///< The value when OFF for CA connections
#endif
#ifndef CC_OFF
   #define CC_OFF                LOW   ///< The value when OFF for CC connections
#endif

// USED to define the DEFAULT_12HR_MODE boolean value (below), AMPM_MODE (true) or HR24_MODE (false). 
#define AMPM_MODE               true   ///< AM/PM mode flag value (AM/PM is true)
#define HR24_MODE              false   ///< 24 Hour mode flag value, (24 Hr. is false)

#define ALARM_1                    1   ///< Alarm 1. available on the RTC DS3231, adds seconds.
#define ALARM_2                    2   ///< Alarm 2, the default alarm used by the shield.

#define BINARY_CLOCK_LIB        true   ///< Identifies this as the BinaryClock project (used by other libraries)

// __has_include is C++17 and beyond, or an extension in some compilers.
#ifdef __has_include
   #if __has_include("board_select.h")
      #include "board_select.h"        ///< Include user board selection and overrides when available
   #endif
#else
   #warning "BinaryClock.ProjectConfig.h - Cannot check for 'board_select.h' file (requires C++17 or later), including by default."
   #include "board_select.h"           ///< Include by default when __has_include is unavailable
#endif // __has_include

/// @name Undefine_Temporary_Defines
/// The LOW and HIGH defines are needed for the `board_select.h` file, but they may cause redefinition warnings in 
/// other libraries that also define them. They are undefined to avoid redefinition warnings in other libraries 
/// that also define LOW and HIGH and don't expect them to be redefined earlier.
#ifdef UNDEF_LOW                       // We defined LOW just above for CC/CA_ON/OFF; undefine it to avoid redefinitions
   #undef LOW
   #undef UNDEF_LOW
#endif
#ifdef UNDEF_HIGH                      // We defined HIGH just above for CC/CA_ON/OFF; undefine it to avoid redefinitions
   #undef HIGH
   #undef UNDEF_HIGH
#endif

#endif // __BINARYCLOCK_PROJECT_CONFIG_H__
