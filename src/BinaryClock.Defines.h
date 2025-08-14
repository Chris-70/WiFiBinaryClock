
#ifndef _BinaryClock_Defines_h_
#define _BinaryClock_Defines_h_

//################################################################################//
// This is a Binary Clock Shield for Arduino by Marcin Saj https://nixietester.com
// 
// The following are defines for the currently supported boards. One must be used to compile.
// Add your own UNO style board definitions below for any different UNO style board you have.
//
// #define ESP32_D1_R32_UNO   // If defined, the code will use Wemos D1 R32 ESP32 UNO board definitions     (ESP32 WiFi)
// #define METRO_ESP32_S3     // If defined, the code will use Adafruit Metro ESP32-S3 board definitions    (ESP32 WiFi)
// #define ESP32_S3_UNO       // If defined, the code will use generic ESP32-S3 UNO board definitions       (ESP32 WiFi)
// #define UNO_R4_WIFI        // If defined, the code will use Arduino UNO R4 WiFi board definitions        (ESP32 WiFi)
// #define UNO_R4_MINIMA      // If defined, the code will use Arduino UNO R4 Minima board definitions      (No WiFi)
// #define UNO_R3             // If defined, the code will use Arduino UNO R3 (ATMEL 328) board definitions (NO WiFi)
// 
//################################################################################//

// Debug Time PIN to print out the current time over serial monitor (if ON)
// The SERIAL_MENU and/or SERIAL_TIME_CODE are defined (i.e. true) in order
// to compile the code and make them available. They can also be set
// in the software: 'void BinaryClock::set_isSerialSetup(bool value)' and
// 'void BinaryClock::set_isSerialTime(bool value)' methods.
// The debug time and setup pins are used to enable/disable the serial output at runtime.
// without the need to change the software. The Serial Time is a switch to enable/disable 
// the serial time display, displays while switch is ON. The Serial Setup is a momentary button
// to toggle enable/disable the serial setup display. 
// When the PIN value is -1 (-ve) the associated code is removed.

//################################################################################//
//             Defines for the different UNO sized boards                         //
//################################################################################//
// Generic AliExpress copy of Wemos D1 R32 ESP32 based UNO board (validate against the board you receive)
// NOTE: This requires a hardware modification to the board to use the LED pin 15 instead of pin A3/34.
//       see the 'readme.md' file on GitHub for details: https://github.com/Chris-70/WiFiBinaryClock.
#if defined(ESP32_D1_R32_UNO)     // ESP32 Wemos D1 R32 UNO board definitions
   #define ESP32UNO               // Define ESP32UNO as a common base architecture for ESP32 UNO boards

   // ESP32 UNO pin definitions
   #define RTC_INT           25   // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             23   // The number of the Piezo pin
   #define LED_PIN           15   // Data pin that LEDs data will be written out. Requires board modification to use pin 15
                                  // You need to modify the board by removing the connector at pin 34 (A3)
                                  // Solder a jumper wire from PIN 15 to the LED pin (A3 location) on the shield. 
                                  // The Wemos ESP32 UNO PIN 34 is Read-Only and cannot be used for output.

   #define S1                35   // Push buttons connected to the A2, A1, A0 Arduino pins (CC)
   #define S2                 4   // A1
   #define S3                 2   // A0

   #define DEBUG_SETUP_PIN   16   // Set to -1 to disable the Serial Setup display control by H/W (CA)
   #define DEBUG_TIME_PIN    27   // Set to -1 to disable the Serial Time display control by H/W (CA)
   #define ESP32_INPUT_PULLDOWN   INPUT_PULLDOWN

// Adafruit Metro ESP32-S3 board (https://www.adafruit.com/product/5500)
//          A very capable UNO style board. See: https://learn.adafruit.com/adafruit-metro-esp32-s3
#elif defined(METRO_ESP32_S3)
   #define ESP32UNO

   // Adafruit Metro ESP32-S3 pin definitions
   #define RTC_INT            3   // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             11   // The number of the Piezo pin
   #define LED_PIN           A3   // Data pin that LEDs data will be written out.

   #define S1                A2   // Push buttons connected to the A2, A1, A0 Arduino pins (CC)
   #define S2                A1   // A1
   #define S3                A0   // A0

   #define DEBUG_SETUP_PIN    5   // Set to -1 to disable the Serial Setup display control by H/W (CA)
   #define DEBUG_TIME_PIN     6   // Set to -1 to disable the Serial Time display control by H/W (CA)
   #define ESP32_INPUT_PULLDOWN   INPUT_PULLDOWN

