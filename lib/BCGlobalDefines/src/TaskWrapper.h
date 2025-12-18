#pragma once
#ifndef __TASKWRAPPER_H__
#define __TASKWRAPPER_H__

/// @file TaskWrapper.h - Generic template task creator and wrapper for instance methods to be called by `xTaskCreate()`.
/// @brief This file provides a generic task wrapper for FreeRTOS `xTaskCreate()` that need to call an instance method.
/// @details It allows creating tasks that can call instance methods with any number of arguments,
///          handling argument forwarding, task creation, and cleanup automatically.  
///          This is particularly useful in embedded systems using FreeRTOS where tasks often need to
///          run member functions of classes without making the methods static. FreeRTOS tasks require
///          a C-style function pointer, so this wrapper bridges that gap by packaging the instance
///          method call into a compatible task function.  
///          These series of functions are all template-based to allow for maximum flexibility with different
///          class types and method signatures.   
///          The main components are:
///          - `TaskParamWrapper<T, Args...>`: A structure that holds the instance pointer, method pointer,
///            and arguments for the task. This is the parameter passed in the `xTaskCreate()` call.
///          - `TaskWrapper<T, Args...>()`: The FreeRTOS task function that unpacks the parameters and 
///             calls the instance method through `CallInstance<T, Args...>()`.
///          - `CreateInstanceTask<T, Args...>()`: A helper function to create the task with the 
///            appropriate parameters.
/// 
///          This implementation also supports static / free methods, wrapping the calls similarly.
///          This allows for the same methods to be called as tasks without modification and 
///          without needing to create custom wrapper methods for each method.   
///          The main components are:  
///          - `MethodParamWrapper<...Args>`: A structure that holds the method pointer and all
///            arguments for the task. This is the parameter passed in the `xTaskCreate()` call.
///          - `MethodWrapper<Args...>()`: The `FreeRTOS` task function that unpacks the parameters and
///            calls the method through `CallMethod<Args...>()`.
///          - `CreateMethodTask<Args...>()`: A helper function to create the task with the 
///            appropriate parameters.
/// 
///          This header file makes use of the `__has_include` preprocessor directive to conditionally
///          include headers and enable features based on their availability. This is defined in the 
///          C++17 standard, however, some compiles have implemented this as an extension for earlier 
///          language standards. Using C++17 or later is recommended for full compatibility.    
///          This code makes use of optional output statements for error handling and development. 
///          These output statements can be enabled or disabled based on the presence of the 
///          `SerialOutput.Defines.h` file. If this file is not available, all output statements
///          are removed from compilation to save code space.
/// @design This design uses variadic templates and tuple unpacking to handle any number of method arguments.  
///          It ensures type safety and flexibility while maintaining compatibility with FreeRTOS task creation.
///          The use of heap allocation for the parameter wrapper allows for dynamic argument passing,
///          but care must be taken to manage memory correctly to avoid leaks. With this in mind, the 
///          `TaskWrapper()` function is responsible for cleaning up the allocated memory after the task completes.   
///          `CreateInstanceTask()` handles task and parameter creation along with error checking, 
///          providing a simple interface for users to create tasks that run instance methods.
/// 
///          The motivation behind this implementation was the need to use the same instance methods in
///          both blocking and async task contexts depending on whether FreeRTOS multitasking was enabled.  
///          This avoids code duplication and keeps the method implementations consistent across different
///          execution contexts. I also wanted to bypass the limitation of FreeRTOS requiring static or 
///          free functions for task entry points, allowing for more object-oriented designs. I also wanted
///          to maintain type safety and flexibility with method signatures, which is achieved through 
///          the use of templates and variadic arguments. 
/// 
///          The compiler handles most of the heavy lifting leaving the methods to be distilled down to
///          creating the parameter instance on the heap, and calling `xTaskCreate()`. The function that
///          `xTaskCreate()` calls is the `TaskWrapper()` function template that just call the instance method
///          with unpacked arguments and cleans up afterwards. The impact should be minimal on code size and
///          performance due to compiler optimizations, if not just write a static or free function.
/// @remarks This implementation requires at least the C++14 language standard due to the use of
///          variadic templates and `std::index_sequence()`
/// @note This implementation assumes that the instance method does not return a value (i.e., it has a `void` 
///          return type) as is required by `xTaskCreate()`.
/// @author Chris-70 (2025/11)

