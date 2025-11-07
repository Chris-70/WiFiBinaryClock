#include <arduino.h>

#include "BinaryClock.h"

#if STL_USED
   #include <string>
   #include <vector>
   #include <functional>
#endif

#if WIFI
   #include "BinaryClockWAN.h"
#endif

// Development board specific; has additional hardware buttons and OLED display.
// Not part of the Binary Clock Shield, but used to develop and test the code.
// This board replaces the Binary Clock Shield during development.
#if DEV_BOARD
   #include <Adafruit_GFX.h>
   #include <Adafruit_SSD1306.h>

   #ifndef NO_ADAFRUIT_SSD1306_COLOR_COMPATIBILITY
      #define BLACK   SSD1306_BLACK     ///< Draw 'off' pixels (i.e. 0)
      #define WHITE   SSD1306_WHITE     ///< Draw 'on' pixels  (i.e. 1)
      #define INVERSE SSD1306_INVERSE   ///< Invert pixels     (i.e. 2)
   #endif
   #define SCREEN_WIDTH 128 // OLED display width, in pixels
   #define SCREEN_HEIGHT 32 // OLED display height, in pixels
   #define OLED_RESET    -1 // / QT-PY  XIAO
   #define I2C_ADDRESS 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
                        // e.g. the one with GM12864-77 written on it
   Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 100000UL, 100000UL);
   // Wrap the display initialization in a MACRO to allow for the display being optional.
   #define BEGIN(ADDR,RESET) display.begin(SSD1306_SWITCHCAPVCC, (ADDR), (RESET), true)
   // Wrap the OLED display in a MACRO to first test if the display is available.
   #define OLED_DISPLAY(CMD) if (oledValid) { display.CMD; }
   // Wrap this method in a MACRO definition only used with a dev. board that has an OLED display.
   #define TIME_OLED(DATE_N_TIME) TimeOLED(DATE_N_TIME);
   void TimeOLED(const DateTime & time);
#else
   // Removes the code from compilation, replaced with whitespace.
   #define BEGIN(ADDR, RESET)
   #define OLED_DISPLAY(CMD) 
   #define TIME_OLED(DATE_N_TIME)
#endif

#define I2C_SIZE      16
#define RTC_ADDR      0x68
#define RTC_EEPROM    0x57
#define OLED_IIC_ADDR 0x3c
#ifndef LED_HEART
   // LED used to communicate status / errors.
   #define LED_HEART LED_BUILTIN 
#endif

using namespace BinaryClockShield;

// All function declarations here (Req'd for PlatformIO):
void setup(void);
void loop(void);
bool checkWatchdog();
void TimeAlert(const DateTime& time);
#ifdef UNO_R3
   #define ScanI2C(ARRAY, SIZE) 0
#else
int ScanI2C(byte* addrList, size_t listSize);
#endif 

#if WIFI
static BinaryClockWAN& wifi = BinaryClockWAN::get_Instance();
#endif

#if (DEVELOPMENT || SERIAL_OUTPUT) && !defined(UNO_R3)
char buffer[32] = {0};
// const char *format12 = { "HH:mm:ss AP" }; // 12 Hour time format
// const char *format24 = { "hh:mm:ss" };    // 24 Hour time format
const char *timeFormat = nullptr; // format24;
const char *daysOfTheWeek[7] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
#endif 

// Get the singleton instance of BinaryClock
BinaryClock& binClock = BinaryClock::get_Instance();

// Initialize the watchdog field values.
static long watchdogTimeout = 2100; 
volatile long timeWatchdog = 0;
bool wdtFault = false;

long curMillis = 0;
long deltaMillis = 0;

int HeartbeatLED = LED_HEART;
int heartbeat = LOW;             // Heartbeat LED state: OFF

#ifdef UNO_R3
   #define oledValid    false 
   #define rtcValid     true     // Assume we have the shield
   #define eepromValid  false
#else
bool oledValid = false;
bool rtcValid = false;
bool eepromValid = false;
byte i2cList[I2C_SIZE] = { 0 };
// std::vector<WiFiInfo> wifiApList; // Vector to hold the found WiFi access point info.
#endif

