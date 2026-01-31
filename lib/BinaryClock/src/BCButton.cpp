/// @file BCButton.cpp
/// @brief This file contains the implementation of the `BCButton` class.
/// @author Chris-70 (2025/09)

#include <BinaryClock.Defines.h>       /// BinaryClock project-wide definitions and MACROs.
#include "BCButton.h"

namespace BinaryClockShield
   {
   // Initialize static member
   unsigned long BCButton::bounceDelay = DEFAULT_DEBOUNCE_DELAY;

   BCButton::BCButton(uint8_t pin, uint8_t onValue) 
         : pin(pin)
         , onValue(onValue)
         , state(onValue == CC_ON ? CC_OFF : CA_OFF)
         , lastRead(onValue == CC_ON ? CC_OFF : CA_OFF)
         , lastReadTime(0)
         , lastDebounceTime(0)
      { }

   void BCButton::Initialize()
      {
      pinMode(pin, (onValue == HIGH) ? ESP32_INPUT_PULLDOWN : INPUT_PULLUP);
      Reset();
      }

   bool BCButton::IsPressedRaw() const
      {
      return get_Value() == onValue;
      }

   bool BCButton::IsPressed()
      {
      bool result = false;
      uint8_t curValue = get_Value();

      // If the hasn't been read before OR the state and lastRead are different
      // indicates that the button hasn't been debounced. Call IsPressedNew() to
      // debounce the value and get a stable reading.
      if ((lastReadTime == 0) || (state != curValue) || (state != lastRead))
         {
         result = IsPressedNew();
         }
      else
         {
         result = (state == onValue);  // && (state == lastRead);
         }

      return result;
      }

   bool BCButton::IsPressedNew()
      {
      #ifdef UNO_R3
      // Simplified debounce for UNO_R3 (~100 bytes savings)
      bool result = false;
      uint8_t currentRead = get_Value();
      unsigned long currentTime = millis();

      if (currentRead != lastRead) {
         lastDebounceTime = currentTime;
         lastRead = currentRead;
      }

      if ((currentTime - lastDebounceTime) > bounceDelay && currentRead != state) {
         state = currentRead;
         lastReadTime = currentTime;
         result = (state == onValue);
      }

      return result;
      #else
      // Full debounce logic for other boards
      bool result = false;
      int currentRead = get_Value();
      unsigned long currentTime = millis();

      // Handle first read - if this is the very first call and button is pressed
      if ((lastReadTime == 0) && (currentRead == onValue))
         {
         state = !currentRead;  // Set opposite state to force state change detection
         lastRead = currentRead;
         }

      // Check for state change - this resets the debounce timer
      if (currentRead != lastRead)
         {
         lastDebounceTime = currentTime;
         }

      // Check if debounce period has passed and we have a stable reading
      if ((currentTime - lastDebounceTime) > bounceDelay)
         {
         // Check if the stable reading represents a state change
         if (currentRead != state)
            {
            state = currentRead;           // Update our tracked state
            lastReadTime = currentTime;    // Record when this state change occurred

            // Return true only if the new state is "pressed"
            if (state == onValue)
               {
               result = true;
               }
            }
         }

      // Always update lastRead for next iteration
      lastRead = currentRead;
      return result;
      #endif
      }

   uint8_t BCButton::get_Value() const
      {
      return digitalRead(pin);
      }

   void BCButton::Reset()
      {
      uint8_t currentValue = get_Value();
      state = currentValue;
      lastRead = currentValue;
      lastReadTime = 0;
      lastDebounceTime = 0;
      }

   void BCButton::ClearPressedNew()
      {
      if (IsPressed())
         {
         Reset();
         lastReadTime = millis();
         }
      }
   }