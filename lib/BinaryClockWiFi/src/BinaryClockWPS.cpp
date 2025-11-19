
#ifndef INLINE_HEADER
   #define INLINE_HEADER false
#endif   
#if INLINE_HEADER
// If you need the class definition in the same file, e.g. for CoPilot, 
// just copy-paste the contents of the header file here and change the define.
#else
   #include "BinaryClockWPS.h"            /// Header for BinaryClockWPS class.
#endif // INLINE_HEADER

//################################################################################//
#ifndef SERIAL_OUTPUT
   #define SERIAL_OUTPUT   true  // true to enable; false to disable
#endif
#ifndef DEV_CODE
   #define DEV_CODE        true  // true to enable; false to disable
#endif
#ifndef DEBUG_OUTPUT
   #define DEBUG_OUTPUT    true  // true to enable; false to disable
#endif
#ifndef PRINTF_OK
   #define PRINTF_OK       true  // true to enable; false to disable
#endif

#include "SerialOutput.Defines.h"      // For all the serial output macros.
//################################################################################//

namespace BinaryClockShield
   {
   BinaryClockWPS::BinaryClockWPS()
         : timeout(DEFAULT_WPS_TIMEOUT_MS), wpsActive(false), wpsSuccess(false), wpsTimeout(false)
      {
      memset(&wpsConfig, 0, sizeof(wpsConfig));
      }

   BinaryClockWPS::~BinaryClockWPS()
      {
      CancelWPS();
      cleanupWPS();
      }

   BinaryClockWPS& BinaryClockWPS::get_Instance()
      {
      static BinaryClockWPS instance;
      return instance;
      }

   WPSResult BinaryClockWPS::ConnectWPS()
      {
      WPSResult result;
      uint32_t startTime = millis();

      SERIAL_STREAM(endl << "Starting WPS Push Button connection (timeout: " << timeout << "ms)" << endl)

      // Ensure WiFi is in station mode
      WiFi.enableSTA(true);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      WiFi.disconnect(true);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      WiFi.mode(WIFI_STA);

      // Initialize WPS
      if (!initWPS())
         {
         result.errorMessage = "Failed to initialize WPS";
         return result;
         }

      // Reset event flags
      wpsSuccess = false;
      wpsTimeout = false;
      wpsError = "";
      wpsActive = true;

      // Register event handler
      esp_err_t err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler, this);
      if (err != ESP_OK)
         {
         result.errorMessage = "Failed to register WiFi event handler: " + EspErrorToString(err);
         cleanupWPS();
         return result;
         }

      // Start WPS
      err = esp_wifi_wps_start(0);
      if (err != ESP_OK)
         {
         result.errorMessage = "Failed to start WPS: " + EspErrorToString(err);
         esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
         cleanupWPS();
         return result;
         }

      SERIAL_STREAM("WPS started - Please press the WPS button on your router now..." << endl)

      // Wait for WPS completion or timeout
      uint32_t lastStatus = millis();
      while (wpsActive && (millis() - startTime) < timeout)
         {
         // Print status every 10 seconds
         if (millis() - lastStatus > 10000)
            {
            SERIAL_STREAM("WPS still waiting... (" << (millis() - startTime) / 1000 << "s elapsed)" << endl)
            lastStatus = millis();
            }

         // Check for completion
         if (wpsSuccess)
            {
            SERIAL_STREAM("WPS process completed successfully." << endl)
            // Wait a bit for WiFi to fully connect
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            if (WiFi.status() == WL_CONNECTED)
               {
               result.success = true;
               result.credentials = extractCredentials();
               result.connectionTimeMs = millis() - startTime;

               SERIAL_STREAM("WPS connection successful!" << endl)
               SERIAL_STREAM("Connected to: " << result.credentials.ssid << endl)
               SERIAL_STREAM("IP Address: " << WiFi.localIP() << endl)
               SERIAL_STREAM("Connection time: " << result.connectionTimeMs/1000.0 << " seconds." << endl)
               }
            else
               {
               APCreds localCreds = extractCredentials();
               result.errorMessage = "WPS succeeded but WiFi not connected: " + WiFiStatusString(WiFi.status());
               SERIAL_STREAM("WPS: creds: (" << localCreds.ssid << ", " << localCreds.bssid << ", " << localCreds.pw << ")" << endl)
               result.credentials = localCreds;
               uint8_t bssidArray[6];
               localCreds.bssidToBytes(bssidArray);
               SERIAL_STREAM("WPS: BSSID bytes: " << String(bssidArray[0], HEX) << ":" << String(bssidArray[1], HEX) << ":" << String(bssidArray[2], HEX) 
                     << ":" << String(bssidArray[3], HEX) << ":" << String(bssidArray[4], HEX) << ":" << String(bssidArray[5], HEX) << endl)
               WiFi.begin(localCreds.ssid.c_str(), localCreds.pw.c_str(), 0, bssidArray, true);
               }

            // esp_wifi_wps_disable();
            WiFi.setAutoReconnect(true);
            break;
            }

         if (wpsTimeout || !wpsError.isEmpty())
            {
            result.errorMessage = wpsTimeout ? "WPS timeout" : wpsError;
            break;
            }

         vTaskDelay(100 / portTICK_PERIOD_MS); // Small delay to prevent busy waiting
         }

      // Handle overall timeout
      if (wpsActive && (millis() - startTime) >= timeout)
         {
         result.errorMessage = "WPS connection timeout (" + String(timeout / 1000) + " seconds)";
         }

      // Cleanup
      esp_wifi_wps_disable();
      esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
      wpsActive = false;

      if (!result.success)
         {
         SERIAL_STREAM("WPS connection failed: " << result.errorMessage << endl)
         WiFi.disconnect(true);
         }

      return result;
      }

   void BinaryClockWPS::CancelWPS()
      {
      if (wpsActive)
         {
         SERIAL_STREAM("Cancelling WPS connection..." << endl)
         esp_wifi_wps_disable();
         esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
         wpsActive = false;
         WiFi.disconnect(true);
         }
      }

   bool BinaryClockWPS::initWPS()
      {
      // Configure WPS
      wpsConfig.wps_type = WPS_TYPE_PBC; // Push Button Configuration
      strcpy(wpsConfig.factory_info.manufacturer,  "Espressif");
      strcpy(wpsConfig.factory_info.model_number,  "ESP32");
      strcpy(wpsConfig.factory_info.model_name,    "Binary Clock");
      strcpy(wpsConfig.factory_info.device_name,   "WiFiBinaryClock");

      // Enable WPS
      esp_err_t err = esp_wifi_wps_enable(&wpsConfig);
      SERIAL_STREAM("WPS enabled, esp_wifi_wps_enable(): " << EspErrorToString(err) << endl)
      if (err != ESP_OK)
         {
         SERIAL_STREAM("Failed to enable WPS: " << EspErrorToString(err) << endl)
         return false;
         }

      return true;
      }

   void BinaryClockWPS::cleanupWPS()
      {
      esp_wifi_wps_disable();
      }

   APCreds BinaryClockWPS::extractCredentials()
      {
      APCreds creds;

      // Get SSID
      creds.ssid = WiFi.SSID();

      // Get BSSID
      creds.bssid = WiFi.BSSIDstr();

      //Get PSK (pw)
      creds.pw = WiFi.psk();

      return creds;
      }

   void BinaryClockWPS::wpsEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
      {
      BinaryClockWPS* wps = static_cast<BinaryClockWPS*>(arg);
      if (!wps) { return; }

      switch (event_id)
         {
         case WIFI_EVENT_STA_START:
            SERIAL_STREAM("WPS: WiFi station started" << endl)
            ESP_ERROR_CHECK(esp_netif_init());
            break;

         case WIFI_EVENT_STA_CONNECTED:
            SERIAL_STREAM("WPS: WiFi station connected" << endl)
            wps->wpsSuccess = true;
            break;

         case WIFI_EVENT_STA_DISCONNECTED:
            {
            wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*)event_data;
            SERIAL_STREAM("WPS: Disconnected " << (char*)(disconnected->ssid) << ", reason: " << WiFiDisconnectUint8tString(disconnected->reason) << endl)

            // Don't treat disconnect as failure during WPS process
            if (!wps->wpsSuccess)
               {
               // This might be part of the normal WPS process
               SERIAL_STREAM("WPS: Disconnect during WPS process (normal)" << endl)
               }
            }
            break;

         case WIFI_EVENT_STA_WPS_ER_SUCCESS:
            {
            SERIAL_STREAM("WPS: ER Success" << endl)
            ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_SUCCESS");
            esp_wifi_wps_disable();
            vTaskDelay(10 / portTICK_PERIOD_MS);
            // wl_status_t wlRes = WiFi.begin();
            // SERIAL_STREAM("WPS: esp_wifi_connect() result: " << WiFiStatusString(wlRes) << endl)
            esp_err_t err = esp_wifi_connect();
            SERIAL_STREAM("WPS: esp_wifi_connect() result: " << EspErrorToString(err) << endl)
            }
            break;

         case WIFI_EVENT_STA_WPS_ER_FAILED:
            SERIAL_STREAM("WPS: ER Failed" << endl)
            wps->wpsError = "WPS ER Failed";
            wps->wpsActive = false;
            break;

         case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
            SERIAL_STREAM("WPS: ER Timeout" << endl)
            wps->wpsTimeout = true;
            wps->wpsActive = false;
            break;

         case WIFI_EVENT_STA_WPS_ER_PIN:
            SERIAL_STREAM("WPS: ER PIN mode (not supported)" << endl)
            wps->wpsError = "WPS PIN mode not supported";
            wps->wpsActive = false;
            break;

         default:
            SERIAL_STREAM("WPS: Unhandled WiFi event: " << event_id << endl)
            break;
         }
      }

   } // namespace BinaryClockShield

