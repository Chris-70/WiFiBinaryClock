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
         : timeout(DEFAULT_WPS_TIMEOUT_MS)
         , wpsActive(false)
         , wpsSuccess(false)
         , wpsTimeout(false)
      {
      memset(&wpsConfig, 0, sizeof(wpsConfig));
      }

   BinaryClockWPS::~BinaryClockWPS()
      {
      cleanupWPS(false);
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
      vTaskDelay(pdMS_TO_TICKS(100));
      WiFi.disconnect(true);
      vTaskDelay(pdMS_TO_TICKS(100));
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
      wpsError   = "";
      wpsActive  = true;

      // Register event handler
      esp_err_t err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler, this);
      if (err != ESP_OK)
         {
         result.errorMessage = "Failed to register WiFi event handler: " + EspErrorToString(err);
         cleanupWPS(true);
         return result;
         }

      // Start WPS
      err = esp_wifi_wps_start(0);
      if (err != ESP_OK)
         {
         result.errorMessage = "Failed to start WPS: " + EspErrorToString(err);
         // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
         cleanupWPS(true);
         return result;
         }

      SERIAL_STREAM("WPS started - Please press the WPS button on your router now..." << endl)

      // ===== PHASE 1: Wait for WPS to complete =====
      uint32_t lastStatus = millis();
      while (wpsActive && (millis() - startTime) < timeout)
         {
         // Print status every 10 seconds
         if (millis() - lastStatus > 10000)
            {
            SERIAL_STREAM("WPS still waiting... (" << (millis() - startTime) / 1000 << " sec. elapsed)" << endl)
            lastStatus = millis();
            }

         // Check for WPS completion
         if (wpsSuccess)
            {
            break;  // Exit WPS wait loop
            }

         if (wpsTimeout || !wpsError.isEmpty())
            {
            result.errorMessage = wpsTimeout ? "WPS timeout" : wpsError;
            SERIAL_STREAM("WPS connection failed: " << result.errorMessage << endl)
            // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
            cleanupWPS(true);
            wpsActive = false;
            return result;
            }

         vTaskDelay(pdMS_TO_TICKS(100));
         }

      // Handle WPS timeout
      if (wpsActive && (millis() - startTime) >= timeout)
         {
         result.errorMessage = "WPS timeout (" + String(timeout / 1000) + " seconds)";
         SERIAL_STREAM("WPS connection failed: " << result.errorMessage << endl)
         // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
         cleanupWPS(true);
         wpsActive = false;
         return result;
         }

      SERIAL_PRINTLN("WPS: WPS enrollment completed, credentials received");
      
      // ===== PHASE 2: Connect to WiFi with obtained credentials =====
      SERIAL_PRINTLN("WPS: Disconnecting from any previous connections...");
      WiFi.disconnect(true);  // Turn off radio
      vTaskDelay(pdMS_TO_TICKS(500));
      
      SERIAL_PRINTLN("WPS: Re-enabling WiFi station mode...");
      WiFi.mode(WIFI_OFF);
      vTaskDelay(pdMS_TO_TICKS(100));
      WiFi.mode(WIFI_STA);
      vTaskDelay(pdMS_TO_TICKS(100));
      
      SERIAL_PRINTLN("WPS: Attempting WiFi connection with received credentials...");
      esp_err_t connectErr = esp_wifi_connect();
      if (connectErr != ESP_OK)
         {
         result.errorMessage = "esp_wifi_connect() failed: " + EspErrorToString(connectErr);
         SERIAL_STREAM("WPS connection failed: " << result.errorMessage << endl)
         // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
         cleanupWPS(true);
         // WiFi.disconnect(true);
         return result;
         }

      vTaskDelay(pdMS_TO_TICKS(1000));  // Give it a second to process

      SERIAL_STREAM("WiFi config after reconnection attempt:" << endl)
      wifi_config_t conf;
      esp_wifi_get_config(WIFI_IF_STA, &conf);
      SERIAL_STREAM("  SSID: " << (char*)conf.sta.ssid << endl)
      SERIAL_STREAM("  BSSID: " << WiFi.BSSIDstr() << endl)
      SERIAL_STREAM("  Status: " << WiFiStatusString(WiFi.status()) << endl)

      // Wait for connection with timeout
      uint32_t connectionStart = millis();
      const uint32_t connectionTimeout = 15000;  // 15 seconds
      
      while ((millis() - connectionStart) < connectionTimeout)
         {
         wl_status_t status = WiFi.status();
         
         SERIAL_PRINT(".");
         
         if (status == WL_CONNECTED)
            {
            SERIAL_PRINTLN("\n✅ WiFi Connected!");
            break;
            }
         
         if (status == WL_CONNECT_FAILED)
            {
            result.errorMessage = "WiFi connection failed";
            SERIAL_STREAM("WPS connection failed: " << result.errorMessage << endl)
            // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
            cleanupWPS(true);
            // WiFi.disconnect(true);
            return result;
            }
         
         vTaskDelay(pdMS_TO_TICKS(500));
         }

      // Check if we actually connected
      if (WiFi.status() != WL_CONNECTED)
         {
         result.errorMessage = "WiFi connection timeout";
         SERIAL_STREAM("WPS connection failed: " << result.errorMessage << endl)
         // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
         cleanupWPS(true);
         // WiFi.disconnect(true);
         return result;
         }

      SERIAL_PRINTLN("");  // Newline after dots
      
      // ===== PHASE 3: Verify DHCP configuration =====
      uint32_t dhcpStart = millis();
      const uint32_t dhcpTimeout = 10000;  // 10 seconds
      IPAddress ip;
      
      while ((millis() - dhcpStart) < dhcpTimeout)
         {
         ip = WiFi.localIP();
         if (ip[0] != 0)  // Got valid IP
            {
            SERIAL_PRINT("✅ IP Address: ");
            SERIAL_PRINTLN(ip);
            break;
            }
         vTaskDelay(pdMS_TO_TICKS(100));
         }
      
      // Final verification
      if (WiFi.status() != WL_CONNECTED || ip[0] == 0)
         {
         result.errorMessage = "WiFi connected but DHCP failed or connection lost";
         SERIAL_STREAM("WPS connection failed: " << result.errorMessage << endl)
         // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
         cleanupWPS(true);
         // WiFi.disconnect(true);
         return result;
         }

      // ===== SUCCESS =====
      result.success = true;
      result.credentials = extractCredentials();
      result.connectionTimeMs = millis() - startTime;

      SERIAL_STREAM("✅ WPS connection successful!" << endl)
      SERIAL_STREAM("Connected to: " << result.credentials.ssid << endl)
      SERIAL_STREAM("IP Address: " << ip << endl)
      SERIAL_STREAM("Connection time: " << result.connectionTimeMs / 1000.0 << " seconds" << endl)

      // Cleanup
      // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
      cleanupWPS(false);
      wpsActive = false;

      return result;
      } // ConnectWPS

   void BinaryClockWPS::CancelWPS()
      {
      if (wpsActive)
         {
         SERIAL_STREAM("Cancelling WPS connection..." << endl)
         // esp_wifi_wps_disable();
         // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
         // wpsActive = false;
         cleanupWPS(true);
         // WiFi.disconnect(true);
         }
      }

   bool BinaryClockWPS::initWPS()
      {
      // Configure WPS
      wpsConfig.wps_type = WPS_TYPE_PBC; // Push Button Configuration
      strncpy(wpsConfig.factory_info.manufacturer,  "Espressif",       sizeof(wpsConfig.factory_info.manufacturer) - 1);
      strncpy(wpsConfig.factory_info.model_number,  "ESP32",           sizeof(wpsConfig.factory_info.model_number) - 1);
      strncpy(wpsConfig.factory_info.model_name,    "Binary Clock",    sizeof(wpsConfig.factory_info.model_name)   - 1);
      strncpy(wpsConfig.factory_info.device_name,   "WiFiBinaryClock", sizeof(wpsConfig.factory_info.device_name)  - 1);

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

   void BinaryClockWPS::cleanupWPS(bool disconnectWiFi)
      {
      wpsActive = false;
      esp_wifi_wps_disable();
      esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wpsEventHandler);
      if (disconnectWiFi)
         { WiFi.disconnect(true); }
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
      char ssid[33] = { 0 };

      switch (event_id)
         {
         case WIFI_EVENT_STA_START:
            SERIAL_STREAM("WPS: WiFi station started" << endl)
            ESP_ERROR_CHECK(esp_netif_init());
            break;

         case WIFI_EVENT_STA_CONNECTED:
            {
            SERIAL_STREAM("WPS: WiFi station connected" << endl)
            wifi_event_sta_connected_t* connectData = static_cast<wifi_event_sta_connected_t*>(event_data);
            strncpy((char*)ssid, (const char*)connectData->ssid, max(connectData->ssid_len, (uint8_t)(sizeof(ssid) - 1)));
            SERIAL_STREAM("  SSID: " << ssid << ", Channel: " << (int)connectData->channel << endl)
            wps->OnWPSSuccess();
            }
            break;

         case WIFI_EVENT_STA_DISCONNECTED:
            {
            wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*)event_data;
            SERIAL_STREAM("WPS: Disconnected " << (char*)(disconnected->ssid) << ", reason: " << WiFiDisconnectUint8tString(disconnected->reason) << endl)

            // Don't treat disconnect as failure during WPS process
            if (!wps->wpsSuccess)
               {
               // This might be part of the normal WPS process
               SERIAL_STREAM("  WPS: Disconnect during WPS process (normal)" << endl)
               }
            }
            break;

         case WIFI_EVENT_STA_WPS_ER_SUCCESS:
            SERIAL_STREAM("WPS: WiFi station success." << endl)
            wps->OnWPSSuccess();
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
            SERIAL_STREAM("WPS: Error: PIN mode not supported." << endl)
            wps->wpsError = "WPS PIN mode not supported";
            wps->wpsActive = false;
            break;

         default:
            SERIAL_STREAM("WPS: Unhandled WiFi event: " << event_id << endl)
            break;
         }
      }

   void BinaryClockWPS::OnWPSSuccess()
      {
      SERIAL_PRINTLN("WPS: ER Success - credentials received");
      esp_wifi_wps_disable();
      wpsSuccess = true;
      wpsActive = false;  // ← Tell main loop WPS is done
      }

   bool BinaryClockWPS::WaitForConnection(uint32_t timeoutMs)
      {
      uint32_t startTime = millis();

      while (millis() - startTime < timeoutMs)
         {
         wl_status_t status = WiFi.status();

         SERIAL_PRINT(".");  // Progress indicator

         if (status == WL_CONNECTED)
            {
            SERIAL_PRINTLN("\n✅ Connected!");
            return true;
            }

         if (status == WL_CONNECT_FAILED)
            {
            SERIAL_PRINTLN("\n❌ Connection failed");
            return false;
            }

         delay(500);
         }

      SERIAL_PRINTLN("\n❌ Connection timeout");
      return false;
      }

   bool BinaryClockWPS::EnsureDHCPConfigured()
      {
      uint32_t startTime = millis();
      
      while (millis() - startTime < 10000)
         {
         IPAddress ip = WiFi.localIP();
         if (ip[0] != 0)
            {
            SERIAL_PRINT("✅ IP: ");
            SERIAL_PRINTLN(ip);
            return true;
            }

         delay(100);
         }
      
      SERIAL_PRINTLN("❌ DHCP timeout");
      return false;
      }

   } // namespace BinaryClockShield

