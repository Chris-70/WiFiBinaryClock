/// @file DummyBinaryClock.cpp
/// @brief Implementation of DummyBinaryClock class
/// @author Chris-70 (2025/11)

#include "DummyBinaryClock.h"

namespace BinaryClockShield
   {
   DummyBinaryClock::DummyBinaryClock()
         : currentTime(DateTime(2025, 11, 15, 12, 0, 0))
         , currentAlarm()
         , is12HourFormat(false)
         , dummyButton(0, CC_ON)  // Dummy button on pin 0, CC wiring
      {
      strcpy(timeFormat, "hh:mm:ss");
      strcpy(alarmFormat, "hh:mm");

      currentAlarm.number = 0;
      currentAlarm.clear();
      currentAlarm.time = DateTime(2025,8,5, 15,19,7);
      currentAlarm.freq = AlarmTime::Weekly;
      }

      // Time property
   void DummyBinaryClock::set_Time(DateTime value)
      {
      currentTime = value;
      }

   DateTime DummyBinaryClock::get_Time() const
      {
      return currentTime;
      }

      // Alarm property
   void DummyBinaryClock::set_Alarm(AlarmTime value)
      {
      currentAlarm = value;
      }

   AlarmTime DummyBinaryClock::get_Alarm() const
      {
      return currentAlarm;
      }

      // 12-hour format property
   void DummyBinaryClock::set_Is12HourFormat(bool value)
      {
      is12HourFormat = value;
      }

   bool DummyBinaryClock::get_Is12HourFormat() const
      {
      return is12HourFormat;
      }

      // Format properties
   char* const DummyBinaryClock::get_TimeFormat() const
      {
      return const_cast<char*>(timeFormat);
      }

   char* const DummyBinaryClock::get_AlarmFormat() const
      {
      return const_cast<char*>(alarmFormat);
      }

      // Serial properties
   bool DummyBinaryClock::get_IsSerialSetup() const
      {
      return false;
      }

   bool DummyBinaryClock::get_IsSerialTime() const
      {
      return false;
      }

      // Button properties
   const BCButton& DummyBinaryClock::get_S1TimeDec() const
      {
      return dummyButton;
      }

   const BCButton& DummyBinaryClock::get_S2SaveStop() const
      {
      return dummyButton;
      }

   const BCButton& DummyBinaryClock::get_S3AlarmInc() const
      {
      return dummyButton;
      }

      // Time reading
   DateTime DummyBinaryClock::ReadTime()
      {
      return currentTime;
      }

      // Display operations
   void DummyBinaryClock::DisplayLedPattern(LedPattern patternType)
      {
         // Do nothing - dummy implementation
      (void)patternType;  // Suppress unused parameter warning
      }

   void DummyBinaryClock::DisplayBinaryTime(int hours, int minutes, int seconds, bool use12Hour)
      {
         // Do nothing - dummy implementation
      (void)hours;
      (void)minutes;
      (void)seconds;
      (void)use12Hour;
      }

      // Time callback registration
   bool DummyBinaryClock::RegisterTimeCallback(void (*callback)(const DateTime&))
      {
      (void)callback;
      return true;  // Always succeed
      }

   bool DummyBinaryClock::UnregisterTimeCallback(void (*callback)(const DateTime&))
      {
      (void)callback;
      return true;  // Always succeed
      }

      // Alarm callback registration
   bool DummyBinaryClock::RegisterAlarmCallback(void (*callback)(const DateTime&))
      {
      (void)callback;
      return true;  // Always succeed
      }

   bool DummyBinaryClock::UnregisterAlarmCallback(void (*callback)(const DateTime&))
      {
      (void)callback;
      return true;  // Always succeed
      }

      // Utility
   void DummyBinaryClock::PlayAlarm(const AlarmTime& alarm) const
      {
         // Do nothing - dummy implementation
      (void)alarm;
      }

   #if STL_USED
   bool DummyBinaryClock::PlayMelody(size_t id) const
      {
      (void)id;
      return true;  // Always succeed
      }

   size_t DummyBinaryClock::RegisterMelody(const std::vector<Note>& melody)
      {
      (void)melody;
      return 0;  // Return default melody ID
      }

   const std::vector<Note>& DummyBinaryClock::GetMelodyById(size_t id) const
      {
      (void)id;
      static std::vector<Note> emptyMelody;
      return emptyMelody;
      }
   #endif

   } // namespace BinaryClockShield