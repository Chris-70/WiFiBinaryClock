#pragma once
#ifndef __TASKGROUPBITS_H__
#define __TASKGROUPBITS_H__

#include <stdint.h>                    /// Integer types: uint8_t; uint16_t; etc.
#include <stddef.h>                    /// Macros & defines: size_t, NULL, etc.

#include <freertos/FreeRTOS.h>         /// For FreeRTOS types and functions.
#include <freertos/task.h>             /// For FreeRTOS Task functions and types.
#include <freertos/event_groups.h>     /// For FreeRTOS EventGroup functions and types.

// FreeRTOS Task Event Group Bit definitions for Binary Clock tasks
#define SPLASH_COMPLETE_BIT       5
#define SPLASH_COMPLETE_MASK      (1 << SPLASH_COMPLETE_BIT)

// The TICK_TYPE_WIDTH_32_BITS should be defined, if not we mighth be missing an include file.
// Just define the configTICK_TYPE_WIDTH_IN_BITS to be 32, assume we are using an ESP32.
// This is just to validate the NTP Event bits don't exceed the maximum bit position.
#ifndef TICK_TYPE_WIDTH_32_BITS
#define TICK_TYPE_WIDTH_32_BITS        32
#define configTICK_TYPE_WIDTH_IN_BITS  TICK_TYPE_WIDTH_32_BITS
#endif 

