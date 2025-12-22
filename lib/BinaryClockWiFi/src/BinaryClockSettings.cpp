
#include <stdint.h>                    /// Integer types: uint8_t; uint16_t; etc.

#ifndef INLINE_HEADER
   #define INLINE_HEADER false
#endif   
#if INLINE_HEADER
// If you need the class definition in the same file, e.g. for CoPilot, 
// just copy-paste the contents of the header file here and change the define.
#else
   #include "BinaryClockSettings.h"
#endif   // INLINE_HEADER...ELSE
   
#include <String>
#include <vector>
#include <map>

#include <Streaming.h>    /// Streaming serial output with `operator<<` (https://github.com/espressif/arduino-esp32/blob/master/libraries/Streaming/)
#include <Preferences.h>  /// ESP32 NVS storage preferences functions.

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

///////////////////////////////////////////////////////////////////////////////////////////////////
//                Implementation of BinaryClockSettings class                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace BinaryClockShield
   {
   BinaryClockSettings::BinaryClockSettings() 
      { }

   BinaryClockSettings::~BinaryClockSettings()
      { 
      if (initialized)
         { End(); }
      }

   void BinaryClockSettings::Begin()
      {
      SERIAL_PRINTLN("Begin(): Initializing BinaryClockSettings...")   // *** DEBUG ***
      // Open NVS namespace
      if (!nvs.begin(nvsNamespace, true))
         {
         SERIAL_PRINTLN("Begin(): Failed to open NVS namespace in RO mode.") // *** DEBUG ***
         if (!nvs.begin(nvsNamespace, false))
            {
            SERIAL_PRINTLN("Begin(): Failed to open NVS namespace in RW mode.") // *** DEBUG ***
            return;
            }
         }

      // Get number of access points stored
      numAPs = nvs.getUChar(nvsKeyNumAPs, 0);
      lastID = nvs.getUChar(nvsKeyLastID, 0);
      timezone = nvs.getString(nvsKeyTimezone, TIMEZONE_UTC);

      // Clear existing data
      Clear();

      if (numAPs > 0)
         {
         // Get the size of the stored blob
         size_t blobSize = nvs.getBytesLength(nvsKeyAPCreds);
         
         if (blobSize > 0)
            {
            // Allocate buffer for reading the blob
            uint8_t* buffer = new uint8_t[blobSize];
            
            // Read the blob from NVS
            size_t readSize = nvs.getBytes(nvsKeyAPCreds, buffer, blobSize);
            
            if (readSize == blobSize)
               {
               // Deserialize the APCreds from the buffer
               size_t offset = 0;
               
               for (uint8_t i = 0; i < numAPs && offset < blobSize; i++)
                  {
                  ApAllInfo credsInfo;
                  
                  // Deserialize base APCreds
                  deserializeAPCreds(buffer, offset, credsInfo);
                  if (credsInfo.id > lastID)
                     { lastID = credsInfo.id; } // Update lastID if needed
                           
                  
                  // Reset all modification flags since we're loading from NVS
                  credsInfo.modifiedAP = false;
                  credsInfo.toBeDeleted = false;

                  // Only add if we have valid data (at least SSID)
                  if (!credsInfo.ssid.isEmpty())
                     {
                     // Add ApAllInfo object to apCreds vector
                     apCreds.push_back(credsInfo);

                     // Add to idList map: ID -> apCreds vector index
                     size_t vectorIndex = apCreds.size() - 1;
                     idList[credsInfo.id] = vectorIndex;
                     }
                  }
               }
            else
               {
               SERIAL_PRINTLN("Begin(): Failed to read AP credentials blob from NVS.")   // *** DEBUG ***
               }
               
            delete[] buffer;
            }
         }

      // Mark as initialized and not modified
      initialized = true;
      modified = false;
      nvs.end();

      SERIAL_PRINT("Loaded ")                        // *** DEBUG ***
      SERIAL_PRINT(apCreds.size())
      SERIAL_PRINTLN(" WiFi credentials from NVS")   // *** DEBUG ***
      }

   void BinaryClockSettings::Clear()
      {
      apCreds.clear();
      idList.clear();
      }
      
   bool BinaryClockSettings::Save()
      {
      SERIAL_STREAM("Save(): Saving " << numAPs << " WiFi credentials to NVS..." << endl)  // *** DEBUG ***
      bool result = false;
      if (!initialized || !modified) { return !modified; } // Nothing to save

      // Open NVS namespace in RW mode
      if (!nvs.begin(nvsNamespace, false))
         {
         SERIAL_PRINTLN("Save(): Failed to open NVS namespace in RW mode")   // *** DEBUG ***
         return result;
         }

      if (numAPs > 0)
         {
         SERIAL_STREAM("Save(): Saving " << numAPs << " AP credentials to NVS..." << endl) // *** DEBUG ***
         // Calculate total size needed for serialization
         size_t totalSize = calculateTotalSize();
         
         // Allocate buffer
         uint8_t* buffer = new uint8_t[totalSize];
         size_t offset = 0;

         // Serialize all APCreds to buffer that aren't marked for deletion.
         for (auto it = apCreds.begin(); it != apCreds.end(); ++it)
            {
            const auto& creds = *it;
            // Remove the entries marked for deletion and continue
            if (creds.toBeDeleted)
               { 
               idList.erase(creds.id);
               apCreds.erase(it);
               continue; 
               } 

            serializeAPCreds(buffer, offset, creds);
            }

         // Store the blob in NVS
         size_t written = nvs.putBytes(nvsKeyAPCreds, buffer, offset);
         
         if (written != offset)
            {
            SERIAL_PRINTLN("Save(): Failed to save AP credentials blob to NVS") // *** DEBUG ***
            result = false;
            }
         else
            {
            SERIAL_PRINT("Saved ")                      // *** DEBUG ***
            SERIAL_PRINT(numAPs)
            SERIAL_PRINTLN(" WiFi credentials to NVS")  // *** DEBUG ***
            for (auto& creds : apCreds)
               { creds.modifiedAP = false; } // Clear modified flag after successful save

            modified = false; // Clear modified flag after successful save
            result = true;
            }

         delete[] buffer;
         }
      else
         {
         // No APs to save, remove the blob
         nvs.remove(nvsKeyAPCreds);
         modified = false;
         // TODO: Is this result a success or failure? Success for now.
         SERIAL_PRINTLN("Save(): No AP credentials to save, removed blob from NVS.")  // *** DEBUG ***
         result = true;
         }

      // Update numAPs based on current vector size
      numAPs = apCreds.size();
      
      nvs.putUChar(nvsKeyNumAPs, numAPs);
      nvs.putUChar(nvsKeyLastID, lastID);
      nvs.putString(nvsKeyTimezone, timezone);
      SERIAL_STREAM("Save(): Saved timezone: [" << timezone << "]" << endl) // *** DEBUG ***
      nvs.end();

      return result;
      }

   void BinaryClockSettings::serializeAPCreds(uint8_t* buffer, size_t& offset, const APCredsPlus& creds) const
      {
      // Store ID
      buffer[offset++] = creds.id;
      
      // Store SSID length and data
      uint16_t ssidLen = creds.ssid.length();
      memcpy(buffer + offset, &ssidLen, sizeof(ssidLen));
      offset += sizeof(ssidLen);
      memcpy(buffer + offset, creds.ssid.c_str(), ssidLen);
      offset += ssidLen;
      
      // Store BSSID length and data
      uint16_t bssidLen = creds.bssid.length();
      memcpy(buffer + offset, &bssidLen, sizeof(bssidLen));
      offset += sizeof(bssidLen);
      memcpy(buffer + offset, creds.bssid.c_str(), bssidLen);
      offset += bssidLen;
      
      // Store password length and data
      uint16_t pwLen = creds.pw.length();
      memcpy(buffer + offset, &pwLen, sizeof(pwLen));
      offset += sizeof(pwLen);
      memcpy(buffer + offset, creds.pw.c_str(), pwLen);
      offset += pwLen;
      }

   void BinaryClockSettings::deserializeAPCreds(const uint8_t* buffer, size_t& offset, APCredsPlus& creds) const
      {
      // Read ID
      creds.id = buffer[offset++];
      
      // Read SSID
      uint16_t ssidLen;
      memcpy(&ssidLen, buffer + offset, sizeof(ssidLen));
      offset += sizeof(ssidLen);
      char* ssidBuffer = new char[ssidLen + 1];
      memcpy(ssidBuffer, buffer + offset, ssidLen);
      ssidBuffer[ssidLen] = '\0';
      creds.ssid = String(ssidBuffer);
      delete[] ssidBuffer;
      offset += ssidLen;
      
      // Read BSSID
      uint16_t bssidLen;
      memcpy(&bssidLen, buffer + offset, sizeof(bssidLen));
      offset += sizeof(bssidLen);
      char* bssidBuffer = new char[bssidLen + 1];
      memcpy(bssidBuffer, buffer + offset, bssidLen);
      bssidBuffer[bssidLen] = '\0';
      creds.bssid = String(bssidBuffer);
      delete[] bssidBuffer;
      offset += bssidLen;
      
      // Read password
      uint16_t pwLen;
      memcpy(&pwLen, buffer + offset, sizeof(pwLen));
      offset += sizeof(pwLen);
      char* pwBuffer = new char[pwLen + 1];
      memcpy(pwBuffer, buffer + offset, pwLen);
      pwBuffer[pwLen] = '\0';
      creds.pw = String(pwBuffer);
      delete[] pwBuffer;
      offset += pwLen;
      }

   size_t BinaryClockSettings::calculateAPCredsSize(const APCredsPlus& creds) const
      {
      return 1 + // ID (uint8_t)
             sizeof(uint16_t) + creds.ssid.length() + // SSID length + data
             sizeof(uint16_t) + creds.bssid.length() + // BSSID length + data  
             sizeof(uint16_t) + creds.pw.length(); // Password length + data
      }

   size_t BinaryClockSettings::calculateTotalSize() const
      {
      size_t total = 0;
      for (const auto& creds : apCreds)
         {
         total += calculateAPCredsSize(creds);
         }

      return total;
      }

   uint8_t BinaryClockSettings::GetID(const APNames& names) const
      {
      SERIAL_STREAM("- GetID(): Looking for SSID: " << names.ssid << " BSSID: " << names.bssid << endl) // *** DEBUG ***
      uint8_t result = 0;
      if (!initialized || names.ssid.isEmpty()) { return result; } // Error

      for (const auto& creds : apCreds)
         {
         if (creds == names && !creds.toBeDeleted)
            {
            result = creds.id;
            break;
            }
         }

      return result;
      }

std::vector<uint8_t> BinaryClockSettings::GetIDs(const String& ssid) const
      {
      SERIAL_STREAM("- GetIDs(): Looking any matches for SSID: " << ssid << endl)  // *** DEBUG ***
      std::vector<uint8_t> result;
      if (!initialized || ssid.isEmpty()) { return result; } // Error

      for (size_t i = 0; i < apCreds.size(); i++)
         {
         const ApAllInfo& creds = apCreds[static_cast<uint8_t>(i)];
         if (creds.ssid == ssid && !creds.toBeDeleted)
            {
            result.push_back(creds.id);
            }
         }

      return result;
      }

   int BinaryClockSettings::GetIndex(uint8_t ID) const
      {
      int result = -1;
      if (!initialized) { return result; } // Error

      auto it = idList.find(ID);
      if (it != idList.end())
         {
         size_t index = it->second;
         result = static_cast<int>(index);
         }

      return result;
      }

   bool BinaryClockSettings::changeDeleteStatus(uint8_t id, bool toDelete)
      {
      bool result = false;
      if (!initialized) { return result; } // Error

      // Find the index of the credentials, -1 indicates an error.
      int index = GetIndex(id);
      if (index >= 0 && index < apCreds.size())
         {
         // Mark the entry for deletion
         apCreds[index].toBeDeleted = toDelete;
         modified = true; // Mark NVS as modified
         result = true;
         }

      return result;
      }

   uint8_t BinaryClockSettings::GetNewID()
      {
      SERIAL_STREAM("GetNewID(): Generating new ID... Last ID: " << static_cast<int>(lastID) << ". idList size: " << idList.size() 
             << " Initialized? " << (initialized ? "Yes" : "No") << endl)  // *** DEBUG ***
      uint8_t result = 0; // 0 == error
      if (!initialized || idList.size() >= MAX_ID_SIZE) { return result; } // Error

      unsigned counter = 0;
      auto it = idList.find(lastID);
      if (lastID == 0 && it == idList.end())
         {
         // First ID
         result = 1;
         lastID = 1;
         return result;
         }
         
      // Loop looking for an available ID number, wrap around if needed.
      // Stop when we find an unused number, or we have checked all possible numbers.
      while ((++counter < MAX_ID_SIZE) && (it != idList.end()))
         {
         if (++lastID == 0) { lastID = 1; }

         it = idList.find(lastID);
         if (it == idList.end())
            {
            result = lastID;
            break;
            }
         }

      return result; 
      }

   uint8_t BinaryClockSettings::AddWiFiCreds(const APCreds& creds)
      {
      uint8_t id = 0;
      if (!initialized) { return id; } // Error

      SERIAL_PRINT(creds.ssid)
      // First check if the entry exists in our list (i.e. same ssid && same bssid)
      for(auto& existingCreds : apCreds)
         {
         if (existingCreds == creds)
            {
            // SERIAL_PRINT(creds.ssid)
            id = existingCreds.id;
            // Do we update the PW?
            if (existingCreds.pw != creds.pw)
               {
               SERIAL_PRINTLN(" - WiFi SSID and BSSID already exist with different password. Updating password.")   // *** DEBUG ***
               // Update password
               existingCreds.pw = creds.pw;
               existingCreds.modifiedAP = true;
               existingCreds.toBeDeleted = false; // In case it was marked for deletion
               modified = true; // Mark NVS as modified
               }
            else
               {
               existingCreds.toBeDeleted = false; // In case it was marked for deletion
               SERIAL_PRINTLN(" - WiFi credentials already exist. Not adding duplicate.")   // *** DEBUG ***
               }

            return id; // Return existing ID
            }
         }

      id = GetNewID();
      if (id != 0)
         {
         ApAllInfo credsInfo = ApAllInfo(creds);
         credsInfo.id = id;
         credsInfo.modifiedAP = true; // New entry, mark as modified
         apCreds.push_back(credsInfo);

         numAPs = apCreds.size();
         idList[id] = apCreds.size() - 1; // Map new ID to vector index
         modified = true; // Mark NVS as modified
         SERIAL_STREAM("Added new WiFi credentials, SSID: " << creds.ssid << " with ID " << static_cast<int>(id) 
                << ". Total APs: " << static_cast<int>(numAPs) << endl) // *** DEBUG ***
         }
      else
         {
         SERIAL_PRINTLN("Error: Unable to generate new ID for WiFi credentials.")  // *** DEBUG ***
         }

      return id;
      }

   void BinaryClockSettings::End(bool save)
      {
      if (save && modified)
         {
         Save();
         }

      apCreds.clear();
      idList.clear();
      initialized = false;
      modified = false;
      numAPs = 0;
      nvs.end();
      }

   APCredsPlus BinaryClockSettings::GetWiFiAP(uint8_t id) const
      {
      APCredsPlus result;
      if (!initialized) { return result; } // Error

      // Find the index of the credentials
      int index = GetIndex(id);
      if (index >= 0 && index < apCreds.size())
         {
         result = apCreds[index];
         // // TODO: Think about protecting passwords and if it's needed. Also from whom? What is the threat model?
         // result.pw.clear();
         }

      return result;
      }

   std::vector<APCredsPlus> BinaryClockSettings::GetWiFiAPs(const String& ssid) const
      {
      std::vector<APCredsPlus> result;
      if (!initialized || ssid.isEmpty()) { return result; } // Error

      // Find the index of the credentials
      std::vector<uint8_t> ids = GetIDs(ssid);
      for (const auto& id : ids)
         {
         if (id < apCreds.size())
            {
            APCredsPlus cred = apCreds[id];
            // TODO: Think about protecting passwords and if it's needed. Also from whom? What is the threat model?
            cred.pw.clear(); // Clear password for security theater
            result.push_back(cred);
            }
         }

      return result;
      }

   std::vector<APCredsPlus> BinaryClockSettings::GetWiFiAPs(const std::vector<APNames>& names) const
      {
      SERIAL_STREAM("GetWiFiAPs(APNames): Looking for " << names.size() << " APs. Initialized? " << (initialized ? "Yes" : "No") << endl)  // *** DEBUG ***
      std::vector<APCredsPlus> result;
      if (!initialized || names.empty()) { return result; } // Error

      for (const auto& name : names)
         {
         uint8_t id = GetID(name);
         if (id != 0)
            {
            APCredsPlus cred = GetWiFiAP(id);
            result.push_back(cred);
            }
         }

      return result;
      }

   std::vector<std::pair<APCredsPlus, WiFiInfo>> BinaryClockSettings::GetWiFiAPs(const std::vector<WiFiInfo>& wifiInfos) const
      {
      SERIAL_STREAM("GetWiFiAPs(WiFiInfo): Looking for " << wifiInfos.size() << " APs. Initialized? " << (initialized ? "Yes" : "No") << endl)  // *** DEBUG ***
      std::vector<std::pair<APCredsPlus, WiFiInfo>> result;
      if (!initialized || wifiInfos.empty()) { return result; } // Error

      for (const auto& info : wifiInfos)
         {
         uint8_t id = GetID(info);
         if (id != 0)
            {
            SERIAL_STREAM("GetWiFiAPs(WiFiInfo): Found matching AP SSID: " << info.ssid << " BSSID: " << info.bssid << " with ID: " << static_cast<int>(id) << endl)  // *** DEBUG ***
            APCredsPlus cred = GetWiFiAP(id);
            result.push_back(std::make_pair(cred, info));
            }
         }

      return result;
      }

   uint8_t BinaryClockSettings::AddWiFiCreds(const String& ssid, const String& password, const String& bssid)
      {
      uint8_t result = 0;
      if (!initialized || ssid.isEmpty()) { return result; } // Error
      
      APCreds creds;
      creds.ssid = ssid;
      creds.bssid = bssid;
      creds.pw = password;
      result = AddWiFiCreds(creds);

      return result;
      }
   
   } // namespace BinaryClockShield