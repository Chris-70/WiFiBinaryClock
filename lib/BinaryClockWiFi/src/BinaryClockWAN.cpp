/// @file BinaryClockWAN.cpp
/// @details This file contains the implementation of the `BinaryClockWAN` class which
///          manages the WiFi connection for the Binary Clock. It handles connecting to
///          WiFi networks and managing the connection state.
/// @author Chris-70 (2025/09)

#define INLINE_HEADER false
#if INLINE_HEADER
// If you need the class definition in the same file, e.g. for CoPilot, 
// just copy-paste the contents of the header file here and change the define.
#else
   #include "BinaryClockWAN.h"
#endif 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"

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

#define WPS_TIMEOUT_MS         150000  ///< The default timeout for a WPS connection (e.g. 2 min or 2:30, etc.).

namespace BinaryClockShield
   {
   BinaryClockWAN::BinaryClockWAN()
      {
      SERIAL_PRINT("BinaryClockWAN() constructor with IBinaryClockBase*: ")
      SERIAL_PRINTLN(clockPtr? clockPtr->get_IdName() : "NULL")
      WiFi.mode(WIFI_STA);
      zuluOffset = TimeSpan(0, -5, 0, 0); // Default to EST (UTC-5) // *** DEBUG ***
      }

   BinaryClockWAN::~BinaryClockWAN()
      {
      End(false);
      }

   bool BinaryClockWAN::Connect(APCreds& creds)
      {
      if (!initialized) { return initialized; } // Ensure Begin() was called

      bool result = false;
      // Connect to the specified WiFi network using the provided credentials.
      auto status = WiFi.begin(creds.ssid.c_str(), creds.pw.c_str());
      SERIAL_STREAM("BinaryClockWAN() connecting to " << creds.ssid << ", result: " << WiFiStatusString(status) << endl)
      if (status == WL_CONNECTED)
         {
         SERIAL_STREAM("Connected to " << creds.ssid << " with IP address " << WiFi.localIP() << endl)
         settings.AddWiFiCreds(creds);
         localIP = WiFi.localIP();
         localCreds = creds;
         result = true;
         }

      return result;
      }

   BinaryClockWAN& BinaryClockWAN::get_Instance()
      {
      static BinaryClockWAN instance;  // Guaranteed to be destroyed.
      return instance;
      }

   bool BinaryClockWAN::connectLocalWiFi(bool bypassCheck)
      {
      if (!bypassCheck && !initialized) { return initialized; } // Ensure Begin() was called

      bool result = false;
      bool sta = WiFi.mode(WIFI_STA);
      SERIAL_STREAM("WiFi Station Mode: " << (sta ? "YES" : "NO") << endl)  // *** DEBUG ***

      std::vector<std::pair<APCredsPlus, WiFiInfo>> apCredList = settings.GetWiFiAPs(localAPs);

      for (const auto& [cred, info] : apCredList)
         {
         SERIAL_STREAM("  SSID: " << cred.ssid << ", BSSID: [" << cred.bssid << "], P/W: " << cred.pw 
                << ", RSSI: " << info.rssi << ", AuthMode: " << AuthModeString(info.authMode) << endl) // *** DEBUG ***

         // Ensure clean state before connection attempt
         WiFi.disconnect(true);
         vTaskDelay(pdMS_TO_TICKS(100));

         wl_status_t status;
         uint8_t bssidArray[6];

         if (!cred.bssid.isEmpty() && cred.bssidToBytes(bssidArray))
            {
            // Single WiFi.begin() call with BSSID
            status = WiFi.begin(cred.ssid.c_str(), cred.pw.c_str(), info.channel, bssidArray, true);
            SERIAL_STREAM("BinaryClockWAN() connecting to " << cred.ssid << ", on channel: " << info.channel << ", with BSSID" << endl)   // *** DEBUG ***
            }
         else
            {
            SERIAL_STREAM("    Missing/Invalid BSSID format in credentials: [" << cred.bssid << "]" << endl) // *** DEBUG ***
            status = WiFi.begin(cred.ssid.c_str(), cred.pw.c_str());
            SERIAL_STREAM("BinaryClockWAN() connecting to " << cred.ssid << " without BSSID" << endl) // *** DEBUG ***
            }

         // Wait for connection with proper timeout
         uint32_t startTime = millis();
         const uint32_t CONNECT_TIMEOUT = 15000;  // 15 seconds
         uint8_t count = 0;

         while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < CONNECT_TIMEOUT)
            {
            wl_status_t currentStatus = WiFi.status();

            // Check for failure states
            if  (    currentStatus == WL_CONNECT_FAILED
                  || currentStatus == WL_NO_SSID_AVAIL 
                  || currentStatus == WL_CONNECTION_LOST)
               {
               SERIAL_STREAM("Connection failed with status: " << WiFiStatusString(currentStatus) << endl)   // *** DEBUG ***
               break;
               }

            vTaskDelay(pdMS_TO_TICKS(500));
            SERIAL_PRINT(".")   // *** DEBUG ***
            count++;
            }
         SERIAL_PRINTLN()

         // Check final status
         wl_status_t finalStatus = WiFi.status();
         SERIAL_STREAM("BinaryClockWAN() final result: " << WiFiStatusString(finalStatus) << endl) // *** DEBUG ***

         if (finalStatus == WL_CONNECTED)
            {
            SERIAL_PRINTLN("  >> Connected! <<")  // *** DEBUG ***
            SERIAL_STREAM("Connected to " << cred.ssid << " with IP address " << WiFi.localIP() << endl)  // *** DEBUG ***

            localIP = WiFi.localIP();
            localCreds = cred;
            WiFi.persistent(true);
            WiFi.setAutoReconnect(true);

            result = true;
            break; // Exit the loop on successful connection
            }
         else
            {
            SERIAL_STREAM("Failed to connect to " << cred.ssid << ", final status: " << WiFiStatusString(finalStatus) << endl) // *** DEBUG ***
            }
         }

      return result;
      }

   std::vector<WiFiInfo> BinaryClockWAN::GetAvailableNetworks()
      {
      size_t n = WiFi.scanNetworks(false, true);
      SERIAL_STREAM("GetAvailableNetworks() - scan done, found " << n << " networks" << endl) // *** DEBUG ***
      std::vector<WiFiInfo> networks(n);
      for (size_t i = 0; i < n; ++i)
         {
         WiFiInfo info;
         info.ssid = WiFi.SSID(i);                 // The display name of the Access Point (AP).
         info.bssid = WiFi.BSSIDstr(i);            // MAC address in the format: "XX:XX:XX:XX:XX:XX"
         info.rssi = WiFi.RSSI(i);                 // Current signal strength in dBm.
         info.channel = WiFi.channel(i);           // The WiFi channel of the AP, 0 = unknown.
         info.authMode = WiFi.encryptionType(i);   // Authentication Mode (e.g. WEP, WPA2, etc.) not encryption (e.g. AES128, 3DES, etc.).
         
         // networks.push_back(info);
         networks[i] = info;
         SERIAL_STREAM(i + 1 << ": " << info.ssid << ", BSSID: [" << info.bssid << "] (" << info.rssi << "dBm) " 
                    << AuthModeString(info.authMode) << endl) // << WiFi.persistent(true) << WiFi.isProvEnabled() << endl)  // *** DEBUG ***
         }

      WiFi.scanDelete();

      return networks;
      }

   bool BinaryClockWAN::Begin(IBinaryClockBase& binClock, bool autoConnect, uint32_t startDelay)
      {
      clockPtr = &binClock;   // Save the pointer to the implementation of IBinaryClockBase
      bool result = !autoConnect;

      SERIAL_STREAM("BinaryClockWAN::Begin(IBinaryClockBase& binClock, bool autoConnect) called with: " << binClock.get_IdName() 
                  << " Saved as: " << clockPtr->get_IdName() << endl)   // *** DEBUG ***
      if (clockPtr == nullptr || clockPtr != &binClock)  // Safety check
         {
         SERIAL_PRINTLN("ERROR: Invalid IBinaryClockBase reference!")
         return false;
         }
         
      try
         {
         if (startDelay > 0)
            {
            SERIAL_STREAM("BinaryClockWAN::Begin() - Delaying start by: " << startDelay << " milli seconds." << endl)
            vTaskDelay(pdMS_TO_TICKS(startDelay));
            }

         // Register the `WiFiEvent()` instance method, through a lambda, to get all WiFi events.
         eventID = WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
               this->WiFiEvent(event, info);
               });

         settings.Begin();    // Read the settings from the Non-Volatile Storage.
         localAPs = GetAvailableNetworks();  // Find all the APs in the area.
         SERIAL_STREAM("BinaryClockWAN::Begin() - found " << localAPs.size() << " networks" << endl)

         if (autoConnect)
            {
            bool apResult = connectLocalWiFi(true);
            SERIAL_STREAM("Begin(): Connected to local AP: " << (apResult ? WiFi.SSID() : "false") << endl) // *** DEBUG ***

            // Wait for connection to stabilize BEFORE initializing SNTP
            vTaskDelay(pdMS_TO_TICKS(2000));  // Give WiFi time to stabilize

            // Check connection is still active
            if (!apResult || !WiFi.isConnected())
               {
               //
               WPSResult wpsResult = wps.ConnectWPS();
               if (wpsResult.success)
                  {
                  SERIAL_STREAM("    WPS connected to " << wpsResult.credentials.ssid << " with IP " << WiFi.localIP() << endl) // *** DEBUG ***
                  localIP = WiFi.localIP();
                  localCreds = wpsResult.credentials;
                  settings.AddWiFiCreds(wpsResult.credentials);
                  settings.Save();
                  result = ConnectSNTP();
                  }
               else
                  {
                  SERIAL_STREAM("    WPS connection failed: " << wpsResult.errorMessage << endl) // *** DEBUG ***
                  result = false;
                  }
               }
            else
               {
               SERIAL_STREAM("    Connected to WiFi. ")
               WiFi.setAutoReconnect(true);

               // Disable WiFi power saving
               WiFi.setSleep(false);
               esp_wifi_set_ps(WIFI_PS_NONE);

               SERIAL_STREAM("BinaryClockWAN::Begin() - Connection is stable, now initializing NTP..." << endl) // *** DEBUG ***
               result = ConnectSNTP();
               }
            }
         }  // try
      catch (const std::exception& e)
         {
         SERIAL_OUT_STREAM("    Exception occurred in BinaryClockWAN::Begin(): " << e.what() << endl)
         result = initialized = false;
         }
      catch (...)
         {
         SERIAL_OUT_STREAM("    Unknown exception caught in BinaryClockWAN::Begin(" << binClock.get_IdName() << ")")
         result = initialized = false;
         }

      initialized = result;   // Sync the flag with the final result.
      SERIAL_STREAM("    BinaryClockWAN::Begin() Result: " << (result ? "Success" : "Failure") << endl) // *** DEBUG ***
      binClock.DisplayLedPattern((result ? LedPattern::okText : LedPattern::xAbort));
      vTaskDelay(1250 / portTICK_PERIOD_MS);
      return result;
      }
      
   bool BinaryClockWAN::ConnectSNTP()
      {
      ntp.Begin(NTP_SERVER_LIST, 3000);
      SERIAL_STREAM("    initialized NTP; Updating time..." << endl) // *** DEBUG ***

      // Register `SyncAlert()` to get called when SNTP syncs the time.
      bool regResult = ntp.RegisterSyncCallback([this](const DateTime& time) {
            SERIAL_STREAM("[" << millis() << "] BinaryClockWAN::Begin() - SyncAlert callback calling at: " << time.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl) // *** DEBUG ***
            this->SyncAlert(time);  // Call the instance method
            });
      SERIAL_STREAM("    Registered SyncAlert callback: " << (regResult ? "Success" : "Failure") << endl) // *** DEBUG ***

      return regResult;
      }

   void BinaryClockWAN::End(bool save)
      {
      ntp.UnregisterSyncCallback();
      WiFi.disconnect();
      WiFi.removeEvent(eventID);
      // WiFi.removeEvent(WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
      settings.End(save);
      initialized = false;
      }
      
   bool BinaryClockWAN::UpdateTime()
      {
      DateTime time = ntp.get_LocalNtpTime();

      return UpdateTime(time);
      }

   bool BinaryClockWAN::UpdateTime(DateTime& time)
      {
      if (!initialized) { return initialized; } // Ensure Begin() was called

      bool result = false;
      if (time > DateTime::DateTimeEpoch)
         {
         SERIAL_STREAM("Setting time on binClock: " << clockPtr->get_IdName() << "; " << (clockPtr == nullptr? "NULL" : "Valid") << endl) // *** DEBUG ***
         clockPtr->set_Time(time);
         DateTime validateTime = clockPtr->get_Time();
         SERIAL_STREAM("UpdateTime(): Time synchronized: " << time.timestamp(DateTime::TIMESTAMP_DATETIME12) << " Result time: " 
                     << validateTime.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl) // *** DEBUG ***
         result = (time == validateTime); // Success IFF the time was set correctly.
         }
      
      return result;
      }

   DateTime BinaryClockWAN::SyncTimeNTP()
      {
      if (!initialized) { return DateTime::DateTimeEpoch; } // Ensure Begin() was called

      NTPResult syncResult = ntp.SyncTime();
      if (syncResult.success)
         {
         SERIAL_STREAM("SyncTimeNTP(): Success; Time (internal) synchronized: " << syncResult.dateTime.timestamp(DateTime::TIMESTAMP_DATETIME12) 
                    << "; Calling UpdateTime()" << endl) // *** DEBUG ***
         bool updateRes = UpdateTime(syncResult.dateTime);
         }

      return syncResult.dateTime;
      }

   void BinaryClockWAN::SyncAlert(const DateTime& dateTime)
      {
      String prefix("[" + String(millis()) + "] BinaryClockWAN::SyncAlert(" + dateTime.timestamp(DateTime::TIMESTAMP_DATETIME12) + "): ");
      if (!initialized) { SERIAL_PRINTLN(prefix + "- Not initialized") return; } // Ensure Begin() was called

      clockPtr->set_Time(dateTime);
      SERIAL_STREAM(prefix << " Time synchronized: " << dateTime.timestamp(clockPtr->get_Is12HourFormat()
            ? DateTime::TIMESTAMP_DATETIME12 : DateTime::TIMESTAMP_DATETIME) << endl)  // *** DEBUG ***
      }

   void BinaryClockWAN::set_Timezone(String value)
      {
      if (!initialized) { return; } // Ensure Begin() was called

      String curZone = settings.get_Timezone();
      ntp.set_Timezone(value.c_str());
      SERIAL_STREAM("[" << millis() << "] BinaryClockWAN::set_Timezone(): Changing timezone from [" << curZone << "] to [" << value << "]" << endl) // *** DEBUG ***
      if (curZone != value)
         {
         settings.set_Timezone(value);
         bool saveRes = settings.Save();
         SERIAL_STREAM("    Saved new timezone [" << value << "] to settings " << (saveRes ? "successfully." : "with errors.") << endl) // *** DEBUG ***
         }
      }

   String BinaryClockWAN::get_Timezone() const
      {
      if (!initialized) { return String(); } // Ensure Begin() was called

      return String(ntp.get_Timezone());
      }

   // WARNING: This function is called from a separate FreeRTOS task (thread)!
   void BinaryClockWAN::WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
      {
      SERIAL_PRINTF("[%lu] {WiFi-event} event %2d: ", millis(), event) // *** DEBUG ***
      // SERIAL_PRINTF("[WiFi-event] event %2d \"%s\" - ", event, WiFiEventToString(event))

      switch (event)
         {
         case ARDUINO_EVENT_WIFI_READY:               SERIAL_PRINTLN("WiFi interface ready") break;
         case ARDUINO_EVENT_WIFI_SCAN_DONE:           SERIAL_PRINTLN("Completed scan for access points") break;
         case ARDUINO_EVENT_WIFI_STA_START:           SERIAL_PRINTLN("WiFi client started") break;
         case ARDUINO_EVENT_WIFI_STA_STOP:            SERIAL_PRINTLN("WiFi clients stopped") break;
         case ARDUINO_EVENT_WIFI_STA_CONNECTED:       SERIAL_PRINTLN("Connected to access point") break;
         case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:    SERIAL_PRINTLN("Disconnected from WiFi access point") break;
         case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE: SERIAL_PRINTLN("Authentication mode of access point has changed") break;
         case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            SERIAL_PRINT("Obtained IP address: ")
            SERIAL_PRINTLN(WiFi.localIP())
            break;
         case ARDUINO_EVENT_WIFI_STA_LOST_IP:        SERIAL_PRINTLN("Lost IP address and IP address is reset to 0") break;
         case ARDUINO_EVENT_WPS_ER_SUCCESS:          SERIAL_PRINTLN("WiFi Protected Setup (WPS): succeeded in enrollee mode") break;
         case ARDUINO_EVENT_WPS_ER_FAILED:           SERIAL_PRINTLN("WiFi Protected Setup (WPS): failed in enrollee mode") break;
         case ARDUINO_EVENT_WPS_ER_TIMEOUT:          SERIAL_PRINTLN("WiFi Protected Setup (WPS): timeout in enrollee mode") break;
         case ARDUINO_EVENT_WPS_ER_PIN:              SERIAL_PRINTLN("WiFi Protected Setup (WPS): pin code in enrollee mode") break;
         case ARDUINO_EVENT_WIFI_AP_START:           SERIAL_PRINTLN("WiFi access point started") break;
         case ARDUINO_EVENT_WIFI_AP_STOP:            SERIAL_PRINTLN("WiFi access point  stopped") break;
         case ARDUINO_EVENT_WIFI_AP_STACONNECTED:    SERIAL_PRINTLN("Client connected") break;
         case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED: SERIAL_PRINTLN("Client disconnected") break;
         case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:   SERIAL_PRINTLN("Assigned IP address to client") break;
         case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:  SERIAL_PRINTLN("Received probe request") break;
         case ARDUINO_EVENT_WIFI_AP_GOT_IP6:         SERIAL_PRINTLN("AP IPv6 is preferred") break;
         case ARDUINO_EVENT_WIFI_STA_GOT_IP6:        SERIAL_PRINTLN("STA IPv6 is preferred") break;
         case ARDUINO_EVENT_ETH_GOT_IP6:             SERIAL_PRINTLN("Ethernet IPv6 is preferred") break;
         case ARDUINO_EVENT_ETH_START:               SERIAL_PRINTLN("Ethernet started") break;
         case ARDUINO_EVENT_ETH_STOP:                SERIAL_PRINTLN("Ethernet stopped") break;
         case ARDUINO_EVENT_ETH_CONNECTED:           SERIAL_PRINTLN("Ethernet connected") break;
         case ARDUINO_EVENT_ETH_DISCONNECTED:        SERIAL_PRINTLN("Ethernet disconnected") break;
         case ARDUINO_EVENT_ETH_GOT_IP:              SERIAL_PRINTLN("Ethernet obtained IP address") break;
         default:                                    SERIAL_PRINTLN("default case") break;
         }
      }

   // // WARNING: This function is called from a separate FreeRTOS task (thread)!
   // void BinaryClockWAN::WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
   //    {
   //    SERIAL_PRINTF("[%7lu] {WiFiGotIp} event %2d: ", millis(), event) // *** DEBUG ***
   //    SERIAL_PRINTLN("        WiFi connected")
   //    SERIAL_PRINT("        IP address: ")
   //    SERIAL_PRINTLN(IPAddress(info.got_ip.ip_info.ip.addr))           // *** DEBUG ***
   //    }

   } // namespace BinaryClockShield
