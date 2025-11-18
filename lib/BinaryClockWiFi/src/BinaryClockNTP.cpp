
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

namespace BinaryClockShield
   {
   BinaryClockNTP::BinaryClockNTP()
         : syncInProgress(false)
         , lastSyncStatus(false)
         , initialized(false)
      {
      // Private constructor - no initialization here
      // Use Initialize() method instead
      }

   BinaryClockNTP::~BinaryClockNTP()
      {
      stopSNTP();
      }

   void BinaryClockNTP::Begin(const std::vector<String>& servers, size_t delayMS, bool block)
      {
      if (initialized)
         {
         Serial << "BinaryClockNTP::Initialize() - already initialized; Call End() then reinitialize." << endl;
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

      try
         {
         // Create a heap-allocated initNTP closure that performs the initialization.
         // This reuses the same logic for both blocking and non-blocking modes.
         std::function<void(TaskParam*)> initNTP = [](TaskParam *param)
            {
            if ((param == nullptr) || (param->instance == nullptr)) { return; }

            BinaryClockNTP* instance = param->instance;

            if (param->delayMS > 0U)
               {
               Serial << "BinaryClockNTP::initNTP() - delaying initialization for " << param->delayMS << " ms" << endl;
               vTaskDelay(param->delayMS / portTICK_PERIOD_MS);
               }

            if (!param->servers.empty())
               {
               instance->ntpServers = param->servers;
               }

            Serial << "    Initializing SNTP..." << endl;
            instance->initialized = instance->initializeSNTP();

            Serial << "[" << millis() << "] BinaryClockNTP singleton " << (instance->initialized ? "initialized" : "failed to initialize") << endl;
            };

         // Non-capturing wrapper so it can be passed as a FreeRTOS task function pointer
         auto taskWrapper = [](void* p) {
            try
               {
               TaskParam* param = static_cast<TaskParam*>(p);
               // This is a task, we can't return. Test for a valid `TaskParam` pointer
               // which we'll delete, test for a valid `work` function before calling.
               if (param != nullptr) 
                  {
                  if (param->work)
                     {
                     param->work(param);
                     }

                  Serial.println("    deleting param in taskWrapper().");
                  delete param;
                  }

               Serial.println("    taskWrapper() done, deleting task.");
               // Destroy this task, we are done.
               vTaskDelete(nullptr);
               }
            catch (const std::exception& e)
               {
               Serial << "    Exception occurred in BinaryClockNTP::Initialize.taskWrapper(): " << e.what() << endl;
               }
            catch (...)
               { Serial << "    Unknown exception occurred in BinaryClockNTP::Initialize.taskWrapper() " << __LINE__ << endl; }
            };

         // Set timezone to the default value.
         if (get_Timezone() == nullptr)
            { set_Timezone(DEFAULT_TIMEZONE); }

         // Create the task parameter structure with the given/known values.
         TaskParam* taskParam = new TaskParam{ this, servers, delayMS, initNTP };

         if (block)
            {
            Serial << "    Blocking call to initNTP()..." << endl;
            initNTP(taskParam);
            delete taskParam;
            }
         else
            {
            Serial << "    New task for non-blocking call to initNTP()..." << endl;
            // Create the task to run initialization asynchronously
            xTaskCreate(
               taskWrapper,
               "NTPInitTask",
               4096,
               taskParam,
               tskIDLE_PRIORITY + 1,
               nullptr
               );
            }
         } // try
      catch (const std::exception& e)
         {
         Serial << "    Exception occurred in BinaryClockNTP::Initialize(): " << e.what() << endl;
         }
      catch (...)
         { Serial << "    Unknown exception occurred in BinaryClockNTP::Initialize() " << __LINE__ << endl; }
      } // Initialize

   void BinaryClockNTP::End()
      {
      if (initialized)
         {
         stopSNTP();
         ntpServers.clear();
         initialized = false;
         Serial << "[" << millis() << "] BinaryClockNTP singleton End" << endl;
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

   NTPResult BinaryClockNTP::SyncTime(const String& serverName, uint16_t port)
      {
      NTPResult result;
      result.success = false;

      if (serverName.isEmpty()) 
         {
         result.errorMessage = "The `serverName` string is empty.";
         return result;
         }

      unsigned long startTime = millis();
      unsigned long endTime = __UINT32_MAX__;
      DateTime utcTime;
      WiFiUDP udp;

      NtpPacket& packet = result.packet;
      // packet.li_vn_mode = 0b00100011;  // LI = 0, VN = 4, Mode = 3 (client mode)
      packet.mode = 3;
      packet.vn = 4;
      packet.li = 0;

      result.serverUsed = serverName;
      int resBegin = udp.beginPacket(serverName.c_str(), port);
      int resWrite = udp.write((const uint8_t*)&packet, sizeof(packet));
      int resEnd = udp.endPacket();

      int count = 0;
      while (udp.peek() < 0 && count < 10)
         {
         vTaskDelay(100);
         count++;
         }

      Serial.printf("SyncTime(): UDP beginPacket=%d, write=%d, endPacket=%d, waitCount=%d\n", resBegin, resWrite, resEnd, count);  // *** DEBUG ***
      if (udp.parsePacket() >= sizeof(packet))
         {
         udp.read((uint8_t*)&packet, sizeof(packet));
         endTime = millis();

         uint32_t unixTime = ntpToUnix(packet.txTime);
         // Convert to DateTime
         utcTime = DateTime(unixTime);
         result.success = true;
         Serial.println("get_CurrentNtpTime(): NTP time = " + utcTime.timestamp(DateTime::TIMESTAMP_DATETIME12)); // *** DEBUG ***
         }

      udp.stop();

      if (result.success)
         {
         // Successful sync
         time_t now = utcTime.unixtime();
         struct tm timeinfo = { 0 };

         struct tm* localTimeInfo = localtime_r(&now, &timeinfo);

         DateTime local = DateTime(timeinfo);
         // Check if we have valid time, otherwise return the UTC time.
         if (!local.isValid())
            { local = utcTime; }

         result.dateTime = local;
         result.serverUsed = serverName;

         Serial << "[" << millis() << "] NTP sync successful!" << endl;
         Serial << " Time: " << result.dateTime.timestamp(DateTime::TIMESTAMP_DATETIME) << endl;
         Serial << " Server: " << result.serverUsed << endl;
         Serial << " Round trip: " << MILLIS_TO_MS(endTime - startTime) << "ms" << endl;
         struct timeval tv = ntpToTimeval(packet.txTime);
         int setRes = settimeofday(&tv, NULL);
         result.success = (setRes == 0);
         DateTime internal(time(&now));
         Serial << "Internal: " << internal.timestamp(DateTime::TIMESTAMP_DATETIME) << endl;
         }
      else
         {
         result.errorMessage = "NTP sync failed - no valid time received";
         }

      return result;
      }

   bool BinaryClockNTP::RegisterSyncCallback(std::function<void(const DateTime&)> callback)
      {
      if (!callback || syncCallback) { return false; }

      syncCallback = callback;
      return true;
      }

   bool BinaryClockNTP::UnregisterSyncCallback()
      {
      if (!syncCallback) { return false; }

      syncCallback = nullptr;
      return true;
      }

   DateTime BinaryClockNTP::get_CurrentNtpTime()
      {
      DateTime result = DateTime::DateTimeEpoch;

      WiFiUDP udp;
      NtpPacket packet = { 0 };
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

      DateTime result = DateTime(timeinfo);
      // Check if we have valid time, we are beyond 2020 now.
      if (!result.isValid())
         { result = utcTime; }

      Serial.println("get_LocalNtpTime(): Local time = " + result.timestamp(DateTime::TIMESTAMP_DATETIME12)); // *** DEBUG ***

      return result;
      }

   bool BinaryClockNTP::isTimeSynchronized()
      {
      sntp_sync_status_t sync_status = get_SyncStatus();
      bool result =  (sync_status == SNTP_SYNC_STATUS_COMPLETED);
      // Check that the updates are within the time window allotted.
      unsigned long curMillis = millis();
      if (result && ((curMillis - lastSyncMillis) / 1000) > get_SyncStaleThreshold())
         { result = false; }

      return result;
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
      unsigned long curMillis = millis();
      
      Serial << "[" << curMillis << "] ";  // *** DEBUG ***
      Serial << "NTP time sync notification received. Delta: " << (curMillis - lastSyncMillis) << " ms" << endl; // *** DEBUG ***
      DateTime utcTime(tv->tv_sec); // *** DEBUG ***
      Serial << "[" << curMillis << "] ";  // *** DEBUG ***
      Serial << "Current time from NTP: " << utcTime.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl; // *** DEBUG ***

      // Convert utc timeval to `DateTime` in local timezone.
      time_t now = tv->tv_sec;
      struct tm timeinfo = { 0 };
      struct tm* localTimeInfo = localtime_r(&now, &timeinfo);

      lastSyncMillis = millis();
      lastSyncTimeval = *tv;
      lastSyncDateTime = DateTime(timeinfo);
      Serial << "[" << millis() << "] ";  // *** DEBUG ***
      Serial << "Local time from NTP: " << lastSyncDateTime.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl; // *** DEBUG ***

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
            Serial << "[" << millis() << "] ERROR: Sync callback threw an exception!" << endl;  // *** DEBUG ***
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

   time_t BinaryClockNTP::ntpToUnix(uint32_t ntpSeconds, uint32_t ntpFraction, bool round)
      {
      if (round && (ntpFraction >= 0x80000000U))
         {
         // Round up if fractional part >= 0.5
         ntpSeconds += 1;
         }

      // NTP  timestamp is seconds since 1900-01-01
      // Unix timestamp is seconds since 1970-01-01
      // The difference is 2208988800 seconds (i.e. 70 years) => NtpTimestampDelta.
      if (ntpSeconds >= NtpTimestampDelta)
         {
         return ntpSeconds - NtpTimestampDelta;
         }
      else
         {
         // Handle timestamp wraparound (will occur in 2036)
         // Assume timestamps are from after 2000
         return ntpSeconds + (0x100000000ULL - NtpTimestampDelta);
         }
      }

   timeval BinaryClockNTP::ntpToTimeval(uint32_t ntpSeconds, uint32_t ntpFraction)  
      {
      timeval tv;
      // Convert fractional part to microseconds. fraction is the value over 2^32.
      // Reduce denominator from 2^32 to 2^20 for calculation efficiency.
      // Multiply by 1,000,000, the divide by 2^20 to get microseconds.
      uint64_t microSec = ((ntpFraction >> (32 - 20)) * 1000000ULL) >> 20; 
      tv.tv_sec = ntpToUnix(ntpSeconds);
      tv.tv_usec = (suseconds_t)microSec;

      return tv;
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