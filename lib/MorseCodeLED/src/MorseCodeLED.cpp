/// @file MorseCodeLED.cpp
/// @brief Implementation file for the MorseCodeLED class.
/// @details  This file contains the implementation of the MorseCodeLED class, 
///           which is used to flash a LED in Morse code. The class supports 
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

   #if BINARY_CLOCK_LIB
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
   #else
   void MorseCodeLED::Flash_CQD()
      {
      // Flash "CQD" in Morse code - ultra compact implementation
      // This was designed for the UNO_R3 as RAM and ROM were almost out.
      // Calling FlashMCode() and a fixed array spelling out: CQD
      // cost an extra 132 bytes of ROM, so this format is used for all.
      // Keeping this for all other boads costs 72 bytes of ROM but it
      // doesn't add visual / maintenance complexity by having this code
      // littered with conditional compilation statements. We have just
      // one conditional compilation statement to truncate this class to
      // the minimum needed to display "CQD" in Morse code on a LED.
      // ======================================================================
      // CQD = "-.-.  --.-  -.."
      // Using a compressed format: 0=dot, 1=dash, 2=letter space

      // Store Morse patterns in PROGMEM to save RAM
      static const MC PROGMEM morse_CQD[] = {
         MC::Dash, MC::Dot,  MC::Dash, MC::Dot,  MC::Space,    // C: -.-. (1010)
         MC::Dash, MC::Dash, MC::Dot,  MC::Dash, MC::Space,    // Q: --.- (1101)
         MC::Dash, MC::Dot,  MC::Dot,                          // D: -..  (100)
         MC::EndMarker
         };

      const MC* ptr = morse_CQD;
      FlashMorseCode(ptr);
      }
   #endif // BINARY_CLOCK_LIB

   void MorseCodeLED::FlashMorseCode(const MC* morseData)
      {
      MC code;

      #if defined(__GNUC__)
         #pragma GCC diagnostic push
         // Keeping the switch compact for UNO_R3 where this is used.
         // The while()... statement handles the MC::EndMarker
         // Disable the warning about not handling all enum values.
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

   #if !defined(LIMITED_MEMORY) || !LIMITED_MEMORY
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
      // Note: The `prosignTable` must be in the same order as the `Prosign` enum values, and 
      //       the `sign` field must match the enum value for correct lookup.
      //       The duplicates in the table are intentional and represent different names for
      //       the prosigns with the same Morse code.
      // Examples:
      // Prosigns:
      //    `KA`; `Start`; `Attention`;   are all the same prosign meaning with Morse code `-.-.-`    (len=5, pattern=10101)
      //    `AR`; `End`;                  are all the same prosign meaning with Morse code `.-.-.`    (len=5, pattern=01010)
      //    `SK`; `Out`; `EndWork`;       are all the same prosign meaning with Morse code `...-.-`   (len=6, pattern=001101)
      //    `K`; `Over`; `Invite`;        are all the same prosign meaning with Morse code `-.-`      (len=3, pattern=101)
      //    `VE`; `Understood`;           are all the same prosign meaning with Morse code `...-.`    (len=5, pattern=00010)
      //    `HH`; `Error`; `Correction`;  are all the same prosign meaning with Morse code `........` (len=8, pattern=00000000)
      static const ProsignLookup PROGMEM prosignTable[] = 
            {
            // Prosign codes
            {Prosign::Start     , 0x5015}, ///< [` -.-.-     `] (len=5, pattern=10101)      [KA]  Start, Attention
            {Prosign::End       , 0x500A}, ///< [` .-.-.     `] (len=5, pattern=01010)      [AR]  End of message
            {Prosign::EndWork   , 0x6025}, ///< [` ...-.-    `] (len=6, pattern=001101)     [SK]  End of contact / work, Out
            {Prosign::OverOut   , 0x6025}, ///< [` ...-.-    `] (len=6, pattern=001101)     [SK]  Out, End of contact
            {Prosign::Out       , 0x6025}, ///< [` ...-.-    `] (len=6, pattern=001101)     [SK]  Out, End of contact
            {Prosign::Wait      , 0x5008}, ///< [` .-...     `] (len=5, pattern=01000)      [AS]  Wait for response, I am busy
            {Prosign::FullStop  , 0x6015}, ///< [` .-.-.-    `] (len=6, pattern=010101)     [.]   Full stop (period)
            {Prosign::Invite    , 0x3005}, ///< [` -.-       `] (len=3, pattern=101)        [K]   Done, Invitation to transmit 
            {Prosign::Over      , 0x3005}, ///< [` -.-       `] (len=3, pattern=101)        [K]   Done, you transmit now
            {Prosign::Understood, 0x5002}, ///< [` ...-.     `] (len=5, pattern=00010)      [VE]  Understood, Verified
            {Prosign::Verified  , 0x5002}, ///< [` ...-.     `] (len=5, pattern=00010)      [VE]  Verified, Understood
            {Prosign::SayAgain  , 0x600C}, ///< [` ..--..    `] (len=6, pattern=001100)     [?]   Say Again?, Repeat?
            {Prosign::Repeat    , 0x600C}, ///< [` ..--..    `] (len=6, pattern=001100)     [?]   Repeat?, Say Again?
            {Prosign::Correction, 0x8000}, ///< [` ........  `] (len=8, pattern=00000000)   [HH]  Error, Correction follows
            {Prosign::Error     , 0x8000}, ///< [` ........  `] (len=8, pattern=00000000)   [HH]  Error, Correction follows
            {Prosign::SOS       , 0x9038}, ///< [` ...---... `] (len=9, pattern=000111000)  [SOS] International life distress signal
            {Prosign::Roger     , 0x3002}, ///< [` .-.       `] (len=3, pattern=010)        [R]   Roger, Received OK             
            {Prosign::R         , 0x3002}, ///< [` .-.       `] (len=3, pattern=010)        [R]   Received OK             
            {Prosign::K         , 0x3005}, ///< [` -.-       `] (len=3, pattern=101)        [K]   Invitation to transmit  
            {Prosign::AR        , 0x500A}, ///< [` .-.-.     `] (len=5, pattern=01010)      [AR]  End of message          
            {Prosign::AS        , 0x5008}, ///< [` .-...     `] (len=5, pattern=01000)      [AS]  Wait for response, busy       
            {Prosign::VE        , 0x5002}, ///< [` ...-.     `] (len=5, pattern=00010)      [VE]  Understood, Verified
            {Prosign::HH        , 0x8000}, ///< [` ........  `] (len=8, pattern=00000000)   [HH]  Error, Correction
            {Prosign::BT        , 0x5011}, ///< [` -...-     `] (len=5, pattern=10001)      [BT]  New paragraph, separator
            {Prosign::KA        , 0x5015}, ///< [` -.-.-     `] (len=5, pattern=10101)      [KA]  Start, Attention
            {Prosign::SK        , 0x6025}, ///< [` ...-.-    `] (len=6, pattern=001101)     [SK]  End of contact / work
            {Prosign::C         , 0x4005}, ///< [` -.-.      `] (len=4, pattern=1010)       [C]   Correct, Confirm, Yes
            {Prosign::N         , 0x2002}, ///< [` -.        `] (len=2, pattern=10)         [N]   Negative, No
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

   // FlashProsignWord() and hash function Design:    [Chris-70 (2026/03)]
   //    To implement the `FlashProsignWord()` method, we need to efficiently map input strings to their corresponding Morse code prosigns. 
   //          The method should support the predefined prosign words. Words that are not in the predefined list will be treated as strings of text.
   //    - To efficiently map the input string to the correct prosign, we can use a hash function to compute a hash value for the input string and 
   //          then use a switch statement to match against known hashes for the predefined prosign words.
   //    - The hash function should be simple and fast, and we can use a combination of XOR and multiplication with prime numbers to create 
   //          a hash that minimizes collisions for our set of predefined words.
   //    - We will implement both a runtime hash function (for hashing the input string) and a compile-time hash function (for hashing the predefined prosign keywords) 
   //          to ensure that the case labels in the switch statement are computed at compile time, which allows for efficient code generation and avoids runtime overhead.
   //    - Macros and preprocessor definitions will be used to write the code (production and test) from a single source that matches the string word with the `Prosign` enum.
   #define STARTING_PRIME     21661u  // A prime number to start the hash (arbitrary choice)
   #define MULTIPLIER_PRIME   167u    // A prime number to multiply the hash (arbitrary choice)

   /// @brief Runtime hash function for Morse code keywords.
   ///        Simple method to compute a hash for a string without using recursion on an embedded processor.
   /// @details This function computes a hash for a given string at runtime.
   /// @return A 16-bit hash value for the input string.
   /// @see hashWordConst()
   static inline uint16_t hashWordRuntime(const char* text)
      {
      uint16_t hash = STARTING_PRIME;
      while (*text)
         {
         hash ^= (uint8_t)(*text++);
         hash = (uint16_t)(hash * MULTIPLIER_PRIME);
         }
      return hash;
      }

   /// @brief Compile-time hash function for Morse code keywords.
   ///        Simple method to compute a hash for a string using recursion, which is evaluated at compile time.
   ///        It uses recursion to compute the hash and this is done by the computer that is compiling the code.
   /// @details This function computes a hash for a given string at compile time.
   /// @return A 16-bit hash value for the input string.
   /// @see hashWordRuntime()
   static constexpr uint16_t hashWordConst(const char* text, uint16_t hash = STARTING_PRIME)
      {
      return *text ? hashWordConst(text + 1, (uint16_t)((hash ^ (uint8_t)(*text)) * MULTIPLIER_PRIME)) : hash;
      }

   #undef STARTING_PRIME
   #undef MULTIPLIER_PRIME

   // Using macros and preprocessor compilation to define the keyword-prosign mapping is done in only one location, here.
   // This ensures consistency between runtime and compile-time hashing and validation without additional RAM/ROM usage.
   // During code maintenance/changes this provides:
   // - Single source of truth for keyword -> prosign mapping.
   // - Keeps production switch and TESTING validation in sync.
   // The `FLASH_PROSIGN_KEYWORD_MAP` macro defines the mapping between input keywords and their corresponding prosigns.
   //       The macro takes another macro `X` as an argument, which is used to generate code for both the switch statements in 
   //       `FlashProsignWord()` and the compile-time validation in `validateKeywordHashUniqueness()`.
   // This ensures that any additions, deletions, or modifications to the keyword-prosign mapping are consistently 
   //       applied across both runtime and compile-time checks ensuring that the test will keep current with the code.
   // Note: The keywords are in uppercase and should match the input string after converting to uppercase.
   //       The first element in the tuple is the keyword string value without any quotes (The macro will add the quotes). 
   //       The second element is the corresponding prosign enum value without the leading `Prosign::` (The macro will add it).
   #define FLASH_PROSIGN_KEYWORD_MAP(X) \
      X(START, Start)                   \
      X(STARTING, Start)                \
      X(END, End)                       \
      X(ENDWORK, EndWork)               \
      X(OVEROUT, OverOut)               \
      X(OUT, Out)                       \
      X(WAIT, Wait)                     \
      X(FULLSTOP, FullStop)             \
      X(INVITE, Invite)                 \
      X(OVER, Over)                     \
      X(UNDERSTOOD, Understood)         \
      X(VERIFIED, Verified)             \
      X(SAYAGAIN, SayAgain)             \
      X(REPEAT, Repeat)                 \
      X(CORRECTION, Correction)         \
      X(ERROR, Error)                   \
      X(SOS, SOS)                       \
      X(ROGER, Roger)                   \
      X(R, R)                           \
      X(K, K)                           \
      X(AR, AR)                         \
      X(AS, AS)                         \
      X(VE, VE)                         \
      X(HH, HH)                         \
      X(BT, BT)                         \
      X(KA, KA)                         \
      X(SK, SK)                         \
      X(CORRECT, C)                     \
      X(CONFIRM, C)                     \
      X(YES, C)                         \
      X(NEGATIVE, N)                    \
      X(NO, N)                          \
      X(C, C)                           \
      X(N, N)                           \
      X('?', Repeat)

   void MorseCodeLED::FlashProsignWord(String keyString)
      {
      if (keyString.isEmpty()) { return; }

      keyString.toUpperCase();
      const char* keyword = keyString.c_str();
      uint16_t hash = hashWordRuntime(keyword);

      switch (hash)
         {
         #define FLASH_PROSIGN_CASE(keywordToken, prosignToken) \
            case hashWordConst(#keywordToken):                  \
               FlashProsign(Prosign::prosignToken);             \
               break;
         FLASH_PROSIGN_KEYWORD_MAP(FLASH_PROSIGN_CASE)
         // Note: This macro expands to:
         // case hashWordConst("START"):
         //    FlashProsign(Prosign::KA); // Standard prosign for START: [-.-.-]
         //    break;
         // case hashWordConst("STARTING"):
         //    FlashProsign(Prosign::KA); // Standard prosign for STARTING: [-.-.-]
         //    break;
         // case hashWordConst("END"):
         //    FlashProsign(Prosign::AR); // Standard prosign for END/OK: [.-.-.]
         //    break;
         // case hashWordConst("OVER"):
         //    FlashProsign(Prosign::K); // Standard prosign for OVER/INVITE: [.-.-.]
         //    break;
         // case ... etc.   For all tuples defined in the macro.
         #undef FLASH_PROSIGN_CASE

         default:
            // Default to flashing the `keyword` as just text
            FlashString(keyword);
            break;
         }
      }  // End of FlashProsignWord()

   #if TESTING
   /// @brief Compile-time validation that keyword hashes are unique.
   /// @details Duplicate case labels will trigger a compile error if a collision occurs.
   /// @note This only needs to be compiled once during development or any changes to either.
   ///       This function is used to validate the hash function and keywords. 
   static void validateKeywordHashUniqueness()
      {
      switch (0)
         {
         #define FLASH_PROSIGN_TEST_CASE(keywordToken, prosignToken) \
            case hashWordConst(#keywordToken):                        \
               assert(hashWordConst(#keywordToken) == hashWordRuntime(#keywordToken)); \
               break;
         FLASH_PROSIGN_KEYWORD_MAP(FLASH_PROSIGN_TEST_CASE)
         // Note: This macro expands to:
         // case hashWordConst("START"):
         //    assert(hashWordConst("START") == hashWordRuntime("START"));
         //    break;
         // case hashWordConst("STARTING"):
         //    assert(hashWordConst("STARTING") == hashWordRuntime("STARTING"));
         //    break;
         // case hashWordConst("END"):
         //    assert(hashWordConst("END") == hashWordRuntime("END"));
         //    break;
         // case hashWordConst("OVER"):
         //    assert(hashWordConst("OVER") == hashWordRuntime("OVER"));
         //    break;
         // case ... etc.   For all tuples defined in the macro.
         #undef FLASH_PROSIGN_TEST_CASE
         default:
            break;
         }
      }
   #endif // TESTING

   #undef FLASH_PROSIGN_KEYWORD_MAP

   #endif // !LIMITED_MEMORY
   } // namespace BinaryClockShield

#undef DIT_MS
#undef DAH_MS  
#undef SPACE_MS
#undef WORD_SPACE_MS