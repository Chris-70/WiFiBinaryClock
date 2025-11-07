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

#define WPS_TIMEOUT_MS         150000  ///< The default timeout for a WPS connection (e.g. 2 min or 2:30, etc.).

namespace BinaryClockShield
   {
   BinaryClockWAN::BinaryClockWAN(IBinaryClock& clock) : binClock(clock), settings()
      {
      settings.Begin();    // Read the settings from the Non-Volatile Storage.
      WiFi.mode(WIFI_STA);
      localAPs = GetAvailableNetworks();  // Find all the APs in the area.
      Serial << "BinaryClockWAN() found " << localAPs.size() << " networks" << endl;
      zuluOffset = TimeSpan(0, -4, 0, 0); // Default to EDT (UTC-4)
      }

   BinaryClockWAN::~BinaryClockWAN()
      {
      End(false);
      }

   bool BinaryClockWAN::Connect(APCreds& creds)
      {
      bool result = false;
      // Connect to the specified WiFi network using the provided credentials.
      auto status = WiFi.begin(creds.ssid.c_str(), creds.pw.c_str());
      Serial << "BinaryClockWAN() connecting to " << creds.ssid << ", result: " << WiFiStatusString(status) << endl;
      if (status == WL_CONNECTED)
         {
         Serial << "Connected to " << creds.ssid << " with IP address " << WiFi.localIP() << endl;
         settings.AddWiFiCreds(creds);
         localIP = WiFi.localIP();
         localCreds = creds;
         result = true;
         }

      return result;
      }

   BinaryClockWAN& BinaryClockWAN::get_Instance()
      {
      static BinaryClockWAN instance(*(IBinaryClock*)nullptr); // Placeholder, should be initialized properly.
      return instance;
      }

   bool BinaryClockWAN::ConnectLocal()
      {
      bool result = false;
      bool sta = WiFi.mode(WIFI_STA);
      Serial << "WiFi Station Mode: " << (sta ? "YES" : "NO") << endl;  // *** DEBUG ***

      std::vector<std::pair<APCredsPlus, WiFiInfo>> apCredList = settings.GetWiFiAPs(localAPs);

      for (const auto& [cred, info] : apCredList)
         {
         Serial << "  SSID: " << cred.ssid << ", BSSID: [" << cred.bssid << "], P/W: " << cred.pw 
                << ", RSSI: " << info.rssi << ", AuthMode: " << AuthModeString(info.authMode) << endl; // *** DEBUG ***

         // Ensure clean state before connection attempt
         WiFi.disconnect(true);
         vTaskDelay(100 / portTICK_PERIOD_MS);

         wl_status_t status;
         uint8_t bssidArray[6];

         if (!cred.bssid.isEmpty() && cred.bssidToBytes(bssidArray))
            {
            // Single WiFi.begin() call with BSSID
            status = WiFi.begin(cred.ssid.c_str(), cred.pw.c_str(), info.channel, bssidArray, true);
            Serial << "BinaryClockWAN() connecting to " << cred.ssid << ", on channel: " << info.channel << ", with BSSID" << endl;   // *** DEBUG ***
            }
         else
            {
            Serial << "    Missing/Invalid BSSID format in credentials: [" << cred.bssid << "]" << endl; // *** DEBUG ***
            status = WiFi.begin(cred.ssid.c_str(), cred.pw.c_str());
            Serial << "BinaryClockWAN() connecting to " << cred.ssid << " without BSSID" << endl; // *** DEBUG ***
            }

         // Wait for connection with proper timeout
         uint32_t startTime = millis();
         const uint32_t CONNECT_TIMEOUT = 15000;  // 15 seconds
         uint8_t count = 0;

         while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < CONNECT_TIMEOUT)
            {
            wl_status_t currentStatus = WiFi.status();

            // Check for failure states
            if (currentStatus == WL_CONNECT_FAILED ||
                currentStatus == WL_NO_SSID_AVAIL ||
                currentStatus == WL_CONNECTION_LOST)
               {
               Serial << "Connection failed with status: " << WiFiStatusString(currentStatus) << endl;   // *** DEBUG ***
               break;
               }

            vTaskDelay(500 / portTICK_PERIOD_MS);
            Serial.print(".");   // *** DEBUG ***
            count++;
            }
         Serial.println();

         // Check final status
         wl_status_t finalStatus = WiFi.status();
         Serial << "BinaryClockWAN() final result: " << WiFiStatusString(finalStatus) << endl; // *** DEBUG ***

         if (finalStatus == WL_CONNECTED)
            {
            Serial.println("  >> Connected! <<");  // *** DEBUG ***
            Serial << "Connected to " << cred.ssid << " with IP address " << WiFi.localIP() << endl;  // *** DEBUG ***

            localIP = WiFi.localIP();
            localCreds = cred;
            WiFi.persistent(true);
            WiFi.setAutoReconnect(true);

            result = true;
            break; // Exit the loop on successful connection
            }
         else
            {
            Serial << "Failed to connect to " << cred.ssid << ", final status: " << WiFiStatusString(finalStatus) << endl; // *** DEBUG ***
            }
         }

      return result;
      }

   std::vector<WiFiInfo> BinaryClockWAN::GetAvailableNetworks()
      {
      size_t n = WiFi.scanNetworks(false, true);
      Serial << "GetAvailableNetworks() - scan done, found " << n << " networks" << endl; // *** DEBUG ***
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
         Serial << i + 1 << ": " << info.ssid << ", BSSID: [" << info.bssid << "] (" << info.rssi << "dBm) " 
                << AuthModeString(info.authMode) << endl; // << WiFi.persistent(true) << WiFi.isProvEnabled() << endl;  // *** DEBUG ***
         }

      WiFi.scanDelete();

      return networks;
      }

   bool BinaryClockWAN::Begin(IBinaryClock& clock, bool autoConnect)
      {
      binClock = clock;
      bool result = !autoConnect;
      // Register the `WiFiEvent()` instance method, through a lambda, to get all events.
      eventID = WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
            this->WiFiEvent(event, info);
            });

      // Register `SyncAlert()` to get called when SNTP syncs the time.
      bool regResult = ntp.RegisterSyncCallback([this](const DateTime& time) {
            this->SyncAlert(time);  // Call the instance method
            });

      if (autoConnect && ConnectLocal())
         {
         Serial << "Begin(): Connected to local AP: " << WiFi.SSID() << endl; // *** DEBUG ***

         // Wait for connection to stabilize BEFORE initializing SNTP
         vTaskDelay(2000 / portTICK_PERIOD_MS);  // Give WiFi time to stabilize

         // Check connection is still active
         if (WiFi.isConnected())
            {
            WiFi.setAutoReconnect(true);
            Serial << "Connection stable, initializing NTP..." << endl; // *** DEBUG ***

            // Disable WiFi power saving
            WiFi.setSleep(false);
            esp_wifi_set_ps(WIFI_PS_NONE);

            ntp.Initialize(NTP_SERVER_LIST, 10000);
            result = UpdateTime();
            }
         else
            {
            Serial << "Connection lost during stabilization" << endl;   // *** DEBUG ***
            result = false;
            }
         }

      binClock.DisplayLedPattern((result ? LedPattern::okText : LedPattern::xAbort));
      vTaskDelay(1250 / portTICK_PERIOD_MS);
      return result;
      }

