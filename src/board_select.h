// Define the target board here.
// Uncomment the board to use; comment/remove all others.
//
// #define ESP32_D1_R32_UNO   // If defined, the code will use Wemos D1 R32 ESP32 UNO board definitions     (ESP32 WiFi)
// #define METRO_ESP32_S3     // If defined, the code will use Adafruit Metro ESP32-S3 board definitions    (ESP32 WiFi)
// #define ESP32_S3_UNO       // If defined, the code will use generic ESP32-S3 UNO board definitions       (ESP32 WiFi)
// #define UNO_R4_WIFI        // If defined, the code will use Arduino UNO R4 WiFi board definitions        (ESP32 WiFi)
// #define UNO_R4_MINIMA      // If defined, the code will use Arduino UNO R4 Minima board definitions      (No WiFi)
// #define UNO_R3             // If defined, the code will use Arduino UNO R3 (ATMEL 328) board definitions (NO WiFi)
// 

// If you are using a different UNO style board, "#define CUSTOM_UNO true" and then
// modify/define the pin numbers for your board in the section below for CUSTOM_UNO.
// If you are using a supported board, set "#define CUSTOM_UNO false" or comment it out.
//
// #define CUSTOM_UNO true

#if CUSTOM_UNO
   // Arduino UNO based pin definitions (R3 & R4) Modify for your board.
   #define RTC_INT            3   // Interrupt. Arduino pin no.3 <-> Shield RTC INT/SQW pin           
   #define PIEZO             11   // The number of the Piezo pin
   #define LED_PIN           A3   // Data pin that LEDs data will be written out over

   #define S1                A2   // Push buttons connected to the A2, A1, A0 Arduino pins
   #define S2                A1   // A1
   #define S3                A0   // A0

   #define ESP32_INPUT_PULLDOWN  INPUT   // Define for INPUT without an internal pull-down resistor or INPUT_PULLDOWN

   #define DEBUG_SETUP_PIN   -1   // Set to -1 to disable the Serial Setup display control by H/W (CC)
   #define DEBUG_TIME_PIN    -1   // Set to -1 to disable the Serial Time display control by H/W (CA)
#endif
