#include "sdkconfig.h"
#if CONFIG_ESP_WIFI_REMOTE_ENABLED
   #error "WPS is only supported in SoCs with native Wi-Fi support"
   #include <StopCompilationNow.h>
#endif

// #undef  TAG
// #define TAG "WPS_BIN"

#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_wps.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <Preferences.h>

// #include <WiFi.h>

/*
Change the definition of the WPS mode
from WPS_TYPE_PBC to WPS_TYPE_PIN in
the case that you are using pin type
WPS (pin is 00000000)
*/
// #define WPS_MODE WPS_TYPE_PBC

#define MAX_RETRY_ATTEMPTS     2

#define AP_PREFERENCES_NAME "WiFiAPs"
#define WPS_PARTITION_NAME  "WPS"
#define KEY_COUNTER_NAME    "APcount"

// #define xSemaphoreGive(xMutex)                      // *** DEBUG *** Disable the Semaphore code for now
// #define xSemaphoreTake(xMutex, portMAX_DELAY) true  // *** DEBUG *** Disable the Semaphore code for now
#define delay(x) vTaskDelay(pdMS_TO_TICKS(x))       // Use FreeRTOS delay instead of Arduino delay

const char *TAG = "WPS_BIN";
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

static int s_ap_creds_num = 0;
static int s_retry_num = 0;


String pskStr;  // The password for the WiFi
String ssidStr; // The SSID of the WiFi
wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();

String ssid;  // The SSID of the WiFi
String psk;   // The PSK value of the WiFi
Preferences wifiValues; // The Preferences memory for the WiFi info.

xSemaphoreHandle xMutex;  // Mutex for updating the screen.

String wifiGetAPinfo(String& ssid);
bool wifiSetAPinfo(String& ssid, String& psk);
static void start_wps(void);
void wpsStop();
String wpspin2string(uint8_t a[]);
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void got_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

/// @brief Get the PSK value for the given 'ssid'
/// @param ssid - the SSID of the AP to get the associated PSK value.
/// @return The PSK value if found, and empty string otherwise.
String wifiGetAPinfo(String& ssid)
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

 /// @brief Store the given 'ssid' and 'psk' in the namespace AP_PREFERENCES_NAME and partition WPS_PARTITION_NAME
 /// @param ssid - The SSID name to store. This is a key and the name must be unique.
 /// @param psk - The PSK value to store or update for the 'ssid'
 /// @return - Flag: true - Success, value stored/updated; false - Failure, unable to store/update the values.
bool wifiSetAPinfo(String& ssid, String& psk)
   {
   bool result = false;
   if (!ssid.isEmpty())
      {
      const char* targetSSID = ssid.c_str();
      bool opened = wifiValues.begin(AP_PREFERENCES_NAME, false, WPS_PARTITION_NAME);
      Serial.printf("Store WiFi Info SSID: %s and PSK: %s in namespace %s, partition %s\n",
                    ssid.c_str(), psk.c_str(), AP_PREFERENCES_NAME, WPS_PARTITION_NAME);
      if (opened)
         {
         if (!wifiValues.isKey(KEY_COUNTER_NAME))
            {
              // If the KEY_COUNTER_NAME is not present, clear the memory, initialize counter to 0.

            size_t storedBytes = wifiValues.putUChar(KEY_COUNTER_NAME, 0);
            if (storedBytes == 0)
               {
               Serial.printf("Failed to initialize %s in %s partition. Counter %s can't be stored.\n", KEY_COUNTER_NAME, WPS_PARTITION_NAME, KEY_COUNTER_NAME);
               return false;
               }
            }
         if (wifiValues.isKey(targetSSID))
            {
            String curValue = wifiValues.getString(targetSSID);
            if (curValue.isEmpty() || curValue != psk)
               {
               size_t storedSize = wifiValues.putString(targetSSID, psk);
               if (storedSize > 0) { result = true; }
               }
            }
         else // Add a new SSID & PSK to this partition
            {
            size_t freeEntries = wifiValues.freeEntries();
            if (freeEntries > 0)
               {
               uint8_t apSize = wifiValues.getUChar(KEY_COUNTER_NAME);
               apSize++;
               uint8_t storedBytes = wifiValues.putUChar(KEY_COUNTER_NAME, apSize);
               size_t storedString = wifiValues.putString(targetSSID, psk);
               if (storedBytes > 0 && storedString > 0) { result = true; }
               }
            }

         Serial.printf("Store WiFi SSID: %s and PSK: %s result: %s\n", ssid, psk, (result ? "SUCCESS" : "FAILED"));
         }

      wifiValues.end();
      }

   return result;
   }

