#include <ctype.h>

#include "MorseCodeLED.h"

// Use preprocessor defines instead of variables (no RAM usage)
#define DIT_MS 100
#define DAH_MS (DIT_MS * 3)
#define SPACE_MS (DIT_MS * 3)
#define WORD_SPACE_MS (DIT_MS * 7)

namespace BinaryClockShield
   {
   MorseCodeLED::MorseCodeLED(int ledPin) : ledPin(ledPin) { }

   void MorseCodeLED::begin()
      {
      pinMode(ledPin, OUTPUT);
      digitalWrite(ledPin, LOW);
      }

   // Private helper function to reduce code duplication
   void MorseCodeLED::flashLED(int duration)
      {
      digitalWrite(ledPin, HIGH);
      delay(duration);
      digitalWrite(ledPin, LOW);
      delay(DIT_MS);  // Standard gap after each element
      }

   void MorseCodeLED::dot()
      {
      flashLED(DIT_MS);
      }

   void MorseCodeLED::dash()
      {
      flashLED(DAH_MS);
      }

   void MorseCodeLED::space()
      {
      digitalWrite(ledPin, LOW);  // Ensure LED is off
      delay(SPACE_MS);
      }

   void MorseCodeLED::wordSpace()
      {
      digitalWrite(ledPin, LOW);  // Ensure LED is off
      delay(WORD_SPACE_MS);
      }

   // Flash "CQD NO RTC" in Morse code - ultra compact implementation
   void MorseCodeLED::flash_CQD_NO_RTC()
      {
      // CQD NO RTC = "-.-.  --.-  -..    -.  ---    .-.  -  -.-."
      // Using a compressed format: 0=dot, 1=dash, 2=letter space, 3=word space

      // Store Morse patterns in PROGMEM to save RAM
      static const MC PROGMEM morse_CQD_NO_RTC[] = {
         MC::Dash, MC::Dot,  MC::Dash, MC::Dot,  MC::Space,    // C: -.-. (1010)
         MC::Dash, MC::Dash, MC::Dot,  MC::Dash, MC::Space,    // Q: --.- (1101)
         MC::Dash, MC::Dot,  MC::Dot,  MC::Word,               // D: -..  (100)
         MC::Dash, MC::Dot,  MC::Space,                        // N: -.   (10)
         MC::Dash, MC::Dash, MC::Dash, MC::Word,               // O: ---  (111)
         MC::Dot,  MC::Dash, MC::Dot,  MC::Space,              // R: .-.  (010)
         MC::Dash, MC::Space,                                  // T: -    (1)
         MC::Dash, MC::Dot,  MC::Dash, MC::Dot,                // C: -.-. (1010)
         MC::EndMarker                                         // 255 = end marker
         };

      const MC* ptr = morse_CQD_NO_RTC;
      FlashMorseCode(ptr);
      }

   void MorseCodeLED::FlashMorseCode(const MC* morseData)
      {
      MC code;

      while ((code = (MC)(pgm_read_byte(morseData++))) != MC::EndMarker)
         {
         switch ((MC)code)
            {
            case MC::Dot:   dot(); break;
            case MC::Dash:  dash(); break;
            case MC::Space: space(); break;      // Letter space
            case MC::Word:  wordSpace(); break;  // Word space
            }
         }
      }

   #ifndef UNO_R3
   /*
      A: .-    (01)     F: ..-.  (0010)   K: -.-   (101)    P: .--.  (0110)   U: ..-   (001)
      B: -...  (1000)   G: --.   (110)    L: .-..  (0100)   Q: --.-  (1101)   V: ...-  (0001)
      C: -.-.  (1010)   H: ....  (0000)   M: --    (11)     R: .-.   (010)    W: .--   (011)
      D: -..   (100)    I: ..    (00)     N: -.    (10)     S: ...   (000)    X: -..-  (1001)
      E: .     (0)      J: .---  (0111)   O: ---   (111)    T: -     (1)      Y: -.--  (1011)
                                                                              Z: --..  (1100)
   */
   /*
      morse.flashString("CQD NO RTC");      // Original message
      morse.flashString("ERROR 404");       // Numbers and letters
      morse.flashString("HELP!");           // With punctuation
      morse.flashPredefinedMessage("ROGER"); // Standard radio term
      morse.flashPredefinedMessage("ERROR"); // Error signal (8 dots)
   */
   #define NUMBER_OFFSET 26  // Offset for numbers in the lookup table
   // Morse code lookup table stored in PROGMEM (flash memory)
   // Format: 4-bit pattern length + 12-bit pattern (dots=0, dashes=1)
   // Maximum 12 elements per character (more than enough for any letter)
   // Morse code lookup table stored in PROGMEM (flash memory)
   // Format: 4-bit pattern length + 12-bit pattern (dots=0, dashes=1)
   const MorseCodeLED::MCode PROGMEM MorseCodeLED::morseTable[26 + 10] =
         {
         0x2002, // A: .-     (len=2, pattern=01)   - CORRECTED
         0x4001, // B: -...   (len=4, pattern=1000)
         0x4005, // C: -.-.   (len=4, pattern=1010)
         0x3001, // D: -..    (len=3, pattern=100)
         0x1000, // E: .      (len=1, pattern=0)
         0x4002, // F: ..-.   (len=4, pattern=0010) - CORRECTED
         0x3006, // G: --.    (len=3, pattern=110)  - CORRECTED
         0x4000, // H: ....   (len=4, pattern=0000)
         0x2000, // I: ..     (len=2, pattern=00)
         0x4007, // J: .---   (len=4, pattern=0111) - CORRECTED
         0x3005, // K: -.-    (len=3, pattern=101)
         0x4004, // L: .-..   (len=4, pattern=0100) - CORRECTED
         0x2003, // M: --     (len=2, pattern=11)
         0x2002, // N: -.     (len=2, pattern=10)   - CORRECTED
         0x3007, // O: ---    (len=3, pattern=111)
         0x4006, // P: .--.   (len=4, pattern=0110)
         0x400D, // Q: --.-   (len=4, pattern=1101)
         0x3002, // R: .-.    (len=3, pattern=010)
         0x3000, // S: ...    (len=3, pattern=000)
         0x1001, // T: -      (len=1, pattern=1)
         0x3001, // U: ..-    (len=3, pattern=001)  - CORRECTED
         0x4001, // V: ...-   (len=4, pattern=0001) - CORRECTED
         0x3003, // W: .--    (len=3, pattern=011)  - CORRECTED
         0x4009, // X: -..-   (len=4, pattern=1001)
         0x400B, // Y: -.--   (len=4, pattern=1011)
         0x400C, // Z: --..   (len=4, pattern=1100)
         // Numbers: 0-9
         0x5005, // 0: -----  (len=5, pattern=11111) ✅ CORRECT
         0x5001, // 1: .----  (len=5, pattern=01111) ✅ CORRECT
         0x5003, // 2: ..---  (len=5, pattern=00111) ✅ CORRECT
         0x5007, // 3: ...--  (len=5, pattern=00011) ✅ CORRECT
         0x500F, // 4: ....-  (len=5, pattern=00001) ✅ CORRECTED (was 0x5003)
         0x501F, // 5: .....  (len=5, pattern=00000) ✅ CORRECT
         0x501E, // 6: -....  (len=5, pattern=10000) ✅ CORRECT
         0x501C, // 7: --...  (len=5, pattern=11000) ✅ CORRECT
         0x5018, // 8: ---..  (len=5, pattern=11100) ✅ CORRECT
         0x5010  // 9: ----.  (len=5, pattern=11110) ✅ CORRECT
         };

   #define MORSETABLE_SIZE sizeof(morseTable) / sizeof(morseTable[0])

   // Memory-efficient string to Morse code converter
   // Converts string directly to flashing without creating arrays
   void MorseCodeLED::flashString(const char* text)
      {
      // Iterate through each character in the string
      // Convert each character to uppercase and flash its Morse code
      for (const char* p = text; *p; p++)
         {
         char c = *p;
         flashCharacter(c);
         }
      }

   void MorseCodeLED::flashMCode(const MCode& morseData)
      {
      uint8_t length = morseData.len; // (morseData >> 12) & 0x0F;   // Extract length (4 bits)
      uint16_t pattern = morseData.code; // morseData & 0x0FFF;       // Extract pattern (12 bits)
      if (length == 0 || length > 12) { return; } // Invalid length check

      // Flash the pattern from MSB to LSB, 0 for dot, 1 for dash.
      for (int8_t i = length - 1; i >= 0; i--)
         {
         if (pattern & (1 << i))
            {
            dash();  // 1 = dash
            }
         else
            {
            dot();   // 0 = dot
            }
         }

      space(); // Space between letters
      }

   void MorseCodeLED::flashCharacter(char c)
      {
      // Test the character, c, and call the method to flash it.
      if (c == ' ')
         {
         wordSpace(); // Space between words
         }
      else if (isalpha(c))
         {
         flashCharIndex(toupper(c) - 'A');
         }
      else if (isdigit(c))
         {
         flashCharIndex(c - '0' + NUMBER_OFFSET);
         }
      else if (ispunct(c))
         {
         flashExtendedCharacter(c); // Handle punctuation and special characters
         }
         
      // ToDo: Could add handling for unrecognized characters if needed
      }

   // Helper function to flash a single character using lookup table
   void MorseCodeLED::flashCharIndex(uint8_t charIndex)
      {
      if (charIndex < MORSETABLE_SIZE) 
         {
         MCode morseData = (MCode)(pgm_read_word(&morseTable[charIndex].pattern));
         flashMCode(morseData);
         }
      }

   
   // Helper function for extended characters (numbers, punctuation)
   void MorseCodeLED::flashExtendedCharacter(char c)
      {
         
      /*
         Numbers:
         0: -----  1: .----  2: ..---  3: ...--  4: ....-
         5: .....  6: -....  7: --...  8: ---..  9: ----.
         
         Punctuation:
         . : .-.-.-   , : --..--   ? : ..--..   ' : .----.   ! : -.-.--
         / : -..-.    ( : -.--.    ) : -.--.-   & : .-...   : : ---...
         ; : -.-.-.   = : -...-    + : .-.-.    - : -....-  " : .-..-.
         $ : ...-..-  @ : .--.-.
      */
                 
      // Extended lookup table for numbers and special characters
      static const xLookup PROGMEM extendedLookup[] = {
             // Punctuation and symbols - MANY CORRECTED
             {'.', 0x6015}, // .-.-.-    (len=6, pattern=010101) ✅ CORRECT
             {',', 0x6033}, // --..--    (len=6, pattern=110011) ✅ CORRECT
             {'?', 0x600C}, // ..--..    (len=6, pattern=001100) ✅ CORRECT
             {'\'',0x601E}, // .----.    (len=6, pattern=011110) ✅ CORRECTED (was 0x501E)
             {'!', 0x602B}, // -.-.--    (len=6, pattern=101011) ✅ CORRECT
             {'/', 0x5012}, // -..-.     (len=5, pattern=10010) ✅ CORRECT - 5009
             {'(', 0x5016}, // -.--.     (len=5, pattern=10110) ✅ CORRECT
             {')', 0x602D}, // -.--.-    (len=6, pattern=101101) ✅ CORRECT
             {'&', 0x5008}, // .-...     (len=5, pattern=01000) ✅ CORRECT - 5002
             {':', 0x6038}, // ---...    (len=6, pattern=111000) ✅ CORRECT
             {';', 0x602A}, // -.-.-.    (len=6, pattern=101010) ✅ CORRECT
             {'=', 0x5011}, // -...-     (len=5, pattern=10001) ✅ CORRECT
             {'+', 0x500A}, // .-.-.     (len=5, pattern=01010) ✅ CORRECT
             {'-', 0x6021}, // -....-    (len=6, pattern=100001) ✅ CORRECT
             {'_', 0x600D}, // ..--.-    (len=6, pattern=001101) ✅ CORRECT
             {'"', 0x6012}, // .-..-.    (len=6, pattern=010010) ✅ CORRECT
             {'$', 0x7009}, // ...-..-   (len=7, pattern=0001001) ✅ CORRECTED 7012
             {'@', 0x601A}, // .--.-.    (len=6, pattern=011010) ✅ CORRECT - 6016
            };

      #define EXTENDED_SIZE sizeof(extendedLookup) / sizeof(extendedLookup[0])
      // Rest of function remains the same...
      // Search for the character
      for (uint8_t i = 0; i < EXTENDED_SIZE; i++)
         {
         char lookupChar = pgm_read_byte(&extendedLookup[i].character);
         if (lookupChar == c)
            {
            MCode morseData = (MCode)(pgm_read_word(&extendedLookup[i].mc.pattern));
            flashMCode(morseData);

            return;
            }
         }
      // ToDo: Character not found - could add a default action here
      #undef EXTENDED_SIZE
      }

   void MorseCodeLED::flashControl(ProSign sign)
      {
      // Control codes for prosigns and special commands
      static const controlLookup PROGMEM extendedLookup[] = 
            {
            // Prosign codes
            {ProSign::Start     , 0x5015}, // -.-.-     (len=5, pattern=01010)      []    Start                    
            {ProSign::End       , 0x500A}, // .-.-.     (len=5, pattern=11001)      [AR]  End of message          
            {ProSign::FullStop  , 0x5008}, // .-.-.-    (len=5, pattern=00100)      [AS]  Full stop (period)
            {ProSign::Invite    , 0x3005}, // -.-       (len=3, pattern=101)        [K]   Invitation to transmit 
            {ProSign::Understood, 0x5002}, // ...-.     (len=5, pattern=00010)      []    Understood
            {ProSign::AR        , 0x500A}, // .-.-.     (len=5, pattern=01010)      [AR]  End of message          
            {ProSign::AS        , 0x5008}, // .-...     (len=5, pattern=01000)      [AS]  Wait for response       
            {ProSign::K         , 0x3005}, // -.-       (len=3, pattern=101)        [K]   Invitation to transmit  
            {ProSign::R         , 0x3002}, // .-.       (len=3, pattern=010)        [R]   Received OK             
            {ProSign::C         , 0x4005}, // -.-.      (len=4, pattern=1010)       [C]   Call to specific station
            {ProSign::SK        , 0x5025}, // ...-.-    (len=5, pattern=00101)      [SK]  End of contact / work
            {ProSign::BT        , 0x5011}, // -...-     (len=5, pattern=10001)      [BT]  New paragraph, separator
            {ProSign::SOS       , 0x9038}, // ...---... (len=9, pattern=000111000)  [SOS] International distress signal
            {ProSign::Error     , 0x5000}, // .....     (len=5, pattern=00000000)   [Error] Error signal (8 dots)
            // End marker
            {ProSign::EndMark   , 0xFFFF} // End of command list
            };

      #define CONTROL_SIZE sizeof(extendedLookup) / sizeof(extendedLookup[0])
      // Control flashing behavior based on index
      for (uint8_t i = 0; i < CONTROL_SIZE; i++)
         {
         ProSign lookup = (ProSign)(pgm_read_byte(&extendedLookup[i].sign));
         if (lookup == sign)
            {
            MCode morseData = (MCode)(pgm_read_word(&extendedLookup[i].mc.pattern));
            flashMCode(morseData);

            return;
            }
         }
      // ToDo: If not found, do nothing or handle error
      #undef CONTROL_SIZE
      }

   void MorseCodeLED::flashPredefinedMessage(const char* keyword)
      {
      // This isn't very efficient in terms of memory usagean  and speed but
      // there are very few predefined messages so it is acceptable.
      if (strcmp(keyword, "END") == 0 || strcmp(keyword, "OK") == 0)
         {
         flashControl(ProSign::End); // Standard prosign for END/OK: .-.-.
         }
      else if (strcmp(keyword, "OVER") == 0)
         {
         flashControl(ProSign::K); // Standard prosign for OVER: -.-.
         }
      else if (strcmp(keyword, "ROGER") == 0)
         {
         flashControl(ProSign::R); // Standard prosign for ROGER: .-.
         }
      else if (strcmp(keyword, "STARTING") == 0)
         {
         flashControl(ProSign::Start); // Standard prosign for STARTING: -.-.-
         }
      else if (strcmp(keyword, "ERROR") == 0)
         {
         flashControl(ProSign::Error); // Standard error signal: ......
         }
      else
         {
         // Default to flashing the keyword as text
         flashString(keyword);
         }
      }
   #endif // END ...#ifndef UNO_R3
   } // namespace BinaryClockShield

#undef DIT_MS
#undef DAH_MS  
#undef SPACE_MS
#undef WORD_SPACE_MS