#if __cplusplus < 201402L
#error "TaskWrapper.h requires at least the C++14 language standard"
#endif

// Standard C++ includes
#include <tuple>                       // For std::tuple
#include <utility>                     // For std::forward, std::index_sequence, std::index_sequence_for
#include <exception>                   // For std::exception
#include <cstddef>                     // For std::size_t
#include <type_traits>                 // For std::decay

#if DEV_CODE
   #include <Arduino.h>                // For millis(), used in debug output only.
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
// Generic Task Wrapper for Instance Methods
////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_STACKSIZE     2048U                   /// The default stack size for a small task, in words.
#define DEFAULT_PRIORITY      (tskIDLE_PRIORITY + 1U) /// The default task priority, just above idle.

/// @brief Parameter wrapper for `FreeRTOS` tasks that need to call an instance method.
/// @details This structure is the key to creating a task to run the instance method.
///          The `TaskWrapper()` function is passed to `xTaskCreate()` along with an
///          instance of this structure taht has everything necessary to call the
///          instance method.
/// @tparam `T` The class type that contains the instance method
/// @tparam `Args` Variadic template for method arguments
/// @author Chris-70 (2025/11)
template<typename T, typename... Args>
struct TaskParamWrapper
   {
   const char* taskName;               ///< Name for task ID to find the task.
   T* instance;                        ///< Pointer to the class `T` instance.
   void (T::* method)(Args...);        ///< Pointer to the instance method to call.
   std::tuple<Args...> args;           ///< Tuple containing the arguments for the method.

   /// @brief Constructor to populate the parameter fields required for an instance method call.
   /// @details This constructor initializes the task name, instance pointer, method pointer,
   ///          and argument tuple for the task to call the instance method.
   /// @param name Name of the task for the task ID and debugging.
   /// @param inst Pointer to the class `T` instance on which to call the method.
   /// @param methodPtr Pointer to the instance method to call.
   /// @param arguments Variadic arguments to pass to the instance method.
   TaskParamWrapper(const char* name, T* inst, void (T::* methodPtr)(Args...), Args&&... arguments)
         : taskName(name)
         , instance(inst)
         , method(methodPtr)
         , args(std::forward<Args>(arguments)...)
      { }
   };

/// @brief Parameter wrapper for `FreeRTOS` tasks that need to call an method.
/// @details This structure is the key to creating a task to run the method.
///          The `TaskWrapper()` function is passed to `xTaskCreate()` along with an
///          instance of this structure that has everything necessary to call the
///          method.
/// @tparam `Args` Variadic template for method arguments.
/// @author Chris-70 (2025/11)
template<typename... Args>
struct MethodParamWrapper
   {
   const char* taskName;               ///< Name for task ID to find the task.
   void (*method)(Args...);            ///< Pointer to the method to call.
   std::tuple<Args...> args;           ///< Tuple containing the arguments for the method.

   /// @brief Constructor to populate the parameter fields required for a method call.
   /// @details This constructor initializes the task name, method pointer,
   ///          and argument tuple for the task to call the instance method.
   /// @param name Name of the task for the task ID and debugging.
   /// @param methodPtr Pointer to the method to call.
   /// @param arguments Variadic arguments to pass to the method.
   MethodParamWrapper(const char* name, void (*methodPtr)(Args...), Args&&... arguments)
         : taskName(name)
         , method(methodPtr)
         , args(std::forward<Args>(arguments)...)
      { }
   };

