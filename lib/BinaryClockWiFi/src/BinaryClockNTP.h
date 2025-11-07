/// @file BinaryClockNTP.h
/// @brief The header file for the `BinaryClockNTP` class.
/// @details This file contains the declaration of the `BinaryClockNTP` class, which provides NTP
///          synchronization features for the Binary Clock project.
/// @author Chris-80 (2025/09)

#pragma once
#ifndef __BINARYCLOCKNTPSNTP_H__
#define __BINARYCLOCKNTPSNTP_H__

#include <stdint.h>                    /// Integer types: uint8_t; uint16_t; etc.
#include <time.h>                      /// For time_t & struct tm types
#include <stddef.h>                    ///

// STL classes required to be included:
#include <vector>
#include <functional> 

#include <WiFi.h>                      /// For WiFi connectivity class: `WiFiClass`
#include <WiFiUdp.h>                   /// For WiFi UDP class: `WiFiUDP`
#include <sntp.h>                      /// For ESP-IDF SNTP functions and types.

#include "DateTime.h"                  /// DateTime and TimeSpan classes (part of RTClibPlus library).

#define NTP_PACKET_SIZE             48             ///< NTP time stamp is in the first 48 bytes of the message
#define DEFAULT_NTP_TIMEOUT_MS      10000          ///< Default NTP server connection timeout in ms (e.g. 10 sec).
#define SNTP_SYNC_INTERVAL_MS       900000UL       ///< SNTP default sync interval in milliseconds (e.g. 900 sec, 15 min).
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

