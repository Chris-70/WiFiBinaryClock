#include <arduino.h>

#include "BinaryClock.h"

// #include <Wire.h>
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
#define OLED_IIC_ADDR 0x3c
#define I2C_ADDRESS 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
                       // e.g. the one with GM12864-77 written on it
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 100000UL, 100000UL);
#define BEGIN(ADDR,RESET) display.begin(SSD1306_SWITCHCAPVCC, (ADDR), (RESET), true)
#define I2C_SIZE     16
#define RTC_ADDR     0x68
#define RTC_EEPROM   0x57
#ifndef LED_HEART
#define LED_HEART LED_BUILTIN    // Heartbeat LED
#endif

using namespace BinaryClockShield;
#define BINARYCLOCK  BinaryClock::get_Instance()
// put function declarations here:
int ScanI2C(byte* addrList, size_t listSize);

char buffer[32] = {0};
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

BinaryClock& binClock = BinaryClock::get_Instance(); // Get the singleton instance of BinaryClock
static long watchdogTimeout = 2100; 
volatile long timeWatchdog = 0;
bool wdtFault = false;
long curMillis = 0;
long deltaMillis = 0;
bool oledValid = false;
bool rtcValid = false;
bool eepromValid = false;
int heartbeat = LOW;             // Heartbeat LED state.
byte i2cList[I2C_SIZE] = { 0 };

#define DISPLAY(CMD) if (oledValid) { display.CMD; }

void TimeAlert(DateTime time)
   {
   timeWatchdog = millis();
   heartbeat = (heartbeat == LOW ? HIGH : LOW);
   if (wdtFault)
      {
      wdtFault = false;
      heartbeat = HIGH;
      DISPLAY(clearDisplay())
      }

   digitalWrite(LED_HEART, heartbeat);
   
   DISPLAY(setCursor(0, 0))
   DISPLAY(setTextSize(2))
   DISPLAY(write(time.toString(buffer, sizeof(buffer), "hh:mm:ss")))
   DISPLAY(setTextSize(1))
   DISPLAY(setCursor(0, 16))
   DISPLAY(write(daysOfTheWeek[time.dayOfTheWeek()]))
   DISPLAY(setCursor(0, 24))
   DISPLAY(write(time.toString(buffer, sizeof(buffer), "YYYY/MM/DD")))
   DISPLAY(display())
   }

int BuiltinLED = LED_BUILTIN;

void setup()
   {
   Serial.begin(115200); // Start the serial communication at 115200 baud rate

   pinMode(BuiltinLED, OUTPUT);
   digitalWrite(BuiltinLED, HIGH);
   pinMode(LED_HEART, OUTPUT);
   digitalWrite(LED_HEART, LOW);

   Wire.begin();
   int i2cDevices = ScanI2C(i2cList, I2C_SIZE);
   Serial << endl << "Found: " << i2cDevices << " I2C Devices." << endl << "  Known devices are:" << endl;   // *** DEBUG ***
   for (int i = 0; i < i2cDevices; i++)
      {
      if (i2cList[i] == OLED_IIC_ADDR) { oledValid = true; Serial << "  - OLED display is present." << endl; } // *** DEBUG *** Serial...
      if (i2cList[i] == RTC_ADDR)       { rtcValid = true; Serial << "  - RTC is present." << endl;}           // *** DEBUG *** Serial...
      if (i2cList[i] == RTC_EEPROM)  { eepromValid = true; Serial << "  - RTC EEPROM is present." << endl; }   // *** DEBUG *** Serial...
      }
   delay(2500);

   // // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
   // display.begin(SSD1306_SWITCHCAPVCC, OLED_IIC_ADDR);  // initialize with the I2C addr 0x3C (for the 128x32)
   bool displayResult = false;
   if (oledValid)
      {
      displayResult = BEGIN(I2C_ADDRESS, true);
      oledValid = displayResult;
      }
   DISPLAY(setTextColor(WHITE, BLACK))
   DISPLAY(setTextSize(1))
   DISPLAY(setTextWrap(false))
   DISPLAY(display())
   delay(500);

   Serial << "OLED display is: " << (oledValid? "Installed;" : "Missing; ") << "Begin: " << (displayResult? "Success: " : "Failure: ") << "Clear Display." << endl; // *** DEBUG ***
   DISPLAY(clearDisplay())
   DISPLAY(print("BinaryClock Setup\n"))
   DISPLAY(display())
   Serial << "Starting the BinaryClock Setup" << endl;

   binClock.setup(!oledValid);   // If the OLED display is installed, it's likely a dev board, not the shield.
   binClock.set_Brightness(20);

   binClock.registerTimeCallback(&TimeAlert);
   delay(250);
   DISPLAY(setCursor(0, 8))
   DISPLAY(println("Entering Loop() now"))
   Serial << "Entering Loop() now" << endl;
   DISPLAY(display())
   delay(250);
   DISPLAY(clearDisplay())
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
      DISPLAY(clearDisplay())
      DISPLAY(setCursor(0, 0))
      DISPLAY(setTextSize(4))
      DISPLAY(write("Fault"))
      DISPLAY(display())
      wdtResult = false;
      digitalWrite(LED_HEART, LOW);
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
      Serial << "Watchdog Timer Triggered after " << deltaMillis / 1000.0 << "seconds. " << endl;
      }
   }

int ScanI2C(byte *addrList, size_t listSize)
   {
   bool saveList = (addrList != nullptr) && (listSize > 0);
   byte error, address;
   int nDevices = 0;

   vTaskDelay(pdMS_TO_TICKS(500));

   Serial.println("Scanning for I2C devices ...");
   for (address = 0x01; address < 0x7f; address++)
      {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      if (error == 0)
         {
         Serial.printf("I2C device found at address 0x%02X\n", address);
         if (OLED_IIC_ADDR == address)
            {
            oledValid = true;   // Found the OLED address.
            Serial.printf("Success: I2C OLED Address 0x%02X has been found.\n", address);
            }

         if (saveList && nDevices < listSize)
            {
            addrList[nDevices] = address;
            }

         nDevices++;
         }
      else if (error != 2)
         {
         Serial.printf("Error %d at address 0x%02X\n", error, address);
         }
      }

   if (nDevices == 0)
      {
      Serial.println("No I2C devices found");
      }

   vTaskDelay(pdMS_TO_TICKS(500));
   return nDevices;
   }