/// @brief Generic task wrapper that calls an instance method and cleans up.
/// @details This helper function is called internally from `TaskWrapper()` as the
///          method to unpack all the arguments needed by the instance method
///          and finally call the instance method.
/// @remarks This function requires a different template signature from `TaskWrapper()` and
///          `CreateInstanceTask()` as it needs the index sequence for unpacking 
///          the tuple arguments `std::index_sequence`. It is cleaner to make a
///          standalone function to unpack and call the instance method. Any decent
///          optimizing compiler will inline this inside of the `TaskWrapper()` function.
/// @tparam `T` Class type of the instance method to run.
/// @tparam `Args` Method argument types. Mirrors the method signature.
/// @tparam `Is` Index sequence for unpacking tuple arguments.
/// @param instance Pointer to the class instance of type `T`.
/// @param method Pointer to the instance method to call.
/// @param args Variadic tuple containing the method arguments.
/// @tparam `Is` Index sequence for unpacking tuple arguments.
/// @author Chris-70 (2025/11)
template<typename T, typename... Args, std::size_t... Is>
inline void CallInstance( T* instance
                        , void (T::* method)(Args...)
                        , std::tuple<Args...>& args
                        , std::index_sequence<Is...>)
      {
      (instance->*method)(std::get<Is>(args)...);
      }

/// @brief Generic task wrapper that calls a method and cleans up.
/// @details This helper function is called internally from `TaskWrapper()` as the
///          method to unpack all the arguments needed by the method
///          and finally call the method.
/// @remarks This function requires a different template signature from `TaskWrapper()` and
///          `CreateMethodTask()` as it needs the index sequence for unpacking 
///          the tuple arguments `std::index_sequence`. It is cleaner to make a
///          standalone function to unpack and call the instance method. Any decent
///          optimizing compiler will inline this inside of the `TaskWrapper()` function.
/// @tparam `Args` Method argument types. Mirrors the method signature.
/// @tparam `Is` Index sequence for unpacking tuple arguments.
/// @param instance Pointer to the class instance of type `T`.
/// @param method Pointer to the instance method to call.
/// @param args Variadic tuple containing the method arguments.
/// @tparam `Is` Index sequence for unpacking tuple arguments.
/// @author Chris-70 (2025/11)
template<typename... Args, std::size_t... Is>
inline void CallMethod( void (*method)(Args...)
                      , std::tuple<Args...>& args
                      , std::index_sequence<Is...>)
   {
   (method)(std::get<Is>(args)...);
   }

/// @brief Helper FreeRTOS task function wrapper to run an instance method in a task and clean up.
/// @details This helper function is internally called from `CreateInstanceTask()` as the 
///          task method passed to the `xTaskCreate()` function call. The parameter `param` that 
///          `xTaskCreate()` passes to this function is of type `TaskParamWrapper` that contains the:  
///          - pointer to the class `T` instance.
///          - Pointer to the instance method to call.
///          - Variadic tuple containing the method arguments.
///          - Task name for debugging.
/// 
///          This function is compatible with `xTaskCreate()` and handles:  
///          - Calling instance methods with any number of arguments
///          - Automatic cleanup of the parameter wrapper
///          - Self-deletion of the task when complete
/// @tparam `T` Class type of the instance method to run.
/// @tparam `Args` Method argument types. Mirrors the method signature.
/// @param param TaskParamWrapper<T, Args...>* pointer to the parameter wrapper containing 
///              instance, method, and arguments. 
/// @see TaskParamWrapper
/// @author Chris-70 (2025/11)
template<typename T, typename... Args>
void TaskWrapper(void* param)
   {
   using ParamType = TaskParamWrapper<T, Args...>;
   ParamType* taskParam = static_cast<ParamType*>(param);

   if (taskParam == nullptr)
      {
      SERIAL_PRINTLN("ERROR: TaskWrapper received null parameter!")
      vTaskDelete(nullptr);
      return;
      }

   SERIAL_STREAM("[" << millis() << "] Task '" << taskParam->taskName << "' started" << endl)

   try
      {
      if (taskParam->instance && taskParam->method)
         {
         // Call the instance method with unpacked arguments
         CallInstance<T, Args...> ( taskParam->instance
                                          , taskParam->method
                                          , taskParam->args
                                          , std::index_sequence_for<Args...>{});

         SERIAL_STREAM("[" << millis() << "] Task '" << taskParam->taskName << "' completed successfully" << endl)
         }
      else
         {
         SERIAL_PRINTLN("ERROR: Invalid instance or method pointer in TaskWrapper!")
         }
      }
   catch (const std::exception& e)
      {
      // This threw an exception, log the error message.
      SERIAL_OUT_STREAM("ERROR in TaskWrapper(): Exception in task '" << taskParam->taskName << "': " << e.what() << endl)
      }
   catch (...)
      {
      // Unknown exception, log generic error message.
      SERIAL_OUT_STREAM("ERROR in TaskWrapper(): Unknown exception in task '" << taskParam->taskName << "'" << endl)
      }

   SERIAL_STREAM("[" << millis() << "] Task '" << taskParam->taskName << "' deleted" << endl)

   // Clean up, delete the parameter wrapper allocated on the heap.
   delete taskParam;

   // Delete this task, the instance method has finished.
   vTaskDelete(nullptr);
   }
   
