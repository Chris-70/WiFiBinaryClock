/// @file BinaryClockNTP.h
/// @brief The header file for the `BinaryClockNTP` class.
/// @details This file contains the declaration of the `BinaryClockNTP` class, which provides NTP
///          and SNTP synchronization features for the Binary Clock project.   
///          The `BinaryClockNTP` class manages NTP server communication, time synchronization,
///          and timezone handling.
/// @author Chris-80 (2025/09)

#pragma once
#ifndef __BINARYCLOCKNTPSNTP_H__
#define __BINARYCLOCKNTPSNTP_H__

#include <stdint.h>                    /// Integer types: uint8_t; uint16_t; etc.
#include <stddef.h>                    /// Macros & defines: size_t, NULL, etc.
#include <time.h>                      /// For time_t & struct tm types
#include <sys/time.h>                  /// For struct timeval type

// STL classes required to be included:
#include <vector>
#include <functional> 

#include <WiFi.h>                      /// For WiFi connectivity class: `WiFiClass`
#include <WiFiUdp.h>                   /// For WiFi UDP class: `WiFiUDP`
#include <esp_sntp.h>                  /// For ESP-IDF SNTP functions and types.

#include "DateTime.h"                  /// DateTime and TimeSpan classes (part of RTClibPlus library).

#define SECONDS_MS                  1000UL         ///< Number of milliseconds in 1 second.
#define MINUTES_MS                  60000UL        ///< Number of milliseconds in 1 minute.///< 
#define HOURS_MS                    3600000UL      ///< Number of milliseconds in 1 hour.
#define NTP_PACKET_SIZE             48             ///< NTP time stamp is in the first 48 bytes of the message
#define DEFAULT_NTP_TIMEOUT_MS      (10 * SECONDS_MS) ///< Default NTP server connection timeout in ms (e.g. 10 sec).
#define SNTP_SYNC_INTERVAL_MS       (3 * HOURS_MS)    ///< SNTP default sync interval in milliseconds (e.g. 900 sec, 15 min).
#define NTP_UNIX_EPOCS_DELTA        2208988800UL   ///< Difference between NTP (1900/01/01) and Unix (1970/01/01) epochs in seconds

#define NTP_SERVER_1 "time.nrc.ca"     ///< The primary NTP server
#define NTP_SERVER_2 "pool.ntp.org"    ///< The secondary NTP server
#define NTP_SERVER_3 "time.nist.gov"   ///< The tertiary NTP server

#ifndef NTP_SERVER_LIST
   ///< The list of NTP servers if they haven't been defined.
   #define NTP_SERVER_LIST { NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3 }  
#endif

#ifndef NTP_DEFAULT_PORT 
   #define NTP_DEFAULT_PORT 123
#endif

#define UTC_TIMEZONE_ENV "UTC0"        ///< UTC timezone string
#ifndef DEFAULT_TIMEZONE
   ///< Default timezone string (Canada Eastern Time with DST) if not defined.
   #define DEFAULT_TIMEZONE "EST+5EDT,M3.2.0/2,M11.1.0/2"  
#endif

/// @brief Convert the result of a call to `millis()` to milliseconds.
/// @details `millis()` returns a count of 1024 microseconds, to convert to         
///          multiply by 1024 (i.e. left shift by 10) and divide by 1000.
#define MILLIS_TO_MS(M)(((M) << 10) / 1000)