namespace BinaryClockShield
   {
   // / @brief NTP Events class for managing NTP related event bit masks.
   // / @details This class provides methods to get bit masks for various NTP events
   // /          such as completion, synchronization, and failure. It allows setting
   // /          an offset for the event bits to avoid conflicts with other EventGroup bits.  
   // /          The static `NtpDefaultOffset` property can be set to define a default offset
   // /          that will be used to set the `NtpBitOffset` for each instance of this class.
   // /          The static methods `GetGetResultBit()` and `GetResultMask()` can be used
   // /          to get the bit and mask values using the default offset without creating
   // /          an instance of this class.
   // / @author Chris-70 (2026/01)
   template<typename... Args>
   class TaskGroupBits
      {
      #define NTP_EVENT_SIZE              4     ///< The size/number of the NTP events, not including END.
      // The starting bit number for NTP events can be defined at compile time, 0 is the default.
      // The NTP_RESERVED_BIT value impacts the enum values in `TaskGroupBits`.N
      // Use the `TaskGroupBits::set_NtpBitOffset()` property to set the offset at runtime.
      // This allows user defined EventGroups / bit flags to be used without bit conflicts.
      // NOTE: The NtpBitOffset is added to NTP_RESERVED_BIT for the final bit mask value.
      #ifndef NTP_RESERVED_BIT
      #define NTP_RESERVED_BIT            0     ///< The starting bit number for NTP the events enum.
      #endif 
      // The maximum offset that can be used based on the current board FreeRTOS implementation.
      // This accounts for the value of NTP_RESERVED_BIT.
      #define MAX_OFFSET ((configTICK_TYPE_WIDTH_IN_BITS - 8 - (int)NTPEventEnd))

   public:
      /// @brief Enumeration of NTP event types used by the NTP class.
      /// @details This enumeration defines the various NTP event types used for
      ///          event bit masking. The values are offset by `NTP_RESERVED_BIT` to
      ///          set the starting value of the enum, this is normally 0.  
      ///          - The `Completed` event indicates that NTP sync initialization has completed.  
      ///          - The `Synced` event indicates that an NTP sync has been received. 
      ///          - The `SyncFailed` event indicates that the NTP sync has failed and 
      ///                 re-initialization is required.  
      ///          - The `NTPEventEnd` value indicates the end of the enum and can be used 
      ///                 to determine the number of events defined by subtracting `Reserved`.  
      ///          The only advantages of the class instance is when multiple programs/libraries
      ///          or task groups need to use this with different offset values.
      ///          The `CompletedMask`; `SyncedMask`; and `FailedMask` properties provide
      ///          easy access to the respective event masks.
      /// @author Chris-70 (2026/01)
      enum ntp_events
         {
         Reserved  = NTP_RESERVED_BIT,     ///< Reserved bit, not used.
         Completed = NTP_RESERVED_BIT + 1, ///< NTP sync initialization completed event.
         Synced    = NTP_RESERVED_BIT + 2, ///< NTP sync received event, set everytime we sync.
         Failed    = NTP_RESERVED_BIT + 3, ///< NTP sync failed event, must re-initialize.
         NTPEventEnd                       ///< Last `TaskGroupBits` value; subtract `Reserved` to get the size.
         };

      /// @brief The number of events in the enum including `Reserved` but excluding the end marker `NTPEventEnd`.
      static const size_t NTPEventsCount = (size_t)(TaskGroupBits::NTPEventEnd)-(size_t)(TaskGroupBits::Reserved);
      // Validate that the enum event size matches the defined size to catch any modification errors.
      static_assert(NTPEventsCount == NTP_EVENT_SIZE, "NTPEventsCount does not match NTP_EVENT_SIZE");
      static_assert((MAX_OFFSET >= 0), "NTP_RESERVED_BIT is too large for current configTICK_TYPE_WIDTH_IN_BITS");

      /// @brief Default constructor for TaskGroupBits class.
      /// @details This constructor initializes the TaskGroupBits instance
      ///          with the default NTP bit offset.
      TaskGroupBits() : ntpBitOffset(get_NtpDefaultOffset()) { }

      /// @brief Constructor for TaskGroupBits class with custom bit offset.
      /// @details This constructor initializes the TaskGroupBits instance
      ///          with a specified NTP bit offset.
      TaskGroupBits(size_t bitOffset) : ntpBitOffset(bitOffset) { }

      /// @brief Static property to set/get the default NTP event bit offset.
      /// @details This static property allows setting a default offset for NTP event bits.
      ///          The default value is used by the default constructor to set the initial offset value.
      ///          It is also used by the static methods `GetResultBit()` and `GetResultMask()`.
      /// Note The enum value of `NTPEventEnd` plus `NtpDefaultOffset` must NOT exceed the value of
      ///      (`configTICK_TYPE_WIDTH_IN_BITS` - 8). e.g. 8; 24; or 56 for 16; 32; or 64 bit widths.
      /// @param value The default offset value to set.
      /// @see get_NtpDefaultOffset()
      /// @see GetResultBit()
      /// @see GetResultMask()
      static void set_NtpDefaultOffset(size_t value)
         {
         ntpDefaultOffset = (value > MAX_OFFSET ? MAX_OFFSET : value);
         }
      /// @copydoc set_NtpDefaultOffset()
      /// @return The current default NTP event bit offset.
      /// @see set_NtpDefaultOffset()
      static size_t get_NtpDefaultOffset()
         {
         return ntpDefaultOffset;
         }

      /// @brief Static method to get the result bit for a given NTP event using the default offset.
      /// @param ntpEvent The NTP event type.
      /// @return The bit number for the specified NTP event with the `NtpDefaultOffset`.
      /// @see GetResultMask()
      static size_t GetResultBit(enum ntp_events  ntpEvent)
         {
         return (ntpEvent + ntpDefaultOffset);
         }

      /// @brief Static method to get the result mask for a given NTP event using the default offset.
      /// @param ntpEvent The NTP event type.
      /// @return The bit mask for the specified NTP event with the `NtpDefaultOffset`.
      /// @see GetResultBit()
      static size_t GetResultMask(enum ntp_events  ntpEvent)
         {
         return (1 << GetResultBit(ntpEvent));
         }

      /// @brief Property to set/get the NTP event bit offset for this instance.
      /// @details This property allows setting a custom offset for NTP event bits
      ///          for this instance of the class.
      /// @param value The offset value to set.
      /// Note The enum value of `NTPEventEnd` plus `NtpBitOffset` must NOT exceed the value of
      ///      (`configTICK_TYPE_WIDTH_IN_BITS` - 8). i.e. 8; 24; or 56 for 16; 32; or 64 bit widths.
      /// @see get_NtpBitOffset()
      void set_NtpBitOffset(size_t value)
         {
         ntpBitOffset = (value > MAX_OFFSET ? MAX_OFFSET : value);
         }
      /// @copydoc set_NtpBitOffset()
      /// @return The current offset of the NTP event bits for this instance.
      /// @see set_NtpBitOffset()
      size_t get_NtpBitOffset()
         {
         return ntpBitOffset;
         }

      /// @brief Method to get the bit number for the given `ntpEvent`
      /// @param ntpEvent The ntp_events enum to get the bit number.
      /// @returns the bit bumber corresponding to the given `ntpEvent`.
      /// @see GetMask()
      size_t GetBit(enum ntp_events  ntpEvent)
         {
         return (ntpEvent + ntpBitOffset);
         }

      /// @brief Method to get the bit mask for the given `ntpEvent`
      /// @param ntpEvent The ntp_events enum to get the bit mask.
      /// @returns the bit mask corresponding to the given `ntpEvent`.
      /// @see GetBit()
      size_t GetMask(enum ntp_events  ntpEvent)
         {
         return (1 << GetBit(ntpEvent));
         }

      /// @brief Property - Read only: The bit mask for the `Completed` event.
      /// @returns The bit mask for the `Completed` event.
      size_t get_CompletedMask()
         {
         return (GetMask(Completed));
         }
      /// @brief Property - Read only: The bit mask for the `Synced` event.
      /// @returns The bit mask for the `Synced` event.
      size_t get_SyncedMask()
         {
         return (GetMask(Synced));
         }
      /// @brief Property - Read only: The bit mask for the `Failed` event.
      /// @returns The bit mask for the `Failed` event.
      size_t get_FailedMask()
         {
         return (GetMask(Failed));
         }

   private:
      static size_t ntpDefaultOffset;  ///< Static default NTP event bit offset.
      size_t ntpBitOffset = 0;         ///< Instance NTP event bit offset.
      }; // class TaskGroupBits
 
   } // namespace BinaryClockShield
#endif // __TASKGROUPBITS_H__