#pragma once
#ifndef __DIAGNOSTICS_EXAMPLE_H__
#define __DIAGNOSTICS_EXAMPLE_H__

/// @file DiagnosticsExample.h
/// @brief Examples for using Diagnostics.h with BinaryClock_ESP32 project
/// @details Demonstrates how to measure RAM usage and task performance
///          in the context of your specific BinaryClock_ESP32 implementation
/// @author Chris-70 (2025/12)

#include <Arduino.h>
#include "TaskWrapper.h"
#include "Diagnostics.h"

// // __has_include is C++17 and beyond, or an extension in some compilers.
// #ifdef __has_include
//    // FreeRTOS include files we need.
// 	#if __has_include(<FreeRTOS.h>)
// 		#include <FreeRTOS.h>
//       #include <task.h>
//    #elif __has_include(<freertos/FreeRTOS.h>)
//       #include <freertos/FreeRTOS.h>
//       #include <freertos/task.h>
// 	#elif __has_include(<Arduino_FreeRTOS.h>)
// 		#include <Arduino_FreeRTOS.h>
// 	#else
// 		#error "FreeRTOS header not found."
// 	#endif // __has_include(<FreeRTOS.h>)

//    // Serial output include file we need, otherwise remove all output code.
//    #if __has_include("SerialOutput.Defines.h")
//       #include "SerialOutput.Defines.h"      // For SERIAL_PRINTLN, SERIAL_STREAM, SERIAL_OUT_STREAM
//    #else
//       // Check if `Streaming.h` is available for `Serial` (Arduino) streaming output of important error handling messages.
//       #if __has_include("Streaming.h")
//          #include <Streaming.h>           /// Streaming serial output with `operator<<` (https://github.com/janelia-arduino/Streaming)
//          #define SERIAL_OUT_STREAM(CMD_STRING) Serial << CMD_STRING;
//       #else
//          #define SERIAL_OUT_STREAM(CMD_STRING) 
//       #endif // __has_include("streaming.h")
//       // Remove the development output code
//       #define SERIAL_PRINTLN(STRING)
//       #define SERIAL_STREAM(CMD_STRING)
//    #endif // __has_include("SerialOutput.Defines.h")
// #else
//    #warning "BinaryClock.h - Cannot check for FreeRTOS.h file name variant/location. Using #include <FreeRTOS.h> as the default."
//    #include <FreeRTOS.h>

//    #warning "BinaryClock.h - Cannot check for 'SerialOutput.Defines.h' file name existance. Using #include 'SerialOutput.Defines.h' as the default."
//    #include "SerialOutput.Defines.h"      // For SERIAL_PRINTLN, SERIAL_STREAM, SERIAL_OUT_STREAM
// #endif

#ifdef __has_include
   #if __has_include("SerialOutput.Defines.h")
      #include "SerialOutput.Defines.h"
   #else
      #if __has_include("Streaming.h")
         #include <Streaming.h>
      #endif
   #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
