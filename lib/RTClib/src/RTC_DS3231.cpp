#include "RTClib.h"

#define DS3231_ADDRESS              0x68  ///< I2C address for DS3231 RTC chip
// The DS3231 & DS3232 are very similar, so we can use the same code for both.
// The #defines that start with DS3232_ are for the DS3232 chip only and have the
// bit values set to 0 on the DS3231 chip. The OSF bit on the DS3231 is a status bit
// to signal the oscillator has stopped and can only be cleared, while on the DS3232 
// it is a control bit to enable/disable the oscillator. The DS3232 has a selectable
// temperature conversion rate, while the DS3231 has a fixed 10 seconds conversion rate.
// while on battery. On VCC, both chips have a 1 second conversion rate.
// From: (https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231.pdf)
//       (https://www.analog.com/media/en/technical-documentation/data-sheets/DS3232.pdf)
// DS3231 RTC register numbers:
#define DS3231_TIME                 0x00  ///< Time register start (0x00 - 0x06)
#define DS3231_SECONDS              0x00  ///< Seconds register address for DS3231
#define DS3231_MINUTES              0x01  ///< Minutes register address for DS3231
#define DS3231_HOUR                 0x02  ///< Hour register address for DS3231
#define DS3231_DAY                  0x03  ///< Day register address for DS3231
#define DS3231_DATE                 0x04  ///< Date register address for DS3231
#define DS3231_MONTH                0x05  ///< Month register address for DS3231
#define DS3231_YEAR                 0x06  ///< Year register address for DS3231
#define DS3231_ALARM1               0x07  ///< Alarm 1 register start (0x07 - 0x0A)
#define DS3231_ALARM1_SECONDS       0x07  ///< Alarm 1 seconds register address for DS3231
#define DS3231_ALARM1_MINUTES       0x08  ///< Alarm 1 minutes register address for DS3231
#define DS3231_ALARM1_HOUR          0x09  ///< Alarm 1 hour register address for DS3231
#define DS3231_ALARM1_DAY_DATE      0x0A  ///< Alarm 1 day/date register address for DS3231
#define DS3231_ALARM2               0x0B  ///< Alarm 2 register start (0x0B - 0x0E)
#define DS3231_ALARM2_MINUTES       0x0B  ///< Alarm 2 minutes register address for DS3231
#define DS3231_ALARM2_HOUR          0x0C  ///< Alarm 2 hour register address for DS3231
#define DS3231_ALARM2_DAY_DATE      0x0D  ///< Alarm 2 day/date register address for DS3231
#define DS3231_CONTROL              0x0E  ///< Control register
#define DS3231_STATUSREG            0x0F  ///< Status register
#define DS3231_AGING_OFFSET         0x10  ///< Aging offset register
#define DS3231_TEMPERATUREREG       0x11  ///< Temperature register start (0x11 MSB) - 0x12 (LSB)); scaled(2) 10 bit 2's complement
#define DS3231_TEMPERATURE_MSB      0x11  ///< Temperature MSB register, integer part (-128->+127 C), integer  8 bit 2's complement,
#define DS3231_TEMPERATURE_LSB      0x12  ///< Temperature LSB register, Fractional part (bits 7, 6): 0.00; 0.25; 0.50; 0.75;
// DS3231 Bit Numbers and Masks:
#define DS3231_CONTROL_A1IE_MASK    0x01  ///< Bit 0: Alarm 1 Interrupt Enable bit in control register
#define DS3231_CONTROL_A2IE_MASK    0x02  ///< Bit 1: Alarm 2 Interrupt Enable bit in control register
#define DS3231_CONTROL_INTCN_MASK   0x04  ///< Bit 2: Interrupt Control bit in control register
#define DS3231_CONTROL_RS1_MASK     0x08  ///< Bit 3: Rate Select bit 1 in control register (1 Hz; 1K)
#define DS3231_CONTROL_RS2_MASK     0x10  ///< Bit 4: Rate Select bit 2 in control register (4K  ; 8K)
#define DS3231_CONTROL_RATE_MASK    0x18  ///< Bits 3-4: Rate Select bits in control register (1 Hz; 1K; 4K; 8K)
#define DS3231_CONTROL_CONV_MASK    0x20  ///< Bit 5: Force Temperature Conversion (STATUS BSY flag must be 0)
#define DS3231_CONTROL_BBSQW_MASK   0x40  ///< Bit 6: Battery Backed Square Wave bit in control register
#define DS3231_CONTROL_EOSC_MASK    0x80  ///< Bit 7: Enable Oscillator bit in control register
#define DS3231_CONTROL_SQWMODE_MASK 0x1C  ///< Bits 2-4: Square Wave output mode bits in control register
#define DS3231_STATUS_A1F_MASK      0x01  ///< Bit 0: Alarm 1 Flag bit in status register
#define DS3231_STATUS_A2F_MASK      0x02  ///< Bit 1: Alarm 2 Flag bit in status register
#define DS3231_STATUS_BSY_MASK      0x04  ///< Bit 2: Busy Flag bit, temperature conversion (TCXO), in status register
#define DS3231_STATUS_EN32KHZ_MASK  0x08  ///< Bit 3: Enable 32kHz output bit in status register
#define DS3232_STATUS_CRATE0_MASK   0x10  ///< Bit 4: DS3232: Conversion Rate Select bit 0 (64,  256) in status register
#define DS3232_STATUS_CRATE1_MASK   0x20  ///< Bit 5: DS3232: Conversion Rate Select bit 1 (128, 512) in status register
#define DS3232_STATUS_BB32KHZ_MASK  0x40  ///< Bit 6: DS3232: Battery Backed 32kHz output bit in status register
#define DS3231_STATUS_OSF_MASK      0x80  ///< Bit 7: DS3231: Oscillator Stopped Flag bit in status register
#define DS3232_STATUS_EOSF_MASK     0x80  ///< Bit 7: DS3232: Enable Oscillator bit in status register
#define DS3231_ALARM1_STATUS_MASK   0x01  ///< Bit 1: Alarm 1 status Flag mask in control register
#define DS3231_ALARM2_STATUS_MASK   0x02  ///< Bit 2: Alarm 2 status Flag mask in control register
#define DS3231_ALARM1_FLAG_MASK     0x01  ///< Bit 1: Alarm 1 alarm triggered Mask in status register
#define DS3231_ALARM2_FLAG_MASK     0x02  ///< Bit 2: Alarm 2 alarm triggered Mask in status register
#define DS3231_ALARM1_DAY_DATE_MASK 0x80  ///< Bit 7: Alarm 1 day/date Flag bit in alarm register DAY/Date (0x0A)
#define DS3231_ALARM2_DAY_DATE_MASK 0x80  ///< Bit 7: Alarm 2 day/date Flag bit in alarm register DAY/Date (0x0D)
#define DS3231_ALARM1_A1M1_MASK     0x80  ///< Bit 7: Alarm 1 A1M1 Flag bit in alarm 1 register Seconds (0x07)
#define DS3231_ALARM1_A1M2_MASK     0x80  ///< Bit 7: Alarm 1 A1M2 Flag bit in alarm 1 register Minutes (0x08)
#define DS3231_ALARM1_A1M3_MASK     0x80  ///< Bit 7: Alarm 1 A1M3 Flag bit in alarm 1 register Hours (0x09)
#define DS3231_ALARM1_A1M4_MASK     0x80  ///< Bit 7: Alarm 1 A1M4 Flag bit in alarm 1 register Day (0x0A)
#define DS3231_ALARM1_A2M2_MASK     0x80  ///< Bit 7: Alarm 2 A2M2 Flag bit in alarm 2 register Minutes (0x0B)
#define DS3231_ALARM1_A2M3_MASK     0x80  ///< Bit 7: Alarm 2 A2M3 Flag bit in alarm 2 register Hours (0x0C)
#define DS3231_ALARM1_A2M4_MASK     0x80  ///< Bit 7: Alarm 2 A2M4 Flag bit in alarm 2 register Day (0x0D)
#define DS3231_CENTURY_MASK         0x80  ///< Bit 7: Century bit in month register (0x05)
#define DS3231_TEMP_LSB_MASK        0xC0  ///< Bit 7-6: Temperature LSB mask for DS3231, fractional part.
// Time reading masks for DS chips:
#define DS_SECONDS_MASK             0x7F  ///< Mask for seconds registers (0x00; 0x07)
#define DS_MINUTES_MASK             0x7F  ///< Mask for minutes registers (0x01; 0x08; 0x0B)
#define DS_HOUR_REG_MASK            0x7F  ///< Mask for all hour bits reg.(0x02; 0x09; 0x0C)
#define DS_HOUR_12_24_MASK          0x40  ///< Bit 6: 12/24 hour mode bit in hour registers
#define DS_HOUR_PM_MASK             0x20  ///< Bit 5: PM bit in hour registers (0x02; 0x09; 0x0C)
#define DS_HOUR24_MASK              0x3F  ///< Mask for 24 hours value    (0x02; 0x09; 0x0C) - 24 hour mode
#define DS_HOUR12_MASK              0x1F  ///< Mask for 12 hours value    (0x02; 0x09; 0x0C) - 12 hour mode
#define DS_DAY_MASK                 0x07  ///< Mask for day registers     (0x03; 0x0A; 0x0D)
#define DS_DATE_MASK                0x3F  ///< Mask for date registers    (0x04; 0x0A; 0x0D)
#define DS_MONTH_MASK               0x1F  ///< Mask for month registers   (0x05)
#define DS_YEAR_MASK                0xFF  ///< Mask for year registers    (0x06)

