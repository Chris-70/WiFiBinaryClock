#include "RTClib.h"

#define DS1307_ADDRESS 0x68 ///< I2C address for DS1307
// DS1307 RTC register numbers:
#define DS1307_TIME                 0x00  ///< Time register start (0x00 - 0x06)
#define DS1307_SECONDS              0x00  ///< Seconds register address for DS1307
#define DS1307_MINUTES              0x01  ///< Minutes register address for DS1307
#define DS1307_HOUR                 0x02  ///< Hour register address for DS1307
#define DS1307_DAY                  0x03  ///< Day register address for DS1307
#define DS1307_DATE                 0x04  ///< Date register address for DS1307
#define DS1307_MONTH                0x05  ///< Month register address for DS1307
#define DS1307_YEAR                 0x06  ///< Year register address for DS1307
#define DS1307_CONTROL              0x07  ///< Control register
#define DS1307_NVRAM                0x08  ///< Start of SRAM registers - 56 bytes, 0x08 to 0x3f
// DS1307 Bit Numbers and Masks:
#define DS1307_SECONDS_CH_MASK      0x80  ///< Bit 7: Clock Halt (CH) bit in the seconds register (0x00)
#define DS1307_CONTROL_OUT_MASK     0x04  ///< Bit 2: Square Wave Output bit in the control register
#define DS1307_CONTROL_SQW_MASK     0x10  ///< Bit 4: Square Wave Enable bit in the control register
#define DS1307_CONTROL_RS_MASK      0x03  ///< MASK Bits 0-1: Rate Select bits (RS0, RS1) in the control register
#define DS1307_CONTROL_RS0_MASK     0x01  ///< MASK Bit 0: Rate Select bit (RS0) in the control register
#define DS1307_CONTROL_RS1_MASK     0x02  ///< MASK Bit 1: Rate Select bit (RS1) in the control register

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
            GET_12HR() - Converts the 12 hour AM/PM format to 24 hour format.
                         1) Converts BCD to hour (1 - 12).
                         2) Tests for PM ?
                            - True: Adds 12 hours if not noon (12 PM).
                            - False: Returns 0 for midnight (12 AM).
            ADD_PMHR() - Returns: 12 hours for PM times (1 PM to 11 PM), 0 for noon.
            IS_HR12()  - Tests if the BCD hour value is 12 (noon or midnight).
*/
/**************************************************************************/
#define GET_HOUR(BYTE_VAL) \
                (BYTE_VAL) & DS_HOUR_12_24_MASK    \
                              ? GET_12HR(BYTE_VAL) \
                              : bcd2bin((BYTE_VAL) & DS_HOUR24_MASK)
#define GET_12HR(BYTE_VAL) ((bcd2bin((BYTE_VAL) &  DS_HOUR12_MASK)) \
                           +  (((BYTE_VAL) & DS_HOUR_PM_MASK)       \
                              ? ADD_PMHR(BYTE_VAL)                  \
                              : (IS_HR12(BYTE_VAL) ? -12 : 0)))
#define ADD_PMHR(BYTE_VAL) (IS_HR12(BYTE_VAL) ? 0 : 12) // Add: 0 for Noon; 12 for 1 PM to 11 PM.
#define IS_HR12(BYTE_VAL) (((BYTE_VAL) & DS_HOUR12_MASK) == 0x12) // Test if the BCD hour is 12 (Noon or midnight)

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
               ? SET_12HR((BYTE_VAL) & DS_HOUR_REG_MASK)    \
               : (bin2bcd(BYTE_VAL) & DS_HOUR24_MASK))
