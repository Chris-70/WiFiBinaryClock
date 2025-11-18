/// @file BinaryClockWAN.h
/// @brief The header file for the BinaryClockWAN class.
/// @details This file contains the declaration of the BinaryClockWAN class, which provides WiFi connectivity
///          and time synchronization features for the Binary Clock project.
/// @author Chris-70 (2025/09)
#pragma once
#ifndef __BINARYCLOCKWIFI_H__ 
#define __BINARYCLOCKWIFI_H__

#include <stdint.h>                 /// Integer types: size_t; uint8_t; uint16_t; etc.

// STL classes required to be included:
#include <String>
#include <vector>

#include "BinaryClock.Structs.h"    /// Global structures and enums used by the Binary Clock project. 
#include "IBinaryClock.h"           /// The pure interface class that defines the minimum supported features.
#include "BinaryClockSettings.h"    /// Binary Clock Settings class: handles all settings kept on NVS.
#include "BinaryClockNTP.h"         /// Binary Clock NTP class: handles all NTP related functionality.
#include "BinaryClockWPS.h"         /// Binary Clock WPS class: handles WPS connection functionality.
#include "DummyBinaryClock.h"       // *** DEBUG ***

#include <WiFi.h>                   /// For WiFi connectivity class: `WiFiClass`
#include <esp_wifi_types.h>         /// For `wifi_auth_mode_t` enum and related types.