__attribute__((used)) void setup()
   {
   Serial.begin(115200); // Start the serial communication at 115200 baud rate

   pinMode(HeartbeatLED, OUTPUT);
   digitalWrite(HeartbeatLED, LOW);
   Wire.begin();

   #ifndef UNO_R3
   // Scan the I2C bus looking for the devices we use.
   int i2cDevices = ScanI2C(i2cList, I2C_SIZE);
   SERIAL_STREAM(endl << "Found: " << i2cDevices << " I2C Devices." << endl << "  Known devices are:" << endl)
   for (int i = 0; i < i2cDevices; i++)
      {
      if (i2cList[i] == OLED_IIC_ADDR) { oledValid = true; SERIAL_PRINTLN("  - OLED display is present.") } 
      if (i2cList[i] == RTC_ADDR)       { rtcValid = true; SERIAL_PRINTLN("  - RTC is present.")          } 
      if (i2cList[i] == RTC_EEPROM)  { eepromValid = true; SERIAL_PRINTLN("  - RTC EEPROM is present.")   } 
      }
   delay(500); 

   timeFormat = binClock.get_TimeFormat();
   #endif

   SERIAL_PRINTLN("")
   #if DEV_BOARD
   // // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
   // display.begin(SSD1306_SWITCHCAPVCC, OLED_IIC_ADDR);  // initialize with the I2C addr 0x3C (for the 128x32)
   bool displayResult = false;
   if (oledValid) // If we found the I2C OLED display; try to initialize it.
      {
      displayResult = BEGIN(I2C_ADDRESS, true);
      // Update the flag: OLED is only valid if we can initialize it.
      if ((oledValid = displayResult) == true)
         {
         display.setTextColor(WHITE, BLACK);
         display.setTextSize(1);
         display.setTextWrap(false);
         display.display();
         }
      }
   delay(500);
   #else
      // Non-dev board configuration, no OLED display, define the `displayResult` to be false.
      #define displayResult false
   #endif

   SERIAL_STREAM("OLED display is: " << (oledValid? "Installed;" : "Missing; ") << "OLED Begin: " << (displayResult? "Success: " : "Failure: ") 
                << " Clear Display: " << (oledValid? "YES" : "NO") << endl)
   SERIAL_STREAM("Starting the BinaryClock Setup" << endl)

   binClock.setup(!oledValid);   // If the OLED display is installed, it's likely a dev board, not the shield.
   binClock.set_Brightness(20);

   // Register `TimeAlert()` to get called every second.
   auto callback = [](const DateTime& time) {
         TimeAlert(time);
         };
   bool regResult = binClock.RegisterTimeCallback(callback);
   SERIAL_STREAM("Registered time callback: " << (regResult? "True" : "False") << endl)
   delay(125);

   #if WIFI
   bool wifiResult = wifi.Begin(binClock, true);
   vTaskDelay(125 / portTICK_PERIOD_MS);
////////////////////////////////////
// Add WiFi event handler for reconnection
   WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
      Serial.printf("[%lu] [WiFi] ", millis());
         switch (event)
            {
            case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
               Serial.println("Disconnected - attempting reconnection");
               delay(1000);
               WiFi.reconnect();
               break;
            case ARDUINO_EVENT_WIFI_STA_CONNECTED:
               Serial.println("Reconnected successfully");
               break;
            default:
               Serial.print("Default case for: ");
               Serial.println(event);
               break;
            }
         });
////////////////////////////////////
   APCreds creds = wifi.get_WiFiCreds();
   SERIAL_STREAM("[" << millis() << "] WiFi is: " << (wifi.get_IsConnected()? "Connected" : "Disconnected") << " SSID: " << creds.ssid << " BSSID: " << creds.bssid << " Password: " << creds.pw << endl)
   #endif 

   SERIAL_STREAM("[" << millis() << "] Entering Loop() now" << endl)
   delay(125);
   OLED_DISPLAY(clearDisplay())

   // binClock.set_IsSerialTime(!binClock.buttonDebugTime.read()); // *** DEBUG ***
   timeWatchdog = millis();   // Reset the Watchdog Timer.
   }