/**************************************************************************/
/*!
    @brief  Get the hour from the RTC in BCD encoded byte (bits 0 - 6)
            in either 12 or 24 format DS1307/3231 bit and BCD format
            convert it to 24 hour binary format. 
    @remarks The DS3231/1307 uses bit 6 as a flag for 12 hour format. 
             In 12 hour format, bit 5  indicates PM, add 12 hours to result (if not noon).
             In 24 hour format, bits 4 and 5 are used for 10 and 20 hours. 
             Test for 12/24 hour mode? Convert BCD to hour (1 - 12) then 
             test for PM and add 12 IFF hour isn't 12 (noon) 
             12 in BCD is 0x12, subtract 12 if it is midnight to get 00.
             If in 24 hour mode, just convert from BCD to hours (0 - 23).
    @param  BYTE_VAL The byte (bits 0 - 6) containing the hour (1 - 12) AM/PM or (0 - 23)
    @return The hour in 24-hour format (0 - 23)
    @note   This macro splits the work using other macros to reduce errors, 
            increase understanding and reduce maintenance costs.
           #GET_12HR() - Converts the 12 hour AM/PM format to 24 hour format.
                         1) Converts BCD to hour (1 - 12). Add:
                         2) Tests for PM ? 
                            - True: Adds 12 hours if not noon (12 PM).
                            - False: Returns 0 or -12 for midnight (12 AM).
           #ADD_PMHR() - Returns: 12 for PM times (1 PM to 11 PM), 0 for noon.
           #IS_HR12()  - Tests if the BCD hour value is 12 (noon or midnight).
*/
/**************************************************************************/
#define GET_HOUR(BYTE_VAL) \
                (BYTE_VAL) & DS_HOUR_12_24_MASK ? GET_12HR(BYTE_VAL) \
                                  : bcd2bin((BYTE_VAL) & DS_HOUR24_MASK)
