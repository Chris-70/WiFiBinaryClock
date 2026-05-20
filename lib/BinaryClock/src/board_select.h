/// @file board_select.h
/// @brief This file is used to select the target board or create your own custom board defines for the 
///        Binary Clock Shield library.
/// @details This file is included by the `BinaryClock.Defines.h` file.
///          The file contains a template for creating your own custom UNO style board definitions.
///          You can also uncomment one of the predefined supported board definitions to use it.
///          The Binary Clock Shield uses fixed pin locations for the shield connections, however
///          the pin names may differ on different boards. You need to define the pin numbers
///          for your board at the same physical locations as the shield connections.
/// @verbatim 
/// #####################################################################################################################
/// The following are defines for the currently supported boards. One must be used to compile, or create a CUSTOM_UNO
/// style board definitions for any different UNO style board you have.  
/// Uncomment one of the defined boards below to use it, or `#define CUSTOM_UNO true` and set the pin numbers for your board.
/// 
/// #define ESP32_D1_R32_UNO   // If defined, the code will use Wemos D1 R32 ESP32 UNO board definitions     (ESP32 WiFi)
/// #define METRO_ESP32_S3     // If defined, the code will use Adafruit Metro ESP32-S3 board definitions    (ESP32 WiFi)
/// #define METRO_ESP32_S2     // If defined, the code will use Adafruit Metro ESP32-S2 board definitions    (ESP32 WiFi)
/// #define ESP32_S3_UNO       // If defined, the code will use generic ESP32-S3 UNO board definitions       (ESP32 WiFi)
/// #define UNO_R4_WIFI        // If defined, the code will use Arduino UNO R4 WiFi board definitions        (WiFiS3)
/// #define UNO_R4_MINIMA      // If defined, the code will use Arduino UNO R4 Minima board definitions      (No WiFi)
/// #define UNO_R3             // If defined, the code will use Arduino UNO R3 (ATMEL 328) board definitions (NO WiFi)
///
/// #####################################################################################################################
/// @endverbatim 
/// @addtogroup BoardDefines UNO Board Definitions
/// @name Custom UNO Boards 
/// @{
/// For boards that don't have predefined board settings in BinaryClock.defines.h, create your own.

/// The Binary Clock Shield pins are fixed in their positions, however the name of the pins at
/// each location may be named differently on your board. You can't change the pin locations(*) you just
/// modify/define the pin numbers for your board in the section below for CUSTOM_UNO. This example has
/// the pin assignment for the original Arduino UNO boards sold by Arduino. You'll need to map physical
/// pin locations to the pin numbers used on your board at the same locations. You will also need
/// to indicate if the board is running FreeRTOS and has WiFi or not. Every predefined supported board
/// uses an ESP32 chip of some sort for WiFi and they run FreeRTOS by default.   
/// (*) You can change the pin locations by using an UNO prototype shield, see `@remarks` (*) below.  
/// --------------------------------------------------------------------------------------  
/// If you are using a supported board, set "#define CUSTOM_UNO false" or comment it out.
/// 

// #define CUSTOM_UNO true    ///< This must be defined as true to use the CUSTOM_UNO definitions

/// @name CUSTOM_UNO
/// 
/// Add your own UNO style board definitions below for any different UNO style board you have.
/// Define your CUSTOM_UNO target board here.
#if defined(CUSTOM_UNO) && CUSTOM_UNO
   // Arduino UNO based pin definitions (R3 & R4) Modify for your board.
   #define RTC_INT            3   ///< Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             11   ///< The number of the Piezo pin
   #define LED_DATA_PIN      A3   ///< Data Out pin that the LED data will be written to.

   // Push buttons S1; S2; and S3 connected to the: A2, A1, A0 Arduino pins
   #define S1                A2   ///< A2: S1 button: Time set  & Decrement button   
   #define S2                A1   ///< A1: S2 button: Select    & Confirm/Save button  
   #define S3                A0   ///< A0: S3 button: Alarm set & Increment button  

   // I2C pins used by the shield on Arduino boards
   #define I2C_SDA_PIN      PC4   ///< SDA pin for Arduino UNO_R3, PC4 position near Reset button.
   #define I2C_SCL_PIN      PC5   ///< SCL pin for Arduino UNO_R3, PC5 position near Reset button.

   /// This determines if the menu and/or time are also displayed on the serial monitor.
   /// - If SERIAL_SETUP_CODE is defined, code to display the serial menu is included in the project.
   /// - If SERIAL_TIME_CODE  is defined, code to display the serial time, every second, is included in the project.
   /// - The SERIAL_SETUP_CODE and/or SERIAL_TIME_CODE values are just used to determine if the code is included or not.
   ///                 Setup messages are displayed by default and can be controlled by setting the 
   ///                 `IsSerialSetup` property to true. The Time messages are NOT displayed initially on startup.
   ///                 They can be displayed by setting the `IsSerialTime` property to true.
   /// @see `set_IsSerialSetup()` 
   /// @see `set_IsSerialTime()` 
   #define SERIAL_SETUP_CODE true   ///< If (true) - serial setup code included, (false) - code removed
   #define SERIAL_TIME_CODE  true   ///< If (true) - serial time  code included, (false) - code removed
   
