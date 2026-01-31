/**
 * @file TEST_WIFI_CREDS.h
 * @brief Temporary WiFi credentials for testing NTP crash fix
 * @details 
 * To test the NTP crash fix without WPS:
 * 1. Edit this file with your WiFi SSID and password
 * 2. In BinaryClockWAN.cpp, add at the top: #include "TEST_WIFI_CREDS.h"
 * 3. In BinaryClockWAN.cpp connectLocalWiFi(), replace the settings-based credential lookup with:
 *      
 *      #ifdef TEST_WIFI_SSID
 *          APCredsPlus testCreds;
 *          testCreds.ssid = TEST_WIFI_SSID;
 *          testCreds.pw = TEST_WIFI_PASSWORD;
 *          // Try manual connection...
 *      #endif
 *
 * This is a TEMPORARY TEST FILE ONLY - not intended for production use
 */

#pragma once
#ifndef TEST_WIFI_CREDS_H
#define TEST_WIFI_CREDS_H

// ============================================================================
// EDIT THESE WITH YOUR WIFI CREDENTIALS
// ============================================================================

/**
 * Your WiFi network name (SSID)
 * Example: #define TEST_WIFI_SSID "MyNetwork"
 */
#define TEST_WIFI_SSID "YOUR_WIFI_SSID_HERE"

/**
 * Your WiFi network password
 * Example: #define TEST_WIFI_PASSWORD "MyPassword123"
 */
#define TEST_WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

// ============================================================================
// NOTES FOR TESTING
// ============================================================================
// 
// The NTP crash fix implements:
// 1. Persistent C-string storage for NTP server names (avoids String buffer invalidation)
// 2. Pre-task-creation server copy (eliminates thread-safety race conditions)
// 3. 3000ms async delay for SNTP initialization (allows inter-core synchronization)
//
// When WiFi connects with proper credentials, the expected serial output should be:
//
//    Connected to WiFi. BinaryClockWAN::Begin() - Connection is stable, now initializing NTP...
//    New task for non-blocking call to initNTP()...
//    [xxxxx] Task 'NTPInitTask' started
//    BinaryClockNTP::initNTP() - delaying initialization for 3000 ms
//    [xxxxx] Task 'SetupWiFiTask' completed successfully  ← KEY: Task completes BEFORE SNTP starts
//    [xxxxx] Task 'SetupWiFiTask' deleted
//    Initializing SNTP...
//    [xxxxx] SNTP initialized with 3 servers
//    [xxxxx] Callbacks enabled for SNTP
//    [xxxxx] processTimeSync() - NTP time sync notification received
//
// SUCCESS = No "Guru Meditation Error" or "LoadProhibited" exception
//
// If you see a crash like:
//    Guru Meditation Error: Core 1 panic'ed (LoadProhibited). Exception was unhandled.
// Then the fix did not work and further debugging is needed.
//

#endif // TEST_WIFI_CREDS_H
