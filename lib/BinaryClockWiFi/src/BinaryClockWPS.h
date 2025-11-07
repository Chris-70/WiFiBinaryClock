/// @file BinaryClockWPS.h
/// @brief Header file for BinaryClockWPS class for WPS WiFi connection.
/// @details This file defines the BinaryClockWPS class which provides
///          functionality to connect to WiFi networks using WPS Push Button mode.
/// @author Chris-70 (2025/09)

#pragma once
#ifndef __BINARYCLOCKWPS_H__
#define __BINARYCLOCKWPS_H__

// If DEV_CODE is defined and true, include the `BinaryClock.Defines.h` file for SERIAL_STREAM macro.
// This allows for serial streaming output during development and debugging and removes the
// dependence on `BinaryClock.Defines.h` when not in development mode.
#if DEV_CODE
   #include "BinaryClock.Defines.h"    /// BinaryClock project-wide definitions and MACROs.
#else
   #define SERIAL_STREAM(STRING)       // Define as empty to remove the code if DEV_CODE is not true.
#endif

#ifndef WIFI
   #define WIFI (ESP32_WIFI || WIFIS3)
#endif

#include <stdint.h>              /// Integer types: uint8_t; uint16_t; etc.
#include <Streaming.h>           /// Streaming serial output with `operator<<` https://github.com/janelia-arduino/Streaming

#if ESP32_WIFI
   #include <WiFi.h>
   #include <esp_wps.h>
   #include <esp_wifi.h>
   #include "esp_event.h"
   #include <nvs_flash.h>
#elif WIFIS3
   #include <WiFiS3.h>
#else   
   #error "BinaryClockWPS requires either ESP32_WIFI or WIFIS3 to be defined and true in BinaryClock.Defines.h or board_select.h"
#endif

#include "BinaryClock.Structs.h"       /// Global structures and enums used by the Binary Clock project.

#define DEFAULT_WPS_TIMEOUT_MS 120000  ///< Default WPS timeout in milliseconds (2 minutes)

namespace BinaryClockShield
   {
   /// @brief WPS connection result structure
   struct WPSResult
      {
      bool success = false;           // True if WPS connection was successful
      APCreds credentials;            // Populated AP credentials on success
      String errorMessage;            // Error description if failed
      uint32_t connectionTimeMs = 0;  // Time taken to connect in milliseconds
      };

   /// @brief BinaryClockWPS class for WiFi connection using WPS Push Button mode
   /// @details This class handles WiFi Protected Setup (WPS) connections using the push button method.
   ///          It follows the WPS standard and populates APCreds structure upon successful connection.
   /// @author Chris-80 (2025/09)
   class BinaryClockWPS
      {
   public:
      static BinaryClockWPS& get_Instance();

      /// @brief Start WPS Push Button connection
      /// @details Initiates WPS push button mode. User must press the WPS button on the router
      ///          within the timeout period for connection to succeed.
      /// @return WPSResult containing success status and AP credentials
      WPSResult ConnectWPS();
      
      /// @brief Cancel any ongoing WPS connection attempt
      void CancelWPS();

      /// @brief Check if WPS connection is in progress
      /// @return True if WPS connection attempt is active
      bool get_IsConnecting() const { return wpsActive; }
      
      /// @brief Set the timeout for WPS connection.
      /// @param timeoutMs Timeout in milliseconds.
      void set_Timeout(uint32_t timeoutMs) { timeout = timeoutMs; }

      /// @brief Get the current timeout value
      /// @return Timeout in milliseconds
      uint32_t get_Timeout() const { return timeout; }

   protected:
      /// @brief Constructor
      BinaryClockWPS();

      /// @brief Destructor
      virtual ~BinaryClockWPS();

      // Non-copyable / non-movable to enforce singleton semantics
      BinaryClockWPS(const BinaryClockWPS&) = delete;
      BinaryClockWPS& operator=(const BinaryClockWPS&) = delete;
      BinaryClockWPS(BinaryClockWPS&&) = delete;
      BinaryClockWPS& operator=(BinaryClockWPS&&) = delete;

   private:
      /// @brief Initialize WPS configuration.
      /// @return True if initialization was successful.
      bool initWPS();

      /// @brief Cleanup WPS resources.
      void cleanupWPS();

      /// @brief Extract AP credentials from WiFi connection.
      /// @return APCreds structure with connection details.
      APCreds extractCredentials();

      // Static callback functions for WPS events.
      static void wpsEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
      // static BinaryClockWPS* instance; // Static instance for callback access
      
      uint32_t timeout;             // WPS connection timeout in milliseconds.
      bool wpsActive;               // Flag indicating WPS is active.

      // WPS configuration
      esp_wps_config_t wpsConfig;

      // WPS event handling
      bool wpsSuccess;
      bool wpsTimeout;
      String wpsError;
      }; // class BinaryClockWPS

   } // namespace BinaryClockShield

#endif // __BINARYCLOCKWPS_H__