#endif

/// @name Additional_Defines
/// The following defines from `BinaryClock.Defines.h` can be overridden by defining them here in this header file.  
/// The following `#define`s can be modified here if you need different values, e.g. for your custom board.  
/// The values in the example below are the default values used if they are not defined here.   
/// @details
/// @verbatim
/// ##################################################################################################################### //
/// // This section is board specific, but can be overridden for a custom board. 
/// // The default values are defined per board in `BinaryClock.Defines.h` if not defined here.
/// #define ESP32_WIFI            true    ///< Set to true if the board has onboard ESP32 based WiFi; false otherwise.
/// #define WIFIS3                false   ///< Set to true if the board has onboard WIFIS3 based WiFi (UNO R4 WiFi); false otherwise.
/// #define FREE_RTOS             true    ///< Set to true if the board is running FreeRTOS, e.g. boards with an ESP32.
/// #define STL_USED              true    ///< Set to true if the board can use the C++ STL library (i.e. has enough memory).
/// #define LED_HEART      LED_BUILTIN    ///< Heartbeat LED to show working software, errors or messages.
/// #define PRINTF_OK             true    ///< Use printf style code if supported, usuall true.
/// #define ESP32_INPUT_PULLDOWN  INPUT_PULLDOWN   ///< Pin has an internal pull-down resistor (e.g. ESP32) or just use `INPUT` (e.g. Arduino).
/// -----------------------------------------------------------------------------------------------
/// // This section contains the definitions for accessing and setting the definitions for SNTP time synchronization.
/// #define NTP_SERVERS_LIST { "time.nrc.ca", "pool.ntp.org", "time.nist.gov" }  // The list of NTP servers example, 
/// #define DEFAULT_TIMEZONE "EST+5EDT,M3.2.0/2,M11.1.0/2"  // Default timezone string (Canada/US Eastern Time with DST)
/// #define NTP_DEFAULT_PORT 123                            // Standard NTP port number.
/// -----------------------------------------------------------------------------------------------
/// // This section contains the definitions for the time and alarm formats, and other general settings.
/// #define DEFAULT_DEBOUNCE_DELAY    75  ///< The default debounce delay in milliseconds for the buttons
/// #define DEFAULT_BRIGHTNESS        30  ///< The best tested LEDs brightness range: 20-60
/// #define DEFAULT_ALARM_REPEAT       3  ///< How many times to play the melody alarm
/// #define DEFAULT_SERIAL_SPEED  115200  ///< Default serial output speed in bps
/// #define TIME_FORMAT_24HR    "HH:mm:ss"    ///< 24 Hour time format string
/// #define TIME_FORMAT_AMPM    "HH:mm:ss AP" ///< 12 Hour time format string with AM/PM
/// #define ALARM_FORMAT_24HR   "HH:mm"       ///< 24 Hour alarm format string
/// #define ALARM_FORMAT_AMPM   "HH:mm AP"    ///< 12 Hour alarm format string with AM/PM
/// #define DEFAULT_12HR_MODE    AMPM_MODE    ///< Default time mode is 12: `AMPM_MODE` (true) or `HR24_MODE` (false)
/// #define DEFAULT_TIME_FORMAT  TIME_FORMAT_AMPM  ///< Default time  format, or TIME_FORMAT_24HR 
/// #define DEFAULT_ALARM_FORMAT ALARM_FORMAT_AMPM ///< Default alarm format, or ALARM_FORMAT_24HR
/// -----------------------------------------------------------------------------------------------
/// #define DEFAULT_SERIAL_SETUP    true   ///< Initial serial setup display value (e.g. allow the setup menu to be displayed).
/// #define DEFAULT_SERIAL_TIME    false   ///< Initial serial time display value  (e.g. no continuous serial time display).
/// ##################################################################################################################### //
/// 
/// // If not using the Binary Clock Shield for Arduino (https://nixietester.com/product/binary-clock-shield-for-arduino/)
/// //    and the wiring of the buttons is different, modify the S1_ON, S2_ON, and S3_ON defines below.
/// // Define the physical pin wiring: CC_ON or CA_ON
/// //    CC - Common Cathode, button wired to +ve rail (pin is pulled-down to ground when off), on when HIGH (1). 
/// //    CA - Common Anode,   button wired to ground   (pin is pulled-up to +ve rail when off), on when LOW  (0).
/// #define S1_ON          CC_ON   ///< The button S1 is pulled to ground and is active when the pin reads HIGH.
/// #define S2_ON          CC_ON   ///< The button S1 is pulled to ground and is active when the pin reads HIGH.
/// #define S3_ON          CC_ON   ///< The button S1 is pulled to ground and is active when the pin reads HIGH.
///   
/// // Define the physical and logical  layout of the Neopixel LEDs (if different from the shield).
/// //#####################################################################################//  
/// // General defines for the Binary Clock Shield display layout.
/// // The display layout of the LEDs on the shield, one row each.
/// // The Binary Clock Shield has 3 rows of LEDs for displaying the time:
/// //   - Top row:    Hours   (5 LEDs)
/// //   - Middle row: Minutes (6 LEDs)
/// //   - Bottom row: Seconds (6 LEDs)
/// // These are the LEDs that are used to display the time. The physical layout of a custom
/// // shield may have more LEDs in each row, these are defined separately below.
/// // The LED color values are set from LSB seconds to MSB hours in the display array (e.g. [0-16]).
/// // The array reserved for the FastLED library is defined by TOTAL_LEDS below.
/// // The array size for the display array is defined by NUM_LEDS below.
/// #ifndef DISPLAY_LAYOUT
///    #define DISPLAY_LAYOUT    ///< Define the display layout of the Binary Clock Shield
///    #define NUM_HOUR_LEDS         5         ///< The LEDs on the top row of the shield.
///    #define NUM_MINUTE_LEDS       6         ///< The LEDs on the middle row of the shield.
///    #define NUM_SECOND_LEDS       6         ///< The LEDs on the bottom row of the shield.
/// #endif
/// #ifndef NUM_LEDS
///    // Total number of LEDs used for the time display. This defines the display array size.
///    #define NUM_LEDS (NUM_HOUR_LEDS + NUM_MINUTE_LEDS + NUM_SECOND_LEDS)
/// #endif
/// // Define the logical layout of the LEDs on each row on the shield, one row each. 
/// // The logical layout is the order of the LEDs for displaying the time, from LSB to MSB.
/// // The logical layout may be different from the physical layout of the LEDs on the shield, 
/// // for example to shift the display where each row has more LEDs than needed for the display.
/// #define DISPLAY_OFFSETS   ///< Define the offsets for each row of the Binary Clock Shield
/// #define HOUR_LEDS_OFFSET      (NUM_SECOND_LEDS + NUM_MINUTE_LEDS)
/// #define MINUTE_LEDS_OFFSET    (NUM_SECOND_LEDS)
/// #define SECOND_LEDS_OFFSET    0
/// 
/// // Define the number of physical LEDs in each row, can be overridden if needed.
/// // This allows for different LED configurations if needed.
/// // If the rows have more LEDs than needed, the extra LEDs will be unused (OFF, CRGB::Black).
/// // This is useful for custom shields with different layouts.
/// // Displaying the time on an 8x8 matrix would use 8 LEDs per row, even though only
/// // 5, 6, and 6 LEDs are needed for hours, minutes, and seconds respectively.
/// // TOTAL_LEDS would be 64 in that case, even though only 17 LEDs are used for the time display.
/// // To display the time on the top 3 rows of the matrix, the offset for the start of the seconds,
/// // SECOND_ROW_OFFSET would be 40.
/// // Define the physical number of the LEDs on each row on the shield, one row each.
/// #define PHYSICAL_LAYOUT    ///< Override the physical layout of the Binary Clock Shield.
/// #define HOUR_ROW_LEDS         NUM_HOUR_LEDS    ///< The number of hour LEDs in the hour row.
/// #define MINUTE_ROW_LEDS       NUM_MINUTE_LEDS  ///< The number of minute LEDs in the minute row.
/// #define SECOND_ROW_LEDS       NUM_SECOND_LEDS  ///< The number of second LEDs in the second row.
/// 
/// // Define the physical layout of the LEDs on each row on the shield, one row each. 
/// // The physical layout may have more LEDs in each row than needed for the time display.
/// #define PHYSICAL_OFFSETS   ///< Override the physical offsets for each row of the Binary Clock Shield.
/// #define HOUR_ROW_OFFSET       (MINUTE_ROW_OFFSET + MINUTE_ROW_LEDS)
/// #define MINUTE_ROW_OFFSET     (SECOND_ROW_OFFSET + SECOND_ROW_LEDS)
/// #define SECOND_ROW_OFFSET     0
/// 
/// // This is to account for the physical layout beyond the 3 rows for the time display.
/// // This is used in the `FastLED` setup for the total buffer size. For example to
/// // display the time on an 8x8 matrix, the total LEDs would be 64, even though only
/// // 17 LEDs are used for the time display and each row is 8 LEDs long.
/// #define TOTAL_LEDS     (HOUR_ROW_LEDS + MINUTE_ROW_LEDS + SECOND_ROW_LEDS)
/// 
/// @endverbatim
/// -----------------------------------------------------------------------------------------------
/// @remarks
/// (*) It is only possible to remap the pin location by using an UNO prototype shield and 
///     soldering different paths from the UNO board to the Binary Clock Shield. The prototype
///     shield is connected between the board and the Binary Clock Shield. This is necessary when
///     using a board such as the Wemos ESP32_D1_R32_UNO board as it needs the Neopixel LED 
///     data pin moved to pin 15 as the LED_DATA_PIN, physically on analog pin A3, is pin 34 on the
///     board which is an input only pin for 1st generation ESP32-32 chips. 
///     
///     See the README.md file for a detailed description of the change.  
///     (https://github.com/Chris-70/WiFiBinaryClock/blob/main/README.md#hardware-modifications)
///     (https://github.com/Chris-70/WiFiBinaryClock), section: "Hardware modifications"   
/// @}