// Generic AliExpress ESP32-S3 UNO board definitions (validate against the board you receive)
// These boards use the ESP32-S3 DevKitC-1 pinout definitions, e.g. SDA is pin 8; SCL is pin 9; LED is pin 48, etc.
// See: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html#hardware-reference
#elif defined(ESP32_S3_UNO)
   #define ESP32UNO

   // AliExpress ESP32-S3 UNO pin definitions
   #define RTC_INT           17   // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             11   // The number of the Piezo pin
   #define LED_PIN            6   // Data pin that LEDs data will be written out.

   #define S1                 7   // Push buttons connected to the A2, A1, A0 Arduino pins (CC)
   #define S2                 1    
   #define S3                 2 

   #define DEBUG_SETUP_PIN   20   // Set to -1 to disable the Serial Setup display control by H/W (CA)
   #define DEBUG_TIME_PIN     3   // Set to -1 to disable the Serial Time display control by H/W (CA)
   #define ESP32_INPUT_PULLDOWN   INPUT_PULLDOWN

// Standard Arduino UNO  board definitions for: 
//    UNO R3         (https://store.arduino.cc/products/arduino-uno-rev3)
//    UNO R4 WiFi    (https://store.arduino.cc/products/uno-r4-wifi)
//    UNO R4 Minima  (https://store.arduino.cc/products/uno-r4-minima) 
#elif defined(UNO_R3) || defined(UNO_R4_WIFI) || defined(UNO_R4_MINIMA)
   // Arduino UNO based pin definitions (R3 & R4)
   #define RTC_INT            3   // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             11   // The number of the Piezo pin
   #define LED_PIN           A3   // Data pin that LEDs data will be written out over
   #define LED_HEART         12   // Heartbeat LED to show working software (Dev+H/W)

   #define S1                A2   // Push buttons connected to the A2, A1, A0 Arduino pins
   #define S2                A1   // A1
   #define S3                A0   // A0

   #define DEBUG_SETUP_PIN    5  // Set to -1 to disable the Serial Setup display control by H/W
   #define DEBUG_TIME_PIN     6  // Set to -1 to disable the Serial Time display control by H/W
   #define ESP32_INPUT_PULLDOWN  INPUT

#else
   #pragma message "No supported board defined. Supported boards are:"
   #pragma message "  ESP32_D1_R32_UNO  - generic Wemos D1 R32 UNO with ESP32"
   #pragma message "  METRO_ESP32_S3    - Adafruit Metro ESP32-S3 board"
   #pragma message "  ESP32_S3_UNO      - generic ESP32-S3 UNO board"
   #pragma message "  UNO_R3            - Arduino UNO R3 board"
   #pragma message "  UNO_R4_WIFI       - Arduino UNO R4 WiFi board"
   #pragma message "  UNO_R4_MINIMA     - Arduino UNO R4 Minima board"
   #pragma message "Please define one of the above boards to compile the code."
   #error "Undefined board. Please define the pin numbers for your board."
   #include <NoBoardDefinition_StopCompilationNow> // Include a dummy header file to stop compilation
#endif

#if defined(UNO_R3) || defined(UNO_R4_MINIMA)
   // These defines are used in the code to satisfy the UNO R3 compiler target board
   #define ESP32_WIFI false
   #define FREE_RTOS  false
   #if defined(UNO_R3)
      #ifdef DEV_BOARD
      #undef DEV_BOARD
      #endif
      // The UNO R3 board doesn't have the resources to support the code for an
      // OLED display (on the development board) in addition to this Binary Clock Shield.
      #define DEV_BOARD false
   #endif
#else
   #define ESP32_WIFI true
   #define FREE_RTOS  true
#endif 

#ifndef DEV_BOARD
   #define DEV_BOARD false    // If DEV_BOARD hasn't been defined, don't include code for the development board
   #warning "DEV_BOARD is not defined, setting to false. No development board code will be compiled."
#endif

#if !(DEV_BOARD)
   #undef DEBUG_SETUP_PIN
   #undef DEBUG_TIME_PIN
   #define DEBUG_SETUP_PIN   -1   // No development board, so no H/W debug setup
   #define DEBUG_TIME_PIN    -1   // No development board, so no H/W debug time
#endif
//################################################################################//

// The physical layout of the LEDs on the shield, one row each.
#define NUM_HOUR_LEDS   5
#define NUM_MINUTE_LEDS 6
#define NUM_SECOND_LEDS 6
#define NUM_LEDS (NUM_HOUR_LEDS + NUM_MINUTE_LEDS + NUM_SECOND_LEDS)

