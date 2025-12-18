#pragma once
#ifndef __DIAGNOSTICS_H__
#define __DIAGNOSTICS_H__

/// @file Diagnostics.h - Memory and Performance Monitoring for ESP32 TaskWrapper
/// @brief Comprehensive diagnostic tools for measuring RAM usage and performance metrics
///        of FreeRTOS tasks created with TaskWrapper.h
/// @details This file provides utilities to:
///          - Track real-time heap memory usage and fragmentation
///          - Measure task creation overhead
///          - Measure task execution time
///          - Monitor peak memory usage
///          - Generate detailed diagnostic reports
/// @author Chris-70 (2025/12)
/// @note For use with ESP32 and FreeRTOS task creation via TaskWrapper.h

#include <Arduino.h>
#include <cstdint>

#if __cplusplus < 201402L
   #error "Diagnostics.h requires at least the C++14 language standard"
#endif

#ifdef __has_include
   #if __has_include("SerialOutput.Defines.h")
      #include "SerialOutput.Defines.h"
   #else
      #if __has_include("Streaming.h")
         #include <Streaming.h>
         #define SERIAL_OUT_STREAM(CMD_STRING) Serial << CMD_STRING;
      #else
         #define SERIAL_OUT_STREAM(CMD_STRING)
      #endif
      #define SERIAL_PRINTLN(STRING)
      #define SERIAL_STREAM(CMD_STRING)
   #endif
#else
   #include "SerialOutput.Defines.h"
   #define SERIAL_OUT_STREAM(CMD_STRING) SERIAL_OUT_STREAM(CMD_STRING)
   #define SERIAL_PRINTLN(STRING) SERIAL_PRINTLN(STRING)
   #define SERIAL_STREAM(CMD_STRING) SERIAL_STREAM(CMD_STRING)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
// Memory Monitoring
////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Helper class to monitor heap memory usage on ESP32
/// @details Provides real-time tracking of:
///          - Current free heap
///          - Heap used since initialization
///          - Peak heap usage
///          - Heap fragmentation percentage
///          - Maximum allocatable block size
/// @author Chris-70 (2025/12)
class MemoryMonitor
{
private:
   static uint32_t peakHeapUsed;
   static uint32_t peakFragmentation;
   static uint32_t initialFreeHeap;
   static uint32_t minimumFreeHeap;

public:
   /// @brief Initialize memory monitoring
   /// @details Should be called once at startup to establish baseline
   static void begin()
   {
      initialFreeHeap = ESP.getFreeHeap();
      minimumFreeHeap = initialFreeHeap;
      peakHeapUsed = 0;
      peakFragmentation = 0;
      
      SERIAL_STREAM("====== MEMORY MONITOR INITIALIZED ======" << endl)
      SERIAL_STREAM("Initial Free Heap: " << initialFreeHeap << " bytes (" 
         << (initialFreeHeap / 1024) << " KB)" << endl)
      SERIAL_STREAM("Total Heap: " << ESP.getHeapSize() << " bytes (" 
         << (ESP.getHeapSize() / 1024) << " KB)" << endl)
      SERIAL_STREAM("========================================" << endl << endl)
   }

   /// @brief Get current free heap in bytes
   /// @return Free heap size in bytes
   static uint32_t getFreeHeap()
   {
      uint32_t current = ESP.getFreeHeap();
      if (current < minimumFreeHeap)
      {
         minimumFreeHeap = current;
      }
      return current;
   }

   /// @brief Get heap used since initialization
   /// @return Bytes used since begin() was called
   static uint32_t getHeapUsed()
   {
      uint32_t used = initialFreeHeap - getFreeHeap();
      if (used > peakHeapUsed)
      {
         peakHeapUsed = used;
      }
      return used;
   }

   /// @brief Get peak heap usage
   /// @return Maximum bytes used at any point since initialization
   static uint32_t getPeakHeapUsed()
   {
      return peakHeapUsed;
   }

   /// @brief Get minimum free heap reached
   /// @return Minimum free heap at any point since initialization
   static uint32_t getMinimumFreeHeap()
   {
      return minimumFreeHeap;
   }

   /// @brief Get heap fragmentation percentage
   /// @details Calculates percentage of heap that is fragmented by comparing
   ///          the maximum allocatable block to total free heap
   /// @return Fragmentation as percentage (0-100)
   static uint8_t getFragmentation()
   {
      uint32_t freeHeap = getFreeHeap();
      uint32_t maxAllocHeap = ESP.getMaxAllocHeap();
      
      if (freeHeap == 0)
         return 100;
         
      uint8_t frag = (uint8_t)(100 - (maxAllocHeap * 100 / freeHeap));
      if (frag > peakFragmentation)
      {
         peakFragmentation = frag;
      }
      return frag;
   }

   /// @brief Get peak fragmentation percentage
   /// @return Maximum fragmentation reached since initialization
   static uint8_t getPeakFragmentation()
   {
      return peakFragmentation;
   }