/// @brief Helper FreeRTOS task function wrapper to run a method in a task and clean up.
/// @details This helper function is internally called from `CreateMethodTask()` as the 
///          task method passed to the `xTaskCreate()` function call. The parameter `param` that 
///          `xTaskCreate()` passes to this function is of type `MethodParamWrapper` that contains the:  
///          - Pointer to the method to call.
///          - Variadic tuple containing the method arguments.
///          - Task name for ID/debugging.
/// 
///          This function is compatible with `xTaskCreate()` and handles:  
///          - Calling instance methods with any number of arguments
///          - Automatic cleanup of the parameter wrapper
///          - Self-deletion of the task when complete
/// @tparam `Args` Method argument types. Mirrors the method signature.
/// @param param MethodParamWrapper<Args...>* pointer to the parameter wrapper containing 
///              method, and arguments. 
/// @see MethodParamWrapper
/// @author Chris-70 (2025/11)
template<typename... Args>
void MethodWrapper(void* param)
   {
   using ParamType = MethodParamWrapper<Args...>;
   ParamType* taskParam = static_cast<ParamType*>(param);

   DEBUG_STREAM("MethodWrapper() - Entering MethodWrapper() for task '" << taskParam->taskName << "'" << endl)
   if (taskParam == nullptr)
      {
      // When passed a `nullptr` just delete the task and exit.
      SERIAL_PRINTLN("ERROR: TaskWrapper received null parameter!")
      vTaskDelete(nullptr);
      return;
      }

   SERIAL_STREAM("[" << millis() << "] Task '" << taskParam->taskName << "' started" << endl)
   try
      {
      if (taskParam->method)
         {
         DEBUG_STREAM("MethodWrapper() - Calling the method for task '" << taskParam->taskName << "'" << endl)
         // Call the method with unpacked arguments
         CallMethod<Args...>( taskParam->method
                                    , taskParam->args
                                    , std::index_sequence_for<Args...>{});

         SERIAL_STREAM("[" << millis() << "] Task '" << taskParam->taskName << "' completed successfully" << endl)
         }
      else
         {
         SERIAL_PRINTLN("ERROR: Invalid instance or method pointer in TaskWrapper!")
         }
      }
   catch (const std::exception& e)
      {
      // This threw an exception, log the error message.
      SERIAL_OUT_STREAM("ERROR in TaskWrapper(): Exception in task '" << taskParam->taskName << "': " << e.what() << endl)
      }
   catch (...)
      {
      // Unknown exception, log generic error message.
      SERIAL_OUT_STREAM("ERROR in TaskWrapper(): Unknown exception in task '" << taskParam->taskName << "'" << endl)
      }

   SERIAL_STREAM("[" << millis() << "] Task '" << taskParam->taskName << "' deleted" << endl)

   // Clean up, delete the parameter wrapper allocated on the heap.
   delete taskParam;

   // Delete this task, the instance method has finished.
   vTaskDelete(nullptr);
   } // MethodWrapper()
   
