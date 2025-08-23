#pragma once
#ifndef _BINARY_CLOCK_NTP_H_
#define _BINARY_CLOCK_NTP_H_

#include <Arduino.h>             // Arduino core library

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <esp_system.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_wps.h>
#include <esp_wifi.h>
#include <esp_mac.h>
// #include <wifi.h>
#include <nvs_flash.h>
#include <Preferences.h>

#define MAX_RETRY_ATTEMPTS     2

#define AP_PREFERENCES_NAME "WiFiAPs"
#define WPS_PARTITION_NAME  "WPS"
#define KEY_COUNTER_NAME    "APcount"

// #define xSemaphoreGive(xMutex)                      // *** DEBUG *** Disable the Semaphore code for now
// #define xSemaphoreTake(xMutex, portMAX_DELAY) true  // *** DEBUG *** Disable the Semaphore code for now
#define delay(x) vTaskDelay(pdMS_TO_TICKS(x))       // Use FreeRTOS delay instead of Arduino delay

namespace BinaryClockShield
   {
   /*
   Change the definition of the WPS mode
   from WPS_TYPE_PBC to WPS_TYPE_PIN in
   the case that you are using pin type
   WPS (pin is 00000000)
   */
   // #define WPS_MODE WPS_TYPE_PBC

   class BinaryClockWifi
      {
   public:
      int wifiScan();
      String wifiGetAPinfo(String& ssid);
      bool wifiSetAPinfo(String& ssid, String& psk);
      static void start_wps(void);
      void wpsStop();
      String wpspin2string(uint8_t a[]);
      static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
      static void got_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

      void wps_main(void);

      static BinaryClockWifi& get_Instance()
         {
         static BinaryClockWifi instance; // Guaranteed to be destroyed, instantiated on first use
         return instance;
         }

   protected:
      BinaryClockWifi();
      virtual ~BinaryClockWifi();

      // Singleton pattern - Disable these constructors and assignment operators.
      BinaryClockWifi(const BinaryClockWifi&) = delete;            // Prevent copying
      BinaryClockWifi& operator=(const BinaryClockWifi&) = delete; // Prevent assignment
      BinaryClockWifi(BinaryClockWifi&&) = delete;                 // Prevent move semantics
      BinaryClockWifi& operator=(BinaryClockWifi&&) = delete;      // Prevent move assignment
      void init_wifi();

   private:

      const char* TAG = "WPS_BIN";
      bool timeSet = false;         // Flag: The time was set from the NTP server
      bool wpsRunning = false;      // Flag: WPS is running, used to avoid
      volatile int curPIR = LOW;    // The current PIR value LOW or HIGH
      volatile bool wifiValid = false; // Flag: We have connected over WiFi successfully.
      volatile bool enableWPS = false; // Flag: Indicates the WPS button was pressed.

      const char* ntpServer1 = "ca.pool.ntp.org";
      const char* ntpServer2 = "pool.ntp.org";
      const char* ntpServer3 = "time-a.nist.gov";
      const long  gmtOffset_sec = -5 * 3600;    // -5 hours from GMT
      const int   daylightOffset_sec = 3600;    // DST offset is 1 hour

      // static esp_wps_config_t config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);
      static esp_wps_config_t config;
      static wifi_config_t wps_ap_creds[MAX_WPS_AP_CRED];

      static int s_ap_creds_num;
      static int s_retry_num;


      String pskStr;  // The password for the WiFi
      String ssidStr; // The SSID of the WiFi
      wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();

      String ssid;  // The SSID of the WiFi
      String psk;   // The PSK value of the WiFi
      Preferences wifiValues; // The Preferences memory for the WiFi info.

      xSemaphoreHandle xMutex;  // Mutex for updating the screen.

      }; // END class BinaryClockWiFi

   }  // END namespace BinaryClockShield

#endif