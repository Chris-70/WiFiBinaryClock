/// @file board_select.h
/// 
/// @verbatim 
/// ##################################################################################################################### //
/// The following are defines for the currently supported boards. One must be used to compile, or create a CUSTOM_UNO
/// 
/// #define ESP32_D1_R32_UNO   // If defined, the code will use Wemos D1 R32 ESP32 UNO board definitions     (ESP32 WiFi)
/// #define METRO_ESP32_S3     // If defined, the code will use Adafruit Metro ESP32-S3 board definitions    (ESP32 WiFi)
/// #define ESP32_S3_UNO       // If defined, the code will use generic ESP32-S3 UNO board definitions       (ESP32 WiFi)
/// #define UNO_R4_WIFI        // If defined, the code will use Arduino UNO R4 WiFi board definitions        (ESP32 WiFi)
/// #define UNO_R4_MINIMA      // If defined, the code will use Arduino UNO R4 Minima board definitions      (No WiFi)
/// #define UNO_R3             // If defined, the code will use Arduino UNO R3 (ATMEL 328) board definitions (NO WiFi)
///
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
/// --------------------------------------------------------------------------------------//    
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
   #define LED_PIN           A3   ///< Data pin that LEDs data will be written out over

   #define S1                A2   ///< Push buttons connected to the A2, A1, A0 Arduino pins
   #define S2                A1   ///< A1
   #define S3                A0   ///< A0

   #define ESP32_INPUT_PULLDOWN  INPUT   ///< Define for INPUT without an internal pull-down resistor or INPUT_PULLDOWN

   #define FREE_RTOS       true   ///< Set to true if the board is running FreeRTOS, e.g. boards with an ESP32.
   #define ESP32_WIFI      true   ///< Set to true if the board has onboard ESP32 based WiFi; false otherwise.

#endif

/// (*) It is only possible to remap the pin location by using an UNO development shield and 
///     soldering different paths from the board to the Binary Clock Shield. The development
///     shield is connected between the board and the Binary Clock Shield. This is 
///     necessary when using the Wemos ESP32_D1_R32_UNO board as it has the Neopixel LED 
///     data pin moved to pin 15 as the LED_PIN, physically on analog pin A3, is pin 34 on the
///     board which is input only. 
///     
///     See the README.md file for a detailed description of the change.
///     (https://github.com/Chris-70/WiFiBinaryClock)
/// @}