#define GET_12HR(BYTE_VAL) ((bcd2bin((BYTE_VAL) & DS_HOUR12_MASK)) \
                           +  (((BYTE_VAL) & DS_HOUR_PM_MASK) \
                              ? ADD_PMHR(BYTE_VAL) \
                              : (IS_HR12(BYTE_VAL) ? -12 : 0)))
#define ADD_PMHR(BYTE_VAL) (IS_HR12(BYTE_VAL) ? 0 : 12) ///< Add: 0 for Noon; 12 for 1 PM to 11 PM.
#define IS_HR12(BYTE_VAL) (((BYTE_VAL) & DS_HOUR12_MASK) == 0x12) ///< Test if the BCD hour is 12 (Noon or midnight)

/**************************************************************************/
/*!
    @brief  Set the hour byte (bits 0-6) in BCD 12 or 24 hour formats, from the hour value 0-23.
    @param  BYTE_VAL The byte value of the hour (0-23);
    @param  FLAG_AMPM Flag: true result in 12 hour BCD/DS3231 bit format; false for 24 hour BCD value.
    @return The hour in the format indicated by the 'FLAG_AMPM' parameter, bit 7 is always 0.
    @remarks Bit 7 is always 0 in the result as the DS1307 family use this for the CH (clock halt) flag.
             The 12 hour DS3231/1307 format uses bit 6 as a flag for 12-hour format (1 - 12) with 
             bit 5 for the PM flag. The bits 0 - 4 are used for the hour (1 - 12) in BCD encoding.
             For 24 hour mode the result is simply the BCD value of the hour (0 - 23) and bit 6 == 0.
    @note   This macro splits the work using other macros to reduce errors,
            increase understanding and reduce maintenance costs.
            SET_HOUR() - Sets the hour in 12/24 hour BCD format.
                         Tests for 12 hour mode ? 
                            - True: Calls SET_12HR() to set the hour in 12 hour format.
                            - False: Returns the BCD value of the hour (0 - 23).
            SET_12HR() - Converts the 24 hour format to 12 hour format.
                           Sets: bit 6 for 12 hour mode; 
                                 bit 5 for PM or clear for AM; (AMPM_BIT())
                                 bits 0 - 4 for the BCD hour.  (TO_12HR())
            TO_12HR()  - Converts 24 hour value to 12 hour (1 - 12) format:
                           1) Tests if the hour is Noon (12 PM) or Midnight (12 AM).
                              - True: Sets the hour to 12.
                              - False: Sets the hour to {value % 12} (i.e. 1 - 11).
*/
/**************************************************************************/
#define SET_HOUR(BYTE_VAL, FLAG_AMPM) (uint8_t)((FLAG_AMPM) \
               ? SET_12HR((BYTE_VAL) & DS_HOUR_REG_MASK) \
               : (bin2bcd(BYTE_VAL) & DS_HOUR24_MASK))
// Set the hour in 12 hour format: 
//    bit 6 is set for 12 hour mode (i.e. 0x40), 
//    bit 5 is set 1 for PM (12 Noon, 1 - 11 PM), 0 for AM (1 - 12 AM).
//    bits 0 - 4 for the hour in BCD format, 1 - 12
#define SET_12HR(BYTE_VAL) (0x40 | AMPM_BIT(BYTE_VAL) | TO_12HR(BYTE_VAL))
// Convert 24 hour format to 12 hour BCD format: 
//             0, 12   becomes 12; 
//             1 - 11  stays the same; 
//             13 - 23 becomes: 1 - 11.
#define TO_12HR(BYTE_VAL) bin2bcd(((BYTE_VAL) % 12 == 0) ? 12 : (BYTE_VAL) % 12) 
#define AMPM_BIT(BYTE_VAL) (((BYTE_VAL) >= 12) ? 0x20 : 0x00)

/**************************************************************************/
/*!
    @brief  Start I2C for the DS3231 and test succesful connection
    @param  wireInstance pointer to the I2C bus
    @return True if Wire can find DS3231 or false otherwise.
*/
/**************************************************************************/
bool RTC_DS3231::begin(TwoWire *wireInstance) {
  if (i2c_dev)
    delete i2c_dev;
  i2c_dev = new Adafruit_I2CDevice(DS3231_ADDRESS, wireInstance);
  if (!i2c_dev->begin())
    return false;
  return true;
}

