/// @file TaskGroupBits.h
/// @brief This file contains the declaration of the `TaskGroupBase` and `TaskGroupBits` classes for managing task group event bits.
/// @details The `TaskGroupBase` class is an abstract base class that defines the common, non-template, base interface class.  
///          This parent class provides a common parent for all child template classes, `TaskGroupBits` instances, to allow
///          them to be assigned and managed together in a single event group without the need to know the details of each 
///          component's event bit definitions.
///
///          The `TaskGroupBits` template class provides a structured way to manage task group event bits directly as enums.  
///          The template class takes an enum class as a template parameter and provides properties and methods to work
///          with the `EventBits_t` type using just the enums. The class allows the enums to be defined with any starting bit offset.
///          This is particularly useful for tasks that need to handle multiple events without conflicts in the event group bits.
///          
///          The goal of the `TaskGroupBits` template class is to facilitate combining the enum definitions of several components
///          together in to a single event group without bit conflicts. The architecture of the class is to allow for individual 
///          components to define their own event bits as enums which are unique to the component yet be managed together with 
///          other components in a single event group. The managing class does not need to know anything about the component's
///          event bit definitions, it just needs to know the offset for the component's bits and the enum values. Alternatively
///          the managing class could be aware of the event enum classes and instantiate this class for each component to manage 
///          the bits for each component separately. In either case the managing class must set the offset for each component's 
///          event bits to ensure there are no conflicts in the event group bits.  
/// 
///          The `TaskGroupBase` parent class provides a common interface for all `TaskGroupBits` instances, allowing them to be
///          assigned and managed together in a single event group without needing to know the details of each component's event 
///          bit definitions. 
///          
///          The event enum class<T> has the following restrictions:  
///          - It must be an `enum class` and can inherit from `uint8_t` as we are limited to a maximum of 24 (possibly 8 or 56) events.
///            - e.g. `enum class ntp_events : uint8_t { Reserved = 0, Completed, Synced, Failed, EventEnd };`
///          - The first value of the enum must be `Reserved` and is usually set to 0.
///          - The last enum must be `EventEnd` and is an end marker only, not a valid event.
///          - The enum values between `Reserved` and `EventEnd` must be sequential.
///            - i.e. There must be (EventEnd - Reserved) valid events defined in the enum not including `EventEnd`.
/// 
/// @par Author's notes:
///          The motivation behind this implementation was the need to manage multiple components' event bits together in a single FreeRTOS 
///          event group without conflicts and being able to use the same events from one component freely with other components.  
/// @par
///          I wanted each component to define its own events and be able to manage the events from other components together in
///          one task group. The event values had to be reusable such that the events could be combined with the events from any other
///          components in an arbitrary way without needing to change the event definitions or create custom wrapper methods for each component.  
/// @par
///          I chose to use `enum` classes as the event definitions as it provides for creating meaningful names, sequential numbering, 
///          easy changes to the events (e.g. adding or removing events) without creating any dependencies between components. 
///          I did not want to try and manage the event bit masks directly with all the components included in a task group, this would make 
///          future maintenance and component reuse much more difficult.
/// @par
///          The final solution was to create a template class that takes an `enum class` as a template parameter and provides properties and 
///          methods to work with the event bits directly as enums.
///          - The class allows the enums to be defined with any starting bit offset, this allows for the events from different components to be 
///          combined together in a single event group without conflicts.
///          - The `TaskGroupBase` parent class provides a common interface for all `TaskGroupBits` instances, allowing them to be assigned and 
///          managed together in a single event group without needing to know the details of each component's event bit definitions.
/// @par
///          This might be like using a hammer to kill a fly but I wanted a solution that would be flexible and reusable for any component's 
///          event bits and allow for easy maintenance and future changes without needing to change the event definitions or create custom 
///          wrapper methods for each component. I also wanted a solution that I could build and test that could be reused knowing the
///          solution was functionally correct. I also wanted a solution that would allow me to create events for a component easily just by
///          defining an enum class for the events. Maintenance on the events would be simple and the changes wouldn't impact other code.
/// 
/// @par Design:
///          The design of the `TaskGroupBits` class is to provide a structured way to manage task group event bits directly as enums.  
///          The class is a template that takes an `enum class` as a template parameter, and provides properties and methods to work 
///          with the event bits directly as enums. The resulting enum values would be offset from the enum values by the `BitOffset` 
///          that can be defined by the constructor call or afterwards by setting the `StartBitValue` property to allow for the events from 
///          different components to be combined together in a single event group without conflicts.
/// @par
///          The template class inherits from an abstract class `TaskGroupBase` which defines a common interface for the template class
///          to allow for multiple instances of the template class with different enum classes to be grouped, assigned, passed, etc. 
///          generically without the need to know the details of each component's event bit definitions. 
/// 
/// @par Remarks:
///          This implementation requires at least the C++11 language standard due to the use of `enum class` and template features.
///          The implementation also assumes that the number of events defined in the enum class does not exceed the maximum number of 
///          bits available in the FreeRTOS event group (e.g. 24 bits as 8 bits are reserved by FreeRTOS, or 8, 56 bits for 16, 64 bit integers).
///          Care must be taken to set the bit offset for each component's event bits to ensure there are no conflicts in the event 
///          group bits when managing multiple components together in a single event group. The `get_ReservedBit()` and `get_EventEndBit()`
///          property methods can be used to help with calculating the offsets when combining events.
/// @author Chris-70 (2026/03)