namespace BinaryClockShield
   {
   /// @brief Fixed-point 64-bit data type (32.32) returned by the NTP server.
   /// @details This structure represents a fixed-point 64-bit data type (32.32).  
   ///          The integer part is represented by a union of signed and unsigned 32-bit integers,
   ///          while the fractional part is represented by an unsigned 32-bit integer.   
   ///          The value of the `frac32u` is over 2^32 (i.e. frac32u / 2^32).  
   typedef struct
      {
      union                            ///< Integer part union signed/unsigned
         {
         uint32_t intpart32u;          ///< Integer part, unsigned 32 bits
         int32_t  intpart32;           ///< Integer part, signed 32 bits
         };
      uint32_t frac32u;                ///< Fractional part, unsigned 32 bits
      } fixedpoint64;
      
   /// @brief NTP Packet structure definition, 48 bytes.
   /// @details This structure defines the NTP packet format used for communication
   ///          with NTP servers. It contains fields for various NTP parameters,
   ///          including timestamps, stratum level, and precision.   
   ///          The version number, `vn` and `mode` are set by the caller, they are
   ///          version number is 4 and mode is 3, client mode. The remaining values
   ///          are populated during the call sequences.
   typedef struct ntp_packet 
      {
      union                            ///< Union of full byte and the bit parts.
         {
         uint8_t li_vn_mode;           ///< Leap indicator, version number, mode
         struct
            {
            uint8_t mode : 3;          ///< Mode            bits: 0-2
            uint8_t vn   : 3;          ///< Version number  bits: 3-5
            uint8_t li   : 2;          ///< Leap indicator  bits: 6-7
            };
         };
      uint8_t  stratum;                ///< Stratum level
      uint8_t  poll;                   ///< Polling interval
      int8_t   precision;              ///< Precision of the clock
      uint32_t rootDelay;              ///< Round trip delay
      uint32_t rootDispersion;         ///< Max error from primary clock
      uint32_t refId;                  ///< Reference clock ID
      fixedpoint64 refTime;            ///< Reference timestamp
      fixedpoint64 orgTime;            ///< Originate timestamp
      fixedpoint64 recTime;            ///< Received timestamp
      fixedpoint64 txTime;             ///< Transmit timestamp
      } NtpPacket;
      
   /// @brief Result structure for NTP synchronization.
   /// @brief This structure contains the result of an NTP synchronization attempt,
   ///        including the NTP packet received, success status, synchronized date and time,
   ///        the server used, and any error messages.
   struct NTPResult
      {
      NtpPacket packet = { 0 };        ///< The NtpPacket from UPD call to NTP server.
      bool success = false;            ///< True if synchronization was successful
      DateTime dateTime;               ///< The synchronized date and time (local)
      String serverUsed;               ///< Which server provided the time
      String errorMessage;             ///< Error description if failed
      };

   /// @brief NTP Client class using ESP-IDF SNTP (Singleton pattern).   
   ///        This encapsulates the SNTP functionality for regular NTP time synchronization,
   ///        the timezone and converting UTC time from the NTP server to local time.
   /// @details This class provides NTP client functionality using the ESP-IDF SNTP library.  
   ///          It implements the Singleton design pattern to ensure a single instance
   ///          of the SNTP client is used throughout the application.  
   ///          The timezone can be set and retrieved, and time synchronization can be
   ///          converted to the local time including daylight savings adjustment.
   /// 
   ///          Static methods allow syncing the internal time to s specified NTP server
   ///          without having to setup the SNTP client instance. The timezone must be
   ///          set first in order to convert to local time, otherwise `DateTime` is UTC.
   class BinaryClockNTP
      {
   public:
      /// @brief Singleton access method for the `BinaryClockNTP` instance.
      static BinaryClockNTP& get_Instance()
         {
         static BinaryClockNTP instance; // Guaranteed to be destroyed, instantiated on first use
         return instance;
         }
   
      /// @brief Begin SNTP service with optional servers, delay, and blocking mode.
      /// @details 
      /// @param servers List of NTP servers to use, default is the `NTP_SERVER_LIST` define.
      /// @param delayMS Delay in milliseconds before starting SNTP service, default is 0 ms.
      /// @param block If true, the call will block until initialization is complete, otherwise 
      ///              initialization will be performed asynchronously.
      void Begin(const std::vector<String>& servers = NTP_SERVER_LIST, size_t delayMS = 0U, bool block = false);

      /// @brief End SNTP service
      void End();

      /// @brief Synchronize time with a NTP server.
      /// @return `NTPResult` structure: result of the synchronization attempt
      NTPResult SyncTime()
         {
         String server = (ntpServers.empty() ? NTP_SERVER_1 : ntpServers[0]);
         return SyncTime(server);
         }

      /// @brief Synchronize the internal time with a specific NTP server.
      /// @details This method synchronizes the internal time with the specified NTP server.
      ///          The metod sends a NTP request to the server, the resulting time is
      ///          converted to local time based on the timezone set.
      /// @param serverName The name of the NTP server to synchronize internal time with.
      /// @param port The port number to use for the NTP server, default is `NTP_DEFAULT_PORT`.
      /// @return `NTPResult` structure: result of the synchronization attempt
      static NTPResult SyncTime(const String& serverName, uint16_t port = NTP_DEFAULT_PORT);

      /// @brief Register a callback function to be called on successful time sync.
      /// @details This method registers a callback function that will be called
      ///          whenever a successful time synchronization occurs.
      /// @return True if the callback was registered successfully, false otherwise.  
      /// @note   If a callback is currently registered, the method will fail, 
      ///         call `UnregisterSyncCallback()` first.
      /// @see UnregisterSyncCallback()
      bool RegisterSyncCallback(std::function<void(const DateTime&)> callback);

      /// @brief Unregister the currently registered sync callback function.
      /// @return True if the callback was unregistered successfully, 
      ///         false if no callback was registered.
      /// @see RegisterSyncCallback()
      bool UnregisterSyncCallback();

      /// @brief Check if time is synchronized (i.e. SyncStatus == "COMPLETED")
      /// @remarks `CurrentTime` will return the DateTime object for "2001-01-01T00:00:00" 
      ///          if not synchronized.
      /// @return True if time is synchronized, false otherwise.
      bool isTimeSynchronized();

      /// @brief SyncStatusStr - The synchronization status enum, `sntp_sync_status_t`, as a string.
      /// @details The synchronization status enum, sntp_sync_status_t, as a string:
      ///          - RESET
      ///          - COMPLETED
      ///          - IN_PROGRESS
      ///          - UNKNOWN
      ///          This converts the status value to a string.
      /// @param status The status enum value to convert.
      /// @return Current status enum as string
      /// @see get_SyncStatus()
      String SyncStatusToString(sntp_sync_status_t status)
         {
         switch (status)
            {
               case SNTP_SYNC_STATUS_RESET:       return "RESET";
               case SNTP_SYNC_STATUS_COMPLETED:   return "COMPLETED";
               case SNTP_SYNC_STATUS_IN_PROGRESS: return "IN_PROGRESS";
               default:                           return "UNKNOWN";
            }
         }

      /// @brief Property (RO): CurrentTime - The current local time in `DateTime` format.
      /// @details Gets the current local time in `DateTime` format from the NTP server #1.  
      ///          This calls the UDP server to get the UTC time and converts it to local time.
      ///          This will fail if not connected to a network.   
      ///          To get the current UTC time from the server, call `get_CurrentNtpTime()`.  
      ///          The time is converted to local time using the current timezone setting.  
      ///          get_ : Get current local time.
      /// @return Current local time as `DateTime` object
      /// @see get_CurrentNtpTime()
      /// @see set_Timezone()
      /// @see get_Timezone()
      DateTime get_LocalNtpTime();

      /// @brief Property (RO): CurrentNtpTime - The current UTC time from the NTP server in `DateTime` format.
      /// @details Gets the current UTC time in `DateTime` format from the NTP server #1.  
      ///          This calls the UDP server to get the UTC time and will fail if not on a network.
      ///          To get the current local time, call `get_LocalNtpTime()`.
      ///          get_ : Get current UTC time.
      /// @return Current UTC time as `DateTime` object
      /// @see get_LocalNtpTime()
      DateTime get_CurrentNtpTime();
   
      /// @brief Property (RO): SyncStatus - The synchronization status enum, sntp_sync_status_t, as a string.
      /// @details The synchronization status enum, sntp_sync_status_t, as a string:
      ///          - RESET
      ///          - COMPLETED
      ///          - IN_PROGRESS
      ///          - UNKNOWN
      ///          get_ : Get current sync status as string
      ///          This converts the resul from `esp_sntp_get_sync_status()` to a string.
      /// @return Current sync status enum as string
      /// @see esp_sntp_get_sync_status()
      sntp_sync_status_t get_SyncStatus()
         {  return sntp_get_sync_status(); }

      /// @brief Property: NtpServers - A list of NTP server names strings.
      /// @details The list of NTP servers to use for syncing the time.
      ///          set_ : Set new list of NTP servers
      ///          get_ : Get current list of NTP servers
      /// @param servers New list of NTP server names
      /// @see get_NtpServers()
      void set_NtpServers(const std::vector<String>& servers);

      /// @copydoc set_NtpServers()
      /// @return Current list of NTP server names
      /// @see set_NtpServers()
      std::vector<String> get_NtpServers() const 
         { return ntpServers; }

      /// @brief Property: Timeout - Timeout for sync operations in milliseconds.
      /// @details The timeout period for NTP synchronization attempts.
      ///          set_ : Set new timeout value
      ///          get_ : Get current timeout value
      /// @param timeoutMs New timeout value in milliseconds
      /// @see get_Timeout()
      void set_Timeout(uint32_t timeoutMs) 
         { timeout = timeoutMs; }

      /// @copydoc set_Timeout()
      /// @return Current timeout value in milliseconds
      /// @see set_Timeout()
      uint32_t get_Timeout() const 
         { return timeout; }

      /// @brief Property: SyncInterval - Sync interval for NTP requests in milliseconds.
      /// @details The interval between NTP synchronization attempts.
      ///          set_ : Set new sync interval value
      ///          get_ : Get current sync interval value
      /// @remarks SNTP minimum is 16000 ms (16 sec), maximum is 131040000 ms (36.4 hours).  
      ///          The default is 900000 ms (900 seconds, 15 min).
      /// @param intervalMs New sync interval value in milliseconds
      /// @see get_SyncInterval()
      void set_SyncInterval(unsigned long intervalMs) 
         { 
         intervalMs = max(intervalMs, 16000UL);     // Minimum 16 seconds
         intervalMs = min(intervalMs, 131040000UL); // Maximum 36.4 hours
         syncInterval = intervalMs; 
         }

      /// @copydoc set_SyncInterval()
      /// @return Current sync interval value in milliseconds
      /// @see set_SyncInterval()
      unsigned long get_SyncInterval() const
         { return syncInterval; }

      #define SYNC_STALE_FACTOR  475U   ///< Factor to calculate stale threshold from sync interval (approx. 2.1 times) (1000/475 = ~2.1)
      /// @brief Property (RO): SyncStaleThreshold - The threshold in seconds to consider time stale.
      /// @details The threshold in seconds to consider time stale. This is calculated as approximately 
      ///          2.1 times the sync interval.  Converting from ms to seconds and multiplying by 2.1 => ~475.
      ///          get_ : Get current stale threshold in seconds
      /// @return Current stale threshold in seconds
      /// @see get_SyncInterval()
      unsigned get_SyncStaleThreshold() const
         { return (syncInterval / SYNC_STALE_FACTOR);  }
      #undef SYNC_STALE_FACTOR

      /// @brief Property: Timezone - The timezone string for local time conversion.
      /// @details The timezone string used for local time conversion in Proleptic Format:
      ///          i.e. `std offset dst[offset],[start[/time],end[/time]]` without any spaces: 
      ///               `stdoffset[dst[offset][,start[/time],end[/time]]]`   
      ///        Where:   
      ///          - `std` is the standard time zone abbreviation (e.g. EST, PST, CET, etc.)
      ///          - `offset` is the offset to UTC in hours, +ve west, -ve east (e.g. +5, +8, -1, etc.; est+5==UTC, cest-1==UTC)
      ///          - `dst` is the daylight saving time zone abbreviation (e.g. EDT, PDT, CEST, etc.)
      ///          - `offset` is the optional offset to UTC in daylight savings time. Ahead 1 hour is assumed if not specified.
      ///          - `start` is the start of DST in the format `Mmm.w.d` (month, week, day) week 5 is last; Sunday is 0.
      ///          - `time` is the time of day when the change occurs (default is 02:00)
      ///          - `end` is the end of DST in the same format as `start`   
      ///          set_ : Set new timezone string
      ///          get_ : Get current timezone string
      /// @example
      ///        Examples:   
      ///          - `UTC0` (UTC time, 0 offset, no DST)
      ///          - `EST+5EDT,M3.2.0/2,M11.1.0/2`   UTC==EST+5;   DST: 1st  Sunday in March     @ 02:00 to 1st  Sunday in November @ 02:00 (Canada Eastern Time with DST)
      ///          - `PST+8PDT,M3.2.0/2,M11.1.0/2`   UTC==PST+8;   DST: 1st  Sunday in March     @ 02:00 to 1st  Sunday in November @ 02:00 (US Pacific Time with DST)
      ///          - `CET-1CEST,M3.5.0/2,M10.5.0/3`  UTC==CET-1;   DST: last Sunday in March     @ 02:00 to last Sunday in October  @ 03:00 (Central European Time with DST)
      ///          - `NZST-12NZDT,M9.5.0/2,M4.1.0/3` UTC==NZST-12; DST: last Sunday in September @ 02:00 to 1st  Sunday in April    @ 03:00 (New Zealand Time with DST)
      ///          - `ACWST-8:45`                    UTC==ACWST-8:45;  No DST (Australia Central Western Standard Time)
      ///          - `LHST-10:30LHDT-11,M10.1.0/2,M3.5.0/2` UTC==LHST-10:30; DST: +30 min, 1st Sunday in October @ 02:00 to last Sunday in March @ 02:00 (Lord Howe Standard Time with DST)
      /// @param timezone New timezone string in Proleptic Format.
      /// @see get_Timezone()
      static void set_Timezone(const char* timezone)
         {
         // For null or empty strings, timezone is UTC.
         if (timezone == nullptr || strlen(timezone) == 0)
            { timezone = UTC_TIMEZONE_ENV; }

         setenv("TZ", timezone, 1);
         tzset();
         }

      /// @copydoc set_Timezone()
      /// @return Current timezone string in Proleptic Format.
      /// @see set_Timezone()
      static const char* get_Timezone()
         { return getenv("TZ"); }

   protected:
      /// @brief Protected constructor for Singleton pattern.  
      ///        Use `get_Instance()` to get the single instance.
      /// @see get_Instance()
      BinaryClockNTP();

      /// @brief Protected destructor for Singleton pattern
      virtual ~BinaryClockNTP();

      /// @brief Removed copy constructor for Singleton pattern
      BinaryClockNTP(const BinaryClockNTP&) = delete;
      /// @brief Removed move constructor for Singleton pattern
      BinaryClockNTP(const BinaryClockNTP&&) = delete;
      /// @brief Removed assignment operator for Singleton pattern
      BinaryClockNTP& operator=(const BinaryClockNTP&) = delete;
      /// @brief Removed move assignment operator for Singleton pattern
      BinaryClockNTP& operator=(const BinaryClockNTP&&) = delete;

   private:
      /// @brief Method to initialize the SNTP service using the configured NTP servers.
      /// @details This method initializes the SNTP service with the list of NTP servers
      ///          configured in the `ntpServers` member variable. It sets up the SNTP
      ///          service interval and an internal time sync notification callback to 
      ///          handle synchronization events.  
      /// @remarks This method is called from the `Begin()` method.
      /// @note    `stopSNTP()` must be called to stop the SNTP service when no longer needed.
      /// @return True if initialization was successful, false otherwise.
      /// @see stopSNTP()
      /// @see Begin()
      bool initializeSNTP();

      /// @brief Method to stop the SNTP service.
      /// @details This method stops the SNTP service and cleans up any resources
      ///          associated with it, detaching from the NTP server.
      /// @remarks This method is called from the `End()` method.
      /// @note    `End()` or this method must be called to detach and stop the service.
      void stopSNTP();

      /// @brief Get the current/first NTP server being used.
      String getCurrentServer()
         {
         // esp_sntp doesn't provide easy access to which server was used
         // Return the first server as a reasonable guess
         if (!ntpServers.empty())
            { return ntpServers[0]; }
   
         return NTP_SERVER_1;
         }

      /// @brief Callback for time sync notification from the NTP server.
      /// @details This is called by `timeSyncCallback` which was called by the 
      ///          SNTP service when a time synchronization occurs.  
      ///          This method does all the processing and calls the user callback 
      ///          function if registered.
      /// @param tv Pointer to `timeval` structure with the synchronized time.
      /// @see timeSyncCallback(struct timeval* tv)
      void processTimeSync(struct timeval* tv);

      /// @brief Static callback for time sync notification from the NTP server.
      /// @details This is the static callback function registered with the SNTP service.
      ///          It delegates the call to the instance method `processTimeSync()`.   
      ///          The SNTP expects a callback is a `C` style function pointer which this
      ///          static method provides. The actual processing is done in the instance
      ///          method `processTimeSync`
      /// @param tv Pointer to `timeval` structure with the synchronized time.
      /// @see processTimeSync(struct timeval* tv)
      static void timeSyncCallback(struct timeval* tv)
         {
         BinaryClockNTP& instance = get_Instance();
         // Delegate to instance immediately
         instance.processTimeSync(tv);
         }
      
      // Utility functions
      /// @brief Convert NTP timestamp to Unix timestamp. NTP input are in local host order.
      /// @details Converts NTP timestamp (seconds since 1900-01-01) to Unix timestamp
      ///          (seconds since 1970-01-01), handling wraparound for timestamps.  
      ///          The parameters are assumed to be in local host byte order, not network order.  
      ///          Call `ntpToUnix(fixedpoint64, bool)` to use network byte order input.
      /// @param ntpSeconds NTP seconds part (host byte order)
      /// @param ntpFraction NTP fractional seconds part (host byte order)
      /// @param round Whether to round the result based on the fractional part.
      /// @return Converted Unix timestamp in seconds since 1970-01-01
      /// @see ntpToUnix(fixedpoint64, bool)
      static time_t ntpToUnix(uint32_t ntpSeconds, uint32_t ntpFraction = 0U, bool round = true);

      /// @brief Convert NTP `fixedpoint64` timestamp to Unix timestamp.  
      ///        NTP input values are in network byte order.
      /// @details Converts NTP timestamp (seconds since 1900-01-01) to Unix timestamp
      ///          (seconds since 1970-01-01), handling wraparound for timestamps.  
      ///          The parameter is assumed to be in network byte order, so conversion to 
      ///          local host order is done on each part before processing.  
      ///          Call `ntpToUnix(uint32_t, uint32_t, bool)` to bypass the conversion.
      /// @param ntpTime NTP fixedpoint64 timestamp in network byte order.
      /// @return Converted Unix timestamp in seconds since 1970-01-01
      /// @see ntpToUnix(uint32_t, uint32_t, bool)
      static time_t ntpToUnix(fixedpoint64 ntpTime, bool round = true) 
         { return ntpToUnix(ntohl(ntpTime.intpart32u), ntohl(ntpTime.frac32u), round); }

      /// @brief Convert NTP timestamp to `timeval` structure. NTP input are in local host order.
      /// @details Converts NTP timestamp (seconds since 1900-01-01) to timeval structure
      ///          (seconds and microseconds since 1970-01-01), handling wraparound for timestamps.  
      ///          The parameters are assumed to be in local host byte order, not network order.  
      ///          Call `ntpToTimeval(fixedpoint64)` to use network byte order input.
      /// @param ntpSeconds NTP seconds part (host byte order)
      /// @param ntpFraction NTP fractional seconds part (host byte order)
      /// @return Converted `timeval` structure, seconds and microseconds since 1970-01-01.
      /// @see ntpToTimeval(fixedpoint64)
      static timeval ntpToTimeval(uint32_t ntpSeconds, uint32_t ntpFraction);

      /// @brief Convert NTP `fixedpoint64` timestamp to `timeval` structure.  
      ///        NTP input values are in network byte order.
      /// @details Converts NTP timestamp (seconds since 1900-01-01) to `timeval` structure 
      ///          (seconds and microseconds since 1970-01-01), handling wraparound for timestamps.  
      ///          The parameter is assumed to be in network byte order, so conversion to 
      ///          local host order is done on each part before processing.  
      ///          Call `ntpToTimeval(uint32_t, uint32_t)` to bypass the conversion.
      /// @param ntpTime NTP fixedpoint64 timestamp in network byte order.
      /// @return Converted `timeval` structure, seconds and microseconds since 1970-01-01.
      /// @see ntpToTimeval(uint32_t, uint32_t)
      static timeval ntpToTimeval(fixedpoint64 ntpTime)
         { return ntpToTimeval(ntohl(ntpTime.intpart32u), ntohl(ntpTime.frac32u)); }

      /// @brief Swap endianness of a 32-bit unsigned integer: 
      ///        bigendian to littleendian; littleendian to bigendian
      /// @param value 32-bit value to swap.
      /// @return Swapped 32-bit value in opposite byte order.
      static uint32_t swapEndian(uint32_t value);
       
   private:
      std::vector<String> ntpServers = NTP_SERVER_LIST;     ///< Default NTP servers
      uint32_t timeout = DEFAULT_NTP_TIMEOUT_MS;            ///< 10 second default timeout
      unsigned long syncInterval = SNTP_SYNC_INTERVAL_MS;   ///< Sync interval in ms (e.g. 15 min).
      bool syncInProgress;             ///< Flag: syncing with NTP server is in progress.
      bool lastSyncStatus;             ///< Flag: Result of the last sync attempt.
      bool initialized;                ///< Flag: the this object and the SNTP service are initialized.

      WiFiUDP udp;                     ///< A `WiFiUDP` object for getting the time from the server.
      unsigned port = NTP_DEFAULT_PORT;   ///< The port to use for the NTP server.
      // NtpPacket ntpTime;

      struct timeval lastSyncTimeval;     ///< The `timeval` values from the last sync event.
      DateTime lastSyncDateTime;          ///< The `DateTime` value at the last sync event.
      unsigned long lastSyncMillis = 0UL; ///< The value of `millis()` at the last sync event.

      /// @brief Callback user function for SNTP time sync notifications.
      /// @details This user function is called when the SNTP service receives a time sync notification.
      ///          The value is set by `RegisterSyncCallback()` and cleared by `UnregisterSyncCallback()`
      /// @see RegisterSyncCallback(std::function<void(const DateTime&)> callback)
      /// @see UnregisterSyncCallback()
      std::function<void(const DateTime&)> syncCallback = nullptr; ///< Callback function for SNTP time sync notifications

      /// @brief The time delta between NTP and UNIX EPOCs, expressed in seconds.
      ///        There are 70 years between the EPOCs, i.e. 1900-01-01 vs 1970-01-07.
      static const uint32_t NtpTimestampDelta = NTP_UNIX_EPOCS_DELTA;
      };

   } // namespace BinaryClockShield

#endif // __BINARYCLOCKNTPSNTP_H__
