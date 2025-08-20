#include <arduino.h>

#include "BinaryClock.h"

// Development board specific; has additional hardware buttons and OLED display.
// Not part of the Binary Clock Shield, but used to develop and test the code.
// This board replaces the Binary Clock Shield during development.
#if DEV_BOARD
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef NO_ADAFRUIT_SSD1306_COLOR_COMPATIBILITY
#define BLACK SSD1306_BLACK     ///< Draw 'off' pixels (i.e. 0)
#define WHITE SSD1306_WHITE     ///< Draw 'on' pixels  (i.e. 1)
#define INVERSE SSD1306_INVERSE ///< Invert pixels     (i.e. 2)
#endif
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // / QT-PY  XIAO
#define I2C_ADDRESS 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
                       // e.g. the one with GM12864-77 written on it
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 100000UL, 100000UL);
#define BEGIN(ADDR,RESET) display.begin(SSD1306_SWITCHCAPVCC, (ADDR), (RESET), true)
#else
// Removes the code from compilation, replaced with whitespace.
#define BEGIN(ADDR, RESET)
#endif

#define I2C_SIZE     16
#define RTC_ADDR     0x68
#define RTC_EEPROM   0x57
#define OLED_IIC_ADDR 0x3c
#ifndef LED_HEART
// LED used to communicate errors.
#define LED_HEART LED_BUILTIN 
#endif

using namespace BinaryClockShield;
#define BINARYCLOCK  BinaryClock::get_Instance()

// put function declarations here:
void TimeAlert(DateTime time);
void setup();
void loop();
bool checkWatchdog();
int ScanI2C(byte* addrList, size_t listSize);

#if DEV_BOARD
char buffer[32] = {0};
const char *format12 = { "HH:mm:ss AP" }; // 12 Hour time format
const char *format24 = { "hh:mm:ss" };    // 24 Hour time format
const char *timeFormat = format24;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
#endif 

BinaryClock& binClock = BinaryClock::get_Instance(); // Get the singleton instance of BinaryClock
static long watchdogTimeout = 2100; 
volatile long timeWatchdog = 0;
bool wdtFault = false;
long curMillis = 0;
long deltaMillis = 0;
bool oledValid = false;
bool rtcValid = false;
bool eepromValid = false;
int heartbeat = LOW;             // Heartbeat LED state: OFF
byte i2cList[I2C_SIZE] = { 0 };
int HeartbeatLED = LED_HEART;

#if DEV_BOARD
// Wrap the OLED display in a MACRO to first test if the display is available.
#define OLED_DISPLAY(CMD) if (oledValid) { display.CMD; }
#else
// Removes OLED_DISPLAY() code from compilation, replaced with whitespace.
#define OLED_DISPLAY(CMD) 
#endif

void TimeAlert(DateTime time)
   {
   timeWatchdog = millis();
   heartbeat = (heartbeat == LOW ? HIGH : LOW);
   if (wdtFault)
      {
      wdtFault = false;
      heartbeat = HIGH;
      OLED_DISPLAY(clearDisplay())
      }

   digitalWrite(HeartbeatLED, heartbeat);

   #if DEV_BOARD
   if (!oledValid) { return; } // If OLED display is not valid, do not proceed.

   timeFormat = binClock.get_Is12HourFormat() ? format12 : format24;
   int cursor;

   OLED_DISPLAY(setCursor(0, 0))
   OLED_DISPLAY(setTextSize(2))
   char timeBuf[32] = { 0 };
   char *timeStr = timeBuf;
   strncpy (timeBuf, time.toString(buffer, sizeof(buffer), timeFormat), 32);
   if (timeStr[0] == ' ') { timeStr++; }
   char* spaceAP = strchr(timeStr, ' ');
   if (spaceAP) 
      { 
      *spaceAP = '\0'; // Split the string at the space before AM/PM
      spaceAP++;       // set pointer to AM/PM
      }
   OLED_DISPLAY(write(timeStr)) // Write just the numbers in size 2
   OLED_DISPLAY(write(" "))     // Add a space after the time to clear.
   OLED_DISPLAY(setTextSize(1))
   if (spaceAP) 
      { 
      OLED_DISPLAY(write(" "))    
      int timeStrLen = strlen(timeStr);
      int cursor = (timeStrLen) * 12;     // Each character is 6 pixels wide * TextSize (2) = 12;
      cursor += 4;                        // only 1/2 a space before we write the AM/PM.
      OLED_DISPLAY(setCursor(cursor, 8))  // Position at the end of the 1/2 space.
      OLED_DISPLAY(fillRect(cursor, 0, 128 - cursor, 8, BLACK)) // Clear the rest of the line.
      OLED_DISPLAY(write(spaceAP)) 
      cursor += 2 * 6; // Move cursor to the end of the " " + AM/PM string
      OLED_DISPLAY(setCursor(cursor, 8)) // Position at the end of the AM/PM string.
      // OLED_DISPLAY(write("  ")) // Add two spaces after the AM/PM to clear the screen AM/PM.
      OLED_DISPLAY(fillRect(cursor, 8, 128 - cursor, 8, BLACK)) // Clear the rest of the line.
      }

   OLED_DISPLAY(setCursor(0, 16))
   int dow = time.dayOfTheWeek() % 7;
   OLED_DISPLAY(write(daysOfTheWeek[dow]))
   cursor = strlen(daysOfTheWeek[dow]) * 6;
   OLED_DISPLAY(fillRect(cursor, 16, 128 - cursor, 8, BLACK)) // Clear the rest of the line.
   OLED_DISPLAY(setCursor(0, 24))
   OLED_DISPLAY(write(time.toString(buffer, sizeof(buffer), "YYYY/MM/DD")))
   cursor = strlen("YYYY/MM/DD") * 6; // Each character is 6 pixels wide.
   OLED_DISPLAY(fillRect(cursor, 24, 128 - cursor, 8, BLACK)) // Clear the rest of the line.
   OLED_DISPLAY(display())
   #endif   // DEV_BOARD
   }

