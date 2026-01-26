/// @file DummyBinaryClock.h
/// @brief A dummy implementation of IBinaryClockBase for testing and development purposes.
/// @details This class provides a no-op implementation of all IBinaryClockBase methods,
///          useful for testing code that depends on IBinaryClockBase without needing
///          actual hardware or full implementation.
/// @author Chris-70 (2025/11)

#pragma once
#ifndef __DUMMYBINARYCLOCK_H__
#define __DUMMYBINARYCLOCK_H__


#include <IBinaryClock.h>

namespace BinaryClockShield
{
   /// @brief Dummy implementation of IBinaryClock that does nothing
   /// @details All methods are implemented but perform no actual operations.
   ///          This is useful for testing and development when you need a valid
   ///          IBinaryClock object but don't need actual functionality.
   class DummyBinaryClock : public IBinaryClock
   {
   private:
      DateTime currentTime;
      AlarmTime currentAlarm;
      bool is12HourFormat;
      char timeFormat[32];
      char alarmFormat[32];
      IBCButtonBase& dummyButton;

   public:
      DummyBinaryClock();
      virtual ~DummyBinaryClock() = default;

      // Time property
      virtual void set_Time(DateTime value) override;
      virtual DateTime get_Time() const override;

      // Alarm property
      virtual void set_Alarm(AlarmTime value) override;
      virtual AlarmTime get_Alarm() const override;

      // 12-hour format property
      virtual void set_Is12HourFormat(bool value) override;
      virtual bool get_Is12HourFormat() const override;

      // Format properties
      virtual char* const get_TimeFormat() const override;
      virtual char* const get_AlarmFormat() const override;

      // Serial properties
      virtual bool get_IsSerialSetup() const override;
      virtual bool get_IsSerialTime() const override;

      // Button properties
      virtual const IBCButtonBase& get_S1TimeDec() const override;
      virtual const IBCButtonBase& get_S2SaveStop() const override;
      virtual const IBCButtonBase& get_S3AlarmInc() const override;

      virtual const char* get_IdName() const override { return IBinaryClock_IdName; }

      // Time reading
      virtual DateTime ReadTime() override;

      // Display operations
      virtual void DisplayLedPattern(LedPattern patternType) override;
      virtual void DisplayBinaryTime(int hours, int minutes, int seconds, bool use12Hour = false) override;

      // Time callback registration
      virtual bool RegisterTimeCallback(void (*callback)(const DateTime&)) override;
      virtual bool UnregisterTimeCallback(void (*callback)(const DateTime&)) override;

      // Alarm callback registration
      virtual bool RegisterAlarmCallback(void (*callback)(const DateTime&)) override;
      virtual bool UnregisterAlarmCallback(void (*callback)(const DateTime&)) override;

      // Utility
      virtual void PlayAlarm(const AlarmTime& alarm) const override;

      #if STL_USED
      virtual bool PlayMelody(size_t id) const override;
      virtual size_t RegisterMelody(const std::vector<Note>& melody) override;
      virtual const std::vector<Note>& GetMelodyById(size_t id) const override;
      #endif

   private:
      static constexpr const char* IBinaryClock_IdName = "DummyBinaryClock";
   };

} // namespace BinaryClockShield

#endif // __DUMMYBINARYCLOCK_H__