#pragma once
#ifndef __TASKGROUPBITS_H__
#define __TASKGROUPBITS_H__

#include <stdint.h>                    /// Integer types: uint8_t; uint16_t; etc.
#include <stddef.h>                    /// Macros & defines: size_t, NULL, etc.

#include <cassert>                     /// assert
#include <functional>                  /// std::function
#include <type_traits>                 /// std::is_enum_v, std::is_same_v
#include <vector>                      /// std::vector

#include <freertos/FreeRTOS.h>         /// For FreeRTOS types and functions.
#include <freertos/task.h>             /// For FreeRTOS Task functions and types.
#include <freertos/event_groups.h>     /// For FreeRTOS EventGroup functions and types.

// FreeRTOS Task Event Group Bit definitions for Binary Clock tasks
#define SPLASH_COMPLETE_BIT       5
#define SPLASH_COMPLETE_MASK      (1 << SPLASH_COMPLETE_BIT)

// The TICK_TYPE_WIDTH_BITS should be defined, if not we mighth be missing an include file.
// Just define the configTICK_TYPE_WIDTH_IN_BITS to be 32, assume we are using an ESP32.
// This is just to validate the NTP Event bits don't exceed the maximum bit position.
#ifndef TICK_TYPE_WIDTH_BITS
#define TICK_TYPE_WIDTH_BITS           32
#define configTICK_TYPE_WIDTH_IN_BITS  TICK_TYPE_WIDTH_BITS
#endif 

#ifndef GROUP_EVENT_RESERVED_BITS
#define GROUP_EVENT_RESERVED_BITS      8     ///< The number of bits reserved by FreeRTOS for its own use (e.g. 8 bits for event groups).
#endif

#define SECONDS_MS                  1000UL         ///< Number of milliseconds in 1 second.
#define MINUTES_MS                  60000UL        ///< Number of milliseconds in 1 minute.
#define HOURS_MS                    3600000UL      ///< Number of milliseconds in 1 hour.
#define TICKS_TO_MS(ticks) ((ticks) * portTICK_PERIOD_MS)   ///< Convert FreeRTOS ticks to milliseconds.
#define MS_TO_TICKS(ms) ((ms) / portTICK_PERIOD_MS)         ///< Convert milliseconds to FreeRTOS ticks.