// Set the hour in 12 hour format: 
//    bit 6 is set for 12 hour mode (i.e. 0x40), 
//    bit 5 is set 1 for PM (12 Noon, 1 - 11 PM), 0 for AM (1 - 12 AM).
//    bits 0 - 4 for the hour in BCD format, 1 - 12
#define SET_12HR(BYTE_VAL) (DS_HOUR_12_24_MASK | AMPM_BIT(BYTE_VAL) | TO_12HR(BYTE_VAL))
// Convert 24 hour format to 12 hour BCD format: 
//             0, 12   becomes 12; 
//             1 - 11  stays the same; 
//             13 - 23 becomes: 1 - 11.
#define TO_12HR(BYTE_VAL) bin2bcd(((BYTE_VAL) % 12 == 0) ? 12 : (BYTE_VAL) % 12) 
#define AMPM_BIT(BYTE_VAL) (((BYTE_VAL) >= 12) ? DS_HOUR_PM_MASK : 0x00)

/**************************************************************************/
/*!
    @brief  Start I2C for the DS1307 and test succesful connection
    @param  wireInstance pointer to the I2C bus
    @return True if Wire can find DS1307 or false otherwise.
*/
/**************************************************************************/
bool RTC_DS1307::begin(TwoWire *wireInstance) {
  if (i2c_dev)
    delete i2c_dev;
  i2c_dev = new Adafruit_I2CDevice(DS1307_ADDRESS, wireInstance);
  if (!i2c_dev->begin())
    return false;
  return true;
}

/**************************************************************************/
/*!
    @brief  Is the DS1307 running? Check the Clock Halt bit in register 0
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
uint8_t RTC_DS1307::isrunning(void) { return !(read_register(0) >> 7); }

/**************************************************************************/
/*!
    @brief  Get the current 12/24 hour mode for the Time & alarms.
    @return True if in 12 hour mode, false if in 24 hour mode
    @details This function will read the RTC's time format hour register
             If the current time format is 12 hour then it returns true.
*/
/**************************************************************************/
bool RTC_DS1307::getIs12HourMode()
   {
   uint8_t buffer = DS1307_HOUR;     // Hour register number
   i2c_dev->read(&buffer, 1, false); // Read the hour register
   // Check the 12 hour mode flag, bit 6 (0x40), for 12 hour mode
   return (buffer & DS_HOUR_12_24_MASK) != 0;
   }

/**************************************************************************/
/*!
   @brief  Set the current 12/24 hour mode for the RTC
   @param  value True for 12 hour mode, false for 24 hour mode
   @details This function will set the RTC's time format hour register
            If the current time format is 12 hour then it sets the mode to 24 hour.
            If the current time format is 24 hour then it sets the mode to 12 hour.
            The function will write the date and time in the format specified by the
            'value' parameter.
*/
/**************************************************************************/
void RTC_DS1307::setIs12HourMode(bool value)
   {
   bool curMode12 = getIs12HourMode();
   if (curMode12 != value)
      {
      DateTime dt = now(); // Get current date/time
      adjust(dt, value);   // Set the date/time in the new mode
      }
   }

/**************************************************************************/
/*!
    @brief  Set the date and time in the DS1307
    @param dt DateTime object containing the desired date/time
    @param use12HourFormat True to set the time in 12 hour format, false for 24 hour format
    @details This function will set the date and time in the DS1307.
             The 12 hour mode is indicated by bit 6 of the hour register.
             In 12 hour mode, bit 5 indicates PM, add 12 hours to result (if not noon).
             In 24 hour mode, bits 4 and 5 are used for 10 and 20 hours.
             The function will write the date and time in the format specified by the
             'use12HourFormat' parameter.
*/
/**************************************************************************/
void RTC_DS1307::adjust(const DateTime &dt) 
               { adjust(dt, getIs12HourMode()); }

