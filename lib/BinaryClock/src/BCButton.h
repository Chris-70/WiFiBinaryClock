/// @file BCButton.h
/// @brief This file contains the declaration of the `BCButton` class
/// @details The `BCButton` class encapsulates the functionality of a button including
///          state management, debouncing, and hardware interaction.   
///          The class supports buttons wired as Common Cathode (CC) or Common Anode (CA).  
///          - Common Cathode (CC) buttons are normally LOW and connect the pin to HIGH when pressed.  
///          - Common Anode (CA) buttons are normally HIGH and connect the pin to LOW when pressed.
/// @author Chris-70 (2025/09)

#pragma once
#ifndef __BCBUTTON_H__
#define __BCBUTTON_H__

#include <Arduino.h>

#include "BinaryClock.Defines.h"

namespace BinaryClockShield
   {
   /// @brief Button class to encapsulate button state, debouncing, and hardware interface
   /// @details Handles button reading, debouncing, and state management for buttons.  
   ///          The methods `IsPressed()`, `IsPressedRaw()`, and `IsPressedNew()` return the
   ///          current state of the button (pressed or not pressed), some have debouncing.
   /// @remarks How the button is physically wired, Common Cathode (CC) or Common Anode (CA).  
   ///          CC buttons are wired to connect the pin to HIGH when pressed (onValue = HIGH) 
   ///          meaning the pin is LOW when not pressed. CA buttons are wired to connect the pin 
   ///          to LOW when pressed (onValue = LOW) meaning the pin is HIGH when not pressed.  
   ///          During `Initialize()` the pin is configured with the appropriate pull-up or 
   ///          pull-down resistor if the board supports it (e.g. pinMode(pin, INPUT_PULLDOWN)).  
   /// @note The three buttons, S1, S2, and S3 on the Binary Clock Shield, are wired CC.  
   ///          On the development board, for example, the serial time button is wired CA.
   /// @author Chris-70 (2025/09)
   class BCButton
      {
   public:
      /// @brief Constructor for button with pin and connection type (i.e. CC or CA).
      /// @details The button can be wired as Common Cathode (CC) or Common Anode (CA).  
      ///          For CC, the button connects the pin to HIGH when pressed (onValue = HIGH).  
      ///          For CA, the button connects the pin to LOW when pressed (onValue = LOW).  
      ///          The pin is configured with the appropriate pull-up or pull-down resistor if
      ///          the board supports it (i.e.  pinMode(pin, INPUT_PULLDOWN) for CC or
      ///          pinMode(pin, INPUT_PULLUP) for CA).  
      /// @note    `Initialize()` must be called before the button can be used.
      /// @remarks Some boards (e.g. UNO R3) do not support `INPUT_PULLUP` so they just use
      ///          `INPUT` instead. 
      /// @param pin GPIO pin number for the button
      /// @param onValue Value when button is pressed (i.e. ON). Use the `CC_ON` or `CA_ON` 
      ///          constants defined in `BinaryClock.Defines.h` for the value based on the wiring
      ///          (i.e. HIGH for pull-down (CC), LOW for pull-up (CA)).
      /// @see Initialize()
      /// @author Chris-70 (2025/09)
      BCButton(uint8_t pin, uint8_t onValue);

      // Destructor (default is sufficient for now).
      virtual ~BCButton() = default;
  

      /// @brief Initialize the button pin with appropriate pull-up/pull-down settings
      /// @author Chris-70 (2025/09)
      void Initialize();

      /// @brief Check if button is currently pressed (held down) after debounce.
      /// @details This method debounces the button and returns true if it is currently pressed.
      ///          If the current value isn't stable, it calls `IsPressedNew()`  to get a
      ///          stable reading. This requires that the method is called a second time,
      ///          at least `BounceDelay` miliseconds later, for a stable reading.
      /// @remarks This method is designed to be called frequently (every few milliseconds) or 
      ///          the IsPressedNew() method is called frequently to get a stable reading.
      /// @note This method can impact the state of `IsPressedNew()` as it calls that method
      ///       when the reading isn't stable. Call `IsPressedNew()` and only call this method
      ///       after `IsPressedNew()` returns `true` to see if the button is being held or 
      ///       after `IsPressedNew()` returns `false` to see if the button is pressed or not.
      /// @return True if button is currently pressed
      /// @see IsPressedRaw()
      /// @see IsPressedNew()
      /// @author Chris-70 (2025/09)
      bool IsPressed();

      /// @brief Check if the button is pressed(i.e. ON) without any debounce.
      /// @details This method reads the raw pin value and returns true if it is currently pressed.
      ///          No debouncing is done so the value may be unstable.  
      ///          This is a hardware read and doesn't update any state or internal values.
      ///          The methods `IsPressedNew()` or `IsPressed()` should be used for stable readings.
      /// @return True if button is currently pressed (ON), false otherwise
      /// @see IsPressed()
      /// @see IsPressedNew()
      /// @author Chris-70 (2025/09)
      bool IsPressedRaw() const;

      /// @brief Method to check if the button was pressed ON from OFF since the last call.
      /// @details This method was designed to be called frequently (every few milliseconds)
      ///          from the main loop as the UNO_R3 doesn't run FreeRTOS just calls `loop()`. 
      ///          It uses internal state tracking and debouncing to detect clean button press 
      ///          transitions from OFF to ON.
      /// @remarks The method `IsPressed()` will impact the state of `IsPressedNew()` as
      ///          it calls this method when the reading isn't stable, so if this
      ///          method is called after `IsPressed()` returns true, this method will 
      ///          return false as the state hasn't changed to PRESSED (i.e. ON).
      /// @return True if button was just pressed since last call, false otherwise
      /// @see IsPressed()
      /// @see ClearPressedNew()
      /// @author Chris-70 (2025/09)
      bool IsPressedNew();

      /// @brief Method to clear the pressed state so that the next call to `IsPressedNew()`
      ///        will return true if the button is currently pressed (ON).
      /// @author Chris-70 (2025/09)
      void ClearPressedNew();
      
      /// @brief Reset button state, clears all values. Useful for initialization.
      /// @author Chris-70 (2025/09)
      void Reset();

      /// @ingroup properties
      /// @{
      /// @brief Read only property pattern for `Pin` the GPIO pin number.
      /// @return GPIO pin number for this object.
      /// @author Chris-70 (2025/09)
      uint8_t get_Pin() const { return pin; }

      /// @brief Read only property pattern for `Value` the current digital 
      ///        value of the button pin.
      /// @return Current pin value (HIGH or LOW).
      /// @author Chris-70 (2025/09)
      uint8_t get_Value() const;

      /// @brief Read only property pattern for `OnValue` the value when button is pressed.
      /// @return Value when button is pressed (HIGH for CC or LOW for CA wired buttons).
      /// @author Chris-70 (2025/09)
      uint8_t get_OnValue() const { return onValue; }

      /// @brief Read only property pattern for `IsFirstRead` flag.
      /// @details This flag is true if the button has never been read before.
      /// @author Chris-70 (2025/09)
      bool get_IsFirstRead() { return lastReadTime == 0; }

      /// @brief Read only property pattern for `LastReadTime` the time (ms) of the last read.
      /// @remarks The time is in milliseconds as returned by `millis()`. This is a 32 bit 
      ///          value that rolls over every ~49 days. This is just a comparitor value.
      /// @return Time (ms) of the last read, 0 if never read.
      /// @author Chris-70 (2025/09)
      unsigned long get_LastReadTime() const { return lastReadTime; }

      /// @brief Property pattern for `BounceDelay` the current debounce delay in milliseconds.
      /// @param delay Debounce delay in milliseconds
      /// @see get_BounceDelay()
      /// @author Chris-70 (2025/09)
      static void set_BounceDelay(unsigned long delay) { bounceDelay = delay; }

      /// @brief Property pattern for `BounceDelay` the current debounce delay in milliseconds.
      /// @return Current debounce delay in milliseconds
      /// @see set_BounceDelay()
      /// @author Chris-70 (2025/09)
      static unsigned long get_BounceDelay() { return bounceDelay; }
      /// @}

   private:
      uint8_t pin;                    ///< GPIO pin number
      uint8_t onValue;                ///< Value when button is pressed
      uint8_t state;                  ///< Current button state
      uint8_t lastRead;               ///< Last physical read value
      unsigned long lastReadTime;     ///< Time of last state change
      unsigned long lastDebounceTime; ///< Time of last debounce

      static unsigned long bounceDelay; ///< Global debounce delay for all buttons
      };
   }

#endif