#define LED_TYPE           WS2812B     // Datasheet: http://bit.ly/LED-WS2812B
#define COLOR_ORDER          GRB       // For color ordering use this sketch: http://bit.ly/RGBCalibrate   

#define DEFAULT_DEBOUNCE_DELAY    75   // The default debounce delay in milliseconds for the buttons
#define DEFAULT_BRIGHTNESS        30   // The best tested LEDs brightness range: 20-60
#define DEFAULT_ALARM_REPEAT       3   // How many times play the melody alarm
#define ALARM_1                    1   // Alarm 1. available on the RTC DS3231, adds seconds.
#define ALARM_2                    2   // Alarm 2, the default alarm used by the shield.

#define CA_ON                    LOW   // The value when ON  for CA connections
#define CC_ON                   HIGH   // The value when ON  for CC connections
#define CA_OFF                  HIGH   // The value when OFF for CA connections
#define CC_OFF                   LOW   // The value when OFF for CC connections

// This determines if the menu and/or time are also displayed on the serial monitor.
// If SERIAL_SETUP_CODE is defined, code to display the serial menu is included in the project.
// If SERIAL_TIME_CODE  is defined, code to display the serial time, every second, is included in the project.
// The DEFAULT_SERIAL_SETUP and DEFAULT_SERIAL_TIME values are used to determine if the serial 
//                 Setup and/or Time messages are displayed initially or not.
//                 use the public methods 'set_isSerialSetup()' and 'set_isSerialTime()' to
//                 enable/disable the serial output at runtime. H/W buttons, if defined, 
//                 can also be used to enable/disable the serial output at runtime.
#ifndef SERIAL_SETUP_CODE
#define SERIAL_SETUP_CODE       true   // If (true) - serial setup code included, (false) - code removed
#endif
#ifndef SERIAL_TIME_CODE
#define SERIAL_TIME_CODE        true   // If (true) - serial time  code included, (false) - code removed
#endif
#define SERIAL_OUTPUT (SERIAL_SETUP_CODE || SERIAL_TIME_CODE) // If (true) - Allow serial output messages.
#define DEFAULT_SERIAL_SETUP    true   // Initial serial setup display value (e.g. allow the serial setup display).
#define DEFAULT_SERIAL_TIME    false   // Initial serial time display value  (e.g. no serial time display at startup).
// This controls the inclusion/removal of the code to support hardware buttons/switches to also control the serial output.
// The serial output can always be controlled in software if the SERIAL_xxxx_CODE is defined (true).
#define HW_DEBUG_SETUP ((DEBUG_SETUP_PIN >= 0) && (SERIAL_SETUP_CODE))  // Include code to support H/W to control setup display
#define HW_DEBUG_TIME  ((DEBUG_TIME_PIN  >= 0) && (SERIAL_TIME_CODE))   // Include code to support H/W to control time  display
// The delay, in ms, is set to a high value when using a momentary button so the button can be release quickly and the user will
// still see the serial output. When using a switch the delay can be short as the user won't need to keep pressing a button.
#define DEFAULT_DEBUG_OFF_DELAY 3000 
#define HARDWARE_DEBUG (HW_DEBUG_SETUP ||  HW_DEBUG_TIME)

#define FOREVER while(true)            // Infinite loop, e.g. used in task methods.

// Bit numbers for DS3231 RTC registers and register numbers
// Registers:
#define DS3231_CONTROL              0x0E  // Control register address for DS3231
#define DS3231_STATUS               0x0F  // Status register address for DS3231
// Bit Numbers and Masks:
#define DS3231_TEMP_MSB             0x11  // Temperature MSB register address for DS3231
#define DS3231_TEMP_LSB             0x12  // Temperature LSB register address for DS3231
#define DS3231_ALARM1               0x07  // Alarm 1 register address for DS3231
#define DS3231_ALARM2               0x0B  // Alarm 2 register address for DS3231
#define DS3231_ALARM1_MODE_MASK     0x0F  // Mask for Alarm 1 mode in control register
#define DS3231_ALARM2_MODE_MASK     0x0C  // Mask for Alarm 2 mode in control register
#define DS3231_ALARM1_STATUS_MASK   0x01  // Mask for Alarm 1 status in control register
#define DS3231_ALARM2_STATUS_MASK   0x02  // Mask for Alarm 2 status in control register
#define DS3231_ALARM1_FLAG_MASK     0x01  // Mask for Alarm 1 alarm triggered flag in status register
#define DS3231_ALARM2_FLAG_MASK     0x02  // Mask for Alarm 1 alarm triggered flag in status register
#endif // _BinaryClock_Defines_h_