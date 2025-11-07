
#ifndef INLINE_HEADER
   #define INLINE_HEADER false
#endif   
#if INLINE_HEADER
// If you need the class definition in the same file, e.g. for CoPilot, 
// just copy-paste the contents of the header file here and change the define.
#else
   #include "BinaryClockNTP.h"
#endif // INLINE_HEADER

#include "Streaming.h"                 /// For Serial << streaming syntax (https://github.com/janelia-arduino/Streaming)

// STL classes required to be included:
#include <tuple>

#include <time.h>
#include <sys/time.h>

// #include <WiFi.h>                      /// For WiFi connectivity class: `WiFiClass`
#include <WiFiUdp.h>                   /// For WiFi UDP class: `WiFiUDP`
// #include <esp_sntp.h>

namespace BinaryClockShield
   {
   // Private constructor
   BinaryClockNTP::BinaryClockNTP()
         : syncInProgress(false)
         , lastSyncStatus(false)
         , initialized(false)
      {
      // Private constructor - no initialization here
      // Use initialize() method instead
      }

   // Private destructor
   BinaryClockNTP::~BinaryClockNTP()
      {
      stopSNTP();
      }

   // Singleton access method
   BinaryClockNTP& BinaryClockNTP::get_Instance()
      {
      static BinaryClockNTP instance; // Guaranteed to be destroyed, instantiated on first use
      return instance;
      }

   void BinaryClockNTP::Initialize(const std::vector<String>& servers, size_t delayMS, bool block)
      {
      if (initialized)
         {
         Serial << "BinaryClockNTP already initialized" << endl;
         return;
         }

      // Task parameter structure to pass to the FreeRTOS task
      struct TaskParam
         {
         BinaryClockNTP* instance;
         std::vector<String> servers;
         size_t delayMS;
         std::function<void(TaskParam*)> work;
         };

      // Create a heap-allocated initNTP closure that performs the initialization.
      // This reuses the same logic for both blocking and non-blocking modes.
      std::function<void(TaskParam*)> initNTP = [](TaskParam *param)
         {
         if ((param == nullptr) || (param->instance == nullptr)) { return; }

         BinaryClockNTP* instance = param->instance;

         if (param->delayMS > 0U)
            {
            Serial << "BinaryClockNTP delaying initialization for " << param->delayMS << " ms" << endl;
            vTaskDelay(param->delayMS / portTICK_PERIOD_MS);
            }

         if (!param->servers.empty())
            {
            instance->ntpServers = param->servers;
            }

         instance->initializeSNTP();
         instance->initialized = true;

         Serial << "[" << millis() << "] BinaryClockNTP singleton initialized" << endl;
         };

      // Non-capturing wrapper so it can be passed as a FreeRTOS task function pointer
      auto taskWrapper = [](void* p) {
            TaskParam* param = static_cast<TaskParam*>(p);
            // This is a task, we can't return. Test for a valid `TaskParam` pointer
            // which we'll delete, test for a valid `work` function before calling.
            if (param != nullptr) 
               {
               if (param->work)
                  {
                  param->work(param);
                  }

               delete param;
               }

            // Destroy this task, we are done.
            vTaskDelete(nullptr);
            };

      // Set timezone to the default value.
      if (get_Timezone() == nullptr)
         { set_Timezone(DEFAULT_TIMEZONE); }

      // Create the task parameter structure with the given/known values.
      TaskParam* taskParam = new TaskParam{ this, servers, delayMS, initNTP };

      if (block)
         {
         initNTP(taskParam);
         delete taskParam;
         }
      else
         {
         // Create the task to run initialization asynchronously
         xTaskCreate(
            taskWrapper,
            "NTPInitTask",
            4096,
            taskParam,
            tskIDLE_PRIORITY + 1,
            NULL
            );
         }
      }

   void BinaryClockNTP::Shutdown()
      {
      if (initialized)
         {
         stopSNTP();
         ntpServers.clear();
         initialized = false;
         Serial << "[" << millis() << "] BinaryClockNTP singleton Shutdown" << endl;
         }
      }

   bool BinaryClockNTP::initializeSNTP()
      {
      // Stop any existing SNTP service
      if (sntp_enabled())
         { sntp_stop(); }

      // Configure SNTP
      // Set SNTP operating mode
      sntp_setoperatingmode(SNTP_OPMODE_POLL);
      sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
      sntp_set_sync_interval(syncInterval);
      // Set time sync notification callback
      sntp_set_time_sync_notification_cb(timeSyncCallback);

      Serial << "[" << millis() << "] SNTP initialized with " << min((int)ntpServers.size(), SNTP_MAX_SERVERS) << " servers" << endl;

      // Set NTP servers
      for (size_t i = 0; i < ntpServers.size() && i < SNTP_MAX_SERVERS; i++)
         {
         sntp_setservername(i, (char*)ntpServers[i].c_str());
         Serial << "      - SNTP server " << i << " set to: " << ntpServers[i] << endl;
         }

      sntp_init();

      return true;
      }

   NTPResult BinaryClockNTP::SyncTime()
      {
      NTPResult result;

      if (!initialized)
         {
         result.errorMessage = "BinaryClockNTP not initialized - call Initialize() first";
         return result;
         }

      // // Check WiFi connection
      // if (WiFi.status() != WL_CONNECTED)
      //    {
      //    result.errorMessage = "WiFi not connected";
      //    return result;
      //    }

      // Start SNTP if not already started
      if (!sntp_enabled())
         { sntp_init(); }

      // Reset sync status
      syncInProgress = true;
      lastSyncStatus = false;
      uint32_t startTime = millis();

      Serial << "Starting NTP time synchronization..." << endl;

      // Wait for sync with timeout
      while (syncInProgress && (millis() - startTime) < timeout)
         {
         // Check if time has been set
         time_t now = 0;
         struct tm timeinfo = { 0 };
         time(&now);
         localtime_r(&now, &timeinfo);

         // Check if we have a valid time (year > 2020)
         if (timeinfo.tm_year > (2020 - 1900))
            {
            lastSyncStatus = true;
            syncInProgress = false;
            break;
            }

         vTaskDelay(100 / portTICK_PERIOD_MS); // Small delay to prevent busy waiting
         }

      // Handle timeout
      if (syncInProgress)
         {
         result.errorMessage = "NTP sync timeout (" + String(timeout / 1000) + " seconds)";
         syncInProgress = false;
         }
      else if (lastSyncStatus)
         {
         // Successful sync
         time_t now = 0;
         struct tm timeinfo = { 0 };
         time(&now);
         localtime_r(&now, &timeinfo);

         result.success = true;
         result.dateTime = DateTime(timeinfo);
         result.serverUsed = getCurrentServer();
         result.roundTripMs = millis() - startTime;

         // Get sync status for additional info
         sntp_sync_status_t sync_status = esp_sntp_get_sync_status();
         result.stratum = (sync_status == SNTP_SYNC_STATUS_COMPLETED) ? 2 : 0; // Simplified stratum info

         Serial << "[" << millis() << "] NTP sync successful!" << endl;
         Serial << " Time: " << result.dateTime.timestamp() << endl;
         Serial << " Server: " << result.serverUsed << endl;
         Serial << " Round trip: " << result.roundTripMs << "ms" << endl;
         }
      else
         {
         result.errorMessage = "NTP sync failed - no valid time received";
         }

      return result;
      }

   NTPResult BinaryClockNTP::SyncTime(const String& serverName)
      {
      if (!initialized)
         {
         NTPResult result;
         result.errorMessage = "BinaryClockNTP not initialized - call initialize() first";
         return result;
         }

      // Temporarily set single server
      std::vector<String> originalServers = ntpServers;
      ntpServers = { serverName };

      // Reinitialize with single server
      initializeSNTP();

      // Perform sync
      NTPResult result = SyncTime();

      // Restore original server list
      ntpServers = originalServers;
      // initializeSNTP();

      return result;
      }

   bool BinaryClockNTP::RegisterSyncCallback(std::function<void(const DateTime&)> callback)
      {
      if (!callback || syncCallback)
         { return false; }

      syncCallback = callback;
      return true;
      }

   bool BinaryClockNTP::UnregisterSyncCallback()
      {
      if (!syncCallback)
         { return false; }

      syncCallback = nullptr;
      return true;
      }

   DateTime BinaryClockNTP::get_CurrentNtpTime()
      {
      DateTime result = DateTime::DateTimeEpoch;

      WiFiUDP udp;
      ntp_packet packet = { 0 };
      // packet.li_vn_mode = 0b00100011;  // LI = 0, VN = 4, Mode = 3 (client mode)
      packet.mode = 3;
      packet.vn = 4;
      packet.li = 0;

      int resBegin = udp.beginPacket((ntpServers.empty() ? NTP_SERVER_1 : ntpServers[0].c_str()), NTP_DEFAULT_PORT);
      int resWrite = udp.write((const uint8_t*)&packet, sizeof(packet));
      int resEnd   = udp.endPacket();

      int count = 0;
      while (udp.peek() < 0 && count < 10)
         { 
         vTaskDelay(100); 
         count++;
         }

      Serial.printf("get_CurrentNtpTime(): UDP beginPacket=%d, write=%d, endPacket=%d, waitCount=%d\n", resBegin, resWrite, resEnd, count);  // *** DEBUG ***
      if (udp.parsePacket() >= sizeof(packet))
         {
         udp.read((uint8_t*)&packet, sizeof(packet));

         uint32_t unixTime = ntpToUnix(packet.txTime);

         // Convert to DateTime
         result = DateTime(unixTime);
         }

      udp.stop();
      Serial.println("get_CurrentNtpTime(): NTP time = " + result.timestamp(DateTime::TIMESTAMP_DATETIME12)); // *** DEBUG ***

      return result;
      }

   DateTime BinaryClockNTP::get_LocalNtpTime()
      {
      DateTime utcTime = get_CurrentNtpTime();
      time_t now = utcTime.unixtime();

      struct tm timeinfo = { 0 };
      struct tm* localTimeInfo = localtime_r(&now, &timeinfo);

      DateTime result = utcTime; 
      // Check if we have valid time, we are beyond 2020 now.
      if (timeinfo.tm_year > (2020 - 1900))
         { result = DateTime(timeinfo); }

      Serial.println("get_LocalNtpTime(): Local time = " + result.timestamp(DateTime::TIMESTAMP_DATETIME12)); // *** DEBUG ***

      return result;
      }

   bool BinaryClockNTP::isTimeSynchronized()
      {
      sntp_sync_status_t sync_status = esp_sntp_get_sync_status();
      return (sync_status == SNTP_SYNC_STATUS_COMPLETED);
      }

   String BinaryClockNTP::get_SyncStatus()
      {
      sntp_sync_status_t sync_status = esp_sntp_get_sync_status();

      switch (sync_status)
         {
            case SNTP_SYNC_STATUS_RESET:       return "RESET";
            case SNTP_SYNC_STATUS_COMPLETED:   return "COMPLETED";
            case SNTP_SYNC_STATUS_IN_PROGRESS: return "IN_PROGRESS";
            default:                           return "UNKNOWN";
         }
      }

   void BinaryClockNTP::set_NtpServers(const std::vector<String>& servers)
      {
      if (!initialized)
         {
         Serial << "Warning: BinaryClockNTP not initialized - call initialize() first" << endl;
         return;
         }

      ntpServers = servers;
      initializeSNTP(); // Reinitialize with new servers
      }

   String BinaryClockNTP::getCurrentServer()
      {
      // esp_sntp doesn't provide easy access to which server was used
      // Return the first server as a reasonable guess
      if (!ntpServers.empty())
         {
         return ntpServers[0];
         }

      return "unknown";
      }

   void BinaryClockNTP::stopSNTP()
      {
      if (esp_sntp_enabled())
         {
         esp_sntp_stop();
         Serial << "[" << millis() << "] ";  // *** DEBUG ***
         Serial << "SNTP stopped" << endl;   // *** DEBUG ***
         }
      }

   void BinaryClockNTP::processTimeSync(struct timeval* tv)
      {
      syncInProgress = false;
      lastSyncStatus = true;
      
      Serial << "[" << millis() << "] ";  // *** DEBUG ***
      Serial << "NTP time sync notification received. Delta: " << (millis() - lastSyncMillis) << " ms" << endl; // *** DEBUG ***
      DateTime utcTime(tv->tv_sec); // *** DEBUG ***
      Serial << "[" << millis() << "] ";  // *** DEBUG ***
      Serial << "Current time from NTP: " << utcTime.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl; // *** DEBUG ***

      // Convert timeval to readable format for debugging
      time_t now = tv->tv_sec;
      struct tm timeinfo = { 0 };
      struct tm* localTimeInfo = localtime_r(&now, &timeinfo);

      lastSyncMillis = millis();
      lastSyncTimeval = *tv;
      lastSyncDateTime = DateTime(timeinfo);
      Serial << "[" << millis() << "] ";  // *** DEBUG ***
      Serial << "Local time from NTP: " << lastSyncDateTime.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl; // *** DEBUG ***

      // If there is a callback function, call it now with the synced DateTime.
      // if (syncCallback != nullptr)
      //    { syncCallback(lastSyncDateTime); }
      // Guard against dangling callback
      if (syncCallback)
         {
         Serial << "[" << millis() << "] Invoking sync callback..." << endl;  // *** DEBUG ***

         try
            {
            syncCallback(lastSyncDateTime);
            Serial << "[" << millis() << "] Sync callback completed successfully." << endl;  // *** DEBUG ***
            }
         catch (const std::exception& e)
            {
            Serial << "[" << millis() << "] ERROR: Sync callback threw exception: " << e.what() << endl; // *** DEBUG ***
            }
         catch (...)
            {
            Serial << "[" << millis() << "] ERROR: Sync callback threw exception!" << endl;  // *** DEBUG ***
            }
         }
      else
         {
         Serial << "[" << millis() << "] No sync callback registered, nothing to call." << endl;   // *** DEBUG ***
         }
     
      char timeStr[64];
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
      Serial << "[" << millis() << "] ";  // *** DEBUG ***
      Serial << "Synchronized time: " << timeStr << endl << endl;   // *** DEBUG ***
      }

   time_t BinaryClockNTP::ntpToUnix(uint32_t ntpSeconds, uint32_t ntpFraction)
      {
      if (ntpFraction >= 0x80000000U)
         {
         // Round up if fractional part >= 0.5
         ntpSeconds += 1;
         }

      // Convert NTP values to local host format (ntohl)
      uint32_t seconds  = ntohl(ntpSeconds);
      uint32_t fraction = ntohl(ntpFraction);

      // NTP  timestamp is seconds since 1900-01-01
      // Unix timestamp is seconds since 1970-01-01
      // The difference is 2208988800 seconds (i.e. 70 years) => NtpTimestampDelta.

      if (seconds >= NtpTimestampDelta)
         {
         return seconds - NtpTimestampDelta;
         }
      else
         {
         // Handle timestamp wraparound (will occur in 2036)
         // Assume timestamps are from after 2000
         return seconds + (0x100000000ULL - NtpTimestampDelta);
         }
      }

   uint32_t BinaryClockNTP::swapEndian(uint32_t value)
      {
      // Convert from little-endian to big-endian or vice versa
      return ((value & 0xFF000000) >> 24) |
             ((value & 0x00FF0000) >>  8) |
             ((value & 0x0000FF00) <<  8) |
             ((value & 0x000000FF) << 24);
      }

   } // namespace BinaryClockShield