   /// @brief Get maximum allocatable heap block size
   /// @return Size in bytes of largest contiguous heap block
   static uint32_t getMaxAllocableBlock()
   {
      return ESP.getMaxAllocHeap();
   }

   /// @brief Print detailed heap diagnostic report
   static void printReport()
   {
      uint32_t currentFree = getFreeHeap();
      uint32_t totalHeap = ESP.getHeapSize();
      uint32_t used = getHeapUsed();
      uint32_t maxAlloc = getMaxAllocableBlock();
      uint8_t frag = getFragmentation();

      SERIAL_STREAM(endl << "========== HEAP DIAGNOSTIC REPORT ==========" << endl)
      SERIAL_STREAM("Total Heap:        " << totalHeap << " bytes (" << (totalHeap / 1024) << " KB)" << endl)
      SERIAL_STREAM("Current Free:      " << currentFree << " bytes (" << (currentFree / 1024) << " KB)" << endl)
      SERIAL_STREAM("Current Used:      " << used << " bytes (" << (used / 1024) << " KB)" << endl)
      SERIAL_STREAM("Peak Used:         " << getPeakHeapUsed() << " bytes (" << (getPeakHeapUsed() / 1024) << " KB)" << endl)
      SERIAL_STREAM("Minimum Free:      " << getMinimumFreeHeap() << " bytes (" << (getMinimumFreeHeap() / 1024) << " KB)" << endl)
      SERIAL_STREAM("Max Allocable:     " << maxAlloc << " bytes (" << (maxAlloc / 1024) << " KB)" << endl)
      SERIAL_STREAM("Fragmentation:     " << (int)frag << "% (Peak: " << (int)getPeakFragmentation() << "%)" << endl)
      
      // Warn if fragmentation is high
      if (frag > 20)
      {
         SERIAL_STREAM("⚠️  WARNING: High heap fragmentation detected!" << endl)
      }
      
      // Warn if free heap is low
      if (currentFree < (totalHeap / 4))
      {
         SERIAL_STREAM("⚠️  WARNING: Free heap below 25% threshold!" << endl)
      }
      
      SERIAL_STREAM("============================================" << endl << endl)
   }
};

// Static member initialization
uint32_t MemoryMonitor::peakHeapUsed = 0;
uint32_t MemoryMonitor::peakFragmentation = 0;
uint32_t MemoryMonitor::initialFreeHeap = 0;
uint32_t MemoryMonitor::minimumFreeHeap = 0;

////////////////////////////////////////////////////////////////////////////////////////////////
// Performance Monitoring
////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Structure to hold performance metrics for a task
/// @author Chris-70 (2025/12)
struct TaskPerformanceMetrics
{
   uint32_t creationTimeUs;         ///< Microseconds to create the task
   uint32_t executionTimeUs;        ///< Microseconds for task to execute
   uint32_t scheduleDelayUs;        ///< Microseconds from creation to first run
   uint32_t paramWrapperSizeBytes;  ///< Size of parameter wrapper on heap
   uint32_t heapUsedBytes;          ///< Total heap used by task creation
   const char* taskName;            ///< Name of the task measured
};

/// @brief Helper class to monitor TaskWrapper performance metrics
/// @details Provides utilities to:
///          - Track task creation time
///          - Measure task execution duration
///          - Calculate overhead percentages
///          - Generate performance reports
/// @author Chris-70 (2025/12)
class PerformanceMonitor
{
private:
   static TaskPerformanceMetrics lastMetrics;
   static uint32_t measurementStartTime;
   static uint32_t heapBeforeMeasurement;

public:
   /// @brief Start a task creation measurement
   /// @details Call this before CreateInstanceTask() to measure creation time
   static void startCreationMeasurement()
   {
      heapBeforeMeasurement = MemoryMonitor::getFreeHeap();
      measurementStartTime = micros();
   }

   /// @brief End task creation measurement
   /// @param taskName Name of the task that was created
   /// @param paramWrapperSize Size of the TaskParamWrapper used
   static void endCreationMeasurement(const char* taskName, uint32_t paramWrapperSize)
   {
      uint32_t creationTime = micros() - measurementStartTime;
      uint32_t heapAfter = MemoryMonitor::getFreeHeap();
      uint32_t heapUsed = heapBeforeMeasurement - heapAfter;

      lastMetrics.creationTimeUs = creationTime;
      lastMetrics.paramWrapperSizeBytes = paramWrapperSize;
      lastMetrics.heapUsedBytes = heapUsed;
      lastMetrics.taskName = taskName;

      SERIAL_STREAM("[PERF] Task '" << taskName << "' creation: " << creationTime 
         << " µs (Heap: " << heapUsed << " bytes)" << endl)
   }

   /// @brief Record task execution time
   /// @param taskName Name of the task
   /// @param executionMicros Execution time in microseconds
   static void recordExecutionTime(const char* taskName, uint32_t executionMicros)
   {
      lastMetrics.executionTimeUs = executionMicros;
      lastMetrics.taskName = taskName;

      SERIAL_STREAM("[PERF] Task '" << taskName << "' execution: " << executionMicros 
         << " µs (" << (executionMicros / 1000.0f) << " ms)" << endl)
   }