/**************************************************************************/
/*!
    @brief  Check the status register Oscillator Stop Flag to see if the DS3231
            stopped due to power loss
    @return True if the bit is set (oscillator stopped) or false if it is
            running
*/
/**************************************************************************/
bool RTC_DS3231::lostPower(void) {
  return (read_register(DS3231_STATUSREG) & 0x80) > 0;
}

/**************************************************************************/
/*!
    @brief  Set the date and clear the Oscillator Stop Flag
    @param dt DateTime object containing the date/time to set
    @param is12HourMode Flag: true store in 12-hour mode; false 24-hour mode.
    @remarks The DS3231 can store the time in 12 or 24 hour modes. 
             The 12 hour mode is indicated by bit 6 of the hour register.
*/
/**************************************************************************/
void RTC_DS3231::adjust(const DateTime& dt) 
               { adjust(dt, getIs12HourMode()); }
               
void RTC_DS3231::adjust(const DateTime &dt, bool use12HourMode) 
               { adjust(dt, use12HourMode, nullptr); }

uint8_t* RTC_DS3231::adjust(const DateTime& dt, bool use12HourMode, uint8_t* buf)
   {
  // if (!dt.isValid()) { return; } // Invalid date, do not set // *** DEBUG ***
    uint8_t buffer[8] = {DS3231_TIME,
             (uint8_t)(bin2bcd(dt.second() % 60)            & DS_SECONDS_MASK), // 0-59
             (uint8_t)(bin2bcd(dt.minute() % 60)            & DS_MINUTES_MASK), // 0-59
             (uint8_t)(SET_HOUR(dt.hour(), use12HourMode)   & DS_HOUR_REG_MASK),// 0-23 or 1-12
             (uint8_t)(bin2bcd(dt.dayOfTheWeek() + 1)       & DS_DAY_MASK),  // (0-6) +1 => (1-7)
             (uint8_t)(bin2bcd(dt.day() % (31 + 1))         & DS_DATE_MASK), // 1-31
            (uint8_t)((bin2bcd(dt.month() % (12 + 1))       & DS_MONTH_MASK) // 1-12
                                                            | (dt.year() < 2100U ? 0x00 : DS3231_CENTURY_MASK)),
             (uint8_t)(bin2bcd(dt.year() % 100U)            & DS_YEAR_MASK)   // 0-99
      };

  i2c_dev->write(buffer, 8);
   if (buf != nullptr) {
      memmove(buf, buffer, 8);
   }
  uint8_t statreg = read_register(DS3231_STATUSREG);
  statreg &= ~0x80; // force OSF bit to ON (0 == NOT STOPPED).
  write_register(DS3231_STATUSREG, statreg);
  return buf;
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object with the current date/time
*/
/**************************************************************************/
DateTime RTC_DS3231::now() {
  uint8_t buffer[7];
  buffer[0] = 0;
  i2c_dev->write_then_read(buffer, 1, buffer, 7);
  
  return DateTime(bcd2bin(buffer[6]) 
               + (buffer[5] & DS3231_CENTURY_MASK? 2100U :2000U), // Year    (2000 - 2199)
                  bcd2bin(buffer[5]  & DS_MONTH_MASK),            // Month   (1 - 12)
                  bcd2bin(buffer[4]  & DS_DATE_MASK),             // Day     (1 - 31)
                  GET_HOUR(buffer[2] & DS_HOUR_REG_MASK),         // Hours   (0 - 23)
                  bcd2bin(buffer[1]  & DS_MINUTES_MASK),          // Minutes (0 - 59)
                  bcd2bin(buffer[0]  & DS_SECONDS_MASK));         // Seconds (0 - 59)
}

/**************************************************************************/
/*!
    @brief  Read the SQW pin mode
    @return Pin mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
Ds3231SqwPinMode RTC_DS3231::readSqwPinMode() {
  int mode;
  mode = read_register(DS3231_CONTROL) & DS3231_CONTROL_SQWMODE_MASK;
  if (mode & DS3231_CONTROL_INTCN_MASK)
    mode = DS3231_OFF;
  return static_cast<Ds3231SqwPinMode>(mode);
}

/**************************************************************************/
/*!
    @brief  Set the SQW pin mode
    @param mode Desired mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
void RTC_DS3231::writeSqwPinMode(Ds3231SqwPinMode mode) {
  uint8_t ctrl = read_register(DS3231_CONTROL);

  ctrl &= ~DS3231_CONTROL_INTCN_MASK; // turn off INTerruptCoNtrol (Enable SQWave)
  ctrl &= ~DS3231_CONTROL_RATE_MASK;  // set freq bits to 00

  write_register(DS3231_CONTROL, ctrl | mode);
}

/**************************************************************************/
/*!
    @brief  Get the current temperature from the DS3231's temperature sensor
    @return Current temperature (float)
*/
/**************************************************************************/
float RTC_DS3231::getTemperature() {
  uint8_t buffer[2] = {DS3231_TEMPERATUREREG, 0};
  i2c_dev->write_then_read(buffer, 1, buffer, 2);
  return (float)buffer[0] + (buffer[1] >> 6) * 0.25f;
}

/**************************************************************************/
/*!
    @brief  Get the current temperature from the DS3231's temperature sensor
            as an integer, in degrees C.
    @details The DS3231 temperature sensor has a resolution of 0.25 degrees C.
             This method returns the integer part of the temperature only.
             The accuracy of the sensor is only +/- 3 degrees C, so reading
             just the integer portion is sufficient. The bonus is that if
             you remove the getTemperature() method, which returns a float
             and you don't use anyother floats in your program you can 
             compile a smaller program on chips that don't support any
             floating point in hardware and must import the software 
             libraries for floating point calculations (e.g. UNO R3).
    @return Current temperature (int) in degrees C.
*/
/**************************************************************************/
int RTC_DS3231::getIntTemperature() {
   uint8_t buffer = read_register(DS3231_TEMPERATUREREG);
   return (int8_t)buffer; // Return the temperature as signed integer
}
/**************************************************************************/
/*!
    @brief  Get the current 12/24 hour mode for the Time & alarms.
    @return True if in 12 hour mode, false if in 24 hour mode
    @details This function will read the RTC's time format hour register
             If the current time format is 12 hour then it returns true.
*/             
/**************************************************************************/
bool RTC_DS3231::getIs12HourMode()
   {
   uint8_t buffer = read_register(DS3231_HOUR); // Read the hour register
   // Check bit 6 (0x40) for 12 hour mode
   return (buffer & 0x40) != 0;
   }

/**************************************************************************/
/*!
    @brief  Set the 12/24 hour mode for the Time & alarms.
    @param  value True for 12 hour mode, false for 24 hour mode
    @details This function will update the RTC's time format and the format 
             of any stored alarms. If the current time format is different
             from the 'value' parameter, then the hour register for the 
             time and both alarms will be converted to the other format.
             If the current time format is the same as provided 'value'
             then nothing changes, the method returns.
    @design  The time format for the hour register is 12-hour (true) or 
             24-hour (false). The value needs to be converted and the
             corresponding bits need to be set/cleared meaning the register
             needs to be converted for each change.
        @par For an alarm to set the fired flag (A1F & A2F) the time
             registers must match the alarm registers up to the level set by
             the mode (e.g. seconds, minutes, hours, day/date). If the time
             format is different from the alarm format, then the hour register
             will never match and the alarm will never trigger. This is
             a method to 'turn OFF' an alarm without changing its time.
             For this reason, when we change the time mode we then will
             flip each of the alarm modes to preserve this logic. To
             change the alarm mode, we read the current alarm time and
             alarm modes, then call the setAlarm1(const DateTime&, Ds3231Alarm1Mode, bool) 
             or setAlarm2(const DateTime&, Ds3231Alarm2Mode, bool) and 
             set the desired new mode for the alarm.
*/
/**************************************************************************/
void RTC_DS3231::setIs12HourMode(bool value)
   {
   bool curMode12 = getIs12HourMode();
   if (curMode12 != value) 
      { 
      // Mode has changed, we need to update the time and both alarms to match.
      // For each of the stored time values: 
      //    1) Read the time values from the RTC
      //    2) Write the formatted time values back to the RTC in the new mode.
      //    3) For the alarm, read and write back the alarm mode without change.
      // Store the time back in the opposite Time format.
      DateTime dt = now(); // Get current date/time
      adjust(dt, value);   // Set the date/time in the new mode

      // Alarm1: Read the time; Read the mode; Store the alarm back in the new format.
      uint8_t buffer = DS3231_ALARM1_HOUR;
      // Read the current alarm 1 hour mode, get the current 12 hour mode value.
      uint8_t alarmMode12 = (i2c_dev->read(&buffer, 1, false) & 0x40) > 0;
      Ds3231Alarm1Mode mode1 = getAlarm1Mode();
      dt = getAlarm1(); // Get the current alarm 1 time
      // Flip the current alarm 1 time mode.
      setAlarm1(dt, mode1, !alarmMode12);

      // Alarm2: Read the time; Read the mode; Store the alarm back in the new format.
      buffer = DS3231_ALARM2_HOUR;
      alarmMode12 = (i2c_dev->read(&buffer, 1, false) & 0x40) > 0;
      Ds3231Alarm2Mode mode2 = getAlarm2Mode();
      dt = getAlarm2(); // Get the current alarm 2 time
      // Flip the current alarm 2 time mode.
      setAlarm2(dt, mode2, !alarmMode12);
      }
   }

/**************************************************************************/
/*!
    @brief  Set alarm 1 for DS3231
    @param 	alarmTime   DateTime object: for day of week alarm (i.e. every week)
                        set the 'day' to 1 - 7 for the weekday to match.
                        For date alarm, set the 'day' to 1 - 31 for the date
    @param 	alarm_mode Desired mode, see Ds3231Alarm1Mode enum
    @param is12HourMode Flag: true store in 12-hour mode; false 24-hour mode.
                              Defaults to the current time mode (Reg.2, bit 6).
    @return False if control register is not set, otherwise true
    @remarks The DS3231 days of the week are 1 - 7 (DateTime is: 0 - 6)
             The first day of the week is defined by 'WeekdayEpoch' where the
             the weekday for the 1st of the month (WeekdayEpoch.month()) is
             the same as the weekday that is start of the week. See the DateTime
             'WeekdayEpoch' variable in RTClib.h for a list of months in
             2000 where the 1st of the month is the first day of the week.
    @note   If the time mode (12/24) is different from the alarm time mode
            then the alarm will never trigger. For the alarm to trigger
            the time registers must match the alarm time registers up to
            the level set by the mode (e.g. seconds, minutes, hours, day/date).
            If the time mode is different from the alarm time mode, then the
            hour registers will never match and the alarm will be OFF.
            Call 'setIs12HourMode(bool)' to change all the time modes.
    @see setIs12HourMode(bool) to change the time mode for all alarms and time registers.
    @see getAlarm1Mode() to get the current alarm mode.
    @see getAlarm1() to get the current alarm time.
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm1(const DateTime& alarmTime, Ds3231Alarm1Mode alarm_mode) 
        { return setAlarm1(alarmTime, alarm_mode, getIs12HourMode()); }

bool RTC_DS3231::setAlarm1(const DateTime &alarmTime, Ds3231Alarm1Mode alarm_mode, bool use12HourMode) {
  if (!alarmTime.isValid()) { return false; } // Invalid date, do not set alarm
  
  uint8_t ctrl = read_register(DS3231_CONTROL);

  // Set the 'alarmTime' year and month so day of week and day of month match (1 - 7)
  DateTime dt = DateTime(DateTime::WeekdayEpoch.year(), DateTime::WeekdayEpoch.month(), alarmTime.day(),
                         alarmTime.hour(), alarmTime.minute(), alarmTime.second());
  uint8_t A1M1 = (alarm_mode & 0x01) << 7; // Seconds bit 7.
  uint8_t A1M2 = (alarm_mode & 0x02) << 6; // Minutes bit 7.
  uint8_t A1M3 = (alarm_mode & 0x04) << 5; // Hour bit 7.
  uint8_t A1M4 = (alarm_mode & 0x08) << 4; // Day/Date bit 7.
  uint8_t DY_DT = (alarm_mode & 0x10) << 2; // Day/Date bit 6. Date when 0, day of week when 1.
  uint8_t day = (DY_DT) ? (dt.dayOfTheWeek() + 1) : dt.day();

  uint8_t buffer[5] = {DS3231_ALARM1, uint8_t(bin2bcd(dt.second()) | A1M1),
                       uint8_t(bin2bcd(dt.minute()) | A1M2),
                       uint8_t(SET_HOUR(dt.hour(), use12HourMode) | A1M3),
                       uint8_t(bin2bcd(day) | A1M4 | DY_DT)};
  i2c_dev->write(buffer, 5);

  write_register(DS3231_CONTROL, ctrl | 0x01); // AI1E

  return true;
}

/**************************************************************************/
/*!
    @brief  Set alarm 2 for DS3231
        @param 	alarmTime  DateTime object: for day of week alarm (i.e. every week)
                             set the 'day' to 1 - 7 for the weekday to match.
                             For date alarm, set the 'day' to 1 - 31 for the date
        @param 	alarm_mode Desired mode, see Ds3231Alarm2Mode enum
        @param    is12HourMode Flag: true store in 12-hour mode; false 24-hour mode.
                                     Defaults to the current time mode (Reg.2, bit 6).
    @return False if control register is not set, otherwise true
    @remarks The DS3231 days of the week are 1 - 7 (DateTime is: 0 - 6)
             The first day of the week is defined by 'WeekdayEpoch' where the
             the weekday for the 1st of the month (WeekdayEpoch.month()) is 
             the same as the weekday that is start of the week. See the 'DateTime
             ::WeekdayEpoch' variable in RTClib.h for a list of months in
             2000 where the 1st of the month is the first day of the week.
    @note   If the time mode (12/24) is different from the alarm time mode
            then the alarm will never trigger. For the alarm to trigger
            the time registers must match the alarm time registers up to
            the level set by the mode (e.g. seconds, minutes, hours, day/date).
            If the time mode is different from the alarm time mode, then the
            hour registers will never match and the alarm will be OFF.
            Call 'setIs12HourMode(bool)' to change all the time modes.
    @see setIs12HourMode(bool) to change the time mode for all alarms and time registers.
    @see getAlarm2Mode() to get the current alarm mode.            
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm2(const DateTime& alarmTime, Ds3231Alarm2Mode alarm_mode) 
        { return setAlarm2(alarmTime, alarm_mode, getIs12HourMode()); }

bool RTC_DS3231::setAlarm2(const DateTime& alarmTime, Ds3231Alarm2Mode alarm_mode, bool use12HourMode) {
   if (!alarmTime.isValid()) { return false; } // Invalid date, do not set alarm
   uint8_t ctrl = read_register(DS3231_CONTROL);

  // Set the 'alarmTime' year and month so day of week and day of month match (1 - 7)
  DateTime dt = DateTime(DateTime::WeekdayEpoch.year(), DateTime::WeekdayEpoch.month(), alarmTime.day(),
                         alarmTime.hour(), alarmTime.minute(), alarmTime.second());
  uint8_t A2M2 = (alarm_mode & 0x01) << 7; // Minutes bit 7.
  uint8_t A2M3 = (alarm_mode & 0x02) << 6; // Hour bit 7.
  uint8_t A2M4 = (alarm_mode & 0x04) << 5; // Day/Date bit 7.
  uint8_t DY_DT = (alarm_mode & 0x08)
                  << 3; // Day/Date bit 6. Date when 0, day of week when 1.
  uint8_t day = (DY_DT) ? (dt.dayOfTheWeek() + 1) : dt.day();

  uint8_t buffer[4] = {DS3231_ALARM2, 
              (uint8_t)(bin2bcd(dt.minute()) | A2M2),
              (uint8_t)(SET_HOUR(dt.hour(), use12HourMode) | A2M3),
              (uint8_t)(bin2bcd(day) | A2M4 | DY_DT)};
  i2c_dev->write(buffer, 4);

  write_register(DS3231_CONTROL, ctrl | 0x02); // AI2E

  return true;
}

/**************************************************************************/
/*!
    @brief  Get the date/time value of Alarm1
    @return DateTime object with the Alarm1 data set in the
            day, hour, minutes, and seconds fields
    @see DateTime::DateTime::WeekdayEpoch for day of the week mapping
    @see #GET_HOUR() for hour conversion of 12/24 hr. mode
*/
/**************************************************************************/
DateTime RTC_DS3231::getAlarm1() {
  uint8_t buffer[5] = {DS3231_ALARM1, 0, 0, 0, 0};
  i2c_dev->write_then_read(buffer, 1, buffer, 5);

  uint8_t seconds = bcd2bin(buffer[0] & 0x7F);
  uint8_t minutes = bcd2bin(buffer[1] & 0x7F);
  // Fetching the hour (0-23) from buffer in 12 or 24 hour mode.
  uint8_t hour   = GET_HOUR(buffer[2] & 0x7F);

  // Determine if the alarm is set to fire based on the
  // day of the week, or an explicit date match.
  bool isDayOfWeek = (buffer[3] & 0x40) > 0; // boolean result of bit 6
  uint8_t day;
  if (isDayOfWeek) {
    // Alarm set to match on day of the week
    day = bcd2bin(buffer[3] & 0x0F);
  } else {
    // Alarm set to match on day of the month
    day = bcd2bin(buffer[3] & 0x3F);
  }
  
  // On the first week of May 2000, the day-of-the-week number
  // matches the date numbers for a Monday. For other starting days:
  // Use  (5) May       2000, 1 for Monday    and 7 for Sunday.
  // Use (10) October   2000, 1 for Sunday    and 7 for Saturday.
  // Use  (1) January   2000, 1 for Saturday  and 7 for Friday.
  // Use  (9) September 2000, 1 for Friday    and 7 for Thursday.
  // Use  (6) June      2000, 1 for Thursday  and 7 for Wednesday.
  // Use  (3) March     2000, 1 for Wednesday and 7 for Tuesday.
  // Use  (2) February  2000, 1 for Tuesday   and 7 for Monday.
  // The 'DateTime::WeekdayEpoch' variable is defined based on the
  // selected month defined by 'FIRST_WEEKDAY_MONTH' in 2000.
  return DateTime(2000, DateTime::WeekdayEpoch.month(), day, hour, minutes, seconds);
}

/**************************************************************************/
/*!
    @brief  Get the date/time value of Alarm2
    @return DateTime object with the Alarm2 data set in the
            day, hour, and minutes fields
    @remarks The seconds field is always 0 for Alarm2.
             The day field is set to the day of the week (1 - 7)
             or the day of the month (1 - 31) depending on the alarm mode.
    @see DateTime::WeekdayEpoch for day of the week mapping
*/
/**************************************************************************/
DateTime RTC_DS3231::getAlarm2() {
  uint8_t buffer[4] = {DS3231_ALARM2, 0, 0, 0};
  i2c_dev->write_then_read(buffer, 1, buffer, 4);

  uint8_t minutes = bcd2bin(buffer[0] & 0x7F);
  // Fetching the hour (0-23) from buffer in 12 or 24 hour format
  uint8_t hour   = GET_HOUR(buffer[1] & 0x7F);

  // Determine if the alarm is set to fire based on the
  // day of the week, or an explicit date match.
  bool isDayOfWeek = (buffer[2] & 0x40) > 0; // boolean result of bit 6
  uint8_t day;
  if (isDayOfWeek) {
    // Alarm set to match on day of the week
    day = bcd2bin(buffer[2] & 0x0F);
  } else {
    // Alarm set to match on day of the month
    day = bcd2bin(buffer[2] & 0x3F);
  }
  
  // On the first week of May 2000, the day-of-the-week number
  // matches the date numbers for a Monday. For other starting days:
  // Use  (5) May       2000, 1 for Monday    and 7 for Sunday.
  // Use (10) October   2000, 1 for Sunday    and 7 for Saturday.
  // Use  (1) January   2000, 1 for Saturday  and 7 for Friday.
  // Use  (9) September 2000, 1 for Friday    and 7 for Thursday.
  // Use  (6) June      2000, 1 for Thursday  and 7 for Wednesday.
  // Use  (3) March     2000, 1 for Wednesday and 7 for Tuesday.
  // Use  (2) February  2000, 1 for Tuesday   and 7 for Monday.
  // The 'DateTime::WeekdayEpoch' variable is defined based on the
  // selected month defined by 'FIRST_WEEKDAY_MONTH' in 2000.
  return DateTime(2000, DateTime::WeekdayEpoch.month(), day, hour, minutes, 0);
}

/**************************************************************************/
/*!
    @brief  Get the mode for Alarm1
    @return Ds3231Alarm1Mode enum value for the current Alarm1 mode
*/
/**************************************************************************/
Ds3231Alarm1Mode RTC_DS3231::getAlarm1Mode() {
  uint8_t buffer[5] = {DS3231_ALARM1, 0, 0, 0, 0};
  i2c_dev->write_then_read(buffer, 1, buffer, 5);

  uint8_t alarm_mode =   (buffer[0] & 0x80) >> 7  //  A1M1 - Seconds bit   (every second) (x1111)
                       | (buffer[1] & 0x80) >> 6  //  A1M2 - Minutes bit   (every minute) (x1110)
                       | (buffer[2] & 0x80) >> 5  //  A1M3 - Hour bit      (every hour)   (x1100)
                       | (buffer[3] & 0x80) >> 4  //  A1M4 - Day bit       (every day)    (x1000)
                       | (buffer[3] & 0x40) >> 2; // DY_DT - Day/Date flag (every week or month)

  // Determine which mode the fetched alarm bits map to
  switch (alarm_mode) {
  case DS3231_A1_PerSecond:
  case DS3231_A1_Second:
  case DS3231_A1_Minute:
  case DS3231_A1_Hour:
  case DS3231_A1_Date:
  case DS3231_A1_Day:
    return (Ds3231Alarm1Mode)alarm_mode;
  default:
    // Default if the alarm mode cannot be read
    return DS3231_A1_Date;
  }
}

/**************************************************************************/
/*!
    @brief  Get the mode for Alarm2
    @return Ds3231Alarm2Mode enum value for the current Alarm2 mode
*/
/**************************************************************************/
Ds3231Alarm2Mode RTC_DS3231::getAlarm2Mode() {
  uint8_t buffer[4] = {DS3231_ALARM2, 0, 0, 0};
  i2c_dev->write_then_read(buffer, 1, buffer, 4);

  uint8_t alarm_mode =   (buffer[0] & 0x80) >> 7  //  A2M2 - Minutes bit   (every minute) (x111)
                       | (buffer[1] & 0x80) >> 6  //  A2M3 - Hour bit      (every hour)   (x110)
                       | (buffer[2] & 0x80) >> 5  //  A2M4 - Day bit       (every day)    (x100)
                       | (buffer[2] & 0x40) >> 3; // DY_DT - Day/Date flag (every week or month)

  // Determine which mode the fetched alarm bits map to
  switch (alarm_mode) {
  case DS3231_A2_PerMinute:
  case DS3231_A2_Minute:
  case DS3231_A2_Hour:
  case DS3231_A2_Date:
  case DS3231_A2_Day:
    return (Ds3231Alarm2Mode)alarm_mode;
  default:
    // Default if the alarm mode cannot be read
    return DS3231_A2_Date;
  }
}

/**************************************************************************/
/*!
    @brief  Disable alarm
        @param 	alarm_num Alarm number to disable
   @remarks The alarm is 'disabled' by clearing the Alarm Interrupt Enable
            bit (A1IE or A2IE) in the control register for the alarm.
            This does not prevent the RTC from setting the Alarm Fired
            status bit (A1F or A2F) in the status register.
*/
/**************************************************************************/
void RTC_DS3231::disableAlarm(uint8_t alarm_num) {
  if (alarm_num < 1 || alarm_num > 2) return;
  // Disable the alarm by clearing the corresponding bit in the control register
  uint8_t ctrl = read_register(DS3231_CONTROL);
  ctrl &= ~((alarm_num == 1) ? DS3231_CONTROL_A1IE_MASK : DS3231_CONTROL_A2IE_MASK);
  write_register(DS3231_CONTROL, ctrl);
}

/**************************************************************************/
/*!
    @brief  Clear status of alarm
        @param 	alarm_num Alarm number to clear
*/
/**************************************************************************/
void RTC_DS3231::clearAlarm(uint8_t alarm_num) {
  if (alarm_num < 1 || alarm_num > 2) return;

  uint8_t status = read_register(DS3231_STATUSREG);
  status &= ~((alarm_num == 1) ? DS3231_STATUS_A1F_MASK : DS3231_STATUS_A2F_MASK);
  write_register(DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Get status of alarm
        @param 	alarm_num Alarm number to check status of
        @return True if alarm has been fired otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::alarmFired(uint8_t alarm_num) {
   return (read_register(DS3231_STATUSREG) & 
         (alarm_num == 1? DS3231_STATUS_A1F_MASK : DS3231_STATUS_A2F_MASK)) > 0;
}

/**************************************************************************/
/*!
    @brief  Enable 32KHz Output
    @details The 32kHz output is enabled by default. It requires an external
    pull-up resistor to function correctly
*/
/**************************************************************************/
void RTC_DS3231::enable32K(void) {
  uint8_t status = read_register(DS3231_STATUSREG);
  status |= (DS3231_STATUS_EN32KHZ_MASK);
  write_register(DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Disable 32KHz Output
*/
/**************************************************************************/
void RTC_DS3231::disable32K(void) {
  uint8_t status = read_register(DS3231_STATUSREG);
  status &= ~(DS3231_STATUS_EN32KHZ_MASK);
  write_register(DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Get status of 32KHz Output
    @return True if enabled otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::isEnabled32K(void) {
   return (read_register(DS3231_STATUSREG) & DS3231_STATUS_EN32KHZ_MASK) > 0;
}