/// @brief Helper function to create a task that calls an instance method with any arguments.
/// @details This function creates a FreeRTOS task that calls a member function on an instance of a class,
///          passing any number of arguments to that member function. It handles task creation, argument
///          forwarding, and cleanup automatically.  
///          The template nature allows for flexibility with different class types and method signatures.
///          Instance methods with various number of arguments and argument types can be run in a task.
/// @tparam `T` Class type of the instance to invoke
/// @tparam `Args...` Argument type(s) for the instance method.
/// @param instance Pointer to the class instance of type `T`.
/// @param method Pointer to the type `T` instance method that takes some "`Args...`" as parameter(s).
/// @param taskName Name of the task associated with the instance (for task ID/debugging).
/// @param stackSize Stack size to allocate for the task to run the method.
/// @param priority Task priority for this task to run at.
/// @param args Variadic tuple arguments to pass to the method when called.
/// @return The task handle for the task if successful, `nullptr` otherwise.
/// @see CreateInstanceTask(T*, void (T::*)(Args...), const char*, Args...)
/// @see CreateMethodTask(void (*)(Args...), const char*, uint32_t, UBaseType_t, Args...)
/// @see CreateMethodTask(void (*)(Args...), const char*, Args...)
/// @author Chris-70 (2025/11)
/// @example 
/// @code{.cpp}
/// // Display splash screen(s) during startup for `screenTime` seconds each
/// // Display all screens if in a task, or just one otherwise.
/// void splashScreen(bool multiple, int screenTime);  
/// bool allDisplays = true;
/// int displayTime = 2;
///   ...
/// // Create and run the splash screen task with error handling, task runs just once.
/// TaskHandle_t taskCreated = CreateInstanceTask<BinaryClock, bool, int>(
///       this,                           // Instance pointer
///       &BinaryClock::splashScreen,     // Method pointer
///       "SplashTask",                   // Task name
///       2048,                           // Stack size
///       tskIDLE_PRIORITY + 1,           // Priority
///       allDisplays,                    // Method argument 1
///       displayTime                     // Method argument 2
///       );
///
/// if (taskCreated)
///    {
///    SERIAL_STREAM("[" << millis() << "] Splash screen task created successfully" << endl)
///    }
/// else
///    {
///    SERIAL_PRINTLN("ERROR: Failed to create splash screen task!")
///    // Fall back to direct, blocking, execution
///    splashScreen(false, 1);
///    }
///   ...
/// @endcode
template<typename T, typename... Args>
TaskHandle_t CreateInstanceTask( T* instance
                                     , void (T::* method)(Args...)
                                     , const char* taskName
                                     , uint32_t stackSize
                                     , UBaseType_t priority
                                     , Args... args)
   {
   using ParamType = TaskParamWrapper<T, Args...>;
   TaskHandle_t taskHandle = nullptr;

   // Create a parameter wrapper instance on heap
   ParamType* param = new ParamType ( taskName
                                    , instance
                                    , method
                                    , std::forward<Args>(args)...);

   if (param == nullptr)
      {
      SERIAL_PRINTLN("ERROR: Failed to allocate task parameter!")
      return taskHandle;
      }

   // Create the FreeRTOS task to run the instance method one time.
   BaseType_t result = xTaskCreate
         ( TaskWrapper<T, Args...>  // Task function to call the instance method.
         , taskName
         , stackSize
         , param
         , priority
         , &taskHandle
         );

   if (result != pdPASS)
      {
      SERIAL_OUT_STREAM("ERROR in CreateInstanceTask(): Failed to create task '" << taskName << "'" << endl)
      delete param;
      return taskHandle;
      }

   SERIAL_STREAM("[" << millis() << "] Task '" << taskName << "' created" << endl)
   return taskHandle;
   }

/// @copydoc CreateInstanceTask(T*, void (T::*)(Args...), const char*, uint32_t, UBaseType_t, Args...)
/// @remarks Calls `CreateInstanceTask(T*, void (T::*)(Args...), const char*, uint32_t, UBaseType_t, Args...)`
///          using the default stack size, `DEFAULT_STACKSIZE` (e.g. 1024 words) and 
///          priority `DEFAULT_PRIORITY` (e.g. tskIDLE_PRIORITY + 1).
/// @return The task handle for the task if successful, `nullptr` otherwise.
/// @see CreateInstanceTask(T*, void (T::*)(Args...), const char*, uint32_t, UBaseType_t, Args...)
/// @see CreateMethodTask(void (*)(Args...), const char*, uint32_t, UBaseType_t, Args...)
/// @see CreateMethodTask(void (*)(Args...), const char*, Args...)
/// @author Chris-70 (2025/11)
template<typename T, typename... Args>
TaskHandle_t CreateInstanceTask( T* instance
                               , void (T::* method)(Args...)
                               , const char* taskName
                               , Args... args)
   {
   return CreateInstanceTask<T, Args...>(instance, method, taskName, DEFAULT_STACKSIZE, DEFAULT_PRIORITY, std::forward<Args>(args)...);
   }

