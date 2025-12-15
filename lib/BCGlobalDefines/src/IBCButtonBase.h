
/// @file IBCButtonBase.h
/// @brief Interface for button functionality in Binary Clock.
/// @details This interface defines the contract for button implementations including
///          state management, debouncing, and hardware interaction.
/// @author Chris-70 (2025/11)

#pragma once
#ifndef __IBCBUTTONBASE_H__
#define __IBCBUTTONBASE_H__

#include <stdint.h>                    /// Integer types: size_t; uint8_t; uint16_t; etc.

#ifndef LOW
   #define LOW                    0u   ///< Digital LOW value if not already defined, avoid redefinition warnings
#endif
#ifndef HIGH
   #define HIGH                   1u   ///< Digital HIGH value if not already defined, avoid redefinition warnings
#endif

// Constants for Common Anode (CA) and Common Cathode (CC) button wiring
// A CA wired button is connected HIGH and pulled LOW when pressed.
// A CC wired button is connected LOW and pulled HIGH when pressed.
#define CA_ON                    LOW   ///< The value when ON  for CA connections
#define CC_ON                   HIGH   ///< The value when ON  for CC connections
#define CA_OFF                  HIGH   ///< The value when OFF for CA connections
#define CC_OFF                   LOW   ///< The value when OFF for CC connections

namespace BinaryClockShield
{
   /// @brief Interface for button functionality
   /// @details Defines the contract for button implementations that handle reading,
   ///          debouncing, and state management. Buttons can be wired as Common Cathode (CC)
   ///          or Common Anode (CA).
   /// @remarks CC buttons connect the pin to HIGH when pressed (onValue = HIGH).
   ///          CA buttons connect the pin to LOW when pressed (onValue = LOW).
   /// @author Chris-70 (2025/11)
   class IBCButtonBase
   {
   public:
      /// @brief Virtual destructor
      virtual ~IBCButtonBase() = default;

      /// @brief Initialize the button pin with appropriate pull-up/pull-down settings
      virtual void Initialize() = 0;

      /// @brief Check if button is currently pressed (held down) after debounce
      /// @details This method debounces the button and returns true if it is currently pressed.
      ///          If the current value isn't stable, it calls IsPressedNew() to get a
      ///          stable reading. This requires that the method is called a second time,
      ///          at least BounceDelay milliseconds later, for a stable reading.
      /// @note This method can impact the state of IsPressedNew() as it calls that method
      ///       when the reading isn't stable.
      /// @return True if button is currently pressed
      /// @see IsPressedNew()
      virtual bool IsPressed() = 0;

      /// @brief Check if the button is pressed without any debounce
      /// @details This method reads the raw pin value and returns true if it is currently pressed.
      ///          No debouncing is done so the value may be unstable.
      ///          This is a hardware read and doesn't update any state or internal values.
      /// @return True if button is currently pressed (ON), false otherwise
      virtual bool IsPressedRaw() const = 0;

      /// @brief Check if the button was pressed ON from OFF since the last call
      /// @details This method uses internal state tracking and debouncing to detect clean
      ///          button press transitions from OFF to ON. This method will return 
      ///          false if the button state hasn't changed to OFF since the last call
      ///          (i.e. the button was not released between calls).
      /// @note The method `IsPressed()` can impact the state of `IsPressedNew()` as
      ///       it calls this method when the reading isn't stable.
      /// @return True if button was just pressed since last call, false otherwise
      /// @see IsPressed()
      virtual bool IsPressedNew() = 0;

      /// @brief Clear the pressed state
      /// @details After calling this, the next call to IsPressedNew() will return true
      ///          if the button is currently pressed (ON).
      virtual void ClearPressedNew() = 0;

      /// @brief Reset button state, clears all values
      virtual void Reset() = 0;

      /// @ingroup properties
      /// @{
      
      /// @brief Get the GPIO pin number
      /// @return GPIO pin number for this button
      virtual uint8_t get_Pin() const = 0;

      /// @brief Get the current digital value of the button pin
      /// @return Current pin value (HIGH or LOW)
      virtual uint8_t get_Value() const = 0;

      /// @brief Get the value when button is pressed
      /// @return Value when button is pressed (HIGH for CC or LOW for CA wired buttons)
      virtual uint8_t get_OnValue() const = 0;

      /// @brief Check if this is the first read
      /// @details This flag is true if the button has never been read before
      /// @return True if button has never been read
      virtual bool get_IsFirstRead() = 0;

      /// @brief Get the time (ms) of the last read
      /// @remarks The time is in milliseconds as returned by millis(). This is a 32 bit
      ///          value that rolls over every ~49 days. This is just a comparator value.
      /// @return Time (ms) of the last read, 0 if never read
      virtual unsigned long get_LastReadTime() const = 0;

      /// @}
   }; // class IBCButtonBase

} // namespace BinaryClockShield

#endif // __IBCBUTTONBASE_H__