static void start_wps(void)
   {
   ESP_ERROR_CHECK(esp_netif_init());
   ESP_ERROR_CHECK(esp_event_loop_create_default());
   esp_netif_t* sta_netif = esp_netif_create_default_wifi_sta();
   assert(sta_netif);

   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
   ESP_ERROR_CHECK(esp_wifi_init(&cfg));

   ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
   ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &got_ip_event_handler, NULL));

   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
   ESP_ERROR_CHECK(esp_wifi_start());

   ESP_LOGI(TAG, "start wps...");

   // Initialize config at runtime (C++ does not support designated initializers)
   memset(&config, 0, sizeof(config));
   config.wps_type = WPS_TYPE_PBC;
   strcpy(config.factory_info.manufacturer, "ESPRESSIF");
   strcpy(config.factory_info.model_number, "ESP32");
   strcpy(config.factory_info.model_name, "ESPRESSIF IOT");
   strcpy(config.factory_info.device_name, "ESP STATION");

   ESP_ERROR_CHECK(esp_wifi_wps_enable(&config));
   ESP_ERROR_CHECK(esp_wifi_wps_start(0));
   }

/* /// @brief Method to start the WPS connection to access the WiFi
void wpsStart()
   {
   // esp_wps_config_t config = 
       // WPS_CONFIG_INIT_DEFAULT(ESP_WPS_MODE); 
       // { .wps_type = WPS_TYPE_PBC, .factory_info =
       //   {
       //   {.manufacturer = "ESPRESSIF" },
       //   {.model_number = "ESP32" },
       //   {.model_name = "ESPRESSIF IOT" },
       //   {.device_name = "ESP STATION" },
       //   }
       // };

   //Same as config = WPS_CONFIG_INIT_DEFAULT(ESP_WPS_MODE);
   esp_wps_config_t config;
   memset(&config, 0, sizeof(esp_wps_config_t));
   config.wps_type = ESP_WPS_MODE;
   strcpy(config.factory_info.manufacturer, "ESPRESSIF");
   strcpy(config.factory_info.model_number, CONFIG_IDF_TARGET);
   strcpy(config.factory_info.model_name, "ESPRESSIF IOT");
   strcpy(config.factory_info.device_name, "ESP DEVICE");
   //strcpy(config.pin, "00000000");

   esp_err_t err = esp_wifi_wps_enable(&config);
   if (err != ESP_OK)
      {
      Serial.printf("WPS Enable Failed: 0x%x: %s\n", err, esp_err_to_name(err));
      return;

      err = esp_wifi_wps_start(0);
      if (err != ESP_OK)
         {
         Serial.printf("WPS Start Failed: 0x%x: %s\n", err, esp_err_to_name(err));
         }
      }
   }
 */

 /// @brief Method to stop the WPS WiFi connection sequence.
void wpsStop()
   {
   esp_err_t err = esp_wifi_wps_disable();
   if (err != ESP_OK)
      {
      Serial.printf("WPS Disable Failed: 0x%x: %s\n", err, esp_err_to_name(err));
      }
   }

 /// @brief Method to convert a WPS Pin value to a string
 /// @param a The PIN value as an array of digits.
 /// @return The PIN as a string
