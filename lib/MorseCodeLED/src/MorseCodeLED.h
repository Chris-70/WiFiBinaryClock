

#pragma once
#ifndef __MORSE_CODE_LED_H__
#define __MORSE_CODE_LED_H__

#include <Arduino.h>

#include "BinaryClock.Defines.h"    // Needed for the current board definitions.

namespace BinaryClockShield
   {
   /// @brief Class to flash an LED in Morse Code. Primary, and original, use is to flash 
   ///        an error message when the RTC is not available. The class is truncated when the
   ///        UNO R3 board is used, as it does not have enough resources to include the
   ///        full Morse code functionality. It can only flash the message "CQD NO RTC".
   /// @remarks The Morse code implementation is basic, it does not support all the
   ///         nuances of real Morse code. It is intended to be a simple way to flash
   ///         an LED in Morse code for basic messages. The timing is based on a dot
   ///         duration of 200ms, with a dash being 3 times the dot duration. The space
   ///         between dots and dashes in a character is 1 dot duration, the space
   ///         between characters is 3 dot durations. The space between words is 7 dot durations.
   ///         The implementation supports flashing predefined messages, such as "CQD NO RTC"
   ///         and also supports flashing arbitrary strings of text.
   ///         The class also supports a few prosigns, such as "AR" (end of message) and "SK" (end of contact).
   ///         The class can be extended to support more prosigns if needed.
   /// @design The Morse code patterns are stored in a lookup table for letters A-Z and numbers 0-9.
   ///         The patterns are stored as a bit pattern, with the length of the pattern also stored.
   ///         This was my first attempt at using CoPilot to generate code, and the results were mixed.
   ///         some code was good, for example packing each Morse code "character" in to a 16-bit value
   ///         with the length in the upper 4 bits and the pattern in the lower 12 bits. However, when
   ///         I asked it to generate the lookup table for the characters, it made several mistakes over 
   ///         and over again. None of the code was robust, and there was no error checking. I had to
   ///         rewrite most of the code to make it work properly. I also added the ability to flash
   ///         arbitrary strings of text, which was not in the original code. Overall, it was a good
   ///         learning experience, but I would not rely on CoPilot to generate production code.
   /// @author Chris-70 (2025/08)
   class MorseCodeLED
      {
   public:
      /// @brief Enumeration of the Morse code components, used to define the Morse code patterns.
      /// @details The enumeration defines the different components of Morse code:
      ///         - Dot: A short signal (dit).
      ///         - Dash: A long signal (dah).
      ///         - Space: A space between letters.
      ///         - Word: A space between words.
      ///         - EndMarker: A marker to indicate the end of the Morse code sequence.
      enum class MC : uint8_t
         {
         Dot = 0,          // Dot (dit)
         Dash = 1,         // Dash (dah)
         Space = 2,        // Letter space
         Word = 3,         // Word space
         EndMarker = 255   // End marker
         };

      /// @brief Enumeration of the ProSigns (Procedural Signals) used in Morse code. 
      ///        Some of these prosigns are duplicates but are included as different symbols.
      /// @details ProSigns are special Morse code sequences that have specific meanings.
      ///         - Start: Start of transmission.
      ///         - End: End of message (Identical to 'AR').
      ///         - Wait: Wait for response
      ///         - Invite: Invitation to transmit (Identical to 'K').
      ///         - AR: End of message (Identical to 'End').
      ///         - AS: Wait for response.
      ///         - K: Invitation to transmit (Identical to 'Invite').
      ///         - R: Received OK.
      ///         - C: Call to specific station.
      ///         - SK: End of contact.
      ///         - BT: New paragraph, separator.
      ///         - SOS: Distress signal.
      ///         - Error: Error signal.
      ///         - EndMark: End of command list.
      enum class ProSign : uint8_t
         {
         Start     ,   // -.-.-     (len=5, pattern=01010)     []    Start of transmission
         End       ,   // .-.-.     (len=5, pattern=11001)     [AR]  End of message
         EndWork   ,   // ...-.-    (len=6, pattern=000101)    [SK]  End of contact / work
         FullStop  ,   // .-.-.-    (len=6, pattern=00100)     [AS]  Full stop (period)
         Invite    ,   // -.-       (len=3, pattern=101)       [K]   Invitation to transmit
         Understood,   // ...-.     (len=5, pattern=00010)     []    Understood
         AR        ,   // .-.-.     (len=5, pattern=01010)     [AR]  End of message
         AS        ,   // .-...     (len=5, pattern=01000)     [AS]  Wait for response
         K         ,   // -.-       (len=3, pattern=101)       [K]   Invitation to transmit
         R         ,   // .-.       (len=3, pattern=010)       [R]   Received OK
         C         ,   // -.-.      (len=4, pattern=1010)      [C]   Call to specific station
         SK        ,   // ...-.-    (len=5, pattern=00101)     [SK]  End of contact
         BT        ,   // -...-     (len=5, pattern=10001)     [BT]  New paragraph, separator
         SOS       ,   // ...---... (len=9, pattern=000111000) [SOS] Distress signal
         Error     ,   // ........  (len=8, pattern=00000000) [Error] Error signal  
         EndMark = 0xFF   // End of command list
         };

      /// @brief Structure to hold the Morse code pattern and its length.
      /// @details The structure holds a 16-bit pattern which is composed of
      ///          a 12-bit Morse code pattern and a 4-bit length field.
      ///          The union of a bit structure and a uint16_t allows easy access
      ///          to the individual components of the Morse code pattern.
      union MCode
         {
         uint16_t pattern;
         struct
            {
            uint16_t code : 12;  // [00-11] Morse code pattern (max 12 bits)
            uint16_t len  :  4;  // [12-15] Length of the Morse code sequence (max 12)
            };

         /// @brief Constructor to initialize the MCode with a length and code.
         /// @param len The length of the Morse code sequence (0-12).
         /// @param code The Morse code pattern (0-4095).
         /// @note The length is forced to be within 0-11 to avoid overflow and
         ///       ensure it can't cause memory access outside the integer.
         MCode(uint8_t len, uint16_t code) : code(code), len(len % 12) {}
         /// @brief Constructor to initialize the MCode with a 16-bit value.
         /// @param value The 16-bit value containing the Morse code pattern and length.
         /// @note The length is forced to be within 0-11 to avoid overflow and
         ///       ensure it can't cause memory access outside the integer.
         MCode(uint16_t value) : pattern(value) { this->len %= 12; }
         /// @brief Default constructor initializes the MCode with a pattern of 0.
         MCode() : pattern(0) {}
         };

      /// @brief Structure to hold the lookup table for Morse code characters.
      /// @details The structure holds a character and its corresponding Morse code pattern.
      struct xLookup
         {
         char character;
         MCode mc;
         };

      /// @brief Structure to hold the lookup table for control characters in Morse code.
      /// @details The structure holds a ProSign and its corresponding Morse code pattern.
      struct controlLookup
         {
         ProSign sign;
         MCode mc;
         };

      /// @brief Constructor for the MorseCodeLED class, requires a 
      ///        pin number for the attached LED.
      /// @param ledPin The pin number where the LED is connected.
      MorseCodeLED(int ledPin);

      /// @brief Starting method, needs to be called before anyother method.
      void begin();

      /// @brief Method to flash the predefined message, "CQD NO RTC" in Morse code.
      /// @details This method flashes the "Come Quick Distress No Real Time Clock" message
      ///          called from the BinaryClock::purgatory() method. This is the only message
      ///          that is available on the UNO R3 board, as it lacks resources for more.
      void flash_CQD_NO_RTC();  // Predefined message

      /// @brief Method to flash an array of raw Morse code components.
      /// @details This method flashes the LED according to the Morse code pattern for the 
      ///          given array of MC enumerated types. It uses the dot, dash, space, and wordSpace methods
      ///          to flash the LED for the appropriate durations. The character is flashed with a space
      /// @param morseData The MCode structure containing the Morse code pattern and length.
      void FlashMorseCode(const MC* morseData);

   protected:
      /// @brief Method to flash a dot in Morse code.
      /// @details This method flashes the LED for a duration defined by the dot length.
      void dot();

      /// @brief Method to flash a dash in Morse code.
      /// @details This method flashes the LED for a duration defined by the dash length.
      void dash();

      /// @brief Method to flash (pause for) a space in Morse code.
      /// @details This method turns the LED off for a duration defined by the space length.
      void space();

      /// @brief Method to flash (pause for) a word space in Morse code.
      /// @details This method turns the LED off for a duration defined by the word space length
      void wordSpace();

      void flashLED(int duration);  // Helper function

   private:
      int ledPin;                   /// The pin number where the LED is connected.

      #ifndef UNO_R3
   public:
      /// @brief Method to flash a single character in Morse code. Characters A-Z, 0-9, many punctuation symbols.
      /// @details This method flashes the LED according to the Morse code pattern for the given character.
      ///          This add the character space at the end of each character.
      /// @param c The character to flash in Morse code.
      void flashCharacter(char c);

      /// @brief Method to flash a string of text in Morse code. The string is a null terminated C string.
      /// @details This method flashes the LED according to the Morse code pattern for each character in the string.
      ///          It uses the flashCharacter() method to flash each character in the string and adds the 
      ///          appropriate spaces between characters and words.
      /// @param text The null terminated C string to flash in Morse code.
      void flashString(const char* text);  // New method

      /// @brief Method to flash a given `MCode` structure instance, `morseData`
      /// @details This method flashes the LED according to the Morse code pattern for the given MCode structure.
      ///          It is called by all the other methods that flash Morse code, such as flashCharacter() and 
      ///          flashString(). Any valid pattern can be flashed using this method.
      /// @param morseData The MCode structure containing the Morse code pattern and length.
      /// @remarks This method is what actually calls the: `dot()`; `dash()` and `space()` methods to flash the LED as
      ///          defined by the bits in MCode::code portion of morseData. Ultimately all the flashing is done
      ///          from this method with the exception of flashMorseCode which is used to flash predefined message
      ///          CQD_NO_RTC for the UNO_R3 board.
      void flashMCode(const MCode& morseData);

   protected:
      /// @brief Helper method called by flashCharacter() to flash a single letter, A-Z or number, 0-9.
      /// @details The flashCharacter() method determines the type of character it is: Alphabetic: Numeric;
      ///          or punctuation. For alpha-numeric characters it calls this method with the index for
      ///          the character to flash. This method then sends the MCode for the character to flashMCode().
      /// @param charIndex The index of the character to flash, 0-25 for A-Z, 26-35 for 0-9.
      /// @remarks The index is calculated as: 'A' = 0, 'B' = 1, ..., 'Z' = 25, '0' = 26, ..., '9' = 35.
      ///          The index is used to look up the MCode for the character in the morseTable array.
      void flashCharIndex(uint8_t charIndex);

      /// @brief Helper method to flash a single punctuation or special character.
      /// @details This method handles punctuation and special characters that are not alphanumeric. 
      ///          It looks up the character in the extended character lookup table, of type xLookup, and if 
      ///          found it sends the corresponding MCode for the character to flashMCode().
      /// @param c The character to flash in Morse code.
      /// @remarks If the character isn't in the extended character lookup table, it will not flash anything.
      void flashExtendedCharacter(char c);

      /// @brief Method to flash a Morse code prosign as defined in the ProSign enumeration.
      /// @details This method flashes the LED according to the Morse code pattern for the given ProSign.
      ///          It uses the flashMCode() method to flash the prosign.
      /// @param sign The ProSign to flash in Morse code.
      /// @remarks This method does not contain all prosigns but can be extended to include more.
      void flashControl(ProSign sign);

      /// @brief Method to flash a predefined message in Morse code. Keywords that are equivalent to a prosign 
      ///        are flashed using the prosign instead of flashing the individual keyword characters.
      /// @param keyword The keyword to flash in Morse code.
      /// @remarks The predefined messages that correspond to a prosign are checked first, and if found,
      ///          the prosign is flashed instead of the individual characters. If the keyword is not
      ///          found in the predefined messages, it is flashed as a string of characters. This can be
      ///          expanded to include more predefined messages as needed.
      /// @paragraph The currently supported predefined messages are: 
      ///            - "END" or "OK" for the prosign End (. - . - .)
      ///            - "OVER" for the prosign K (-.-.)
      ///            - "ROGER" for the prosign R (. - .)
      ///            - "STARTING" for the prosign Start (-.-.-)
      ///            - "ERROR" for the prosign Error (......)
      void flashPredefinedMessage(const char* keyword);

   private:
      static const MCode morseTable[26 + 10];  // Morse code lookup table for letters and numbers (A-Z, 0-9).
      static const xLookup extendedLookup[];   // Extended character lookup table for punctuation and special characters.
      static const controlLookup extendedControlLookup[]; // Extended control character lookup table for prosigns
      #endif   // END ...#ifndef UNO_R3
      };

   } // namespace BinaryClockShield
#endif