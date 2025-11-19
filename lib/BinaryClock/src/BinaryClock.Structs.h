#pragma once
#ifndef __BINARYCLOCKSTRUCTS_H__
#define __BINARYCLOCKSTRUCTS_H__

#if !defined(ESP32_WIFI) && !defined(WIFIS3) /// If `ESP32_WIFI` nor `WIFIS3` are defined, then they aren't set to a `false` value.
   #define ESP32_WIFI   true                 /// Define `ESPP32_WIFI` as `true` to include all structures.
   #define WIFIS3      false                 /// Define `WIFIS3` as `false` to exclude Arduino WIFIS3 structures.
   #define WIFI         true                 /// Define `WIFI` as `true` to include all structures as we have ESP32_WIFI
#endif 

#include <stdint.h>                    /// Integer types: uint8_t; uint16_t; etc.
#include "DateTime.h"                  /// DateTime and TimeSpan classes (part of RTClibPlus library).

#if ESP32_WIFI // == true
   #include <WiFi.h>                   /// For WiFi connectivity class: `WiFiClass`
#elif WIFIS3 // == true
   #include <WiFiS3.h>                 /// For WiFi connectivity class: `WiFiS3Class`
#endif

namespace BinaryClockShield
   {
   /// @brief The structure holds all the Alarm information used by the Binary Clock.
   /// @details This structure contains all the information related to a specific alarm, including
   ///          the alarm number, time, melody, status, and whether it has fired or not.   
   ///          While repeating the alarm based the date or day of the week instead of just daily is
   ///          supported by the DS3231 RTC.
   /// @note  The 'melody' selection has been implemented for most boards that support STL.    
   ///        The UNO_R3 will use the internal melody or one user supplied melody.
   /// @author Chris-80 (2025/07)
   typedef struct alarmTime
      {
      enum Repeat             ///< Alarm repeation when ON. Default is Daily.
         {
         Never = 0,           ///< The alarm is turned OFF after it has fired.
         Hourly,              ///< The alarm repeats every hour at the selected minute
         Daily,               ///< The alarm repeats every day at the same time. This is the default.
         Weekly,              ///< The alarm repeats every week on the given day and time.
         Monthly,             ///< The alarm repeats every month on the given date and time.
         endTag               ///< Always the last enum value, defines the size.
         };

      uint8_t  number;        ///< The number of the alarm: 1 or 2
      DateTime time;          ///< The time of the alarm as a DateTime object
      uint8_t  melody;        ///< The melody to play when the alarm is triggered, 0 = internal melody
      uint8_t  status;        ///< Status of the alarm: 0 - inactive, 1 - active
      Repeat   freq;          ///< The alarm repeat frequency, default: Daily.
      bool     fired;         ///< The alarm has fired (e.g. alarm is 'ringing').
      void clear()            ///< Clear all data except the alarm 'number'
         {
         time = DateTime();   // 00:00:00 (2000-01-01)
         melody = 0;          // Default melody number, internal
         status = 0;          // OFF, alarm is not set.
         freq = Daily;        // The alarm repeats every day when ON.
         fired = false;       // Alarm is not ringing (OFF)
         }
      } AlarmTime;
   
   /// @brief The structure to create a note with a sound frequency and duration.
   /// @details The melody used to signal an alarm uses an array of these to create
   ///          the alarm sound.
   struct Note
      {
      unsigned tone;          ///< The tone frequency in Hz.
      unsigned long duration; ///< The duration of the tone in ms.
      };

   /// @brief Enum class to define the index to different LED patterns. Type: uint8_t
   /// @remarks The enum values correspond to the first index of the `ledPatterns_P` 
   ///          array of `CRGB` colors stored in flash memory.
   /// @note  The `endTAG` is equal to the number of patterns defined (7 or 8) and must
   ///        be the last entry in the enum. To reduce the use of flash memory for overhead,
   ///        all full shield patters are stored together on the 2D array. The enum acts
   ///        as the index to each pattern/color set so care must be taken to ensure
   ///        the correct pattern/color set is stored at the correct index.
   /// @author Chris-70 (2025/09)
   enum class LedPattern : uint8_t
         { 
         onColors = 0,  ///< The LED colors when ON (hours; minutes; seconds).
         offColors,     ///< The LED colors when OFF (usually Black; no power).
         onText,        ///< The big Green **`O`** for the On pattern.
         offTxt,        ///< The big RED sideways **`F`** for the oFF pattern.
         xAbort,        ///< The big Pink **`X`** [❌] for the abort/cancel pattern.
         okText,        ///< The big Lime **`✓`** [✅] for the okay/good pattern.
         rainbow,       ///< The colors of the rainbow on the diagnal pattern.
         #if WIFI
         wText,         ///< The big RoyalBlue **`W`** [📶] for the WPS / WiFi pattern.
         #endif
         endTAG         ///< The end marker, also equal to the number of patterns defined (7 or 8).
         };
   
   #if WIFI    // == true
   #define MAX_ID_SIZE  (UINT8_MAX - 1)   ///< Maximum value for an ID, 1-255 (0 is 'Not Set' or 'Error').

   /// @brief The structure to hold the SSID (Name) and BSSID (MAC address) of an Access Point.
   /// @details This structure is used to hold the SSID and BSSID of a WiFi Access Point (AP).
   ///          It includes equality operators to compare two APNames objects.  
   ///          This is the base structure for the other WiFi structures defined below.  
   ///          This combination is assumed to be unique for each AP.  The BSSID can be
   ///          empty, in which case the first AP with the matching SSID will be used.
   /// @author Chris-70 (2025/09)
   struct APNames
      {
      String ssid;   ///< The AP name that is normally displayed when listing APs.
      String bssid;  ///< The AP MAC address in the format "xx:xx:xx:xx:xx:xx". Can be empty.
      bool operator==(const APNames& other) const
         { return ((ssid == other.ssid) && ((bssid == other.bssid) || bssid.isEmpty() || other.bssid.isEmpty())); }
      bool operator!=(const APNames& other) const
         { return !(*this == other); }
      APNames(const String& ssid, const String& bssid) : ssid(ssid), bssid(bssid) { }
      APNames(const APNames& names) = default;
      APNames() = default; // : ssid(""), bssid("") {}
      virtual ~APNames() = default;

      /// @brief Convert the bssid string to an array of 6 bytes. The parameter `bssidArray` is the result.
      /// @details This method parses the `bssid` string (if not empty) and fills the provided
      ///          array with the corresponding byte values. The `bssid` string must be in the exact
      ///          format "xx:xx:xx:xx:xx:xx" (6 pairs of hex digits separated by a ':').
      /// @design  This method assumes that the `bssid` string is the output from the `WiFiClass:BSSIDstr(int)`
      ///          method which returns the BSSID in the correct format. If the `bssid` string is empty,
      ///          the method returns true and fills the array with zeros. An empty string indicates a
      ///          "wild card" BSSID, meaning any BSSID with the matching SSID is acceptable.
      /// @note    The method uses `sscanf` to parse the string and expects exactly 6 byte values in the exact
      ///          format "xx:xx:xx:xx:xx:xx" where "xx" are hexadecimal digits.
      /// @param bssidArray [OUT] Reference to an array of 6 `uint8_t` values to store the bssid bytes.
      /// @return true if parsing was successful, false otherwise
      bool bssidToBytes(uint8_t(&bssidArray)[6]) const
         {
         memset(bssidArray, 0, 6); // Start with empty BSSID
         
         if (bssid.isEmpty() || bssid.length() != 17)
            { return bssid.isEmpty(); } // Empty string is valid (means no BSSID)
         
         // Parse MAC address string, success is 6 bytes extracted.
         return (6 == sscanf(bssid.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
               &bssidArray[0], &bssidArray[1], &bssidArray[2], &bssidArray[3], &bssidArray[4], &bssidArray[5]));
         }
      };  // struct APNames

   /// @brief The structure to hold the WiFi Access Point information including RSSI and AuthMode.
   /// @details This structure extends the `APNames` structure to include the signal strength (RSSI)
   ///          and authentication mode (AuthMode) of the Access Point. It includes equality operators
   ///          that extend APNames to include comparing the `authMode` values.  
   ///          This is used when scanning for available networks to encapsulate the information about each AP.
   /// @author Chris-70 (2025/09)
   struct WiFiInfo : public APNames
      {
      int32_t           rssi;       ///< The signal strength of the AP in dBm (decibel-milliwatts).
      int32_t           channel;    ///< The WiFi channel of the AP, 0 = unknown.
      wifi_auth_mode_t  authMode;   ///< The authentication mode of the AP (e.g. open, WEP, WPA2, etc.). See `wifi_auth_mode_t`.
      bool operator==(const WiFiInfo& other) const
         { return ((static_cast<APNames>(*this) == static_cast<APNames>(other)) && (authMode == other.authMode)); }
      bool operator!=(const WiFiInfo& other) const
         { return !(*this == other); }
      WiFiInfo(const APNames& names) : APNames(names), rssi(INT16_MIN), authMode(WIFI_AUTH_OPEN) { } 
      WiFiInfo(const WiFiInfo& info) = default;
      WiFiInfo() = default; // : APNames(), rssi(0), authMode(0) {}
      virtual ~WiFiInfo() = default;
      }; // struct WiFiInfo

   /// @brief The structure to hold the WiFi Access Point credentials including SSID, BSSID and password.
   /// @details This structure extends the `APNames` structure to include the password (pw) for the Access Point.  
   ///          This structure is used to store the credentials needed to connect to a WiFi Access Point.
   /// @author Chris-70 (2025/09)
   struct APCreds : public APNames
      {
      String   pw;      ///< The password for the AP, can be empty for open networks.
      APCreds(const APNames& names, const String& pw) : APNames(names), pw(pw) { }
      APCreds(const APNames& names) : APNames(names), pw("") { }
      APCreds(const APCreds& creds) = default;
      APCreds() = default; // : APNames(), pw("") {}
      virtual ~APCreds() = default;
      }; // struct APCreds

   /// @brief The structure to hold the WiFi Access Point credentials including an ID.
   /// @details This structure extends the `APCreds` structure to include a unique ID for the Access Point.  
   ///          This structure is used to manage multiple WiFi Access Points and their credentials.
   /// @author Chris-70 (2025/09)
   struct APCredsPlus : public APCreds
      {
      uint8_t id;
      APCredsPlus(const APNames& names) : APCreds(names), id(0) { }
      APCredsPlus(const APCreds& creds) : APCreds(creds), id(0) { }
      APCredsPlus(const APCredsPlus& creds) = default;
      APCredsPlus() = default; // : APCreds(), id(0) {}
      virtual ~APCredsPlus() = default;
      }; // struct APCredsPlus

   ////////////////////////////////////////////////////////////////////////////////////////////////
   //                          ENUMERATION STRINGS                                               //
   ////////////////////////////////////////////////////////////////////////////////////////////////

   static const String EspErrorToString(esp_err_t error)
      { return esp_err_to_name(error); }

   static const String WiFiDisconnectReasonString(wifi_err_reason_t error) 
      { 
      switch (error)
         {
         case WIFI_REASON_UNSPECIFIED:                         return "WIFI_REASON_UNSPECIFIED";
         case WIFI_REASON_AUTH_EXPIRE:                         return "WIFI_REASON_AUTH_EXPIRE";
         case WIFI_REASON_AUTH_LEAVE:                          return "WIFI_REASON_AUTH_LEAVE";
         case WIFI_REASON_ASSOC_EXPIRE:                        return "WIFI_REASON_ASSOC_EXPIRE";
         case WIFI_REASON_ASSOC_TOOMANY:                       return "WIFI_REASON_ASSOC_TOOMANY";
         case WIFI_REASON_NOT_AUTHED:                          return "WIFI_REASON_NOT_AUTHED";
         case WIFI_REASON_NOT_ASSOCED:                         return "WIFI_REASON_NOT_ASSOCED";
         case WIFI_REASON_ASSOC_LEAVE:                         return "WIFI_REASON_ASSOC_LEAVE";
         case WIFI_REASON_ASSOC_NOT_AUTHED:                    return "WIFI_REASON_ASSOC_NOT_AUTHED";
         case WIFI_REASON_DISASSOC_PWRCAP_BAD:                 return "WIFI_REASON_DISASSOC_PWRCAP_BAD";
         case WIFI_REASON_DISASSOC_SUPCHAN_BAD:                return "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
         case WIFI_REASON_BSS_TRANSITION_DISASSOC:             return "WIFI_REASON_BSS_TRANSITION_DISASSOC";
         case WIFI_REASON_IE_INVALID:                          return "WIFI_REASON_IE_INVALID";
         case WIFI_REASON_MIC_FAILURE:                         return "WIFI_REASON_MIC_FAILURE";
         case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:              return "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
         case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:            return "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT";
         case WIFI_REASON_IE_IN_4WAY_DIFFERS:                  return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
         case WIFI_REASON_GROUP_CIPHER_INVALID:                return "WIFI_REASON_GROUP_CIPHER_INVALID";
         case WIFI_REASON_PAIRWISE_CIPHER_INVALID:             return "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
         case WIFI_REASON_AKMP_INVALID:                        return "WIFI_REASON_AKMP_INVALID";
         case WIFI_REASON_UNSUPP_RSN_IE_VERSION:               return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
         case WIFI_REASON_INVALID_RSN_IE_CAP:                  return "WIFI_REASON_INVALID_RSN_IE_CAP";
         case WIFI_REASON_802_1X_AUTH_FAILED:                  return "WIFI_REASON_802_1X_AUTH_FAILED";
         case WIFI_REASON_CIPHER_SUITE_REJECTED:               return "WIFI_REASON_CIPHER_SUITE_REJECTED";
         case WIFI_REASON_TDLS_PEER_UNREACHABLE:               return "WIFI_REASON_TDLS_PEER_UNREACHABLE";
         case WIFI_REASON_TDLS_UNSPECIFIED:                    return "WIFI_REASON_TDLS_UNSPECIFIED";
         case WIFI_REASON_SSP_REQUESTED_DISASSOC:              return "WIFI_REASON_SSP_REQUESTED_DISASSOC";
         case WIFI_REASON_NO_SSP_ROAMING_AGREEMENT:            return "WIFI_REASON_NO_SSP_ROAMING_AGREEMENT";
         case WIFI_REASON_BAD_CIPHER_OR_AKM:                   return "WIFI_REASON_BAD_CIPHER_OR_AKM";
         case WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION:        return "WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION";
         case WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS:         return "WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS";
         case WIFI_REASON_UNSPECIFIED_QOS:                     return "WIFI_REASON_UNSPECIFIED_QOS";
         case WIFI_REASON_NOT_ENOUGH_BANDWIDTH:                return "WIFI_REASON_NOT_ENOUGH_BANDWIDTH";
         case WIFI_REASON_MISSING_ACKS:                        return "WIFI_REASON_MISSING_ACKS";
         case WIFI_REASON_EXCEEDED_TXOP:                       return "WIFI_REASON_EXCEEDED_TXOP";
         case WIFI_REASON_STA_LEAVING:                         return "WIFI_REASON_STA_LEAVING";
         case WIFI_REASON_END_BA:                              return "WIFI_REASON_END_BA";
         case WIFI_REASON_UNKNOWN_BA:                          return "WIFI_REASON_UNKNOWN_BA";
         case WIFI_REASON_TIMEOUT:                             return "WIFI_REASON_TIMEOUT";
         case WIFI_REASON_PEER_INITIATED:                      return "WIFI_REASON_PEER_INITIATED";
         case WIFI_REASON_AP_INITIATED:                        return "WIFI_REASON_AP_INITIATED";
         case WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT:       return "WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT";
         case WIFI_REASON_INVALID_PMKID:                       return "WIFI_REASON_INVALID_PMKID";
         case WIFI_REASON_INVALID_MDE:                         return "WIFI_REASON_INVALID_MDE";
         case WIFI_REASON_INVALID_FTE:                         return "WIFI_REASON_INVALID_FTE";
         case WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED:  return "WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED";
         case WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED:         return "WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED";
         // Reasons >= 200
         case WIFI_REASON_BEACON_TIMEOUT:                      return "WIFI_REASON_BEACON_TIMEOUT";
         case WIFI_REASON_NO_AP_FOUND:                         return "WIFI_REASON_NO_AP_FOUND";
         case WIFI_REASON_AUTH_FAIL:                           return "WIFI_REASON_AUTH_FAIL";
         case WIFI_REASON_ASSOC_FAIL:                          return "WIFI_REASON_ASSOC_FAIL";
         case WIFI_REASON_HANDSHAKE_TIMEOUT:                   return "WIFI_REASON_HANDSHAKE_TIMEOUT";
         case WIFI_REASON_CONNECTION_FAIL:                     return "WIFI_REASON_CONNECTION_FAIL";
         case WIFI_REASON_AP_TSF_RESET:                        return "WIFI_REASON_AP_TSF_RESET";
         case WIFI_REASON_ROAMING:                             return "WIFI_REASON_ROAMING";
         case WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG:        return "WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG";
         case WIFI_REASON_SA_QUERY_TIMEOUT:                    return "WIFI_REASON_SA_QUERY_TIMEOUT";

         default:                            
         return "ESP_ERROR_" + String(error, HEX);
         }
      }
   static const String WiFiDisconnectUint8tString(uint8_t error) { return WiFiDisconnectReasonString(static_cast<wifi_err_reason_t>(error)); }

   /// @brief An array of `wifi_auth_mode_t` authentication mode strings. Diagnostics.
   /// @details This array maps the `wifi_auth_mode_t` enum values to human-readable strings.
   ///          This is useful for logging and debugging purposes to understand the authentication modes.
   /// @see AuthModeString()
   static const char* AuthModeStr[] =
      {
      "Open",                 // WIFI_AUTH_OPEN = 0,      
      "WEP",                  // WIFI_AUTH_WEP,           
      "WPA_PSK",              // WIFI_AUTH_WPA_PSK,       
      "WPA2_PSK",             // WIFI_AUTH_WPA2_PSK,      
      "WPA_WPA2_PSK",         // WIFI_AUTH_WPA_WPA2_PSK,  
      "ENTERPRISE",           // WIFI_AUTH_ENTERPRISE,    
      "WPA3_PSK",             // WIFI_AUTH_WPA3_PSK,      
      "WPA2_WPA3_PSK",        // WIFI_AUTH_WPA2_WPA3_PSK, 
      "WAPI_PSK",             // WIFI_AUTH_WAPI_PSK,      
      "WPA3_ENT_192"          // WIFI_AUTH_WPA3_ENT_192,  
      };
   static const size_t AuthModeSize = sizeof(AuthModeStr) / sizeof(AuthModeStr[0]); ///< The number of strings in the array.
   static_assert(AuthModeSize == WIFI_AUTH_MAX); // Test we have the correct number of strings.

   /// @brief Lookup function for WiFi authentication mode strings. Convert `wifi_auth_mode_t` enum to a string.
   /// @param mode The `wifi_auth_mode_t` authentication mode to lookup.
   /// @return The corresponding authentication mode string.
   /// @see AuthModeStr
   static const String AuthModeString(wifi_auth_mode_t mode)
      {
      String result = "AUTH MODE - UNKNOWN";
      uint8_t index = static_cast<uint8_t>(mode);
      if (index < WIFI_AUTH_MAX)
         {
         result = AuthModeStr[index];
         }

      return result;
      }

   #define WL_STATUS_SIZE 8   // The number of enums defined for `wl_status_t`
   /// @brief An array of `wl_status_t` WiFi status strings. Diagnostics.
   /// @details This array maps the `wl_status_t` enum values to human-readable strings.
   ///          This is useful for logging and debugging purposes to understand the WiFi connection status.
   /// @remarks The `WL_NO_SHIELD` value is 255, so it is placed at index 0 by adding 1 to the enum value.
   /// @see WiFiStatusString()
   static const char* WlStatusStr[WL_STATUS_SIZE] =
      {
      "WL_NO_SHIELD",        // WL_NO_SHIELD        = 255 + 1 == 0,   // for compatibility with WiFi Shield library
      "WL_IDLE_STATUS",      // WL_IDLE_STATUS      = 0   + 1 == 1,
      "WL_NO_SSID_AVAIL",    // WL_NO_SSID_AVAIL    = 1   + 1 == 2,
      "WL_SCAN_COMPLETED",   // WL_SCAN_COMPLETED   = 2   + 1 == 3,
      "WL_CONNECTED",        // WL_CONNECTED        = 3   + 1 == 4,
      "WL_CONNECT_FAILED",   // WL_CONNECT_FAILED   = 4   + 1 == 5,
      "WL_CONNECTION_LOST",  // WL_CONNECTION_LOST  = 5   + 1 == 6,
      "WL_DISCONNECTED"      // WL_DISCONNECTED     = 6   + 1 == 7
      };
   static const size_t WlStatusSize = sizeof(WlStatusStr) / sizeof(WlStatusStr[0]); ///< The number of strings in the array (i.e. 8).

   /// @brief Lookup function for WiFi status strings. Convert `wl_status_t` to a string.
   /// @param status The `wl_status_t` WiFi status to lookup.
   /// @return The corresponding status string. 
   /// @see WLStatusStr
   static const String WiFiStatusString(wl_status_t status)
      {
      String result = "WL - UNKNOWN";
      // Because `WL_NO_SHIELD` has the value of 255 and everything else is 0-6, 
      // Add 1 to `status` and we get 0 - 7 which fits our string array.
      uint8_t index = static_cast<uint8_t>(status) + 1;
      if (index < WlStatusSize)
         {
         result = WlStatusStr[index];
         }

      return result;
      }

   #endif // WIFI 

   } // namespace BinaryClockShield

#endif // __BINARYCLOCKSTRUCTS_H__