namespace BinaryClockShield
   {
   /// @brief The BinaryClockWAN class provides WiFi connectivity and time synchronization 
   ///        features for the Binary Clock project.
   /// @details This class manages WiFi connections, including connecting to known access points, 
   ///          handling disconnections, and maintaining the connection state.  
   ///          The constructor scans for all available WiFi networks and saves this list. 
   ///          When `Begin()` is called, it attempts to connect to one of the known access points
   ///          stored in the `BinaryClockSettings` instance. If a connection is established, 
   ///          it synchronizes the time with an NTP server and updates the `IBinaryClock` instance.
   /// @note    This class requires the `BinaryClockSettings` class to manage access point credentials.
   ///          It also requires an `IBinaryClock` instance to update the time.
   /// @author Chris-70 (2025/09)
   class BinaryClockWAN
      {
   protected:
      /// @brief Construct a `BinaryClockWAN` instance with a reference to the `IBinaryClock` 
      ///        Interface implementation (e.g `BinaryClock`).
      /// @details This constructor initializes the `BinaryClockWAN` instance with a reference
      ///          to an `IBinaryClock` implementation. It also initializes the `BinaryClockSettings`
      ///          instance to manage access point credentials. The constructor scans for all
      ///          available WiFi networks and saves this list.   
      /// @remarks The `Begin()` method must be called before any other methods to attempt a connection.
      /// @see Begin()
      /// @see GetAvailableNetworks()
      /// @author Chris-70 (2025/09)
      BinaryClockWAN();

      /// @brief Destructor for the BinaryClockWAN class.
      /// @details The destructor ensures that the WiFi connection is properly closed and resources are released.
      /// @author Chris-70 (2025/09)
      virtual ~BinaryClockWAN();

      // Disable copy/move to enforce singleton semantics
      BinaryClockWAN(const BinaryClockWAN&) = delete;
      BinaryClockWAN(BinaryClockWAN&&) = delete;
      BinaryClockWAN& operator=(const BinaryClockWAN&) = delete;
      BinaryClockWAN& operator=(BinaryClockWAN&&) = delete;

   //#################################################################################//  
   // Public METHODS                                                                  //   
   //#################################################################################//   

   public:
      bool Connect(APCreds& creds);
      static BinaryClockWAN& get_Instance();

      /// @brief Connect to one of the local access points using the stored credentials.
      /// @details This method checks the list of local access points against the stored credentials.
      ///          If one of the local APs match a stored credential it attempts to connect to it.
      ///          If it fails it continues looking for other APs. If a connection is established, it 
      ///          synchronizes the time with an NTP server and updates the `IBinaryClock` instance.
      /// @return True if the connection was successful, false otherwise.
      /// @author Chris-70 (2025/09)
      bool ConnectLocal()  
         { return connectLocalWiFi(); }

      /// @brief Create a list of all available APs in the area and their characteristics.
      /// @details This method scans for all available WiFi networks and returns a vector of 
      ///          `WiFiInfo` encapsulating the information of each AP.  
      ///          This can be used to cross reference with the list of saved AP credentials to
      ///          automate the reconnection.
      /// @design  This method is static so that is can be used without an instance of the class.
      ///          The constructor calls this method to get a list of the APs in the area.
      /// @return A vector of `WiFiInfo` structures containing the details of each available AP.
      /// @author Chris-70 (2025/09)
      static std::vector<WiFiInfo> GetAvailableNetworks();

      /// @brief Begin the WiFi connection process, prepare the enviroment and optionally connect.
      /// @details This method initiates the WiFi connection process. If `autoConnect` is true,
      ///          it will attempt to connect to a known access point automatically.
      /// @param autoConnect Flag [optional] - the method will attempt to connect to a known AP automatically.  
      ///                    The default is `true` if not specified.
      /// @return True if the connection process was initiated successfully, false otherwise.
      /// @author Chris-70 (2025/09)
      bool Begin(IBinaryClock& clock, bool autoConnect = true);

      /// @brief End the WiFi connection and optionally save the settings.
      /// @details This method disconnects from the current WiFi network and optionally saves the settings.
      ///          After this call, `Begin()` must be called before any other methods.
      /// @param save If true, the current settings will be saved to NVS before ending. Default is false.
      /// @author Chris-70 (2025/09)
      void End(bool save = false);

      /// @brief Save the current settings to NVS.
      /// @details This method saves the current settings to NVS using the `BinaryClockSettings` instance.
      /// @return True if the save operation was successful, false otherwise.
      /// @see BinaryClockSettings::Save()
      /// @author Chris-70 (2025/09)
      bool Save()
         { return settings.Save(); }

      /// @brief Update the time from the NTP server and set it in the `IBinaryClock` instance.
      /// @details This method synchronizes the time with the configured NTP server and updates
      ///          the `IBinaryClock` instance with the new time.
      /// @return True if the time was updated successfully, false otherwise.
      /// @author Chris-70 (2025/09)
      bool UpdateTime();
      bool UpdateTime(DateTime& time);

      /// @brief Synchronize the time with the NTP server.
      /// @details This method contacts the configured NTP server and retrieves the current time.
      /// @note    The `DateTime` of "2001/01/01 00:00:00" indicates failure.
      /// @return A DateTime object containing the synchronized time.
      /// @author Chris-70 (2025/09)
      DateTime SyncTimeNTP();

      void SyncAlert(const DateTime& dateTime);

   //#################################################################################//  
   // Protected METHODS                                                               //   
   //#################################################################################//   

   protected:
      void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

      void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);

   //#################################################################################//  
   // Private METHODS                                                                 //   
   //#################################################################################//   

   private:
      /// @copydoc ConnectLocal()
      /// @param bypassCheck If true, bypass the check for `initialized` flag is set.
      /// @remarks The bypass flag is used so that this method can be called from
      ///          within the `Begin()` method without checking the `initialized` flag.
      bool connectLocalWiFi(bool bypassCheck = false);

   //#################################################################################//  
   // Public PROPERTIES                                                               //   
   //#################################################################################//   

   public:
      /// @brief `NtpServers` Property: List of NTP servers to use for syncronizing the time with.
      /// @details Wrapper for the `BinaryClockNTP` property: `NtpServers`.
      ///          `get_` method returns the list of NTP servers used for time synchronization.
      ///          `set_` method allows modifying the list of NTP servers.
      /// @param value A vector of String objects containing the NTP server addresses.
      /// @see get_NtpServers()
      /// @author Chris-70 (2025/09)
      void set_NtpServers(std::vector<String> value)
         { ntp.set_NtpServers(value); }
      /// @copydoc set_NtpServers()
      /// @return A vector of String objects containing the NTP server addresses.
      /// @see set_NtpServers()
      std::vector<String> get_NtpServers() const
         { return ntp.get_NtpServers(); }

      /// @brief `WiFiCreds` Property (RO): The `APCreds` credentials of the AP for the current connection.
      /// @details This property provides access to the credentials of the currently connected AP.
      /// @return An `APCreds` structure containing the SSID, BSSID, and password of the connected AP.
      /// @see get_LocalIP()
      /// @see get_LocalCreds()
      /// @see set_LocalCreds()
      /// @author Chris-70 (2025/09)
      APCreds get_WiFiCreds() const
         { return localCreds; }
      
      /// @brief `LocalIP` Property: The local IP address assigned when connected to WiFi.
      /// @details This property provides access to the local IP address assigned to the device.
      /// @return An `IPAddress` object containing the local IP address on the network.
      /// @see get_WiFiCreds()
      /// @author Chris-70 (2025/09)
      IPAddress get_LocalIP() const
         { return localIP; }

      /// @brief `IsConnected` Property (RO): Indicates whether the device is currently connected to WiFi.
      /// @details This property checks the connection status of the WiFi interface.
      /// @return True if the device is connected to WiFi, false otherwise.
      /// @see WiFiClass::isConnected()
      /// @author Chris-70 (2025/09)
      bool get_IsConnected() const
         { return WiFi.isConnected(); }

      /// @brief `Timezone` Property: The timezone string used for time calculations.
      /// @details This property provides access to the timezone string used for time calculations.
      ///          The timezone string is in Proleptic Format, e.g. "EST+5EDT,M3.2.0/2,M11.1.0/2".
      ///          See `BinaryClockNTP::set_Timezone()` for the format details.
      ///          `get_` method returns the current timezone string.
      ///          `set_` method allows modifying the timezone string.
      /// @param value A String object containing the timezone in Proleptic Format.
      /// @see get_Timezone()
      /// @see BinaryClockNTP::set_Timezone() 
      /// @author Chris-70 (2025/10)
      void set_Timezone(String value);
         
      /// @copydoc set_Timezone()
      /// @return A String object containing the timezone in Proleptic Format.
      /// @see set_Timezone()
      String get_Timezone() const;

   //#################################################################################//  
   // Protected PROPERTIES                                                            //   
   //#################################################################################//   

   protected:

      /// @brief `LocalCreds` Property: The `APCredsPlus` credentials of the AP for the current connection.
      /// @details This property provides access to the credentials of the currently connected AP.
      ///          `get_` method returns the credentials of the connected AP.
      ///          `set_` method allows modifying the credentials of the connected AP.
      /// @param value An `APCredsPlus` structure containing the SSID, BSSID, password, and ID of the connected AP.
      /// @see get_WiFiCreds()
      /// @see get_LocalIP()
      /// @author Chris-70 (2025/09)
      void set_LocalCreds(APCredsPlus value)
         { localCreds = value; }

      /// @copydoc set_LocalCreds()
      /// @return An `APCredsPlus` structure containing the SSID, BSSID, password, and ID of the connected AP.
      /// @see set_LocalCreds()
      APCredsPlus get_LocalCreds() const
         { return localCreds; }

      /// @copydoc get_LocalIP()
      /// @param value An `IPAddress` object containing the local IP address on the network.
      void set_LocalIP(IPAddress value)
         { localIP = value; }

   //#################################################################################//  
   //                                   FIELDS                                        //  
   //#################################################################################//   

   private:
      DummyBinaryClock dummyClock = DummyBinaryClock();
      /// @brief A pointer to the  `IBinaryClock` implementation instance.  
      ///        The initial value is a placeholder until `Begin()` is called.
      IBinaryClock* clockPtr = &dummyClock;
      BinaryClockSettings& settings = BinaryClockSettings::get_Instance();    ///< Local reference to the setting stored in NVS.
      BinaryClockNTP& ntp = BinaryClockNTP::get_Instance(); ///< NTP client reference to the time synchronization.
      BinaryClockWPS& wps = BinaryClockWPS::get_Instance(); ///< WPS handler reference to the WPS connections.

      APCredsPlus localCreds;          ///< The credentials of the AP for the current connection.
      IPAddress localIP;               ///< The IP address of the local device when connected to WiFi.
      WiFiClient client;               ///< The WiFi client instance for network operations.
      WiFiEventId_t eventID;           ///< The WiFi event ID for managing event handlers.

      DateTime lastSync;               ///< The time of the last sync with the NTP server.
      TimeSpan zuluOffset;             ///< Current time offset to UTC/Zulu time.
      bool initialized = false;        ///< Flag: True if the `Begin()` method has been called.
      bool ntpSynced = false;          ///< Flag: True if the time has been synchronized with NTP.

      std::vector<WiFiInfo> localAPs;  ///< List of local WiFi access points.
      }; // class BinaryClockWAN
   } // namespace BinaryClockShield

#endif // __BINARYCLOCKWIFI_H__
