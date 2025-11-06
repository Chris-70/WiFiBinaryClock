/// @file MorseCodeLED.cpp
/// @brief Implementation file for the MorseCodeLED class.
/// @details  This file contains the implementation of the MorseCodeLED class, 
///           which is used to flash an LED in Morse code. The class supports 
///           flashing predefined messages, as well as arbitrary strings of text.
/// @author Chris-80 (2025/08)          

#include <ctype.h>

#include "MorseCodeLED.h"

// Use preprocessor defines instead of variables (no RAM usage)
#define DIT_MS 200
#define DAH_MS (DIT_MS * 3)
#define SPACE_MS (DIT_MS * 3)
#define WORD_SPACE_MS (DIT_MS * 7)

namespace BinaryClockShield
   {
   MorseCodeLED::MorseCodeLED(int ledPin) : ledPin(ledPin) { }

   void MorseCodeLED::Begin()
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

   void MorseCodeLED::Flash_CQD_NO_RTC()
      {
      // Flash "CQD NO RTC" in Morse code - ultra compact implementation
      // This was designed for the UNO_R3 as RAM and ROM were almost out.
      // Calling FlashMCode() and a fixed array spelling out: CQD NO RTC
      // cost an extra 372 bytes of ROM, so this format is used for all.
      // Keeping this for all other boads costs 208 bytes of ROM but it
      // doesn't add visual / maintenance complexity by having this code
      // littered with conditional compilation statements. We have just
      // one conditional compilation statement to truncate this class to
      // the minimum needed to display "CQD NO RTC" in Morse code on a LED.
      // ======================================================================
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

      #if defined(__GNUC__)
         #pragma GCC diagnostic push
         // Keeping the switch compact for UNO_R3 where this is used.
         // The while()... statement handles the MC::EndMarker
         #pragma GCC diagnostic ignored "-Wswitch"
      #endif

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

      #if defined(__GNUC__)
         #pragma GCC diagnostic pop
      #endif
      }

   #ifndef UNO_R3
   /*
      morse.FlashString("CQD NO RTC");      // Original message as a string.
      morse.FlashString("ERROR 404");       // Numbers and letters
      morse.FlashString("HELP!");           // With punctuation
      morse.FlashProsignWord("ROGER");      // Standard radio term
      morse.FlashProsignWord("ERROR");      // Error/correction signal (8 dots)
   */
   #define NUMBER_OFFSET 26  // Offset for numbers in the lookup table

   /// @brief Morse code lookup table stored in PROGMEM (flash memory)
   /// @details Format: 4-bit pattern length + 12-bit pattern (dots=0, dashes=1)
   /// Maximum 12 elements per character (more than enough for any letter)
   /// Morse code lookup table stored in PROGMEM (flash memory)
   /*!
   @verbatim
      Letters:  
      `A: .-    (01)     F: ..-.  (0010)   K: -.-   (101)    P: .--.  (0110)   U: ..-   (001)`  
      `B: -...  (1000)   G: --.   (110)    L: .-..  (0100)   Q: --.-  (1101)   V: ...-  (0001)`  
      `C: -.-.  (1010)   H: ....  (0000)   M: --    (11)     R: .-.   (010)    W: .--   (011)`  
      `D: -..   (100)    I: ..    (00)     N: -.    (10)     S: ...   (000)    X: -..-  (1001)`  
      `E: .     (0)      J: .---  (0111)   O: ---   (111)    T: -     (1)      Y: -.--  (1011)`  
      `                                                                        Z: --..  (1100)`  
      Numbers:  
      `0: -----  1: .----  2: ..---  3: ...--  4: ....- `  
      `5: .....  6: -....  7: --...  8: ---..  9: ----. `  
   @endverbatim
   */
   const MorseCodeLED::MCode PROGMEM MorseCodeLED::morseTable[26 + 10] =
         {
         0x2002, // A: .-     (len=2, pattern=01)   
         0x4001, // B: -...   (len=4, pattern=1000)
         0x4005, // C: -.-.   (len=4, pattern=1010)
         0x3001, // D: -..    (len=3, pattern=100)
         0x1000, // E: .      (len=1, pattern=0)
         0x4002, // F: ..-.   (len=4, pattern=0010) 
         0x3006, // G: --.    (len=3, pattern=110)  
         0x4000, // H: ....   (len=4, pattern=0000)
         0x2000, // I: ..     (len=2, pattern=00)
         0x4007, // J: .---   (len=4, pattern=0111) 
         0x3005, // K: -.-    (len=3, pattern=101)
         0x4004, // L: .-..   (len=4, pattern=0100) 
         0x2003, // M: --     (len=2, pattern=11)
         0x2002, // N: -.     (len=2, pattern=10)   
         0x3007, // O: ---    (len=3, pattern=111)
         0x4006, // P: .--.   (len=4, pattern=0110)
         0x400D, // Q: --.-   (len=4, pattern=1101)
         0x3002, // R: .-.    (len=3, pattern=010)
         0x3000, // S: ...    (len=3, pattern=000)
         0x1001, // T: -      (len=1, pattern=1)
         0x3001, // U: ..-    (len=3, pattern=001)  
         0x4001, // V: ...-   (len=4, pattern=0001) 
         0x3003, // W: .--    (len=3, pattern=011)  
         0x4009, // X: -..-   (len=4, pattern=1001)
         0x400B, // Y: -.--   (len=4, pattern=1011)
         0x400C, // Z: --..   (len=4, pattern=1100)
         // Numbers: 0-9
         0x5005, // 0: -----  (len=5, pattern=11111) ✅ CORRECT
         0x5001, // 1: .----  (len=5, pattern=01111) ✅ CORRECT
         0x5003, // 2: ..---  (len=5, pattern=00111) ✅ CORRECT
         0x5007, // 3: ...--  (len=5, pattern=00011) ✅ CORRECT
         0x500F, // 4: ....-  (len=5, pattern=00001) ✅ CORRECT
         0x501F, // 5: .....  (len=5, pattern=00000) ✅ CORRECT
         0x501E, // 6: -....  (len=5, pattern=10000) ✅ CORRECT
         0x501C, // 7: --...  (len=5, pattern=11000) ✅ CORRECT
         0x5018, // 8: ---..  (len=5, pattern=11100) ✅ CORRECT
         0x5010  // 9: ----.  (len=5, pattern=11110) ✅ CORRECT
         };

   #define MORSETABLE_SIZE sizeof(morseTable) / sizeof(morseTable[0])

   void MorseCodeLED::flashMCode(const MCode& morseData)
      {
      uint8_t length = morseData.len;    // Extract length (4 bits)
      uint16_t pattern = morseData.code; // Extract pattern (12 bits)
      if (length > 12) { return; }       // Invalid length check

      // Interpret a length of 0 to be a word space, do the delay and exit.
      if (length == 0) 
         {
         wordSpace();
         return; 
         }

      // Flash the pattern from MSb to LSb: 0 for dot; 1 for dash.
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

   void MorseCodeLED::FlashCharacter(char c)
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
      // else: do nothing.
      }

   void MorseCodeLED::FlashString(const String& text)
      {
      for (auto&& i : text)
         {
         FlashCharacter(i);
         }
      }

   void MorseCodeLED::flashCharIndex(uint8_t charIndex)
      {
     // Helper function to flash a single character using lookup table
      if (charIndex < MORSETABLE_SIZE) 
         {
         MCode morseData = (MCode)(pgm_read_word(&morseTable[charIndex].pattern));
         flashMCode(morseData);
         }
      }

   void MorseCodeLED::flashExtendedCharacter(char c)
      {
      // Helper function for extended characters (numbers, punctuation)
         
      /// @brief Extended lookup table for punctuation and symbol characters
      /*!
      @details Common punctuation and symbols
      @verbatim
         Punctuation: (18)   
         ` ! : -.-.--   " : .-..-.   $ : ...-..-  & : .-...    ( : -.--.    '
         ` ) : -.--.-   + : .-.-.    , : --..--   - : -....-   . : .-.-.-   `
         ` / : -..-.    : : ---...   ; : -.-.-.   = : -...-    ? : ..--..   `
         ` @ : .--.-.   \ : .----.   _ : ..--.-                             `
      @endverbatim
      @see morseTable
      @see prosignTable
      */
      static const XcLookup PROGMEM extendedLookup[] = {
             // Punctuation and symbols - (!"$&()+,-./:;=?@\_)
             // 
             // ispunct(!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~) == true
             // uchar(!"$&()+,-./:;=?@\_)[18] In ASCII order; Missing: "(#%'*<>[]^`{|}~)"
             {'!', 0x602B}, // -.-.--    (len=6, pattern=110101)  ✅ CORRECT
             {'"', 0x6012}, // .-..-.    (len=6, pattern=010010)  ✅ CORRECT
             {'$', 0x7009}, // ...-..-   (len=7, pattern=0001001) ✅ CORRECT
             {'&', 0x5008}, // .-...     (len=5, pattern=01000)   ✅ CORRECT
             {'(', 0x5016}, // -.--.     (len=5, pattern=10110)   ✅ CORRECT
             {')', 0x602D}, // -.--.-    (len=6, pattern=110101)  ✅ CORRECT
             {'+', 0x500A}, // .-.-.     (len=5, pattern=01010)   ✅ CORRECT
             {',', 0x6033}, // --..--    (len=6, pattern=110011)  ✅ CORRECT
             {'-', 0x6021}, // -....-    (len=6, pattern=100001)  ✅ CORRECT
             {'.', 0x6015}, // .-.-.-    (len=6, pattern=010101)  ✅ CORRECT
             {'/', 0x5012}, // -..-.     (len=5, pattern=10010)   ✅ CORRECT
             {':', 0x6038}, // ---...    (len=6, pattern=111000)  ✅ CORRECT
             {';', 0x602A}, // -.-.-.    (len=6, pattern=101010)  ✅ CORRECT
             {'=', 0x5011}, // -...-     (len=5, pattern=10001)   ✅ CORRECT
             {'?', 0x600C}, // ..--..    (len=6, pattern=001100)  ✅ CORRECT
             {'@', 0x601A}, // .--.-.    (len=6, pattern=011010)  ✅ CORRECT
             {'\'',0x601E}, // .----.    (len=6, pattern=011110)  ✅ CORRECT
             {'_', 0x600D}, // ..--.-    (len=6, pattern=001101)  ✅ CORRECT
             {' ', 0x0000}  // A valid word space to end the lookup table
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

      #undef EXTENDED_SIZE
      }

   void MorseCodeLED::FlashProsign(Prosign sign)
      {
      if (sign >= Prosign::EndMark) { return; }

      // Control codes for prosigns and special commands
      static const ProsignLookup PROGMEM prosignTable[] = 
            {
            // Prosign codes
            {Prosign::Start     , 0x5015}, ///< [` -.-.-     `] (len=5, pattern=10101)      [KA]  Start, Attention
            {Prosign::End       , 0x500A}, ///< [` .-.-.     `] (len=5, pattern=01010)      [AR]  End of message
            {Prosign::EndWork   , 0x5025}, ///< [` ...-.-    `] (len=6, pattern=001101)     [SK]  End of contact / work, Out
            {Prosign::Out       , 0x5025}, ///< [` ...-.-    `] (len=6, pattern=001101)     [SK]  Out, End of contact
            {Prosign::Wait      , 0x5008}, ///< [` .-...     `] (len=5, pattern=01000)      [AS]  Wait for response, I am busy
            {Prosign::FullStop  , 0x6015}, ///< [` .-.-.-    `] (len=6, pattern=010101)     [.]   Full stop (period)
            {Prosign::Invite    , 0x3005}, ///< [` -.-       `] (len=3, pattern=101)        [K]   Done, Invitation to transmit 
            {Prosign::Over      , 0x3005}, ///< [` -.-       `] (len=3, pattern=101)        [K]   Done, you transmit now
            {Prosign::Understood, 0x5002}, ///< [` ...-.     `] (len=5, pattern=00010)      [VE]  Understood, Verified
            {Prosign::SayAgain  , 0x600C}, ///< [` ..--..    `] (len=6, pattern=001100)     [?]   Say Again?
            {Prosign::Correction, 0x8000}, ///< [` ........  `] (len=8, pattern=00000000)   [HH]  Error, Correction follows
            {Prosign::Error     , 0x8000}, ///< [` ........  `] (len=8, pattern=00000000)   [HH]  Error, Correction follows
            {Prosign::R         , 0x3002}, ///< [` .-.       `] (len=3, pattern=010)        [R]   Received OK             
            {Prosign::K         , 0x3005}, ///< [` -.-       `] (len=3, pattern=101)        [K]   Invitation to transmit  
            {Prosign::AR        , 0x500A}, ///< [` .-.-.     `] (len=5, pattern=01010)      [AR]  End of message          
            {Prosign::AS        , 0x5008}, ///< [` .-...     `] (len=5, pattern=01000)      [AS]  Wait for response, busy       
            {Prosign::VE        , 0x5002}, ///< [` ...-.     `] (len=5, pattern=00010)      [VE]  Understood, Verified
            {Prosign::HH        , 0x8000}, ///< [` ........  `] (len=8, pattern=00000000)   [HH]  Error, Correction
            {Prosign::BT        , 0x5011}, ///< [` -...-     `] (len=5, pattern=10001)      [BT]  New paragraph, separator
            {Prosign::KA        , 0x5015}, ///< [` -.-.-     `] (len=5, pattern=10101)      [KA]  Start, Attention
            {Prosign::SK        , 0x5025}, ///< [` ...-.-    `] (len=6, pattern=001101)     [SK]  End of contact / work
            {Prosign::C         , 0x4005}, ///< [` -.-.      `] (len=4, pattern=1010)       [C]   Correct, Confirm, Yes
            {Prosign::N         , 0x2002}, ///< [` -.        `] (len=2, pattern=10)         [N]   Negative, No
            {Prosign::SOS       , 0x9038}, ///< [` ...---... `] (len=9, pattern=000111000)  [SOS] International distress signal
            // End marker
            };

      #define TABLE_SIZE sizeof(prosignTable) / sizeof(prosignTable[0])
      static_assert((uint8_t)(Prosign::EndMark) == TABLE_SIZE, "Size of the enum Prosign (i.e. value of EndMark) must match `prosignTable` array size");

      uint8_t index = (uint8_t)(sign);
      Prosign lookup = (Prosign)(pgm_read_byte(&prosignTable[index].sign));

      // Coding error check: The `prosignTable` lookup index value, `ProsignLookup::sign`, is not in the correct order.
      // The index order MUST match the `Prosign` enum order. This assert indicates an error was found.
      assert(lookup == sign); 
      if (lookup == sign)
         {
         // All good, spend the time to read from flash memory and display the code.
         MCode morseData = (MCode)(pgm_read_word(&prosignTable[index].mc.pattern));
         flashMCode(morseData);
         }
      #undef TABLE_SIZE
      }

   void MorseCodeLED::FlashProsignWord(String keyString)
      {
      if (keyString.isEmpty()) { return; }

      keyString.toUpperCase();
      const char* keyword = keyString.c_str();
      // This isn't very efficient in terms of memory usage and speed but
      // there are very few predefined messages so it is acceptable.
      if (strcmp(keyword, "START") == 0 || strcmp(keyword, "STARTING") == 0)
         {
         FlashProsign(Prosign::KA); // Standard prosign for START: [-.-.-]
         }
      else if (strcmp_P(keyword, "END") == 0 || strcmp(keyword, "OK") == 0)
         {
         FlashProsign(Prosign::AR); // Standard prosign for END/OK: [.-.-.]
         }
      else if (strcmp_P(keyword, "ENDWORK") == 0 || strcmp(keyword, "OUT") == 0)
         {
         FlashProsign(Prosign::SK); // Standard prosign for ENDWORK: [...-.-]
         }
      else if (strcmp(keyword, "OVER") == 0 || strcmp(keyword, "INVITE") == 0)
         {
         FlashProsign(Prosign::K); // Standard prosign for OVER: [-.-]
         }
      else if (strcmp(keyword, "UNDERSTOOD") == 0)
         {
         FlashProsign(Prosign::VE); // Standard prosign for UNDERSTOOD: [..-.]
         }
      else if (strcmp(keyword, "SAYAGAIN") == 0)
         {
         FlashProsign(Prosign::SayAgain); // Standard prosign for SAYAGAIN: "?" [..--..]
         }
      else if (strcmp(keyword, "ROGER") == 0)
         {
         FlashProsign(Prosign::R); // Standard prosign for ROGER: [.-.]
         }
      else if (strcmp(keyword, "ERROR") == 0 || strcmp(keyword, "CORRECTION") == 0)
         {
         FlashProsign(Prosign::HH); // Standard error signal: [........]
         }
      else if (strcmp(keyword, "OUT") == 0)
         {
         FlashProsign(Prosign::SK); // Standard prosign for OUT: [...-.-]
         }
      else if (strcmp(keyword, "CORRECT") == 0 || strcmp(keyword, "CONFIRM") == 0 || strcmp(keyword, "YES") == 0)
         {
         FlashProsign(Prosign::C); // Standard Conrect signal: "C" [-.-.]
         }
      else if (strcmp(keyword, "NEGATIVE") == 0 || strcmp(keyword, "NO") == 0)
         {
         FlashProsign(Prosign::C); // Standard Negative signal: "N" [-.]
         }
      else if (strcmp(keyword, "SOS") == 0)
         {
         FlashProsign(Prosign::SOS); // Standard distress signal: [...---...]
         }
      else
         {
         // Default to flashing the keyword as text
         FlashString(keyword);
         }
      }
   #endif // END ...#ifndef UNO_R3
   } // namespace BinaryClockShield

#undef DIT_MS
#undef DAH_MS  
#undef SPACE_MS
#undef WORD_SPACE_MS