namespace BinaryClockShield
   {
   /// @brief Result structure for NTP synchronization using ESP SNTP
   struct NTPResult
      {
      bool success = false;            ///< True if synchronization was successful
      DateTime dateTime;               ///< The synchronized date and time
      String serverUsed;               ///< Which server provided the time
      int32_t offsetMs = 0;            ///< Clock offset in milliseconds
      uint32_t roundTripMs = 0;        ///< Round trip time in milliseconds
      uint8_t stratum = 0;             ///< Stratum of the server (simplified)
      String errorMessage;             ///< Error description if failed
      };

   typedef struct                      ///< Fixed-point 64-bit data type (32.32)
      {
      union                            ///< Integer part union signed/unsigned
         {
         uint32_t intpart32u;          ///< Integer part, unsigned 32 bits
         int32_t  intpart32;           ///< Integer part, signed 32 bits
         };
      uint32_t frac32u;                ///< Fractional part, unsigned 32 bits
      } fixedpoint64;
      
   /// @brief NTP Packet structure definition, 48 bytes.
   typedef struct ntp_packet 
      {
      union                         ///< Union of full byte and the bit parts.
         {
         uint8_t li_vn_mode;        ///< Leap indicator, version number, mode
         struct
            {
            uint8_t mode : 3;       ///< Mode            bits: 0-2
            uint8_t vn   : 3;       ///< Version number  bits: 3-5
            uint8_t li   : 2;       ///< Leap indicator  bits: 6-7
            };
         };
      uint8_t  stratum;             ///< Stratum level
      uint8_t  poll;                ///< Polling interval
      int8_t   precision;           ///< Precision of the clock
      uint32_t rootDelay;           ///< Round trip delay
      uint32_t rootDispersion;      ///< Max error from primary clock
      uint32_t refId;               ///< Reference clock ID
      fixedpoint64 refTime;         ///< Reference timestamp
      fixedpoint64 orgTime;         ///< Originate timestamp
      fixedpoint64 recTime;         ///< Received timestamp
      fixedpoint64 txTime;          ///< Transmit timestamp
      } NtpPacket;
      
   /// @brief NTP Client class using ESP-IDF SNTP (Singleton)
   class BinaryClockNTP
      {
   public:
      // Singleton access method
      static BinaryClockNTP& get_Instance();

      /// @brief Initialize with default servers
      void Initialize(const std::vector<String>& servers = NTP_SERVER_LIST, size_t delayMS = 0U, bool block = false);

      /// @brief Shutdown SNTP service
      void Shutdown();

      /// @brief Synchronize time with NTP servers
      NTPResult SyncTime();

      /// @brief Synchronize time with a specific server
      NTPResult SyncTime(const String& serverName);

      bool RegisterSyncCallback(std::function<void(const DateTime&)> callback);
      bool UnregisterSyncCallback();

      /// @brief Check if time is synchronized (i.e. SyncStatus == "COMPLETED")
      /// @remarks `CurrentTime` will return the DateTime object for "2001-01-01T00:00:00" 
      ///          if not synchronized.
      /// @return True if time is synchronized, false otherwise.
      bool isTimeSynchronized();

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
      String get_SyncStatus();

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
      void set_Timezone(const char* timezone)
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
      const char* get_Timezone() const
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
      bool initializeSNTP();
      void stopSNTP();
      String getCurrentServer();

      // Utility functions
      /// @brief Convert NTP timestamp to Unix timestamp. NTP input are in local host order.
      /// @details Converts NTP timestamp (seconds since 1900-01-01) to Unix timestamp
      ///          (seconds since 1970-01-01), handling wraparound for timestamps.  
      ///          The parameters are assumed to be in local host byte order, not network order.  
      ///          Call `ntpToUnix(fixedpoint64)` to use network byte order input.
      /// @param ntpSeconds NTP seconds part
      /// @param ntpFraction NTP fractional seconds part
      /// @return Converted Unix timestamp in seconds since 1970-01-01
      /// @see ntpToUnix(fixedpoint64)
      time_t ntpToUnix(uint32_t ntpSeconds, uint32_t ntpFraction = 0U);

      /// @brief Convert NTP fixedpoint64 timestamp to Unix timestamp.  
      ///        NTP input values are in network byte order.
      /// @details Converts NTP timestamp (seconds since 1900-01-01) to Unix timestamp
      ///          (seconds since 1970-01-01), handling wraparound for timestamps.  
      ///          The parameter is assumed to be in network byte order, so conversion to 
      ///          local host order is done on each part before processing.  
      ///          Call `ntpToUnix(uint32_t, uint32_t)` to bypass the conversion.
      /// @param ntpTime NTP fixedpoint64 timestamp in network byte order.
      /// @return Converted Unix timestamp in seconds since 1970-01-01
      /// @see ntpToUnix(uint32_t, uint32_t)
      time_t ntpToUnix(fixedpoint64 ntpTime) { return ntpToUnix(ntohl(ntpTime.intpart32u), ntohl(ntpTime.frac32u)); }

      /// @brief Swap endianness of a 32-bit unsigned integer: 
      ///        bigendian to littleendian; littleendian to bigendian
      /// @param value 32-bit value to swap.
      /// @return Swapped 32-bit value in opposite byte order.
      uint32_t swapEndian(uint32_t value);

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
       
   private:
      std::vector<String> ntpServers = NTP_SERVER_LIST;     ///< Default NTP servers
      uint32_t timeout = DEFAULT_NTP_TIMEOUT_MS;            ///< 10 second default timeout
      unsigned long syncInterval = SNTP_SYNC_INTERVAL_MS;   ///< Sync interval in ms (e.g. 15 min).
      bool syncInProgress;             ///< Flag: syncing with NTP server is in progress.
      bool lastSyncStatus;             ///< Flag: Result of the last sync attempt.
      bool initialized;                ///< Flag: the this object and the SNTP service are initialized.

      WiFiUDP udp;                     ///< A `WiFiUDP` object for getting the time from the server.
      unsigned port = NTP_DEFAULT_PORT;   ///< The port to use for the NTP server.
      NtpPacket ntpTime;

      struct timeval lastSyncTimeval;
      DateTime lastSyncDateTime;
      unsigned long lastSyncMillis = 0UL;

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