String wpspin2string(uint8_t a[])
   {
   char wps_pin[9];
   for (int i = 0; i < 8; i++)
      {
      wps_pin[i] = a[i];
      }
   wps_pin[8] = '\0';
   return (String)wps_pin;
   }

 // WARNING: WiFiEvent is called from a separate FreeRTOS task (thread)!
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
   {
   static int ap_idx = 1;

   switch (event_id)
      {
      case WIFI_EVENT_STA_START:
         ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
         break;

      case WIFI_EVENT_STA_DISCONNECTED:
         ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
         if (s_retry_num < MAX_RETRY_ATTEMPTS)
            {
            esp_wifi_connect();
            s_retry_num++;
            }
         else if (ap_idx < s_ap_creds_num)
            {
            /* Try the next AP credential if first one fails */
            if (ap_idx < s_ap_creds_num)
               {
               ESP_LOGI(TAG, "Connecting to SSID: %s, Passphrase: %s",
                     wps_ap_creds[ap_idx].sta.ssid, wps_ap_creds[ap_idx].sta.password);
               ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wps_ap_creds[ap_idx++]));
               esp_wifi_connect();
               }
            s_retry_num = 0;
            }
         else
            {
            ESP_LOGI(TAG, "Failed to connect!");
            }

         break;

      case WIFI_EVENT_STA_WPS_ER_SUCCESS:
         ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_SUCCESS");
         {
         wifi_event_sta_wps_er_success_t* evt =
               (wifi_event_sta_wps_er_success_t*)event_data;
         int i;

         if (evt)
            {
            s_ap_creds_num = evt->ap_cred_cnt;
            for (i = 0; i < s_ap_creds_num; i++)
               {
               memcpy(wps_ap_creds[i].sta.ssid, evt->ap_cred[i].ssid,
                  sizeof(evt->ap_cred[i].ssid));
               memcpy(wps_ap_creds[i].sta.password, evt->ap_cred[i].passphrase,
                  sizeof(evt->ap_cred[i].passphrase));
               }
               /* If multiple AP credentials are received from WPS, connect with first one */
            ESP_LOGI(TAG, "Connecting to SSID: %s, Passphrase: %s",
            wps_ap_creds[0].sta.ssid, wps_ap_creds[0].sta.password);
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wps_ap_creds[0]));
            }
            /*
            * If only one AP credential is received from WPS, there will be no event data and
            * esp_wifi_set_config() is already called by WPS modules for backward compatibility
            * with legacy apps. So directly attempt connection here.
            */
         ESP_ERROR_CHECK(esp_wifi_wps_disable());
         esp_wifi_connect();
         }
         break;

      case WIFI_EVENT_STA_WPS_ER_FAILED:
         ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_FAILED");
         ESP_ERROR_CHECK(esp_wifi_wps_disable());
         ESP_ERROR_CHECK(esp_wifi_wps_enable(&config));
         ESP_ERROR_CHECK(esp_wifi_wps_start(0));
         break;

      case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
         ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_TIMEOUT");
         ESP_ERROR_CHECK(esp_wifi_wps_disable());
         ESP_ERROR_CHECK(esp_wifi_wps_enable(&config));
         ESP_ERROR_CHECK(esp_wifi_wps_start(0));
         break;

      case WIFI_EVENT_STA_WPS_ER_PIN:
         ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_PIN");
         {
         /* display the PIN code */
         wifi_event_sta_wps_er_pin_t* event = (wifi_event_sta_wps_er_pin_t*)event_data;
         ESP_LOGI(TAG, "WPS_PIN = " PINSTR, PIN2STR(event->pin_code));
         }
         break;

      default:
         // ESP_LOGI(TAG, "Unknown 'event_id' %d in %s, Line: %d (%s)", event_id, __PRETTY_FUNCTION__, __LINE__, __FILENAME__);
         Serial.printf("Unknown 'event_id' %d in %s, Line: %d (%s)", event_id, __PRETTY_FUNCTION__, __LINE__, __FILENAME__);
         break;
      }
   }

static void got_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
   {
   ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
   ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
   }

void wps_main(void)
   {
    /* Initialize NVS — it is used to store PHY calibration data */
   esp_err_t ret = nvs_flash_init();
   if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
      {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
      }

   ESP_ERROR_CHECK(ret);
   start_wps();
   }


/*
 /// @brief The event for the WiFi. Called by an RTOS thread whenever there is a WiFi event.
 /// @param event The 'WiFiEvent_t' event instance that triggered the call
 /// @param info The associated event information, e.g. an error code.
void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info)
   {
   bool wifiSaveResult = false;
   switch (event)
      {
         case ARDUINO_EVENT_WIFI_READY:
            Serial.println("WiFi Ready event: ");
            break;
         case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("Station Mode Started");
            break;
         case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            wifiValid = true;
            if (wpsRunning)
               {
               wpsStop(); // Stop WPS if it was running.
               wpsRunning = false;
               digitalWrite(PIN_WIFI_LED, HIGH); // Turn OFF the WPS LED  
               }

            Serial.println("***********************************************");
            Serial.println("Connected to :" + String(WiFi.SSID()));
            Serial.print("Got IP: ");
            Serial.println(WiFi.localIP());
            pskStr = WiFi.psk();
            ssidStr = WiFi.SSID();
            Serial.printf("Connected to: %s; Using P/W: %s\n", ssidStr, pskStr);
            wifiSaveResult = wifiSetAPinfo(ssidStr, pskStr);
            Serial.printf("%s: Saving the WiFi Info: SSID % and PSK %s\n", (wifiSaveResult ? "SUCCESS" : "FAILED"), ssidStr, pskStr);
            vTaskDelay(pdMS_TO_TICKS(500));
            flashLEDca(PIN_WIFI_LED, 5);

            esp_wifi_get_config(WIFI_IF_STA, &conf);
            Serial.printf("WPS:: SSID=%s PASSWORD=%s\n", conf.sta.ssid, conf.sta.password);
            Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++");
            break;
         case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Disconnected from station, attempting reconnection");
            WiFi.reconnect();
            break;
         case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.println("WPS Successful, stopping WPS and connecting to: " + String(WiFi.SSID()));
            wpsStop();
            delay(10);
            WiFi.begin();
            break;
         case ARDUINO_EVENT_WPS_ER_FAILED:
            Serial.println("WPS Failed, retrying");
            wpsStop();
            wpsStart();
            break;
         case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println("WPS Timedout, retrying");
            wpsStop();
            wpsStart();
            break;
         case ARDUINO_EVENT_WPS_ER_PIN:
            Serial.println("WPS_PIN = " + wpspin2string(info.wps_er_pin.pin_code));
            break;
         default:
            Serial.printf("Logic Error: (%d) is not included in switch statement at %s line: %d\n", event, __FILE__, __LINE__);
            break;
      }
   }
*/
   