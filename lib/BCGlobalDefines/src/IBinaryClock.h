
#pragma once
#ifndef __IBINARYCLOCK_H__
#define __IBINARYCLOCK_H__

// STL classes required to be included (when using the STL):
#include <vector> 

#include "IBinaryClockBase.h"

namespace BinaryClockShield
   {
   /// @brief Extended interface for BinaryClock functionality.
   /// @details This interface extends the `IBinaryClockBase` interface to provide
   ///          additional functionality specific to the `BinaryClock` class.
   ///          This interface can be used in places where the full functionality
   ///          of the `BinaryClock` class is required, while still allowing for
   ///          decoupling from the actual implementation.
   /// @note    This interface currently does not add any new methods beyond those
   ///          defined in `IBinaryClockBase`, but it serves as a placeholder for
   ///          future extensions and to provide clearer semantics in the code.
   /// @author Chris-70 (2026/01)
   class IBinaryClock : public IBinaryClockBase
      {
   public:
      /// @brief Required virtual destructor for proper memory management and release.
      virtual ~IBinaryClock() = default;

      /// @copydoc DisplayLedPattern(LedPattern patternType)
      /// @param   displayDuration The maximum duration to display the pattern in milliseconds.
      /// @remarks The display duration is only used to pause the binary time display.
      ///          Calling this method before the duration has expired will reset the timer
      ///          with the new value and will display the selected pattern overwriting the previous.
      /// @see DisplayLedPattern(LedPattern patternType)
      /// @author Chris-70 (2025/12)
      virtual void DisplayLedPattern(LedPattern patternType, unsigned long duration) = 0;
      
      /// @brief Play a specific melody by its registry id.
      /// @param id The id of the melody in the melodyRegistry to play.
      /// @return True if the id was valid and melody played, false if id was invalid.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      virtual bool PlayMelody(size_t id) const = 0;

      /// @brief Register a melody in the melody registry. 
      /// @remarks ID 0 is always the default melody stored in ROM (flash memory).  
      ///          The ID can be used as the alarm melody for a given alarm.
      /// @param melody A reference to the vector of Note objects to register.
      /// @return The ID of the registered melody in the registry.
      /// @see set_Alarm()
      /// @see set_Melody()
      /// @see PlayMelody(size_t id)
      /// @see GetMelodyById()
      /// @author Chris-70 (2025/09)
      virtual size_t RegisterMelody(const std::vector<Note>& melody) = 0;

      /// @brief Get a melody from the registry by its ID (returned from `RegisterMelody()`).
      /// @param id The id of the melody in the registry.
      /// @return A reference to the melody vector, or the default melody if id is invalid.
      /// @see RegisterMelody()
      /// @author Chris-70 (2025/09)
      virtual const std::vector<Note>& GetMelodyById(size_t id) const = 0;

      }; // class IBinaryClock
   }  // namespace BinaryClockShield
#endif // __IBINARYCLOCK_H__