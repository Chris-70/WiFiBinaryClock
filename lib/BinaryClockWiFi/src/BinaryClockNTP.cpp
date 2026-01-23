
#ifndef INLINE_HEADER
   #define INLINE_HEADER false
#endif   
#if INLINE_HEADER
// If you need the class definition in the same file, e.g. for CoPilot, 
// just copy-paste the contents of the header file here and change the define.
#else
   #include "BinaryClockNTP.h"
#endif // INLINE_HEADER

#include <Streaming.h>                 /// For Serial << streaming syntax (https://github.com/janelia-arduino/Streaming)

// STL classes required to be included:
#include <tuple>

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
   size_t NTPEventBits::ntpDefaultOffset = 0U; // Initialize static property

   BinaryClockNTP::BinaryClockNTP()
         : syncInProgress(false)
         , lastSyncStatus(false)
         , initialized(false)
         , callbacksEnabled(false)
      {
      // Private constructor - no initialization here
      // Use Begin() method instead
      }

   BinaryClockNTP::~BinaryClockNTP()
      {
      stopSNTP();
      }

   ////////////////////////////////////////////////////////////////////////////////////////////////
   // Task initialization functions (no duplicate struct definition needed - it's in the header)
   ////////////////////////////////////////////////////////////////////////////////////////////////
   
   /// @brief Performs the actual NTP initialization work.
   /// This is extracted to a static function to avoid lambda lifetime issues.
   /// @param param Task parameter containing the NTP instance
   void ntpDoInitialize(NTPTaskParam* param)
      {
      if ((param == nullptr) || (param->instance == nullptr))
         {
         SERIAL_PRINTLN("ERROR: ntpDoInitialize() - param or instance is NULL!")
         return;
         }

      BinaryClockNTP* instance = param->instance;

      // CRITICAL: Wait the specified delay before initializing SNTP to avoid race conditions
      // The delay allows the main SetupWiFiTask to complete and yield the CPU
      // before SNTP callbacks start firing. This prevents Core conflicts and data races.
      // Note: The caller (SetupWiFiTask) also adds a 5-second delay after Begin() returns.
      if (param->delayMS > 0U)
         {
         SERIAL_STREAM("BinaryClockNTP::ntpDoInitialize() - delaying initialization for " << param->delayMS << " ms" << endl)
         vTaskDelay(pdMS_TO_TICKS(param->delayMS));
         SERIAL_STREAM("BinaryClockNTP::ntpDoInitialize() - delay complete, now initializing SNTP" << endl)
         }

      // NOTE: Servers are already stored in instance->ntpServers before task creation
      // No need to copy them here - they're already in the singleton instance

      // Perform SNTP initialization
      SERIAL_STREAM("    BinaryClockNTP::ntpDoInitialize() - Initializing SNTP..." << endl)
      instance->initialized = instance->initializeSNTP();

      SERIAL_STREAM("[" << millis() << "] BinaryClockNTP singleton " << (instance->initialized ? "initialized" : "failed to initialize") << endl)
      }

   /// @brief Static task wrapper for FreeRTOS xTaskCreate.
   /// This must be a true function pointer (not a lambda) for reliable operation with xTaskCreate.
   /// @param pvParameters Pointer to NTPTaskParam structure
   void ntpTaskWrapper(void* pvParameters)
      {
      NTPTaskParam* param = static_cast<NTPTaskParam*>(pvParameters);
      
      if (param == nullptr)
         {
         SERIAL_PRINTLN("ERROR: ntpTaskWrapper() - param is NULL!")
         vTaskDelete(nullptr);
         return;
         }

      try
         {
         // Call the static initialization function
         ntpDoInitialize(param);

         SERIAL_PRINTLN("ntpTaskWrapper() - deleting param.")
         delete param;
         }
      catch (const std::exception& e)
         {
         SERIAL_OUT_STREAM("Exception in ntpTaskWrapper(): " << e.what() << endl)
         if (param != nullptr) { delete param; }
         }
      catch (...)
         {
         SERIAL_OUT_STREAM("Unknown exception in ntpTaskWrapper() at line " << __LINE__ << endl)
         if (param != nullptr) { delete param; }
         }

      SERIAL_PRINTLN("ntpTaskWrapper() - task ending.")
      vTaskDelete(nullptr);
      }

   ////////////////////////////////////////////////////////////////////////////////////////////////

   void BinaryClockNTP::Begin(const std::vector<String>& servers, size_t delayMS, bool block)
      {
      if (initialized)
         {
         SERIAL_STREAM("BinaryClockNTP::Begin() - already initialized; Call End() then reinitialize." << endl)
         return;
         }

      try
         {
         // Set timezone to the default value.
         if (get_Timezone() == nullptr)
            { set_Timezone(DEFAULT_TIMEZONE); }

         // CRITICAL: Store servers in the instance BEFORE creating the async task
         // This avoids passing std::vector through task parameters which can cause
         // heap corruption or threading issues. The async task will find them in the instance.
         if (!servers.empty())
            {
            ntpServers = servers;
            }

         // CRITICAL: Copy server names to persistent C-string storage BEFORE creating async task
         // The ESP-IDF SNTP library holds pointers to these strings, so they must persist
         // for the lifetime of the SNTP service. We cannot use String objects because their
         // internal buffers can be invalidated. We copy them NOW in the main task context
         // to avoid any thread-safety issues with the async task accessing the vector.
         ntpServerCount = 0;
         for (size_t i = 0; i < ntpServers.size() && i < MAX_NTP_SERVERS; i++)
            {
            // Copy String to C-string buffer in the main task context (thread-safe)
            ntpServers[i].toCharArray(ntpServerNames[i], sizeof(ntpServerNames[i]));
            ntpServerCount++;
            }
         SERIAL_STREAM("    BinaryClockNTP::Begin() - copied " << ntpServerCount << " server names to persistent storage" << endl)

         // Create the task parameter structure with the given/known values.
         // NOTE: Servers are NOT stored in taskParam anymore - they're in the instance and persistent C-string array
         NTPTaskParam* taskParam = new NTPTaskParam();
         taskParam->instance = this;
         taskParam->delayMS = delayMS;

         // ALWAYS use async execution - blocking mode disabled permanently (BUILD_MARKER_ASYNC_ONLY_V001)
         // The async task wrapper will handle initialization on a separate task
         SERIAL_STREAM("    [ASYNC_ONLY_V001] Creating async task for NTP initialization" << endl)
         BaseType_t xReturned = xTaskCreate(
               ntpTaskWrapper,          // Static function pointer - reliable with xTaskCreate
               "NTPInitTask",
               4096,                    // Stack size - reduced to free more heap for WiFi/other tasks
               (void*)taskParam,        // Explicit cast to void*
               tskIDLE_PRIORITY + 2,    // Increased priority for more reliable execution
               nullptr
               );
         
         if (xReturned != pdPASS)
            {
            SERIAL_PRINTLN("ERROR: xTaskCreate failed for NTPInitTask!")
            delete taskParam;  // Clean up if task creation failed
            }
         }
      catch (const std::exception& e)
         {
         SERIAL_OUT_STREAM("    Exception occurred in BinaryClockNTP::Begin(): " << e.what() << endl)
         }
      catch (...)
         { SERIAL_OUT_STREAM("    Unknown exception occurred in BinaryClockNTP::Begin() " << __LINE__ << endl) }
      }

   void BinaryClockNTP::End()
      {
      if (initialized)
         {
         // Disable callbacks before stopping SNTP
         callbacksEnabled = false;
         stopSNTP();
         ntpServers.clear();
         initialized = false;
         SERIAL_STREAM("[" << millis() << "] BinaryClockNTP singleton End" << endl)
         }
      }

   bool BinaryClockNTP::initializeSNTP()
      {
      // Stop any existing SNTP service
      if (sntp_enabled())
         { sntp_stop(); }

      // CRITICAL: Disable callbacks before configuring SNTP to prevent
      // callback execution while initialization is in progress
      callbacksEnabled = false;

      // Configure SNTP
      // Set SNTP operating mode
      sntp_setoperatingmode(SNTP_OPMODE_POLL);
      sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
      sntp_set_sync_interval(syncInterval);
      // Set time sync notification callback
      sntp_set_time_sync_notification_cb(timeSyncCallback);

      // NOTE: Server names were already copied to persistent C-string storage in Begin()
      // before the async task was created. The ntpServerCount and ntpServerNames[] 
      // array are already populated and ready to use.

      SERIAL_STREAM("[" << millis() << "] SNTP initialized with " << ntpServerCount << " servers" << endl)

      // Set NTP servers using persistent C-string storage (populated in main task context)
      for (size_t i = 0; i < ntpServerCount && i < MAX_NTP_SERVERS; i++)
         {
         sntp_setservername(i, ntpServerNames[i]);
         SERIAL_STREAM("      - SNTP server " << i << " set to: " << ntpServerNames[i] << endl)
         }

      sntp_init();

      // CRITICAL: Enable callbacks now that SNTP is fully initialized
      // This ensures no callback is invoked until the SNTP service is ready
      callbacksEnabled = true;
      SERIAL_STREAM("[" << millis() << "] Callbacks enabled for SNTP time sync notifications" << endl) // *** DEBUG ***

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
         vTaskDelay(pdMS_TO_TICKS(100));
         count++;
         }

      SERIAL_PRINTF("SyncTime(): UDP beginPacket=%d, write=%d, endPacket=%d, waitCount=%d\n", resBegin, resWrite, resEnd, count)  // *** DEBUG ***
      if (udp.parsePacket() >= sizeof(packet))
         {
         udp.read((uint8_t*)&packet, sizeof(packet));
         endTime = millis();

         uint32_t unixTime = ntpToUnix(packet.txTime);
         // Convert to DateTime
         utcTime = DateTime(unixTime);
         result.success = true;
         SERIAL_PRINTLN("get_CurrentNtpTime(): NTP time = " + utcTime.timestamp(DateTime::TIMESTAMP_DATETIME12)) // *** DEBUG ***
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

         SERIAL_STREAM("[" << millis() << "] NTP sync successful!" << endl)
         SERIAL_STREAM(" Time: " << result.dateTime.timestamp(DateTime::TIMESTAMP_DATETIME) << endl)
         SERIAL_STREAM(" Server: " << result.serverUsed << endl)
         SERIAL_STREAM(" Round trip: " << MILLIS_TO_MS(endTime - startTime) << "ms" << endl)
         struct timeval tv = ntpToTimeval(packet.txTime);
         int setRes = settimeofday(&tv, NULL);
         result.success = (setRes == 0);
         DateTime internal(time(&now));
         SERIAL_STREAM("Internal: " << internal.timestamp(DateTime::TIMESTAMP_DATETIME) << endl)
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

      SERIAL_PRINTLN("BinaryClockNTP::RegisterSyncCallback() - callback registered.") // *** DEBUG ***
      syncCallback = callback;
      return true;
      }

   bool BinaryClockNTP::UnregisterSyncCallback()
      {
      if (!syncCallback) { return false; }

      syncCallback = nullptr;
      callbacksEnabled = false;  // Disable callbacks when unregistering
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
         vTaskDelay(pdMS_TO_TICKS(100)); 
         count++;
         }

      SERIAL_PRINTF("get_CurrentNtpTime(): UDP beginPacket=%d, write=%d, endPacket=%d, waitCount=%d\n", resBegin, resWrite, resEnd, count)  // *** DEBUG ***
      if (udp.parsePacket() >= sizeof(packet))
         {
         udp.read((uint8_t*)&packet, sizeof(packet));

         uint32_t unixTime = ntpToUnix(packet.txTime);

         // Convert to DateTime
         result = DateTime(unixTime);
         }

      udp.stop();
      SERIAL_PRINTLN("get_CurrentNtpTime(): NTP time = " + result.timestamp(DateTime::TIMESTAMP_DATETIME12)) // *** DEBUG ***

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

      SERIAL_PRINTLN("get_LocalNtpTime(): Local   time = " + result.timestamp(DateTime::TIMESTAMP_DATETIME12)) // *** DEBUG ***

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
         SERIAL_STREAM("Warning: BinaryClockNTP not initialized - call initialize() first" << endl)
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
         SERIAL_STREAM("[" << millis() << "] ")  // *** DEBUG ***
         SERIAL_STREAM("SNTP stopped" << endl)   // *** DEBUG ***
         }
      }

   void BinaryClockNTP::processTimeSync(struct timeval* tv)
      {
      syncInProgress = false;
      lastSyncStatus = true;
      unsigned long curMillis = millis();
      
      SERIAL_STREAM("[" << curMillis << "] ")  // *** DEBUG ***
         SERIAL_STREAM("processTimeSync() - NTP time sync notification received. Delta: " << (curMillis - lastSyncMillis) << " ms" << endl) // *** DEBUG ***
      DateTime utcTime(tv->tv_sec); // *** DEBUG ***
      SERIAL_STREAM("[" << curMillis << "] ")  // *** DEBUG ***
      SERIAL_STREAM("Current time from NTP: " << utcTime.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl) // *** DEBUG ***

      // Convert utc timeval to `DateTime` in local timezone.
      time_t now = tv->tv_sec;
      struct tm timeinfo = { 0 };
      struct tm* localTimeInfo = localtime_r(&now, &timeinfo);

      lastSyncMillis = millis();
      lastSyncTimeval = *tv;
      lastSyncDateTime = DateTime(timeinfo);
      SERIAL_STREAM("[" << millis() << "] ")  // *** DEBUG ***
      SERIAL_STREAM("Local   time from NTP: " << lastSyncDateTime.timestamp(DateTime::TIMESTAMP_DATETIME12) << endl) // *** DEBUG ***

      if (syncCallback && callbacksEnabled)  // <-- CRITICAL: Only invoke if callbacks are enabled
         {
         SERIAL_STREAM("[" << millis() << "] Invoking sync callback..." << endl)  // *** DEBUG ***

         try
            {
            syncCallback(lastSyncDateTime);
            SERIAL_STREAM("[" << millis() << "] Sync callback completed successfully." << endl)  // *** DEBUG ***
            }
         catch (const std::exception& e)
            {
            SERIAL_OUT_STREAM("[" << millis() << "] ERROR: Sync callback threw exception: " << e.what() << endl) // *** DEBUG ***
            }
         catch (...)
            {
            SERIAL_OUT_STREAM("[" << millis() << "] ERROR: Sync callback threw an exception!" << endl)  // *** DEBUG ***
            }
         }
      else
         {
         SERIAL_STREAM("[" << millis() << "] No sync callback registered, nothing to call." << endl)   // *** DEBUG ***
         }
     
      char timeStr[64];
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
      SERIAL_STREAM("[" << millis() << "] ")  // *** DEBUG ***
      SERIAL_STREAM("Synchronized time: " << timeStr << endl << endl)   // *** DEBUG ***
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