__attribute__((used)) void loop()
   {
   static bool wdtError = false;
   //////////////////////////////////////
   static bool firstLoop = true;
   if (firstLoop)
      {
      Serial << "[" << millis() << "] === FIRST LOOP ITERATION ===" << endl;
      firstLoop = false;
      }

   // Serial << "[" << millis() << "] Loop start" << endl;

   static uint32_t wifiCheckTime = 0;
   static bool wasConnected = true;

   // Check WiFi status every second
   if (millis() - wifiCheckTime > 1000)
      {
      bool isConnected = WiFi.isConnected();
      if (wasConnected && !isConnected)
         {
         Serial << "[" << millis() << "] !!! WiFi DISCONNECTED in main loop !!!" << endl;
         yield();  // Give WiFi time to process
         }
      wasConnected = isConnected;
      wifiCheckTime = millis();
      }
   //////////////////////////////////////

   binClock.loop();
   yield();  // Give WiFi time to process
   // checkWatchdog();
   if (checkWatchdog())
      {
      wdtError = false;
      }
   else if (!wdtError) // Display just once per WDT fault
      {
      wdtError = true;
      SERIAL_STREAM("[" << millis() << F("] Watchdog Timer Triggered after ") << deltaMillis / 1000.0 << F("seconds. ") << endl)
      }

   yield();  // Give WiFi time to process
   //////////////////////////////////////
   // Serial << "[" << millis() << "] Loop end" << endl;
   // delay(100);  // Small delay to see timing
   //////////////////////////////////////
   }

/// @brief Check the Watchdog Timer and alert it if necessary.
/// @return true if the Watchdog Timer is OK, false if it has triggered.
/// @remarks The WDT is driven by the `TimeAlert()` method being called every second.
///          If the WDT is not reset within the timeout period (~2.1 seconds), it will 
///          trigger and drive the heartbeat LED OFF. If we have a DEV_BOARD, it will 
///          display "FAULT" and that's all for now. We would need to make some
///          code changes to reset the `BinaryClock` and recover. Rebooting is out
///          as we could end up in a reboot cycle that never ends. Best to sit and
///          wait as we'lll recover if we start getting calls to `TimeAlert()` again.
bool checkWatchdog()   
   {
   bool wdtResult = true;

   curMillis = millis();
   deltaMillis = curMillis - timeWatchdog;
   if (deltaMillis > watchdogTimeout)
      {
      wdtFault = true;
      wdtResult = false;
      digitalWrite(HeartbeatLED, LOW);

      #if DEV_BOARD
      if (oledValid)
         {
         display.clearDisplay();
         display.setCursor(0, 0);
         display.setTextSize(4);
         display.write("Fault");
         display.display();
         }
      #endif
      }

   return wdtResult;
   }

/// @brief The 1 Hz "Time" callback function from the `BinaryClock` instance.
/// @details This method is supervised by the watchdog which only tolerates
///          one missed call. A 5% allowence is given for code execution 
///          timing delays, so after ~2.1 seconds the WDT activates.
/// @param time The current time reference from the `BinaryClock` instance.
/// @remarks The `TimeAlert()` function is called every second to update the 
///          time on the OLED display if we have the DEV_BOARD.
void TimeAlert(const DateTime& time)
   {
   timeWatchdog = millis();
   heartbeat = (heartbeat == LOW ? HIGH : LOW);
   // If there was a watchdog fault, clear it and resume, we're getting time again.
   if (wdtFault)
      {
      wdtFault = false;
      heartbeat = HIGH;
      OLED_DISPLAY(clearDisplay())
      }

   digitalWrite(HeartbeatLED, heartbeat);
   TIME_OLED(time)
   }