void RTC_DS1307::adjust(const DateTime &dt, bool use12HourFormat) {
  uint8_t buffer[8] = {0,
              (uint8_t)(bin2bcd(dt.second() % 60)            & DS_SECONDS_MASK), // CH bit = 0
              (uint8_t)(bin2bcd(dt.minute() % 60)            & DS_MINUTES_MASK), // 0-60
              (uint8_t)(SET_HOUR(dt.hour(), use12HourFormat) & DS_HOUR_REG_MASK),// 00-23 or 1-12 + AM/PM
              (uint8_t)(bin2bcd(dt.dayOfTheWeek() + 1)       & DS_DAY_MASK),     // (0-6) +1 => (1-7)
              (uint8_t)(bin2bcd(dt.day() % (31 + 1))         & DS_DATE_MASK),    // (1-31)
              (uint8_t)(bin2bcd(dt.month() % (12 + 1))       & DS_MONTH_MASK),   // (1-12)
              (uint8_t)(bin2bcd(dt.year() % 100U)            & DS_YEAR_MASK)};   // (0-99)
  i2c_dev->write(buffer, 8);
}

/**************************************************************************/
/*!
    @brief  Get the current date and time from the DS1307
    @return DateTime object containing the current date and time
*/
/**************************************************************************/
DateTime RTC_DS1307::now() {
  uint8_t buffer[7];
  buffer[0] = DS1307_TIME;
  i2c_dev->write_then_read(buffer, 1, buffer, 7);

  return DateTime(bcd2bin(buffer[6]) + 2000U,             // Year, 2000-2099
                  bcd2bin(buffer[5]  & DS_MONTH_MASK),    // Month,  1-12
                  bcd2bin(buffer[4]  & DS_DATE_MASK),     // Day,    1-31
                  GET_HOUR(buffer[2] & DS_HOUR_REG_MASK), // Hour,   0-23
                  bcd2bin(buffer[1]  & DS_MINUTES_MASK),  // Minute, 0-59
                  bcd2bin(buffer[0]  & DS_SECONDS_MASK)); // Second, 0-59
}

/**************************************************************************/
/*!
    @brief  Read the current mode of the SQW pin
    @return Mode as Ds1307SqwPinMode enum
*/
/**************************************************************************/
Ds1307SqwPinMode RTC_DS1307::readSqwPinMode() {
  return static_cast<Ds1307SqwPinMode>(read_register(DS1307_CONTROL) & 0x93);
}

/**************************************************************************/
/*!
    @brief  Change the SQW pin mode
    @param mode The mode to use
*/
/**************************************************************************/
void RTC_DS1307::writeSqwPinMode(Ds1307SqwPinMode mode) {
  write_register(DS1307_CONTROL, mode);
}

/**************************************************************************/
/*!
    @brief  Read data from the DS1307's NVRAM
    @param buf Pointer to a buffer to store the data - make sure it's large
   enough to hold size bytes
    @param size Number of bytes to read
    @param address Starting NVRAM address, from 0 to 55
*/
/**************************************************************************/
void RTC_DS1307::readnvram(uint8_t *buf, uint8_t size, uint8_t address) {
  uint8_t addrByte = DS1307_NVRAM + address;
  i2c_dev->write_then_read(&addrByte, 1, buf, size);
}

/**************************************************************************/
/*!
    @brief  Write data to the DS1307 NVRAM (Battery backed SRAM)
    @param address Starting NVRAM address, from 0 to 55
    @param buf Pointer to buffer containing the data to write
    @param size Number of bytes in buf to write to NVRAM
*/
/**************************************************************************/
void RTC_DS1307::writenvram(uint8_t address, const uint8_t *buf, uint8_t size) {
  uint8_t addrByte = DS1307_NVRAM + address;
  i2c_dev->write(buf, size, true, &addrByte, 1);
}

/**************************************************************************/
/*!
    @brief  Shortcut to read one byte from NVRAM
    @param address NVRAM address, 0 to 55
    @return The byte read from NVRAM
*/
/**************************************************************************/
uint8_t RTC_DS1307::readnvram(uint8_t address) {
  uint8_t data;
  readnvram(&data, 1, address);
  return data;
}

/**************************************************************************/
/*!
    @brief  Shortcut to write one byte to NVRAM
    @param address NVRAM address, 0 to 55
    @param data One byte to write
*/
/**************************************************************************/
void RTC_DS1307::writenvram(uint8_t address, uint8_t data) {
  writenvram(address, &data, 1);
}