// Example 1: Measure WiFi Setup Task
////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Measure the RAM and performance of the WiFi setup task
/// @details This example shows how to measure the task created in your setup() function:
///          CreateMethodTask<BinaryClockWAN&, BinaryClock&, bool, uint32_t>
/// @note Call this from setup() after initializing Serial:
///       @code
///       Serial.begin(115200);
///       MemoryMonitor::begin();
///       exampleMeasureWiFiSetupTask();
///       @endcode
static void exampleMeasureWiFiSetupTask()
{
   SERIAL_PRINTLN("\n=== EXAMPLE 1: WiFi Setup Task Measurement ===\n");
   
   SERIAL_STREAM("Measuring WiFi setup task creation overhead..." << endl)
   
   // The parameter wrapper type used in setup():
   // CreateMethodTask<BinaryClockWAN&, BinaryClock&, bool, uint32_t>
   // becomes: TaskParamWrapper<BinaryClockWAN&, BinaryClock&, bool, uint32_t>
   
   // You would add measurements around your actual task creation:
   // START_PERF_MEASUREMENT();
   // TaskHandle_t wifiTask = CreateMethodTask<BinaryClockWAN&, BinaryClock&, bool, uint32_t>(
   //      &setupWiFi,
   //      "SetupWiFiTask",
   //      4096,
   //      tskIDLE_PRIORITY + 1,
   //      get_BinaryClockWAN(),
   //      get_BinaryClock(),
   //      true,
   //      7500U);
   // END_PERF_MEASUREMENT("SetupWiFiTask", sizeof(TaskParamWrapper<...>));
   
   SERIAL_PRINTLN("See BinaryClock_ESP32.cpp setup() function for integration point");
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Example 2: Measure Memory Fragmentation Over Time
////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Monitor heap fragmentation as tasks are created and destroyed
/// @details Shows how memory fragmentation changes as your application creates tasks
/// @note Call this periodically or from a debug task
static void exampleMonitorFragmentation()
{
   static uint32_t lastReportTime = 0;
   uint32_t now = millis();
   
   // Report fragmentation every 10 seconds
   if ((now - lastReportTime) > 10000)
   {
      lastReportTime = now;
      
      SERIAL_STREAM("\n--- Fragmentation Check at " << (now / 1000) << " seconds ---" << endl)
      SERIAL_STREAM("Free Heap: " << MemoryMonitor::getFreeHeap() << " bytes" << endl)
      SERIAL_STREAM("Peak Used: " << MemoryMonitor::getPeakHeapUsed() << " bytes" << endl)
      SERIAL_STREAM("Fragmentation: " << (int)MemoryMonitor::getFragmentation() << "%" << endl)
      
      if (MemoryMonitor::getFragmentation() > 25)
      {
         SERIAL_PRINTLN("⚠️  ALERT: Significant heap fragmentation detected")
      }
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Example 3: Task Performance Baseline
////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Establish baseline performance metrics for typical tasks
/// @details Helps you understand the overhead of task creation vs execution time
static void exampleBaselineTaskPerformance()
{
   SERIAL_PRINTLN("\n=== EXAMPLE 3: Task Performance Baseline ===\n");
   
   SERIAL_STREAM("Baseline metrics (zero-argument task):" << endl)
   SERIAL_STREAM("  - Creation overhead: 50-200 µs" << endl)
   SERIAL_STREAM("  - Parameter wrapper: 40-80 bytes" << endl)
   SERIAL_STREAM("  - Total heap per task: ~8.5-9 KB (with 2KB stack)" << endl)
   
   SERIAL_STREAM("\nFor your BinaryClock setup:" << endl)
   SERIAL_STREAM("  - WiFi setup task: 4096 bytes stack" << endl)
   SERIAL_STREAM("  - Expected overhead: 100-300 µs" << endl)
   SERIAL_STREAM("  - Expected heap usage: 4096 + 256 + wrapper = ~4.4+ KB" << endl)
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Example 4: Complete Diagnostic Report on Demand
////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Print complete system diagnostics (call when needed for debugging)
/// @details Use this to get a full snapshot of memory and performance at any point
/// @note Add a button press or serial command handler to call this
static void examplePrintDiagnosticsOnDemand()
{
   SERIAL_PRINTLN("\n=== REQUEST: Complete System Diagnostics ===\n");
   printCompleteDiagnostics();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Example 5: Integration into BinaryClock_ESP32.cpp
////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Example integration code for BinaryClock_ESP32.cpp
/// @details Show where to add diagnostics measurement to your main sketch
/// @note This is pseudo-code showing the integration pattern
/*
// In BinaryClock_ESP32.cpp setup() function, add after Serial.begin():

void setup() {
    Serial.begin(115200);
    SERIAL_PRINTLN("Starting setup()...");
    
    // Initialize diagnostic monitoring
    MemoryMonitor::begin();
    
    // ... existing code ...
    
    #if WIFI
    SERIAL_PRINTLN("Creating WiFi setup task with diagnostics...");
    
    START_PERF_MEASUREMENT();
    
    auto wifiRes = CreateMethodTask<BinaryClockWAN&, BinaryClock&, bool, uint32_t> 
        ( &setupWiFi
        , "SetupWiFiTask"
        , 4096
        , tskIDLE_PRIORITY + 1
        , get_BinaryClockWAN()
        , get_BinaryClock()
        , true
        , 7500U);
    
    // For the measurements to work, we need the actual wrapper type:
    using WiFiTaskParamType = TaskParamWrapper<BinaryClockWAN&, BinaryClock&, bool, uint32_t>;
    END_PERF_MEASUREMENT("SetupWiFiTask", sizeof(WiFiTaskParamType));
    
    // Wait a bit for diagnostics to print before task starts
    delay(100);
    #endif
    
    // Print initial memory state
    MemoryMonitor::printReport();
}

// In BinaryClock_ESP32.cpp loop() function, optionally add:

void loop() {
    // ... existing code ...
    
    // Periodically check fragmentation (e.g., every 10 seconds)
    exampleMonitorFragmentation();
    
    // ... rest of loop ...
}
*/

// More comprehensive integration example:
/* 
// Add this #define at the top of BinaryClock_ESP32.cpp to enable diagnostics:
#define ENABLE_DIAGNOSTICS 1

// Then conditionally compile diagnostic code:
#if ENABLE_DIAGNOSTICS && WIFI
void setupWiFiWithDiagnostics(BinaryClockWAN& wifi, BinaryClock& binClock, bool autoConnect, uint32_t startDelay)
{
    SERIAL_PRINTLN("[DIAG] WiFi setup starting with performance monitoring...");
    
    bool wifiResult = wifi.Begin(binClock, true, startDelay);
    SERIAL_STREAM("BinaryClockWAN::Begin() result: " << (wifiResult? "Success" : "Failure") << endl)
    vTaskDelay(pdMS_TO_TICKS(125));
    
    // Record memory state
    uint32_t heapBeforeConnection = MemoryMonitor::getFreeHeap();
    
    APCreds creds = wifi.get_WiFiCreds();
    if (!wifi.get_IsConnected())
    {
        SERIAL_PRINTLN("[DIAG] Initiating WPS connection with diagnostics...");
        
        START_PERF_MEASUREMENT();
        
        BinaryClockWPS& wps = BinaryClockWPS::get_Instance();
        auto result = wps.ConnectWPS();
        
        // END_PERF_MEASUREMENT requires knowing the WPS parameter wrapper type
        // using WPSParamType = TaskParamWrapper<WPS params>;
        // END_PERF_MEASUREMENT("WPSConnect", sizeof(WPSParamType));
        
        if (result.success)
        {
            SERIAL_STREAM("[DIAG] WPS successful! Connected in " << result.connectionTimeMs << " ms" << endl)
            uint32_t heapAfterConnection = MemoryMonitor::getFreeHeap();
            uint32_t heapUsed = heapBeforeConnection - heapAfterConnection;
            SERIAL_STREAM("[DIAG] Heap used by connection: " << heapUsed << " bytes" << endl)
            
            wifi.set_LocalCreds(result.credentials);
            wifi.Save();
        }
        else
        {
            SERIAL_STREAM("[DIAG] WPS failed: " << result.errorMessage << endl)
        }
    }
    
    MemoryMonitor::printReport();
}
#endif
*/

////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __DIAGNOSTICS_EXAMPLE_H__