#if DEV_BOARD
/// @brief Update the OLED display with the current time if we have a DEV_BOARD.
/// @details The time is dispayed in the current hour format along with the 
///          weekday and full date.
/// @param time The current time reference from `TimeAlert()` method.
/// @remarks This method is only used if we have a DEV_BOARD which has an 
///          OLED display, otherwise we do nothing.
void TimeOLED(const DateTime &time)
   {
   if (!oledValid) { return; } // If OLED display is not valid, do not proceed.

   timeFormat = binClock.get_TimeFormat();
   if (timeFormat == nullptr)
      { timeFormat = "HH:mm:ss AP"; }

   int cursor;

   display.setCursor(0, 0);
   display.setTextSize(2);
   char timeBuf[32] = { 0 };
   char *timeStr = timeBuf;
   const char* timeStrTemp = time.toString(buffer, sizeof(buffer), timeFormat);
   if (timeStrTemp == nullptr)
      {
      timeStrTemp = "xx:xx:xx";
      }  
   strncpy (timeBuf, timeStrTemp, 32);
   
   if (timeStr[0] == ' ') { timeStr++; }
   char* spaceAP = strchr(timeStr, ' ');
      if (spaceAP) 
      { 
      *spaceAP = '\0'; // Split the string at the space before AM/PM
      spaceAP++;       // set pointer to AM/PM
      }

   display.write(timeStr); // Write just the numbers in size 2
   display.write(" ");     // Add a space after the time to clear.
   display.setTextSize(1);

   if (spaceAP) 
      { 
      display.write(" ");
      int timeStrLen = strlen(timeStr);
      int cursor = (timeStrLen) * 12;     // Each character is 6 pixels wide * TextSize (2) = 12;
      cursor += 4;                        // only 1/2 a small space before we write the AM/PM.
      display.setCursor(cursor, 8);  // Position at the end of the 1/2 space.
      display.fillRect(cursor, 0, 128 - cursor, 8, BLACK); // Clear the rest of the line.
      display.write(spaceAP);
      cursor += 2 * 6; // Move cursor to the end of the (1/2 " ") + AM/PM string
      display.setCursor(cursor, 8); // Position at the end of the AM/PM string.
      display.fillRect(cursor, 8, 128 - cursor, 8, BLACK); // Clear the rest of the line.
      }

   display.setCursor(0, 16);
   int dow = (time.dayOfTheWeek() + DateTime::dayNameOffset()) % 7;
   display.write(daysOfTheWeek[dow]);
   cursor = strlen(daysOfTheWeek[dow]) * 6;
   display.fillRect(cursor, 16, 128 - cursor, 8, BLACK); // Clear the rest of the line.
   display.setCursor(0, 24);
   display.write(time.toString(buffer, sizeof(buffer), "YYYY/MM/DD"));
   cursor = strlen("YYYY/MM/DD") * 6; // Each character is 6 pixels wide.
   display.fillRect(cursor, 24, 128 - cursor, 8, BLACK); // Clear the rest of the line.
   display.display();
   }
#endif   // DEV_BOARD 

#ifndef UNO_R3
/// @brief  Scan the I2C bus for devices and return the number found. We are looking for 
///         the RTC (and OLED display on dev boards) addresses.
/// @details The variables: `rtcValid` and `oledValid` are set if the respective
///          devices are found. These flags just serve to indicate the common I2C addresses
///          were found. All I2C addresses that were discovered are stored in the array.
/// @param  addrList Pointer to an array to store found I2C addresses.
/// @param  listSize Size of the address list array.
/// @return The number of I2C devices found.
/// @note Side effect: The `rtcValid` and `oledValid` flags are set if the respective
///       device addresses were found.  `Wire` is used to access the I2C bus.
int ScanI2C(byte *addrList, size_t listSize)
   {
   bool saveList = (addrList != nullptr) && (listSize > 0);
   byte error, address;
   size_t nDevices = 0;

   Wire.begin();
   delay(500); // vTaskDelay(pdMS_TO_TICKS(500));

   SERIAL_PRINTLN(F("Scanning for I2C devices ..."));
   for (address = 0x01; address < 0x7f; address++)
      {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      if (error == 0)
         {
         SERIAL_STREAM(nDevices + 1 << F(") I2C device found at address: ") << String(address, HEX) << endl)
         if (RTC_ADDR == address)
            {
            rtcValid = true;   // Found the RTC address.
            SERIAL_STREAM(F("    I2C RTC Address 0x") << String(address, HEX) << F(" has been found.") << endl)
            }
         else if (OLED_IIC_ADDR == address)
            {
            oledValid = true;   // Found the OLED address.
            SERIAL_STREAM(F("    I2C OLED Display Address 0x") << String(address, HEX) << F(" has been found.") << endl)
            }

         if (saveList && nDevices < listSize)
            {
            addrList[nDevices] = address;
            }

         nDevices++;
         }
      else if (error != 2)
         {
         SERIAL_STREAM(F("Error ") << error << F(" at address 0x") << String(address, HEX) << endl)
         }
      }

   if (nDevices == 0)
      {
      SERIAL_PRINTLN(F("No I2C devices were found"));
      }

   return nDevices;
   }
#endif

#undef OLED_DISPLAY
// #undef SERIAL_STREAM
// #undef SERIAL_PRINTLN      // Remove the MACRO definitions

#ifdef UNO_R3
   #undef oledValid    
   #undef rtcValid     
   #undef eepromValid  
#endif