/// @brief Helper function to create a task that calls a static/free method with any arguments.
/// @details This function creates a FreeRTOS task that calls a static or free function,  
///          The template nature allows for flexibility with different method signatures.
///          Static/free methods with various number of arguments and argument types can be 
///          run in a task and automatically cleaned up when done.
/// @param method Pointer to the type `T` instance method that takes some "`Args...`" as parameter(s).
/// @param taskName Name of the task associated with the instance (for task ID/debugging).
/// @param stackSize Stack size to allocate for the task to run the method.
/// @param priority Task priority for this task to run at.
/// @param args Variadic tuple arguments to pass to the method when called.
/// @return The task handle for the task if successful, `nullptr` otherwise.
/// @see CreateInstanceTask(T*, void (T::*)(Args...), const char*, uint32_t, UBaseType_t, Args...)
/// @see CreateInstanceTask(T*, void (T::*)(Args...), const char*, Args...)
/// @see CreateMethodTask(void (*)(Args...), const char*, Args...)
/// @author Chris-70 (2025/11)
template<typename... Args>
TaskHandle_t CreateMethodTask( void (*method)(Args...)
                             , const char* taskName
                             , uint32_t stackSize
                             , UBaseType_t priority
                             , Args... args)
   {       
   using ParamType = MethodParamWrapper<Args...>;
   TaskHandle_t taskHandle = nullptr;

   DEBUG_STREAM("CreateMethodTask() - Creating parameters for task '" << taskName << "'" << endl)
   // Create a parameter wrapper instance on heap
   ParamType* param = new ParamType ( taskName
                                    , method
                                    , std::forward<Args>(args)...);

   if (param == nullptr)
      {
      SERIAL_PRINTLN("ERROR: Failed to allocate task parameter!")
      return taskHandle;
      }

   DEBUG_STREAM("CreateMethodTask() - Calling xTaskCreate() for task '" << taskName << "'" << endl)
   // Create the FreeRTOS task to run the instance method one time.
   BaseType_t result = xTaskCreate
         ( MethodWrapper<Args...>  // Task function to call the instance method.
         , taskName
         , stackSize
         , param
         , priority
         , &taskHandle
         );

   if (result != pdPASS)
      {
      SERIAL_OUT_STREAM("ERROR in CreateInstanceTask(): Failed to create task '" << taskName << "'" << endl)
      delete param;
      return taskHandle;
      }

   SERIAL_STREAM("[" << millis() << "] Task '" << taskName << "' created/running." << endl)
   return taskHandle;
   }

/// @copydoc CreateMethodTask(T*, void (T::*)(Args...), const char*, uint32_t, UBaseType_t, Args...)
/// @remarks Calls `CreateMethodTask(T*, void (T::*)(Args...), const char*, uint32_t, UBaseType_t, Args...)`
///          using the default stack size, `DEFAULT_STACKSIZE` (e.g. 1024 words) and 
///          priority `DEFAULT_PRIORITY` (e.g. tskIDLE_PRIORITY + 1).
/// @return The task handle for the task if successful, `nullptr` otherwise.
/// @see CreateInstanceTask(T*, void (T::*)(Args...), const char*, uint32_t, UBaseType_t, Args...)
/// @see CreateInstanceTask(T*, void (T::*)(Args...), const char*, Args...)
/// @see CreateMethodTask(void (*)(Args...), const char*, uint32_t, UBaseType_t, Args...)
/// @author Chris-70 (2025/11)
template<typename... Args>
TaskHandle_t CreateMethodTask( void (*method)(Args...)
                             , const char* taskName
                             , Args... args)
   {
   return CreateMethodTask(method, taskName, DEFAULT_STACKSIZE, DEFAULT_PRIORITY, std::forward<Args>(args)...);
   }

#undef DEFAULT_STACKSIZE
#undef DEFAULT_PRIORITY

////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __TASKWRAPPER_H__