void setup()
   {
   Serial.begin(115200); // Start the serial communication at 115200 baud rate

   pinMode(HeartbeatLED, OUTPUT);
   digitalWrite(HeartbeatLED, LOW);

   Wire.begin();
   int i2cDevices = ScanI2C(i2cList, I2C_SIZE);
   #if DEV_BOARD
   Serial << endl << "Found: " << i2cDevices << " I2C Devices." << endl << "  Known devices are:" << endl;   // *** DEBUG ***
   #endif
   for (int i = 0; i < i2cDevices; i++)
      {
      if (i2cList[i] == OLED_IIC_ADDR) { oledValid = true; Serial << "  - OLED display is present." << endl; /* */ } // *** DEBUG *** Serial...
      if (i2cList[i] == RTC_ADDR)       { rtcValid = true; Serial << "  - RTC is present." << endl;          /* */ } // *** DEBUG *** Serial...
      if (i2cList[i] == RTC_EEPROM)  { eepromValid = true; Serial << "  - RTC EEPROM is present." << endl;   /* */ } // *** DEBUG *** Serial...
      }
   delay(500);

   #if DEV_BOARD
   // // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
   // display.begin(SSD1306_SWITCHCAPVCC, OLED_IIC_ADDR);  // initialize with the I2C addr 0x3C (for the 128x32)
   bool displayResult = false;
   if (oledValid)
      {
      displayResult = BEGIN(I2C_ADDRESS, true);
      oledValid = displayResult;
      }
   OLED_DISPLAY(setTextColor(WHITE, BLACK))
   OLED_DISPLAY(setTextSize(1))
   OLED_DISPLAY(setTextWrap(false))
   OLED_DISPLAY(display())
   delay(500);
   #else
   #define displayResult false
   #endif

   #if DEV_BOARD
   Serial << "OLED display is: " << (oledValid? "Installed;" : "Missing; ") << "Begin: " << (displayResult? "Success: " : "Failure: ") 
          << " Clear Display: " << (oledValid? "YES" : "NO") << endl; // *** DEBUG ***
   Serial << "Starting the BinaryClock Setup" << endl; // *** DEBUG ***
   #endif

   binClock.setup(!oledValid);   // If the OLED display is installed, it's likely a dev board, not the shield.
   binClock.set_Brightness(20);

   binClock.registerTimeCallback(&TimeAlert);
   delay(250);
   Serial << "Entering Loop() now" << endl;
   delay(250);
   OLED_DISPLAY(clearDisplay())

   // binClock.set_isSerialTime(!binClock.buttonDebugTime.read()); // *** DEBUG ***
   timeWatchdog = millis();   // Reset the Watchdog Timer.
   }

bool checkWatchdog()   
   {
   bool wdtResult = true;

   curMillis = millis();
   deltaMillis = curMillis - timeWatchdog;
   if (deltaMillis > watchdogTimeout)
      {
      wdtFault = true;
      OLED_DISPLAY(clearDisplay())
      OLED_DISPLAY(setCursor(0, 0))
      OLED_DISPLAY(setTextSize(4))
      OLED_DISPLAY(write("Fault"))
      OLED_DISPLAY(display())
      wdtResult = false;
      digitalWrite(HeartbeatLED, LOW);
      }

   return wdtResult;
   }

void loop()
   {
   static bool wdtError = false;

   binClock.loop();
   checkWatchdog();
   if (checkWatchdog())
      {
      wdtError = false;
      }
   else if (!wdtError) // Display just once per WDT fault
      {
      wdtError = true;
      Serial << F("Watchdog Timer Triggered after ") << deltaMillis / 1000.0 << F("seconds. ") << endl;
      }
   }

int ScanI2C(byte *addrList, size_t listSize)
   {
   bool saveList = (addrList != nullptr) && (listSize > 0);
   byte error, address;
   size_t nDevices = 0;

   Wire.begin();
   delay(500); // vTaskDelay(pdMS_TO_TICKS(500));

   Serial.println(F("Scanning for I2C devices ..."));
   for (address = 0x01; address < 0x7f; address++)
      {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      if (error == 0)
         {
         #if DEV_BOARD
         Serial << nDevices+1 << F(") I2C device found at address: ") << String(address, HEX) << endl; // *** DEBUG ***
         #endif
         if (OLED_IIC_ADDR == address)
            {
            oledValid = true;   // Found the OLED address.
            #if DEV_BOARD
            Serial << F("    I2C OLED Display Address 0x") << String(address, HEX) << F(" has been found.") << endl; // *** DEBUG ***
            #endif
            }

         if (saveList && nDevices < listSize)
            {
            addrList[nDevices] = address;
            }

         nDevices++;
         }
      else if (error != 2)
         {
         #if DEV_BOARD
         Serial << F("Error ") << error << F(" at address 0x") << String(address, HEX) << endl; // *** DEBUG ***
         #endif
         }
      }

   if (nDevices == 0)
      {
      Serial.println(F("No I2C devices were found"));
      }

   return nDevices;
   }

#if DEV_BOARD   
   #undef OLED_DISPLAY
#endif