   /// @brief Get last recorded metrics
   /// @return TaskPerformanceMetrics structure with last measurement
   static const TaskPerformanceMetrics& getLastMetrics()
   {
      return lastMetrics;
   }

   /// @brief Calculate overhead as percentage of total time
   /// @details Overhead = (Creation Time / (Creation + Execution)) * 100
   /// @return Overhead percentage
   static uint8_t getOverheadPercentage()
   {
      uint32_t total = lastMetrics.creationTimeUs + lastMetrics.executionTimeUs;
      if (total == 0)
         return 0;
      return (uint8_t)((lastMetrics.creationTimeUs * 100) / total);
   }

   /// @brief Print detailed performance report
   static void printReport()
   {
      uint32_t totalTime = lastMetrics.creationTimeUs + lastMetrics.executionTimeUs;
      uint8_t overhead = getOverheadPercentage();

      SERIAL_STREAM(endl << "====== PERFORMANCE METRICS REPORT ======" << endl)
      SERIAL_STREAM("Task Name:              " << lastMetrics.taskName << endl)
      SERIAL_STREAM("Task Creation Time:     " << lastMetrics.creationTimeUs << " µs" << endl)
      SERIAL_STREAM("Task Execution Time:    " << lastMetrics.executionTimeUs << " µs (" 
         << (lastMetrics.executionTimeUs / 1000.0f) << " ms)" << endl)
      SERIAL_STREAM("Total Time:             " << totalTime << " µs (" 
         << (totalTime / 1000.0f) << " ms)" << endl)
      SERIAL_STREAM("Parameter Wrapper:      " << lastMetrics.paramWrapperSizeBytes << " bytes" << endl)
      SERIAL_STREAM("Heap Used by Task:      " << lastMetrics.heapUsedBytes << " bytes" << endl)
      SERIAL_STREAM("Creation Overhead:      " << (int)overhead << "%" << endl)
      
      if (overhead > 50)
      {
         SERIAL_STREAM("⚠️  WARNING: High creation overhead (>50%)" << endl)
      }
      
      SERIAL_STREAM("========================================" << endl << endl)
   }
};

// Static member initialization
TaskPerformanceMetrics PerformanceMonitor::lastMetrics = {};
uint32_t PerformanceMonitor::measurementStartTime = 0;
uint32_t PerformanceMonitor::heapBeforeMeasurement = 0;

////////////////////////////////////////////////////////////////////////////////////////////////
// Integrated Diagnostics Report
////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Generate comprehensive diagnostic report
/// @details Combines memory and performance reports for complete system diagnostics
static void printCompleteDiagnostics()
{
   SERIAL_STREAM(endl << endl)
   SERIAL_STREAM("╔════════════════════════════════════════════════════════════════╗" << endl)
   SERIAL_STREAM("║           COMPLETE SYSTEM DIAGNOSTICS REPORT                   ║" << endl)
   SERIAL_STREAM("╚════════════════════════════════════════════════════════════════╝" << endl << endl)
   
   MemoryMonitor::printReport();
   PerformanceMonitor::printReport();
   
   SERIAL_STREAM("╔════════════════════════════════════════════════════════════════╗" << endl)
   SERIAL_STREAM("║                    END DIAGNOSTICS REPORT                      ║" << endl)
   SERIAL_STREAM("╚════════════════════════════════════════════════════════════════╝" << endl << endl)
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Utility Macros for Measurement
////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Macro to measure task parameter wrapper size at compile time
/// @param TaskParamWrapperType The full template type of the TaskParamWrapper
#define MEASURE_PARAM_WRAPPER_SIZE(TaskParamWrapperType) \
   do { \
      SERIAL_STREAM("TaskParamWrapper Size: " << sizeof(TaskParamWrapperType) << " bytes" << endl) \
   } while(0)

/// @brief Macro to start measuring task creation and heap usage
#define START_PERF_MEASUREMENT() \
   do { \
      PerformanceMonitor::startCreationMeasurement(); \
   } while(0)

/// @brief Macro to end task creation measurement
/// @param TaskName String name of the task
/// @param ParamWrapperSize Size in bytes of the parameter wrapper
#define END_PERF_MEASUREMENT(TaskName, ParamWrapperSize) \
   do { \
      PerformanceMonitor::endCreationMeasurement(TaskName, ParamWrapperSize); \
   } while(0)

/// @brief Macro to measure execution time within a task
/// @param TaskName String name of the task
/// @param ExecutionCode Code block to measure
#define MEASURE_EXECUTION_TIME(TaskName, ExecutionCode) \
   do { \
      uint32_t __startTime = micros(); \
      { ExecutionCode } \
      uint32_t __executionTime = micros() - __startTime; \
      PerformanceMonitor::recordExecutionTime(TaskName, __executionTime); \
   } while(0)

////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __DIAGNOSTICS_H__