
#pragma once
#ifndef __BINARYCLOCKSETTINGS_H__
#define __BINARYCLOCKSETTINGS_H__

#include <stdint.h>                    /// Integer types: uint8_t; uint16_t; etc.

#include "BinaryClock.Structs.h"       /// Global structures and enums used by the Binary Clock project.

// STL classes required to be included:
#include <String>
#include <vector>
#include <map>

#include <Preferences.h>               /// Preferences class, part of the ESP32 NVS storage library
#include "nvs.h"
#include "nvs_flash.h"
#include "nvs_handle.hpp"

#define TIMEZONE_UTC        "UTC"      ///< UTC timezone string, used when no timezone is defined.

namespace BinaryClockShield
   {
   /// @brief The class that manages the Binary Clock settings, including WiFi credentials.
   /// @details This class uses the Preferences library to store and retrieve WiFi credentials.
   ///          It supports storing multiple access points and uses an `ID` value to differentiate
   ///          between them. The AP credentyials are stored as an array of `APCredsPlus` objects.  
   ///          The elements have a unique `id` assigned to them when created. The `APNames` value
   ///          must be unique, however, multiple APs with the same SSID but different `BSSID`s
   ///          are stored as different entries and can have different passwords. The `BSSID` can be
   ///          empty, in which case the first AP with the matching SSID will be used.
   /// @remarks `Begin()` must be called first in order to initialize this instance with the values
   ///          from the Non-Volatile Storage (NVS). `End()` should be called when done to free resources
   ///          and to optionally save any changes. `Clear()` can be used to clear all the stored APs
   ///          `Save()` must be called to save the current settings, including additions and/or deletions.
   /// @note    Calling `Clear()` followed by `Save()` will have the effect of removing all AP credentials
   ///          from the NVS.
   /// @author Chris-70 (2025/09)
   class BinaryClockSettings
      {
   protected:
      BinaryClockSettings();
      virtual ~BinaryClockSettings();

      // Disable copy and move to enforce singleton semantics
      BinaryClockSettings(const BinaryClockSettings&) = delete;
      BinaryClockSettings& operator=(const BinaryClockSettings&) = delete;
      BinaryClockSettings(BinaryClockSettings&&) = delete;
      BinaryClockSettings& operator=(BinaryClockSettings&&) = delete;

   //#################################################################################//  
   // Public METHODS                                                                  //   
   //#################################################################################//   
   public:

      static BinaryClockSettings& get_Instance()
         {
         static BinaryClockSettings instance;  // Guaranteed to be destroyed.
         return instance;
         }

      /// @brief Initialize the BinaryClockSettings instance and load data from NVS.
      /// @details This method opens the NVS namespace and reads the stored AP credentials into
      ///          the internal data structures. It must be called before any other methods are used.
      /// @remarks Call the `End()` method when done. `End(bool)` will save first.
      /// @note    If the NVS namespace does not exist, it will be created.
      /// @see End()
      /// @author Chris-70 (2025/09)
      void Begin();

      /// @brief Clear all stored AP credentials from the internal data structures.
      /// @details This method clears all the AP credentials stored in RAM. It does not affect
      ///          the data stored in NVS. To remove the data from NVS, call `Save()` after
      ///          this method.
      /// @see Save()
      /// @author Chris-70 (2025/09)
      void Clear();

      /// @brief Save the current AP credentials to NVS.
      /// @details This method saves the current AP credentials stored in RAM to NVS. It must be called
      ///          after making any changes to the AP credentials to persist those changes.
      /// @return True if the save operation was successful, false otherwise.
      /// @author Chris-70 (2025/09)
      bool Save();

      /// @brief End the BinaryClockSettings instance and free resources.
      /// @details This method frees any resources used by the instance and optionally saves any changes.
      /// @note After this call, you must call `Begin()` before any other calls. 
      /// @param save If true, the current settings will be saved to NVS before ending. Default is false.
      /// @see Begin()
      /// @see Save()
      /// @author Chris-70 (2025/09)
      void End(bool save = false);

      // WiFi settings
      /// @brief Get the ID of the AP with the given `APNames` names (ssid and bssid).
      /// @param names The APNames structure containing the SSID and BSSID of the access point.
      /// @return The ID of the AP if found, 0 otherwise.
      /// @author Chris-70 (2025/09)
      uint8_t GetID(const APNames& names) const;

      /// @brief Get the AP credentials for the given ID.
      /// @param id The ID of the access point.
      /// @return An APCredsPlus structure containing the AP credentials if found, otherwise an empty structure.
      /// @see GetID()
      /// @see GetWiFiAP(const APNames& names) const
      /// @author Chris-70 (2025/09)
      APCredsPlus GetWiFiAP(uint8_t id) const;

      /// @copydoc` GetWiFiAP(uint8_t) const
      /// @param names The APNames structure containing the SSID and BSSID of the access point.
      /// @see GetID(const APNames& names) const
      /// @see GetWiFiAP(uint8_t id) const
      APCredsPlus GetWiFiAP(const APNames& names) const
            { return GetWiFiAP(GetID(names)); }

      /// @brief Get a list of AP credentials that match the given SSID.
      /// @details This method returns a vector of `APCredsPlus` structures that match the given SSID.
      ///          If multiple APs have the same SSID but different BSSIDs, all matching entries will be returned.
      /// @param ssid The SSID of the access points to search for.
      /// @return A vector of APCredsPlus structures containing the matching AP credentials.
      /// @see GetWiFiAP(const std::vector<APNames>& names) const
      /// @see GetIDs()
      /// @author Chris-70 (2025/09)
      std::vector<APCredsPlus> GetWiFiAPs(const String& ssid) const;

      /// @brief Get a list of stored AP credentials that match the given names.
      /// @details This method returns a vector of `APCredsPlus` structures that match the given names.  
      ///          This is typically used to get the list of stored AP credentials from the list of APs
      ///          found during a WiFi scan.
      /// @param names The APNames structures containing the SSID and BSSID of the access points.
      /// @return A vector of APCredsPlus structures containing the matching AP credentials.
      /// @see GetWiFiAP(const APNames& names) const
      /// @see GetIDs()
      /// @author Chris-70 (2025/09)
      std::vector<APCredsPlus> GetWiFiAPs(const std::vector<APNames>& names) const;

      std::vector<std::pair<APCredsPlus, WiFiInfo>> GetWiFiAPs(const std::vector<WiFiInfo>& wifis) const;

      /// @brief Add the AP credentials for the given SSID, password, and optional BSSID to the internal data structures.
      /// @details This method adds the AP credentials to the internal data structures. If an entry
      ///          with the same SSID and BSSID already exists, it will be updated.
      /// @param ssid The SSID of the access point.
      /// @param password The password for the access point.
      /// @param bssid The BSSID of the access point (optional).
      /// @return The ID of the added or updated access point.
      /// @see Save()
      /// @see AddWiFiCreds(const APCreds& creds)
      /// @author Chris-70 (2025/09)
      uint8_t AddWiFiCreds(const String& ssid, const String& password, const String& bssid = "");

      /// @brief Add the AP credentials from an `APCreds` instance to the AP data structure.
      /// @details This method adds the AP credentials to the internal data structures. If an entry
      ///          with the same SSID and BSSID already exists, it will be updated.
      /// @param creds The `APCreds` structure containing the AP credentials to save.
      /// @return The ID of the added or updated access point.
      /// @see Save()
      /// @see AddWiFiCreds(const String& ssid, const String& password, const String& bssid = "")
      /// @author Chris-70 (2025/09)
      uint8_t AddWiFiCreds(const APCreds& creds);

      /// @brief Mark an AP entry for deletion based on its ID.
      /// @details This method marks the AP entry with the given ID for deletion. The actual deletion
      ///          will occur when the `Save()` method is called.   
      ///          To undo this, call `UndeleteID()` before any call to `Save()` is made.
      /// @see UndeleteID()
      /// @see Save()
      /// @author Chris-70 (2025/09)
      bool DeleteID(uint8_t id)
         { return changeDeleteStatus(id, true); }

      /// @brief Unmark an AP entry for deletion based on its ID.
      /// @details This method removes the deletion mark from the AP entry with the given ID.
      ///          This can be used to undo a previous call to `DeleteID()` before `Save()` is called.
      /// @see DeleteID()
      /// @see Save()
      /// @author Chris-70 (2025/09)
      bool UndeleteID(uint8_t id)
         { return changeDeleteStatus(id, false); }

      void set_Timezone(String value)
         { 
         if (value.isEmpty())
            { value = TIMEZONE_UTC; }
            
         if (value != timezone)
            {
            timezone = value;
            modified = true;
            }
         }

      String get_Timezone() const
         { return timezone; }

   //#################################################################################//  
   // Protected METHODS                                                               //   
   //#################################################################################//   

   protected:
      /// @brief Child structure of `APCredsPlus` to hold additional flags needed by this class.
      /// @details This structure adds flags to indicate if the AP has been modified or marked for deletion.
      ///          The internal `apCreds` vector holds these objects which track the state of each AP and are
      ///          used when saving the `APCredsPlus` objects to NVS.   
      ///          The `Save()` method processes this vector to update NVS and removes the entries 
      ///          marked for deletion.
      /// @see Save()
      /// @author Chris-70 (2025/09)
      struct ApAllInfo : public APCredsPlus
         {
         bool modifiedAP  = false; ///< The AP value(s) has been modified and needs to be saved.
         bool toBeDeleted = false; ///< The AP entry is marked for deletion.
         ApAllInfo(const APCredsPlus& creds) : APCredsPlus(creds) { }
         ApAllInfo(const APCreds& creds) : APCredsPlus(creds) { }
         ApAllInfo(const APNames& names) : APCredsPlus(names) { }
         ApAllInfo(const ApAllInfo& apInfo) = default;
         ApAllInfo() = default; // : APCredsPlus() {}
         virtual ~ApAllInfo() = default;
         };

      /// @brief Get a list of IDs for APs that match the given SSID.
      /// @details This method returns a vector of IDs for APs that all have the specified SSID.
      /// @param ssid The SSID to search for.
      /// @return A vector of IDs for the matching APs. If no matches are found, an empty vector is returned.
      /// @author Chris-70 (2025/09)
      std::vector<uint8_t> GetIDs(const String& ssid) const;

      /// @brief Get the index of an AP entry by its ID.
      /// @details This helper method returns the index of the AP entry with the specified ID.
      ///          This uses the `idList` map for efficient lookup. If the ID is not found, -1 is returned.
      /// @param ID The ID of the AP entry to search for.
      /// @return The index of the matching AP entry, or -1 if not found.
      /// @author Chris-70 (2025/09)
      int GetIndex(uint8_t ID) const;

      /// @brief Generate a new unique ID for a new AP entry.
      /// @details This method generates a new unique ID that is usually one greater than the last assigned ID.
      ///          If the last ID is at the maximum value (255), it searches for the lowest available ID.
      ///          Before assigning the ID it checks `idList` first to confirm it's not used.
      ///          A maximum of `MAX_ID_SIZE` (i.e. 255) unique IDs can be generated.
      /// @return A new unique ID for the AP entry, or 0 if no IDs are available.
      /// @author Chris-70 (2025/09)
      uint8_t GetNewID();

   //#################################################################################//  
   // Private METHODS                                                                 //   
   //#################################################################################//   

   private:
      /// @brief Change the deletion status of an AP entry.
      /// @details This helper method updates the `toBeDeleted` flag of the specified AP entry.
      ///          This is called from `DeleteID()` and `UndeleteID()` methods.
      /// @param id The ID of the AP entry to modify.
      /// @param toDelete The new deletion status to set.
      /// @return True if the status was changed successfully, false otherwise.
      /// @author Chris-70 (2025/09)
      bool changeDeleteStatus(uint8_t id, bool toDelete);

      // Helper methods for serialization
      /// @brief Serialize the AP credentials to a byte buffer.
      /// @details This helper method serializes the given `APCredsPlus` object into a byte buffer.
      ///          This buffer is stored in the NVS and is read back in the `deserializeAPCreds()` method.
      /// @param buffer The byte buffer to write to.
      /// @param offset The current offset in the buffer.
      /// @param creds The `APCredsPlus` credentials to serialize.
      /// @see deserializeAPCreds()
      /// @author Chris-70 (2025/09)
      void serializeAPCreds(uint8_t* buffer, size_t& offset, const APCredsPlus& creds) const;

      /// @brief Deserialize the AP credentials from a byte buffer.
      /// @details This helper method deserializes an `APCredsPlus` object from a byte buffer.
      ///          This buffer is read from the NVS and is written back in the `serializeAPCreds()` method.
      /// @param buffer The byte buffer to read from.
      /// @param offset The current offset in the buffer.
      /// @param creds The `APCredsPlus` credentials to deserialize.
      /// @see serializeAPCreds()
      /// @author Chris-70 (2025/09)
      void deserializeAPCreds(const uint8_t* buffer, size_t& offset, APCredsPlus& creds) const;

      /// @brief Calculate the size needed to serialize an `APCredsPlus` object.
      /// @details This helper method calculates the size in bytes needed to serialize the given
      ///          `APCredsPlus` object.
      /// @param creds The `APCredsPlus` credentials to serialize.
      /// @return The size in bytes needed to serialize the `APCredsPlus` object.
      /// @see calculateTotalSize()
      /// @author Chris-70 (2025/09)
      size_t calculateAPCredsSize(const APCredsPlus& creds) const;

      /// @brief Calculate the total size needed to serialize all stored AP credentials.
      /// @details This helper method calculates the total size in bytes needed to serialize all
      ///          stored `APCredsPlus` objects.
      /// @return The total size in bytes needed to serialize all `APCredsPlus` objects.
      /// @see calculateAPCredsSize()
      /// @author Chris-70 (2025/09)
      size_t calculateTotalSize() const;

   //#################################################################################//  
   //                                   FIELDS                                        //  
   //#################################################################################//   

   protected:

   private:
      Preferences nvs;                    ///< The `Preferences` instance of the Non-Volatile Storage (NVS).

      uint8_t numAPs = 0;                 ///< The number of saved APs in NVS.
      uint8_t lastID = 0;                 ///< The ID assigned to the last `APCredsPlus` object created.
      std::vector<ApAllInfo> apCreds;     ///< Vector to hold the AP credentials in RAM.
      std::map<uint8_t, size_t> idList;   ///< Map of the IDs in NVS, and their index in `apCreds`
      String timezone;                    ///< The timezone string stored in NVS.

      const char* nvsNamespace         = "bc_settings";     ///< The NVS namespace for the AP settings
      const char* nvsKeyAPCreds        = "ap_creds";        ///< Key to store the vector of APCreds as blob
      const char* nvsKeyNumAPs         = "num_aps";         ///< Key to store the number of access points in NVS (i.e. size of `id_array`)
      const char* nvsKeyLastID         = "last_id";         ///< Key to store the last ID saved (next ID = last_id + 1;)
      const char* nvsKeyTimezone       = "timezone";        ///< Key to store the timezone string

      const size_t maxSSIDLength       = 32;    ///< Maximum SSID length
      const size_t maxPasswordLength   = 64;    ///< Maximum password length
      const size_t maxBSSIDLength      = 17;    ///< Maximum BSSID length (e.g. "00:11:22:33:44:55")
      bool initialized                 = false; ///< Flag: The NVS data has been processed to RAM
      bool modified                    = false; ///< Flag: A changes was made to the data.
      }; // class BinaryClockSettings
   } // namespace BinaryClockShield

#endif // __BINARYCLOCKSETTINGS_H__