void BinaryClockWAN::End(bool save)
      {
      ntp.UnregisterSyncCallback();
      WiFi.disconnect();
      WiFi.removeEvent(eventID);
      // WiFi.removeEvent(WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
      settings.End(save);
      }
      
   bool BinaryClockWAN::UpdateTime()
      {
      bool result = false;
      DateTime time = ntp.get_LocalNtpTime();
      if (time > DateTime(2001, 1, 1, 0, 0, 0))
         {
         binClock.set_Time(time);
         Serial << "UpdateTime(): Time synchronized: " << time.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl; // *** DEBUG ***
         result = true;
         }
      
      return result;
      }

   DateTime BinaryClockWAN::SyncTimeNTP()
      {
      return DateTime();  // TODO: Method body.
      }

   void BinaryClockWAN::SyncAlert(const DateTime& dateTime)
      {
      binClock.set_Time(dateTime);
      Serial << "SyncAlert(): Time synchronized: " << dateTime.timestamp(binClock.get_Is12HourFormat()
            ? DateTime::TIMESTAMP_DATETIME12 : DateTime::TIMESTAMP_DATETIME) << endl;  // *** DEBUG ***
      }

   void BinaryClockWAN::set_Timezone(String value)
      {
      settings.set_Timezone(value);
      ntp.set_Timezone(value.c_str());
      }

   String BinaryClockWAN::get_Timezone() const
      {
      return String(ntp.get_Timezone());
      }

   // WARNING: This function is called from a separate FreeRTOS task (thread)!
   void BinaryClockWAN::WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
      {
      Serial.printf("[%lu] {WiFi-event} event %2d: ", millis(), event); // *** DEBUG ***
      // Serial.printf("[WiFi-event] event %2d \"%s\" - ", event, WiFiEventToString(event));

      switch (event)
         {
         case ARDUINO_EVENT_WIFI_READY:               Serial.println("WiFi interface ready"); break;
         case ARDUINO_EVENT_WIFI_SCAN_DONE:           Serial.println("Completed scan for access points"); break;
         case ARDUINO_EVENT_WIFI_STA_START:           Serial.println("WiFi client started"); break;
         case ARDUINO_EVENT_WIFI_STA_STOP:            Serial.println("WiFi clients stopped"); break;
         case ARDUINO_EVENT_WIFI_STA_CONNECTED:       Serial.println("Connected to access point"); break;
         case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:    Serial.println("Disconnected from WiFi access point"); break;
         case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE: Serial.println("Authentication mode of access point has changed"); break;
         case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("Obtained IP address: ");
            Serial.println(WiFi.localIP());
            break;
         case ARDUINO_EVENT_WIFI_STA_LOST_IP:        Serial.println("Lost IP address and IP address is reset to 0"); break;
         case ARDUINO_EVENT_WPS_ER_SUCCESS:          Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode"); break;
         case ARDUINO_EVENT_WPS_ER_FAILED:           Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode"); break;
         case ARDUINO_EVENT_WPS_ER_TIMEOUT:          Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode"); break;
         case ARDUINO_EVENT_WPS_ER_PIN:              Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode"); break;
         case ARDUINO_EVENT_WIFI_AP_START:           Serial.println("WiFi access point started"); break;
         case ARDUINO_EVENT_WIFI_AP_STOP:            Serial.println("WiFi access point  stopped"); break;
         case ARDUINO_EVENT_WIFI_AP_STACONNECTED:    Serial.println("Client connected"); break;
         case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED: Serial.println("Client disconnected"); break;
         case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:   Serial.println("Assigned IP address to client"); break;
         case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:  Serial.println("Received probe request"); break;
         case ARDUINO_EVENT_WIFI_AP_GOT_IP6:         Serial.println("AP IPv6 is preferred"); break;
         case ARDUINO_EVENT_WIFI_STA_GOT_IP6:        Serial.println("STA IPv6 is preferred"); break;
         case ARDUINO_EVENT_ETH_GOT_IP6:             Serial.println("Ethernet IPv6 is preferred"); break;
         case ARDUINO_EVENT_ETH_START:               Serial.println("Ethernet started"); break;
         case ARDUINO_EVENT_ETH_STOP:                Serial.println("Ethernet stopped"); break;
         case ARDUINO_EVENT_ETH_CONNECTED:           Serial.println("Ethernet connected"); break;
         case ARDUINO_EVENT_ETH_DISCONNECTED:        Serial.println("Ethernet disconnected"); break;
         case ARDUINO_EVENT_ETH_GOT_IP:              Serial.println("Ethernet obtained IP address"); break;
         default:                                    Serial.println("default case"); break;
         }
      }

   // WARNING: This function is called from a separate FreeRTOS task (thread)!
   void BinaryClockWAN::WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
      {
      Serial.printf("[%7lu] {WiFiGotIp} event %2d: ", millis(), event); // *** DEBUG ***
      Serial.println("        WiFi connected");
      Serial.print("        IP address: ");
      Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));           // *** DEBUG ***
      }

   } // namespace BinaryClockShield
