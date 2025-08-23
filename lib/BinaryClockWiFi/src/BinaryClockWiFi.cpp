#include "sdkconfig.h"
#if CONFIG_ESP_WIFI_REMOTE_ENABLED
   #error "WPS is only supported in SoCs with native Wi-Fi support"
   #include <StopCompilationNow.h>
#endif

// #undef  TAG
// #define TAG "WPS_BIN"

// #include <stdio.h>
// #include <time.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/semphr.h>
// #include <freertos/event_groups.h>

// #include <esp_system.h>
// #include <esp_netif.h>
// #include <esp_event.h>
// #include <esp_wps.h>
// #include <esp_wifi.h>
// #include <esp_mac.h>
// #include <esp_log.h>

#include <WiFi.h>

#include <nvs_flash.h>
#include <Preferences.h>

#include "BinaryClockWiFi.h"

namespace BinaryClockShield
   {
   void BinaryClockWifi::setup()
      {
      bool wifiValid = false;
      // WiFi.disconnect();
      Serial.println("Scanning for WiFi Access Points.");
      int n = wiFiScan();
      if (n > 0)
         {
         Serial.printf("%d WiFi networks found\n", n);
         for (int i = 0; i < n; ++i) {
            String ssid = WiFi.SSID(i);
            String psk = wifiGetAPinfo(ssid);
            if (!psk.isEmpty())
               {
               Serial.printf("Connecting to SSID: %s with PSK: %s\n", ssid.c_str(), psk.c_str());
               WiFi.begin((char *)ssid.c_str(), (char *)psk.c_str());
               delay(10000);
               if (WiFi.status() == WL_CONNECTED)
                  {
                  Serial.printf("Connected to SSID: %s with IP Address: %s\n", ssid.c_str(), WiFi.localIP().toString().c_str());
                  wifiValid = true;
                  break;
                  }
               else
                  {
                  Serial.printf("Failed to connect to SSID: %s\n", ssid.c_str());
                  }
               }
            else
               {
               Serial.printf("No PSK found for SSID: %s\n", ssid.c_str());
               }
            } // END for
         }

      if (!wifiValid)
         {
         Serial.println("No valid WiFi credentials found: *** Starting WPS ***.");
         }
      // WiFi.begin("your-SSID", "your-PASSWORD");
      }

   void BinaryClockWifi::loop()
      {
      // put your main code here, to run repeatedly:
      }
      
   BinaryClockWifi::BinaryClockWifi()
      {}
   BinaryClockWifi::~BinaryClockWifi()
      {}

   int BinaryClockWifi::wiFiScan()
      {
      int n = WiFi.scanNetworks();
      Serial.printf("scan done, found %d networks\n", n);
      for (int i = 0; i < n; ++i) {
         String ssid = WiFi.SSID(i);
         int32_t rssi = WiFi.RSSI(i);
         Serial.printf("   %d) Found WiFi network: %s (%d dBm)\n", i, ssid.c_str(), rssi);
      }
      return n;
      }

   /// @brief Get the PSK value for the given 'ssid'
   /// @param ssid - the SSID of the AP to get the associated PSK value.
   /// @return The PSK value if found, and empty string otherwise.
   String BinaryClockWifi::wifiGetAPinfo(String& ssid)
      {
      String psk;

      if (!ssid.isEmpty())
         {
         const char* targetSSID = ssid.c_str();
         bool opened = wifiValues.begin(AP_PREFERENCES_NAME, true, WPS_PARTITION_NAME);
         if (opened && wifiValues.isKey(KEY_COUNTER_NAME))
            {
            uint8_t apSize = wifiValues.getUChar(KEY_COUNTER_NAME);
            if (apSize > 0)
               {
               psk.clear();
               if (wifiValues.isKey(targetSSID))
                  {
                  psk = wifiValues.getString(targetSSID);
                  //return psk;
                  }
               }
            }

         Serial.printf("GET WiFi Info SSID: %s and PSK: %s result: %s\n", ssid, psk, (psk.isEmpty() ? "FAILED" : "SUCCESS"));
         wifiValues.end();
         }

      return psk;
      }

   }  // END namespace BinaryClockShield