namespace BinaryClockShield
   {
   #pragma region TaskGroupBase Class

   /// @brief Abstract base class for TaskGroupBits template class, defines the common, non-template, base interface class.
   /// @details This parent class provides a common parent for all child template classes, `TaskGroupBits<T>` instances, to 
   ///          allow them to be assigned and managed together in a single event group without the need to know the details 
   ///          of each component's event bit definitions.   
   ///          This parent class provides the base functionality without knowing the details of the enum class<T> definitions,
   ///          allowing for multiple instances of the template class with different enum classes to be used together in a single 
   ///          event group without conflicts in the event bits. Management of the event group using an array of `TaskGroupBase*` 
   ///          pointers where the methods to handle the different event types can be done without needing to know the details of
   ///          each component's event bit definitions by calling each `ProcessEvents()` method in a loop.
   /// 
   ///          The `TaskGroupBits<T>` template class inherits from this base class and provides the functionality to manage the 
   ///          event bits directly as enums. Individual bit masks for the different enum values can be obtained using the 
   ///          `GetBitMask()` method that takes in to account the `StartBitValue` property for this instance.
   /// @example
   /// ```cpp
   /// // Example of using `TaskGroupBits<T>` with multiple enum classes in a single event group.
   /// // . . . => Indicates the code can appear anywhere in the program, not necessarily in the location/order shown.
   /// // Define two different enum classes for different components in different headers as well.
   /// enum class ComponentAEvents : uint8_t { Reserved = 0, Event1, Event2, Event3, EventEnd };
   /// // . . .  
   /// enum class ComponentBEvents : uint8_t { Reserved = 0, EventX, EventY, EventEnd };
   /// // . . .  
   /// // Create TaskGroupBits instances for each component's events.
   /// TaskGroupBits<ComponentAEvents> componentAEventBits;
   /// // Set the method to handle the events for this instance
   /// componentAEventBits.set_EventsMethod(/* The event handler method for ComponentAEvents */); 
   /// // ...  
   /// TaskGroupBits<ComponentBEvents> componentBEventBits;
   /// // Set the method to handle the events for this instance
   /// componentBEventBits.set_EventsMethod(/* The event handler method for ComponentBEvents */);
   /// // . . .
   /// std::vector<TaskGroupBase*> eventGroups = { &componentAEventBits, &componentBEventBits };
   /// EventGroupHandle_t eventGroupHandle = xEventGroupCreate();
   /// // . . .  
   /// // Set the offsets for each component's event bits to avoid conflicts in the event group bits.
   /// void PrepareTaskGroups(std::vector<TaskGroupBase*>& groups)
   ///    {
   ///    size_t startBit = 0;
   ///    for (TaskGroupBase* group : groups)
   ///       {
   ///       group->set_StartBitValue(startBit); // Set the starting bit number for the enum class<T> instance to avoid conflicts in the event bits
   ///       group->set_EventGroup(eventGroupHandle); // Set the event group handle for this instance
   ///       startBit += group->get_EventsCount();
   ///       }
   ///    }
   /// // . . .  
   /// static EventBits_t getAllBitMasks(std::vector<TaskGroupBase*>& groups)
   ///    {
   ///    EventBits_t allMasks = 0;
   ///    for (TaskGroupBase* group : groups)
   ///       {
   ///       allMasks |= group->getAllBitMasks();
   ///       }
   ///    return allMasks;
   ///    }
   /// // . . .  
   /// // In this example task loop, we can check for events from all components together without needing to know the details of each component's event bit definitions.
   /// void TaskLoop()
   ///    {
   ///    static EventBits_t allEventsBitMask = getAllBitMasks(eventGroups);
   ///    while(true)
   ///       {
   ///       EventBits_t bits = xEventGroupWaitBits(eventGroupHandle, allEventsBitMask, pdTRUE, pdFALSE, portMAX_DELAY);
   ///       if (bits != 0)
   ///          {
   ///          for (TaskGroupBase* group : eventGroups)
   ///             {
   ///             group->ProcessEvents(bits); // Will only process enum class<T> events from the `bits` given.
   ///             }
   ///          }
   ///       else
   ///          { continue; }
   ///       }
   ///    }
   /// // . . .
   /// ```
   /// @see TaskGroupBits<T>
   class TaskGroupBase
      {
   protected:
      /// @brief Protected constructors for TaskGroupBase class, can only be called by derived classes.
      TaskGroupBase()
         {
         set_StartBitValue(0); // Default to 0 if not set by derived classes
         }

      /// @brief Constructor for TaskGroupBase class with event group handle parameter.
      /// @param eventGroupHandle The event group handle to associate with this instance.
      explicit TaskGroupBase(EventGroupHandle_t eventGroupHandle) : eventGroup(eventGroupHandle)
         {
         set_StartBitValue(0); // Default to 0 if not set by derived classes
         }

      /// @brief Constructor for TaskGroupBase class with bit offset parameter.
      /// @param startBitValue The bit offset value to associate with this instance.
      explicit TaskGroupBase(uint8_t startBitValue) 
         {
         set_StartBitValue(startBitValue);
         }

      /// @brief Constructor for TaskGroupBase class with event group handle and bit offset parameters.
      /// @param eventGroupHandle The event group handle to associate with this instance.
      /// @param startBitValue The bit offset value to associate with this instance.
      explicit TaskGroupBase(EventGroupHandle_t eventGroupHandle, uint8_t startBitValue) : eventGroup(eventGroupHandle)
         {
         set_StartBitValue(startBitValue);
         }

   public:
      /// @brief Virtual destructor for TaskGroupBase class.
      virtual ~TaskGroupBase() = default;

      #pragma region Properties
      /// @brief Property (R/W): StartBitValue - the bit value for the start of the events 
      ///        (i.e. T::Reserved) for this instance.
      /// @details This property allows setting the start bit value for event bits (e.g. T::Reserved) 
      ///          for this instance of the class. An offset is added to the enum values to make the
      ///          start event bit equal to `value`. This changes the resulting enum class<T> values
      ///          and bit masks for this instance. Enum class<T> events where T::Reserved > 0
      ///          will have the offset applied to make the `T::Reserved` bit equal to `value`. 
      ///          The enum class<T> event values will be redefined for this instance and will
      ///          remain sequential from `T::Reserved` to `T::EventEnd`.
      /// 
      ///          This allows for multiple instances of the class with different enum classes to be 
      ///          used together in a single event group without conflicts in the event bits.
      /// @remarks The total number of events for one `EventGroup` is limited by the number of bits 
      ///          available in the event group (e.g. 32 bit integers can have 24 bits as 8 bits are 
      ///          reserved by FreeRTOS, 8 / 56 bits for 16 / 64 bit integers).  
      ///          In all cases uint8_t is large enough for all event bit values.
      /// @param value The bit value to set for the start of the enum class<T> (i.e. T::Reserved)
      /// @note The enum value of `EventsCount` plus `StartBitValue` must NOT exceed the value of
      ///       (`configTICK_TYPE_WIDTH_IN_BITS` - 8). i.e. 8; 24; or 56 for 16; 32; or 64 bit widths.
      /// @see get_StartBitValue()
      virtual void set_StartBitValue(uint8_t value)
         {
         bitOffset = ((value + get_EventsCount()) > get_MaxStartBit() ? (int8_t)get_MaxStartBit()
                   : ((int8_t)value - (int8_t)get_ReservedBit()));
         }
      /// @copydoc set_StartBitValue()
      /// @return The current value of the start event bit (i.e. T::Reserved) for this instance.
      /// @see set_StartBitValue()
      virtual uint8_t get_StartBitValue() const
         { return (uint8_t)(bitOffset + get_ReservedBit()); }

      /// @brief Property (RO): BitOffset - the bit offset for the enum class<T> event bits for this instance.
      /// @details This method returns the bit offset for the enum class<T> event bits for this instance. 
      ///          The offset is the value that is added to the enum values to get the actual bit number for this instance.
      ///          The offset is calculated based on the `StartBitValue` and the `T::Reserved` value of the enum class<T>. 
      /// @return The bit offset for the enum class<T> event bits for this instance.
      virtual int8_t get_BitOffset() const
         { return bitOffset; }

      /// @brief  Property (RO): EventsCount - the number of events defined in the enum class <T>.
      /// @return The total number of events defined in the enum class <T> (i.e. T::EventEnd - T::Reserved).
      virtual uint8_t get_EventsCount() const = 0;

      /// @brief Property (RO): MaxStartBit - the maximum allowed bit number for this instance.
      /// @details This method returns the maximum allowed bit number for this instance, which is calculated 
      ///          based on the number of events defined in the enum class <T> and the maximum number of bits 
      ///          available in the event group (i.e. `configTICK_TYPE_WIDTH_IN_BITS` minus the number of events).
      /// @return The maximum allowed starting bit number for this instance based on the enum class <T>.
      virtual uint8_t get_MaxStartBit() const = 0;

      /// @brief Property (RO): ReservedBit - the reserved bit number (T::Reserved) for this instance.
      /// @details This get_ method returns the start/reserved bit number for this instance, which is 
      ///          the starting bit for the events defined in the enum class <T>. i.e. `T::Reserved`.
      /// @return The T::Reserved value in the enum class<T>.
      virtual uint8_t get_ReservedBit() const = 0;

      /// @brief Property (RO): EventEndBit - the end bit number (T::EventEnd) for this instance.
      /// @details This method returns the EventEnd bit number for this instance, which is the  
      ///          bit just after the last event defined in the enum class <T>. i.e. `T::EventEnd`.  
      ///          This is used as a marker, not a valid event. Events are defined from `Reserved` to `EventEnd - 1`.
      /// @return The end bit number for the last event in the enum class<T> instance
      virtual uint8_t get_EventEndBit() const = 0;

      /// @brief Property (R/W): Set/Get the event group handle associated with this instance.
      /// @details This method sets the event group handle associated with this instance, which can be
      ///          used to signal and wait for events in the FreeRTOS event group. The managing class can 
      ///          use this method to set the event group handle for this instance after it has been created.  
      ///          This allows for flexibility in how the event group handle is assigned and managed.
      /// @see get_EventGroup()
      void set_EventGroup(EventGroupHandle_t eventGroupHandle)
         { eventGroup = eventGroupHandle; }

      /// @brief Property (R/W): EventGroup - The event group handle associated with this instance.
      /// @details This get_ method returns the event group handle associated with this instance, which can be
      ///          set by the managing class. The event group handle is used to signal and wait for events 
      ///          in the FreeRTOS event group.
      /// @return The event group handle associated with this instance.
      /// @see set_EventGroup()
      virtual EventGroupHandle_t get_EventGroup() const
         { return eventGroup; }

      /// @brief Property (WO): EventsMethod - The method to call when processing events for this instance.
      /// @details This set_ method allows setting a custom method to be called when processing events
      ///          for this instance. The method should take an `EventBits_t` parameter which will be 
      ///          the event bits to process.
      /// @param method The method to call when processing events for this instance.
      virtual void set_EventsMethod(std::function<void(EventBits_t)> method) = 0;
      #pragma endregion Properties

      #pragma region Methods
      /// @brief Method to get the valid event bits from a given set of event bits.
      /// @details This method checks the given event bits against the defined events in
      ///          the enum (class <T>) and returns only the valid event bits that are set.  
      ///          This is useful for filtering the event bits received from an EventGroup 
      ///          against the expected events defined in the enum. This provides a way to
      ///          filter out only the events related to the enum class <T> events.
      /// @param eventBits The event bits to validate/filter for the enum class <T>.
      /// @returns The valid event bits that are set in the given eventBits for the 
      ///          enum class <T>, or 0 if none are valid.
      /// @see GetMask(T tEvent)
      virtual EventBits_t GetValidBits(EventBits_t eventBits) const = 0;
      
      /// @brief Process incoming event bits with optional callback filtering to this enum's bit range.
      /// @details This method processes the incoming event bits, it can be called by the managing class 
      ///          when events are received from the event group. The method will filter the incoming 
      ///          event bits to only the valid bits for this enum class <T> using the `GetValidBits()` 
      ///          method, and then it will call the `eventsMethod` callback if it is set, passing only the 
      ///          valid event bits to the callback for processing.
      /// @param eventBits The incoming event bits to be processed, which may include bits outside the valid range for this enum class <T>.
      virtual void ProcessEvents(EventBits_t eventBits) = 0;

      /// @brief Class method to signal one or more event bits in the given event group.
      /// @details This class method signals one or more event bits in the given event group. The
      ///          method is a wrapper around the FreeRTOS `xEventGroupSetBits()` function, 
      ///          it checks if the event group handle is valid before calling the function.
      /// @remarks This is the generic base class method to signal events. It is expected that a
      ///          child class will provide the instance methods specific to the event types.
      static EventBits_t SignalEvents(EventGroupHandle_t eventGroup, EventBits_t eventBits)
         {
         EventBits_t result = 0;
         if (eventGroup != NULL)
            { result = xEventGroupSetBits(eventGroup, (eventBits & AllEventsMask)); }

         return result;
         }

      /// @brief Class method to wait for one of the specific event bit(s) to be set in the given event group.
      /// @details This class method waits for one of the specified event bit(s) from the given event group to be set.
      ///          The method will block until one of the specified event bit(s) is set or the specified timeout expires.  
      ///          This method is just a wrapper around the FreeRTOS `xEventGroupWaitBits()` function, it sets the 
      ///          `clearOnExit` parameter to true to clear the bits that were set before returning, and 
      ///          `waitForAllBits` to false to wait for any one of the specified bits.
      /// @remarks This is the generic base class method to wait for events. It is expected that a child class will 
      ///          provide the instance methods specific to the event types.
      /// @note    It waits for any one of the specified bits to be set, not all of them. 
      /// @param eventGroup The event group handle to monitor for events.
      /// @param eventBits The event bit(s) to wait for. Only one of the events bits is needed for success.
      /// @param msToWait The maximum time (in milliseconds) to wait for the event bits.
      /// @param clearOnExit Optional flag: If true, the bits that were set will be cleared before returning. Default is true.
      /// @return The event bit(s) that were set.
      static EventBits_t WaitForBits(EventGroupHandle_t eventGroup, EventBits_t eventBits, size_t msToWait, bool clearOnExit = true)
         {
         EventBits_t result = 0;
         if (eventGroup != NULL)
            { result = xEventGroupWaitBits(eventGroup, (eventBits & AllEventsMask), (clearOnExit ? pdTRUE : pdFALSE), pdFALSE, MS_TO_TICKS(msToWait)); }

         return result;
         }
      #pragma endregion Methods

      #pragma region Fields
   public:
      /// @brief The maximum number of event bits available for the enum class<T> based on the current board FreeRTOS implementation.
      static constexpr uint8_t MaxEventBits = (configTICK_TYPE_WIDTH_IN_BITS - GROUP_EVENT_RESERVED_BITS);

      /// @brief The mask for all valid event bits based on the maximum number of event bits available (i.e. MaxEventBits).
      static constexpr EventBits_t AllEventsMask = (1 << MaxEventBits) - 1; 

   protected:
      EventGroupHandle_t eventGroup = NULL;  ///< The event group handle associated with this instance, can be set by the managing class.
      int8_t bitOffset = 0;                  ///< Instance event bit offset.
      #pragma endregion Fields
      }; // class TaskGroupBase
   #pragma endregion TaskGroupBase Class
      
   #pragma region TaskGroupBits<T> Class
   /// @brief Task Group Events class for managing group related enum class<T> events.
   /// @details This class provides methods to use enums (class <T>) to manage EventGroup bits for 
   ///          task groups. It allows setting an offset for the event bits to avoid conflicts with 
   ///          other EventGroup bits. This makes it easy to combine multiple enum classes to be 
   ///          combined in one task group without bit conflicts.  
   ///          The `BitOffset` value for the class instance can be set in the constructor or from
   ///          the `BitOffset` property. This flexibility allows you to set the offset at runtime, 
   ///          which is useful when you have multiple libraries or task groups that need to use 
   ///          this class with different offset values.   
   ///          The static `DefaultOffset` property can be set to define a default offset that will 
   ///          be used to set the `BitOffset` for each instance of this class when the default constructor is used.
   ///          The static methods `GetGetResultBit()` and `GetResultMask()` can be used
   ///          to get the bit and mask values using the default offset without creating
   ///          an instance of this class.
   /// @tparam T The enum class type that defines the events for this group, it must include 
   ///         `Reserved` as the first enum and `EventEnd` as the last enum.
   /// @example
   ///         To use this class for task group events, you can create an instance like this:
   ///         ```cpp
   ///         enum class MyEvents : uint8_t { Reserved = 0, Event1, Event2, EventEnd };
   ///         TaskGroupBits<MyEvents> myEventBits(myEventGroupHandle); 
   ///         ```
   ///         You can then set the event processing method for this instance: 
   ///         ```cpp
   ///         myEventBits.set_EventsMethod([](EventBits_t bits) {
   ///            // Process the event bits for MyEvents here
   ///         });
   ///         ```
   /// @see TaskGroupBase
   /// @author Chris-70 (2026/01)
   template<typename T>
   class TaskGroupBits : public TaskGroupBase
      {
   public:
      /// @brief The number of events in the enum including `Reserved` but excluding the end marker `EventEnd`.
      static constexpr uint8_t EventsCount = static_cast<uint8_t>(T::EventEnd) - static_cast<uint8_t>(T::Reserved);
      
      /// @brief The maximum offset that can be used based on the enum class<T> and the current board FreeRTOS implementation.
      static constexpr int8_t MaxOffsetSigned 
                              = (static_cast<int8_t>(MaxEventBits)
                              -  static_cast<int8_t>(EventsCount));
      
      static constexpr uint8_t MaxStartBit = MaxOffsetSigned < 0 ? 0 : static_cast<uint8_t>(MaxOffsetSigned);

      /// @brief Clamps the given value to the maximum allowed start bit.
      /// @param value The value to be clamped.
      /// @return The clamped value, which will not exceed the maximum allowed start bit.
      static constexpr uint8_t ClampStartBit(uint8_t value)
         { return (value > MaxStartBit) ? MaxStartBit : value; }

      #pragma region Static assertions
      // Compile-time checks to validate the enum class<T> meets the requirements for this class to function correctly.
      // Check: T must be an enum class.
      static_assert(std::is_enum_v<T>, "T must be an enum class");
      // Stricter check: T must have Reserved and EventEnd enum values.
      static_assert(std::is_same_v<decltype(T::Reserved), T>, "T must have a Reserved enum value");
      static_assert(std::is_same_v<decltype(T::EventEnd), T>, "T must have an EventEnd enum value");
      /// T::Reserved must come before T::EventEnd in the enum definition, and they must be sequential. 
      static_assert(static_cast<uint8_t>(T::Reserved) < static_cast<uint8_t>(T::EventEnd), "T::Reserved must be less than T::EventEnd");
      // Validate that the enum event size does not exceed the maximum allowed by the FreeRTOS event group bits, i.e. MaxOffsetSigned < 0..                           
      static_assert((MaxOffsetSigned >= 0), "Event enum range is too large for current configTICK_TYPE_WIDTH_IN_BITS");
      // Check that all enums between T::Reserved and T::EventEnd are both sequential and valid, i.e. there are no gaps in the enum values.
      static constexpr bool IsSequential()
         {
         bool sequential = true;
         for (size_t i = 0; i < EventsCount; i++)
            {
            // Check if the enum value is sequential and valid by comparing the expected value with the actual value. 
            // The expected value is calculated as `i + T::Reserved`, and the actual value is obtained by casting it to uint8_t. 
            // If they are not equal, then the enum values are not sequential or valid.
            if (static_cast<uint8_t>(static_cast<T>(i + static_cast<uint8_t>(T::Reserved))) != (i + static_cast<uint8_t>(T::Reserved)))
               {
               sequential = false;
               break;
               }
            }
         return sequential;
         }
      static_assert(IsSequential(), "All enum values between T::Reserved and T::EventEnd must be sequential and valid");
      #pragma endregion Static assertions

      /// @brief Default constructor for TaskGroupBits class.
      /// @details This constructor initializes the TaskGroupBits instance
      ///          with the default enum class<T> bit offset (e.g. 0).
      TaskGroupBits() : TaskGroupBase() 
         {}

      /// @brief Constructor for TaskGroupBits class with custom event group handle.
      /// @details This constructor initializes the TaskGroupBits instance with a specified event group handle.
      ///          The bit offset will be set to the default value (e.g. 0) in this constructor.  
      ///          The caller is responsible for creating the `eventGroupHandle` (i.e. calling `xEventGroupCreate()`) 
      ///          and ensuring it is valid before passing it to this constructor.
      /// @param eventGroupHandle The event group handle to be associated with this instance.
      explicit TaskGroupBits(EventGroupHandle_t eventGroupHandle) : TaskGroupBase(eventGroupHandle) 
         {}

      /// @brief Constructor for TaskGroupBits class with custom bit offset.
      /// @details This constructor initializes the TaskGroupBits instance
      ///          with a specified enum class<T> bit offset.
      /// @param startBitValue The bit offset value to be used for this instance.
      explicit TaskGroupBits(uint8_t startBitValue) : TaskGroupBase(ClampStartBit(startBitValue))
         {}

      /// @brief Constructor for TaskGroupBits class with custom event group handle and bit offset.
      /// @details This constructor initializes the TaskGroupBits instance with a specified event group handle and
      ///          a specified enum class<T> bit offset. The caller is responsible for creating the `eventGroupHandle`
      ///          (i.e. calling `xEventGroupCreate()`) and ensuring it is valid before passing it to this constructor.
      /// @param eventGroupHandle The event group handle to be associated with this instance.
      /// @param startBitValue The bit offset value to be used for this instance.
      explicit TaskGroupBits(EventGroupHandle_t eventGroupHandle, uint8_t startBitValue) : TaskGroupBase(eventGroupHandle, ClampStartBit(startBitValue)) 
         {}

      /// @brief Virtual destructor for TaskGroupBits class.
      virtual ~TaskGroupBits() = default;

      /// @brief Compile time Property (RO): EventsMask - The adjusted event bits mask for the enum class @c <T>.
      /// @details This method calculates the adjusted event bits mask for the enum class @c <T> at compile time.
      /// @return The adjusted (bitOffset) events bit mask for the enum class @c <T>.
      constexpr EventBits_t get_EventsMask() const
         {
         EventBits_t validMask = 0;
         for (uint8_t i = 0; i < EventsCount; i++)
            {
            T tEvent = static_cast<T>(i + static_cast<uint8_t>(T::Reserved));
            validMask |= GetMask(tEvent);
            }

         return validMask;
         }

      #pragma region TaskGroupBase overrides
      /// @brief  Read only property to get the number of events defined in the `enum class <T>`.
      /// @return The number of events defined in the `enum class <T>`.
      virtual uint8_t get_EventsCount() const override
         {
         return EventsCount;
         }

      /// @brief Property (RO): MaxStartBit - the maximum allowed bit number for this instance.
      /// @details This method returns the maximum allowed bit number for this instance, which is calculated 
      ///          based on the number of events defined in the enum class @c <T> and the maximum number of bits 
      ///          available in the event group (i.e. `configTICK_TYPE_WIDTH_IN_BITS` minus the number of events).
      /// @return The maximum allowed starting bit number for this instance based on the enum class @c <T>.
      virtual uint8_t get_MaxStartBit() const override
         {
         return MaxStartBit;
         }

      /// @brief Property (RO): EventEndBit - the end bit number (T::EventEnd) for this instance.
      /// @details This method returns the EventEnd bit number for this instance, which is the  
      ///          bit just after the last event defined in the enum class @c <T>. i.e. `T::EventEnd`.  
      ///          This is used as a marker, not a valid event. Events are defined from `Reserved` to `EventEnd - 1`.
      /// @return The end bit number for the last event in the enum class @c <T> instance
      virtual uint8_t get_EventEndBit() const override
         {
         return static_cast<uint8_t>(T::EventEnd);
         }  
         
      /// @brief Property (RO): ReservedBit - the reserved bit number (T::Reserved) for this enum class.
      /// @brief Read only property to get the reserved bit number for this enum class.
      /// @details This method returns the start/reserved bit number for this enum class, which is 
      ///          the starting bit for the events defined in the enum class @c <T>. i.e. `T::Reserved`.
      /// @return The reserved bit number for the start event in the enum class @c <T>.
      virtual uint8_t get_ReservedBit() const override
         {
         return static_cast<uint8_t>(T::Reserved);
         }  

      /// @brief Method to get the valid event bits from a given set of event bits.
      /// @details This method checks the given event bits against the defined events in
      ///          the enum (class @c <T>) and returns only the valid event bits that are set.  
      ///          This is useful for filtering the event bits received from an EventGroup 
      ///          against the expected events defined in the enum. This provides a way to
      ///          filter out only the events related to the enum class @c <T> events.
      /// @param eventBits The event bits to validate/filter for the enum class @c <T>.
      /// @returns The valid event bits that are set in the given eventBits for the 
      ///          enum class @c <T> or 0 if none are valid.
      /// @see GetMask(T tEvent)
      virtual EventBits_t GetValidBits(EventBits_t eventBits) const override
         {
         EventBits_t validBits = (get_EventsMask() & eventBits); // Filter with the valid event mask to ignore unrelated bits.

         return validBits;
         }  

      /// @brief Process incoming event bits with optional callback filtering to this enum's bit range.
      /// @details This method processes the incoming event bits, it can be called by the managing class 
      ///          when events are received from the event group. The method will filter the incoming 
      ///          event bits to only the valid bits for this enum class <T> using the `GetValidBits()` 
      ///          method, and then it will call the `eventsMethod` callback if it is set, passing only the 
      ///          valid event bits to the callback for processing.
      /// @param eventBits The incoming event bits to be processed, which may include bits outside the valid range for this enum class <T>.
      virtual void ProcessEvents(EventBits_t eventBits) override
         {
         try
            {
            if (eventsMethod)
               {
               eventsMethod(GetValidBits(eventBits));
               }
            }
         catch (const std::exception& ex)
            {
            // ToDo: Handle the exception, e.g., log it or ignore it
            }
         catch (...)
            {
            // ToDo: Handle any other types of exceptions, e.g., log it or ignore it
            }
         }

      /// @brief Set callback invoked by ProcessEvents().
      /// @details This method allows setting a custom callback method to be invoked by the `ProcessEvents()` method 
      ///          when processing events for this instance. The method should take an `EventBits_t` parameter which 
      ///          will be the event bits to process.
      /// @param method The callback method to be invoked by `ProcessEvents()`.
      virtual void set_EventsMethod(std::function<void(EventBits_t)> method) override
         {
         eventsMethod = method;
         }
      #pragma endregion TaskGroupBase overrides

      /// @brief Method to get the bit number for the given `tEvent`
      /// @param tEvent The event enum to get the bit number.
      /// @returns the bit bumber corresponding to the given `tEvent`.
      /// @see GetMask()
      virtual uint8_t GetBit(T tEvent) const
         {
         // Perform the addition as a signed 16 bit integer, then convert back.
         return static_cast<uint8_t>(static_cast<int16_t>(tEvent) + get_BitOffset());
         }

      /// @brief Method to get the bit mask for the given `tEvent`
      /// @param tEvent The event enum (class @c <T>) to get the bit mask.
      /// @returns the bit mask corresponding to the given `tEvent`.
      /// @see GetBit()
      virtual EventBits_t GetMask(T tEvent) const
         {
         return static_cast<EventBits_t>(static_cast<EventBits_t>(1U) << GetBit(tEvent));
         }
         
      /// @brief Method to get the combined bit mask for a vector of `tEvents`
      /// @param tEvents A vector of event enums (class @c <T>) to get the combined bit mask.  
      /// @returns the combined bit mask corresponding to the given vector of `tEvents`.
      /// @see GetMask(T tEvent)
      virtual EventBits_t GetMask(const std::vector<T>& tEvents) const
         {
         EventBits_t eventBits = 0;
         for (const auto& tEvent : tEvents)
            {
            eventBits |= GetMask(tEvent);
            }

         return eventBits;
         }

      /// @brief Method to check if a specific event bit is set in the given event bits.
      /// @param eventBits The event bits to check.
      /// @param tEvent The event enum (class @c <T>) to check.
      /// @returns `true` if the specified event bit is set, `false` otherwise.
      /// @see GetMask(T tEvent)
      virtual bool IsBitSet(EventBits_t eventBits, T tEvent) const
         {
         return (eventBits & GetMask(tEvent)) != 0;
         }

      /// @brief Method to signal a specific event enum (class @c <T>) in the given event group.
      /// @param tEvent The event enum (class @c <T>) to signal.
      /// @return The event bit that was set.
      /// @see TaskGroupBase::SignalEvents(EventGroupHandle_t eventGroup, EventBits_t eventBits)
      /// @see SignalEvents(const std::vector<T>& tEvents)
      virtual EventBits_t SignalEvent(T tEvent)
         {
         EventBits_t eventBit = GetMask(tEvent);
         return TaskGroupBase::SignalEvents(eventGroup, eventBit);
         }

      /// @brief Method to signal one or more specific event enums (class @c <T>) in the given event group.
      /// @param tEvents A vector of event enums (class @c <T>) to signal.
      /// @return The event bit(s) that were set.
      /// @see SignalEvent(T tEvent)
      virtual EventBits_t SignalEvents(const std::vector<T>& tEvents)
         {
         EventBits_t eventBits = GetMask(tEvents);
         return TaskGroupBase::SignalEvents(eventGroup, eventBits);
         }

      /// @brief Method to wait for one of the specific events enums (class @c <T>) to be set in the given event group.
      /// @param tEvents A vector of event enums (class @c <T>) to wait for.
      /// @param msToWait The maximum time (in milliseconds) to wait for the event bits.
      /// @param clearOnExit Optional flag: If true, the bits that were set will be cleared before returning. Default is true.
      /// @return The event bit(s) that were set.
      /// @see WaitForBits(EventGroupHandle_t eventGroup, EventBits_t eventBits, size_t msToWait)
      /// @see WaitForBits(EventGroupHandle_t eventGroup, T tEvent, size_t msToWait)
      virtual EventBits_t WaitForBits(const std::vector<T>& tEvents, size_t msToWait, bool clearOnExit = true)
         {
         EventBits_t eventBits = GetMask(tEvents);
         return TaskGroupBase::WaitForBits(eventGroup, eventBits, msToWait, clearOnExit);
         }

      /// @brief Method to wait for a specific event enum (class @c <T>) to be set in the given event group.
      /// @param tEvent The event enum (class <T>) to wait for.
      /// @param msToWait The maximum time (in milliseconds) to wait for the event bit.
      /// @param clearOnExit Optional flag: If true, the bit that was set will be cleared before returning. Default is true.
      /// @return The event bit that was set.
      /// @see WaitForBits(EventGroupHandle_t eventGroup, EventBits_t eventBits, size_t msToWait)
      /// @see WaitForBits(EventGroupHandle_t eventGroup, const std::vector<T>& tEvents, size_t msToWait)
      virtual EventBits_t WaitForBits(T tEvent, size_t msToWait, bool clearOnExit = true)
         {
         return TaskGroupBase::WaitForBits(eventGroup, GetMask(tEvent), msToWait, clearOnExit);
         }

      /// @brief Static method to get the template bit for a given enum event.
      /// @details This static method calculates the bit number for a given enum event. 
      ///          This is equivalent to `static_cast<uint8_t>(tEvent)` for the 
      ///          original `enum class<T>` values.
      /// @param tEvent The enum event type.
      /// @return The bit number for the specified event.
      /// @see GetTemplateMask()
      static uint8_t GetTemplateBit(T  tEvent)
         {
         return (static_cast<uint8_t>(tEvent));
         }

      /// @brief Static method to get the template mask for a given enum event.
      /// @details This static method calculates the bit mask for a given enum event. 
      ///          This is equivalent to `1 << static_cast<EventBits_t>(tEvent)` for the 
      ///          original `enum class<T>` values.
      /// @param tEvent The enum event type.
      /// @return The bit mask for the specified event.
      /// @see GetTemplateBit()
      static EventBits_t GetTemplateMask(T  tEvent)
         {
         return (static_cast<EventBits_t>(static_cast<EventBits_t>(1U) << GetTemplateBit(tEvent)));
         }
   
      /// @brief Static property to set/get the default group event bit offset.
      /// @details This static property allows setting a default offset for group event bits.
      ///          The default value is used by the default constructor to set the initial offset value.
      ///          It is also used by the static methods `GetTemplateBit()` and `GetTemplateMask()`.
      /// Note The enum value of `EventEnd` plus `DefaultOffset` must NOT exceed the value of
      ///      (`configTICK_TYPE_WIDTH_IN_BITS` - 8). e.g. 8; 24; or 56 for 16; 32; or 64 bit widths.
      /// @param value The default offset value to set for all group event bits.
      /// @see get_DefaultOffset()
      /// @see GetTemplateBit()
      /// @see GetTemplateMask()
      static void set_DefaultOffset(uint8_t value)
         {
         defaultOffset = (value > MaxStartBit ? MaxStartBit : value);
         }
      /// @copydoc set_DefaultOffset()
      /// @return The current default group event bit offset.
      /// @see set_DefaultOffset()
      static uint8_t get_DefaultOffset()
         {
         return defaultOffset;
         }

   private:
      inline static uint8_t defaultOffset = 0U;  ///< Static default event bit offset. Used by the constructors without an offset.
      std::function<void(EventBits_t)> eventsMethod = nullptr; ///< Optional callback for processed event bits.
      }; // class TaskGroupBits
   #pragma endregion TaskGroupBits<T> Class
 
   } // namespace BinaryClockShield
#endif // __TASKGROUPBITS_H__