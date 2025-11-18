# WiFiBinaryClock

## Binary Clock Shield for Arduino

[![Binary Clock Shield](./assets/BinaryClockShield.jpg)](https://nixietester.com/product/binary-clock-shield-for-arduino/)   
[![Binary Clock Shield](./assets/Binary-Clock-Shield-for-Arduino-UNO.jpg)](https://nixietester.com/product/binary-clock-shield-for-arduino/)

[The Binary Clock Shield for Arduino](https://nixietester.com/product/binary-clock-shield-for-arduino/) is the best LED Binary Clock I've seen and I own way too many binary clocks.

This project was created to unlock the full potential of this great Binary Clock. Being an Arduino Shield means that we can substitute the UNO R3 for something much more powerful. I wanted to get my Wemos D1 R32 ESP32 UNO board to work with this shield [^1], then it could connect to any NTP server over WiFi and keep the time synced whenever we switch to/from daylight savings time. In addition we could change the colors of the LEDs and upload new alarm melodies, etc. and do it from a phone or computer. The result of this project is the `WiFiBinaryClock` library which supports the following boards:

[^1]: The Wemos D1 R32 ESP32 UNO board requires a minor hardware modification to work with the Binary Clock Shield, see the details in the [Hardware Modifications](#hardware-modifications) section of this document.

## **Usage:**

The user needs to define the target board being used for this code to compile. The choices are:

1.   **ESP32\_D1\_R32\_UNO** - The generic Wemos D1 R32 UNO clone board with the original ESP32-32-WROOM module.
2.   **METRO\_ESP32\_S3** - The great [Adafruit Metro ESP32-S3](https://www.adafruit.com/product/5500) board.
3.   **ESP32\_S3\_UNO** - The generic UNO clone board with the new ESP32-S3 module.
4.   **UNO\_R4\_WIFI** - The new [Arduino UNO R4 WiFi](https://store.arduino.cc/collections/uno/products/uno-r4-wifi) board.
5.   **UNO\_R4\_MINIMA** - The [R4 Minima](https://store.arduino.cc/collections/uno/products/uno-r4-minima) board, no WiFi.
6.   **UNO\_R3** -  The original [Arduino UNO R3](https://store.arduino.cc/collections/uno/products/arduino-uno-rev3) board, no WiFi.
7.   **CUSTOM\_UNO** - An UNO board you define in [`board_select.h`](./lib/BinaryClock/src/board_select.h) and enable.

Add one of these defines to the compiler options (e.g. `-D METRO_ESP32_S3`) or include a preprocessor definition (e.g. `#define METRO_ESP32_S3`) in the [board_select.h](./lib/BinaryClock/src/board_select.h) file. The first 4 boards listed have builtin __WiFi__ [^2] so they will be able to adjust their time over WiFi, while the UNO R3 and R4 Minima do not have WiFi onboard so they are limited to time/alarm setting from the 3 buttons on the shield. If you have an UNO board that isn't listed, modify the **`CUSTOM_UNO`** board definitions in the [board\_select.h](./lib/BinaryClock/src/board_select.h) file with the custom definitions for your board. You will need to `#define CUSTOM_UNO true` before the board definition section to enable the custom board.

[^2]: The UNO R4 WiFi board uses the `WiFiS3.h` library instead of the ESP32 WiFi library, so some features such as WPS are not yet supported by Arduino. WiFi support for the UNO R4 WiFi board is NOT included in this code, the implementation is not complete.

The code was developed using the excellent [PlatformIO IDE](https://platformio.org/) extension for the [VSCode](https://code.visualstudio.com/) editor. The file, `platformio.ini`, includes the setup for each of the supported boards so all you need to do is specify which one you're using.

## **Supported Boards**

The following is a list of the boards that are directly supported by this code:

*   [The Arduino UNO R3 is supported](https://store.arduino.cc/collections/uno/products/arduino-uno-rev3)  This is the original board that the Binary Clock Shield was designed for. It works well with the shield but it doesn't have WiFi so you will need to set the time and alarms using the buttons on the shield. The limited amount of memory and lack of WiFi makes this a less than ideal choice for this project.   
    [![The Arduino UNO R3](./assets/Arduino_UNO_R3.webp)](https://store.arduino.cc/collections/uno/products/arduino-uno-rev3)  
*   [The Arduino UNO R4 Minima is supported](https://store.arduino.cc/collections/uno/products/uno-r4-minima)  This is a good board and a big improvement over the UNO R3. It doesn't have WiFi so you will need to set the time and alarms using the buttons on the shield. It does have a faster processor, more memory but at this price I'd prefer the [Adafruit Metro ESP32-S3](https://www.adafruit.com/product/5500) board as it has WiFi and so much more including a battery charging circuit.   
    [![The Arduino UNO R4 Minima](./assets/Arduino_UNO_R4_Minima.webp)](https://store.arduino.cc/collections/uno/products/uno-r4-minima)  
*   [The Arduino UNO R4 WiFi is supported](https://store.arduino.cc/collections/uno/products/uno-r4-wifi)  A great board and a vast improvement over the UNO R3. The addition of an ESP32-S3 module makes this a very capable board with lots of memory and two processors. The LED matrix is a great addition however, it seems like a waste to cover it up with the Binary Clock Shield. The WiFi is similar to the ESP32 WiFi except it uses `WiFiS3.h` instead and doesn't support WPS.  
    [![The Arduino UNO R4 WiFi](./assets/Arduino_UNO_R4_Wifi.webp)](https://store.arduino.cc/collections/uno/products/uno-r4-wifi)   
*   [The Adafruit Metro ESP32-S3 is supported](https://www.adafruit.com/product/5500).  This is a great board that has so many capabilities over every other choice. The JTAG port is good for development, the micro SD Card reader is a great addition and QT I2C connector on the side means it can be used with the shield in place. This opens up many additional capabilities that can be added, such as sensors for light and human presence. The greatest feature I like for the Binary Clock Shield is the builtin battery charger circuit. The shield will keep displaying the time while you move it around or when the power goes out. **This is the board I recommend** if you want to get the most out of the Binary Clock Shield especially if you create a case for it and have it work on battery.  
    [![The Adafruit Metro ESP32-S3](./assets/Adafruit_Metro_ESP32-S3.jpg)](https://www.adafruit.com/product/5500).  
*   There are other ESP32-S3 UNO style boards, such as one based on the ESP32-S3-DevKitC-1 pinout, these work well with the Binary Clock Shield. They are available from your favorite Chinese website for under $10. This code fully supports this board, no hardware modifications are needed as the pinouts are different and the ESP32-S3 doesn't appear to have INPUT only pins. If I had created a case for the Binary Clock Shield and wanted to sell a fully functional Binary Clock powered from a USB or A/C adapter, this is the board I would use. For a version with a battery, I'd stick with the [Adafruit Metro ESP32-S3](https://www.adafruit.com/product/5500).  
    ![ESP32-S3 UNO](./assets/ESP32-S3_UNO_Board.jpg)
*   The Wemos D1 R32 ESP32 UNO. This is the board that got me started on this project. On face value it has a lot of features compared to the Arduino UNO R3, more memory and WiFi. There are key differences that make it suitable only for those who are willing to make a minor hardware modification and already have this board.  
    ![The Wemos D1 R32 ESP32 UNO](./assets/Wemos_D1_R32_UNO.jpg)  
    While the board is supported, it does require a minor [Hardware Modification](#hardware-modifications) to work with the shield. See the details below.

## Hardware Modifications  
### Wemos D1 R32 UNO

The **Binary Clock Shield for Arduino** was designed to work with the Arduino UNO R3 board, however it can be used with other boards that have the same pinout headers as the UNO. When I got my first ESP32 based UNO board, the Wemos D1 R32, I tried it with the Binary Clock Shield only to find that it didn't work. I got errors for everything, I figured out that the board used different pin number compared to the UNO R3. I made the changes to the pin numbers and I still had errors compiling, this required further investigation. The Wemos D1 R32 ESP32 based UNO type board seemed like the ideal candidate. I read the datasheet and discovered it had a hardware limitation. The shield uses UNO pin `A3` for the Neopixel LED data out pin, this pin corresponds to the ESP32 `GPIO 34` pin which is an __input only__ pin. The only other limitation was the builtin LED which is wired to `GPIO 02` which corresponds to the UNO pin `A0` which is used by the shield for pushbutton `S3` to set the alarm and increment values. This means we can't use the `LED_BUILTIN` LED, so we don't use the builtin LED, instead we remap it to another pin that does nothing.

In order to get this _Wemos D1 R32_ board to work with the shield, the pin corresponding to `A3` on the shield needs to be connected to an output capable pin such as `GPIO 15`. To do this you need to physically remove the `A3/GPIO34` socket from the ESP32 UNO board (cut the plastic and desolder the pin) then connect the corresponding shield pin to `GPIO 15`. If you wand a simpler fix, you could cut the plastic spacer on the shield corresponding to the `A3` pin and bend it in, then connect that bent pin to `GPIO 15`.

![Wemos D1 R32 ESP32 UNO](./assets/Pinout_Wemos_D1_R32.jpg)

The alternative is to get an Arduino UNO Development Shield and modify the development shield by bending the `A3` pin and use a Dupont connector between the bent `A3` pin and `GPIO 15` to use this output capable GPIO pin. This is the easiest but it does add some height, ~12mm or ½ inch, to the assembly.

![UNO Development Shield](./assets/Modified_UNO_Shield.jpg)

The advantages of a development Shield are that you can add additional components, for example, you could add a LDR circuit to monitor and adjust the brightness based on ambient light and or a potentiometer to adjust the brightness.

## The Software

### Background

The software was initially based on the [example/11-BinaryClockRTCInterruptAlarmButtons](https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino/tree/master/example/11-BinaryClockRTCInterruptAlarmButtons) on the GitHub [marcinsaj/Binary-Clock-Shield-for-Arduino](https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino). I refactored everything to create a pure Interface class, [`IBinaryClock`](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/IBinaryClock.h), to fix the minimum capabilities and decouple the classes. The main [`BinaryClock`](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BinaryClock.h) class implements the interface and handles all the base operations of the Binary Clock Shield. The class would perform everything related to the Binary Clock Shield, such as handling the display of the time, alarm and interface screens on the NeoPixel LED matrix. Additional classes were created to handle the button pressing and debouncing, serial output and settings menu. The [`BCSettings`](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BCSettings.h) class handles all the settings menu operations. This class contains most of the settings logic and methods, modified to work in a class with the new user interface. The [`BCButtons`](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BCButtons.h) class handles all the button pressing and debouncing. The `BCSerial` class handles all the serial output, this is useful for debugging and can be disabled to save memory. The [`MorseCodeLED`](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/MorseCodeLED/src/MorseCodeLED.h) class handles the Morse code blinking on an LED for error codes. The `BinaryClockWiFi` class handles all the WiFi operations, such as connecting to an access point, getting the time from an NTP server and uploading new alarm melodies. The `BinaryClock` class uses the [Adafruit Bus IO](https://github.com/adafruit/Adafruit_BusIO) library; [Adafruit RTClib](https://github.com/adafruit/RTClib) library; and [FastLED](https://github.com/FastLED/FastLED) library.

The `BinaryClock` class was designed to be used as a library so it could be part of another project. The class uses callback functions to notify the main program when the time is updated or when an alarm goes off. This allows the main program to handle these events as needed. The main program could be altered for the different board capabilities, or lack of capabilities. The Arduino UNO R3 and R4 Minima boards don't have WiFi so they are limited to setting the time and alarms using the buttons on the shield. The ESP32 based boards have WiFi so they can connect to an NTP server to get the time and adjust for daylight savings time automatically. The ESP32 based boards can also be used to change the LED colors and upload new alarm melodies. The WiFi capabilities are not part of the `BinaryClock` class, they are handled in their own library class, `BinaryClockWiFi`. The [`BinaryClock.Defines.h`](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BinaryClock.Defines.h) file contains all the global definitions for the different boards, their capabilities, and other defines used in the code. The main program can use these definitions to enable or disable features based on the board being used. The [`board_select.h`](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/board_select.h) file can be used to define the pin assignments and board capabilities for any UNO style board not currently supported. The [`BinaryClock.Structs.h`](https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BinaryClock.Structs.h) file contains all the global data structures used in the library.

![ESP32 Binary Clock](./assets/BinaryClock_ESP32.jpg)  

### Features

The `BinaryClock` class extended the basic capabilities of the original code by:

1.  Adding support for many different UNO boards and allowing for users to define their own board:
    *   The code supports the original [Arduino UNO R3](https://store.arduino.cc/collections/uno/products/arduino-uno-rev3) board.
    *   The new [Arduino UNO R4 WiFi](https://store.arduino.cc/collections/uno/products/arduino-uno-rev4-wifi) & [Minima](https://store.arduino.cc/collections/uno/products/arduino-minima) boards.
    *   The [Adafruit Metro ESP32-S3](https://www.adafruit.com/product/5400) board.
    *   The ESP32-S3 based UNO boards.
    *   The Wemos D1 R32 ESP32 based UNO board, which requires a hardware modification to work with the Binary Clock Shield.
    *   The user can define their own board by modifying the [`board_select.h`](./lib/BinaryClock/src/board_select.h) file.
2.  Adding full support for displaying the hours in 12 hour format with AM/PM indicator in addition to the 24 hour format.
    *   The user can change the hour format in the Time setting menu.
    *   The alarm hours format follows the selected time hours format.
    *   The DS3231 RTC chip is set to the selected time format, so the selected format will return after the power is lost.
3.  Changes to the User eXperience (UX) by allowing the user to exit the time and alarm settings menu without making any changes.
    *   For time setting, the user first selects: __12__ hour; __24__ hour; or e**X**it.
        *   The eXit is shown with a large **X** \[❌\] on the LEDs.
        *   The __12__ hour mode is shown with the PM indicator and a binary value of 12 on the hours row.
        *   The __24__ hour mode is shown as the binary value of 24 on the hours row.
        *   The user presses the `S2` button as usual to save and move on to the next section.
            *  If the user selected e**X**it, the time setting menu is aborted and no changes are made.
            * If the user selected __12__ hour or __24__ hour mode, the selection is saved, a large **✓** \[✅\] is shown on the LEDs indicating a the start of the next section, setting the time.   
              The user continues as normal to set the hours; minutes; and seconds.
    *   For the alarm settings, the user first selects: On; oFF; or eXit.
        *   The eXit is shown with a large **X** \[❌\] on the LEDs.
        *   The On is shown as a large **O** on the LEDs.
        *   The oFF is shown as a large sideways **F** on the LEDs.
    *   When the user has finished with the settings menu (Alarm or Time) the following occur:
        *   The screen is filled with the colors of the rainbow signaling the end of the setting menu.
        *   The result of the settings menu is displayed to the user:
            *   Success: a large **✓** \[✅\] is shown on the LEDs indicating the changes were saved.
            *   Cancel/Abort: a large **X** \[❌\] is shown on the LEDs indicating nothing was changed.
4.  Adding support for changing the colors of the NeoPixel LED indicators for the: hours; minutes; and seconds as well as AM and PM indicators.
    *   The user can change the ON color for each individual LED indicator as well as change the OFF color for all the LEDs.
        *   Default ON colors are:
            *   hours = Blue \[🟦\] ;
            *   minutes = Green \[🟢\] ;
            *   seconds = Red \[🟠\] ;
            *   AM = SkyBlue <font size=4 color=skyblue>\[⬤\]</font> ;
            *   PM = Indigo \[🟣\] ;
        *   Default **OFF** color is: Black \[■\] .
            *   Using any color other than black means the LED will always be ON.
5.  Adding support for playing different, user supplied, melodies for the alarms.
    *   The user can upload their own melodies and use them for the alarms.
        *   The melody is stores as an array of `Notes` that represent the pitch and duration of each note.
            *   The pitch is stored as the frequency in Hertz (Hz) of each note.
            *   The duration is stored as the number of milliseconds (ms) each note is played.
        *   The melody is played using the `tone()` function on the UNO R3 and R4 boards.
        *   The melody is played using the `ledcWriteTone()` function on the ESP32 based boards.
6.  Added callback handling for the Time and Alarm. This allows the `BinarClock` class to be implemented as a library and be part of another class or project.
    *   The `Time` callback is called every second when the time is updated.
    *   The `Alarm` callback is called when the alarm goes off.
    *   The callbacks remain active even when the user is in the settings menu and the time isn't being displayed on the shield.
    *   The callback handler is implemented differently depending on the board being used:
        *   For the `UNO R3` and `R4 Minima` boards, the callback is handled in the `loop()` method by checking a flag that is set in the RTC alarm interrupt service routine (ISR). On these boards the callback routine are blocking calls.
        *   For the `ESP32` based boards running `FreeRTOS`, the callback is handled in a separate task. The task waits for a notification from the `BinaryClock::timeDispatch()` method and calls the `Time` or `Alarm` callback routines from its own thread.
7.  Added error handling for critical errors where the program can't continue such as not being able to communicate with the RTC chip.
    *   The error handling consists of displaying an error code on a LED on the board such as the builtin LED.
    *   Watchdog time is triggered after ~2.1 seconds has elapsed without an update.
    *   Instead of using some custom code of blinking LEDs I decided to have some fun and use Morse code to display the error code. My first instinct was to flash SOS, but I learned that this is not cool. SOS is reserved for an actual distress signal and must __never__ be used for anything else. So I decided to write a Morse Code class to blink the message on the LEDs. I also took the opertunity to see what AI (in this case CoPilot) could come up with and it did a mixed job. The encoding of the Morse code was a good idea from CoPilot but then it couldn't get the codes correct for all letters, numbers and punctuation. It also wrote lots of duplicate code in the methods and the code wasn't at all robust. It was an educational experience, you need to have experience or CoPilot will lead you down the garden path.
        *   The error code is displayed as a series of Morse code blinks on the LED. The SOS is **NOT** used as this is an actual distress signal. **CQD** should used instead, which stands for "Come Quick Distress". This is the original distress signal, from the early 1900s, that was fully replaced by SOS after the sinking of the Titanic. **CQD** is probably a good choice when it isn't an actual life critical distress that needs to be communicated.
            *   The default message, when the program enters the `purgatoryTask()` is **CQD NO RTC** in Morse code. This stands for "Come Quick Distress NO Real Time Clock".
            *   The UNO R3 can only play this message as it doesn't have enough memory to store the codes needed for additional messages. All other boards can blink any message in Morse code on the assigned LED. The default is to use the `LED_BUILTIN` LED on the board, however it can be changed to any other LED pin.
        *   The Morse code is implemented from the `MorseCodeLED` class in the [MorseCodeLED.h](./lib/MorseCodeLED/MorseCodeLED.h) and [MorseCodeLED.cpp](./lib/MorseCodeLED/MorseCodeLED.cpp) files.

### Design Overview

The **`WiFiBinaryClock`** Project is designed to be modular and extensible, allowing it to be easily integrated into different projects and platforms. The main library, `BinaryClock` is structured around a core set of classes that handle the various aspects of the binary clock functionality, including timekeeping, alarm management, and user interactions. The `BinaryClockWiFi` library is designed to handle all the WiFi related functionality, such as connecting to an access point, getting the time from an NTP server and uploading new alarm melodies. This library also provides a way to manage the WiFi connection, including WPS and credential setting and management through a web page, saving the credentials and user settings to NVS, and handle any related events. The `RTClibPlus` library is a forked version of the popular [Adafruit RTClib library](https://github.com/adafruit/RTClib) with expanded functionality. The `MorseCodeLED` library handles the Morse code blinking on an LED for error codes and is designed to be used independently of the Binary Clock library.

#### The main components of the `BinaryClock` library include:  

0. **`IBinaryClock`**: An interface class that defines the core functionality of the binary clock. This class provides a common interface for different implementations of the binary clock, and providing a consistent API for interacting with the clock.   
      This provides a level of decoupling that makes testing of the components easier.   
      The Interface class, by design, allows passing an implementing class to the sub-classes without creating circular dependencies.   
      This class is defined in the [`IBinaryClock.h`](./lib/BinaryClock/src/IBinaryClock.h) file.

1. **`BinaryClock`**: The primary class, inherits from the interface class `IBinaryClock`, it manages the overall functionality of the binary clock.   
      It handles timekeeping, alarm management, and user interactions through button presses. It also manages the display of the time and alarm status on the LED matrix.   
      This class is responsible for coordinating the various components of the binary clock and providing a unified interface for users to interact with the clock.   
      This class is defined in the [BinaryClock.h](./lib/BinaryClock/src/BinaryClock.h) and [BinaryClock.cpp](./lib/BinaryClock/src/BinaryClock.cpp) files.

2. **`BCButton`**: A class that abstracts the button functionality, providing a consistent interface for reading button states and managing button presses.   
      This class is responsible for debouncing button presses and translating them into meaningful actions within the binary clock.   
      This class is defined in the [BCButton.h](./lib/BinaryClock/src/BCButton.h) and [BCButton.cpp](./lib/BinaryClock/src/BCButton.cpp) files.

3. **`BCSettings`**: A class that manages the settings and configuration of the binary clock. This class is responsible for storing and retrieving user preferences, such as time format and alarm settings.   
      It provides a consistent interface for accessing and modifying the clock's configuration.   
      This class is defined in the [BCSettings.h](./lib/BinaryClock/src/BCSettings.h) and [BCSettings.cpp](./lib/BinaryClock/src/BCSettings.cpp) files.

4. **`BinaryClock.Defines.h`**: A header file that contains all the preprocessor definitions for the different boards and their capabilities. This file is used to configure the library for different hardware platforms and enable or disable features based on the board being used.  
      This header file is used by every class to simplify the code for all the different boards and configurations.  Defines and MACROs are used to manage the selective code used throughout the library based on the board being used and the configuration.  
      Custom board definitions and configurations are possible using the [board_select.h](./lib/BinaryClock/src/board_select.h) file where the user can define a `CUSTOM_UNO` board that will be used instead of any predefined board. If this file exists, it is automatically included at the start of the `BinaryClock.Defines.h` file overriding the predefined board definitions.   
      This file is located at [BinaryClock.Defines.h](./lib/BinaryClock/src/BinaryClock.Defines.h).

5. **`BinaryClock.Structs.h`**: A header file that contains all the global data structures used in the library. This file defines the various data structures used to represent time, alarms, and other settings within the binary clock.  
      This file is located at [BinaryClock.Structs.h](./lib/BinaryClock/src/BinaryClock.Structs.h).

#### The main components of the `BinaryClockWiFi` library include:

0. **`BinaryClockWiFi`**: A class that handles all the WiFi operations for the binary clock. This class is responsible for connecting to a WiFi access point, getting the time from an NTP server, and uploading new alarm melodies.  
      The class also provides a way to manage the WiFi connection, including WPS and credential setting and management through a web page, saving the credentials and user settings to NVS, and handle any related events.  
      This class is defined in the [BinaryClockWiFi.h](./lib/BinaryClockWiFi/src/BinaryClockWiFi.h) and [BinaryClockWiFi.cpp](./lib/BinaryClockWiFi/src/BinaryClockWiFi.cpp) files.

1. **`BinaryClockWPS`**: A class that handles the WPS operations for the binary clock. This class is responsible for initiating the WPS process and managing the connection to the WiFi access point using WPS.  
      This class is defined in the [BinaryClockWPS.h](./lib/BinaryClockWiFi/src/BinaryClockWPS.h) and [BinaryClockWPS.cpp](./lib/BinaryClockWiFi/src/BinaryClockWPS.cpp) files.

2. **`BinaryClockNTP`**: A class that handles the NTP operations for the binary clock. This class is responsible for getting the time from an NTP server and updating the binary clock's time accordingly.  
      This class is defined in the [BinaryClockNTP.h](./lib/BinaryClockWiFi/src/BinaryClockNTP.h) and [BinaryClockNTP.cpp](./lib/BinaryClockWiFi/src/BinaryClockNTP.cpp) files.            

3. **`BinaryClockSettings`**: A class that manages the WiFi settings and configuration of the binary clock. This class is responsible for storing and retrieving user preferences related to WiFi, such as SSID and password.   
      It provides a consistent interface for accessing and modifying the WiFi configuration.   
      This class is defined in the [BinaryClockSettings.h](./lib/BinaryClockWiFi/src/BinaryClockSettings.h) and [BinaryClockSettings.cpp](./lib/BinaryClockWiFi/src/BinaryClockSettings.cpp) files.

#### The main components of the `MorseCodeLED` library include:

0. **`MorseCodeLED`**: A class that handles the Morse code blinking on an LED for error codes. This class is responsible for encoding messages into Morse code and controlling the LED to blink the corresponding dots and dashes.  
      The class was created to provide visual feedback for error states in the binary clock when the shield components fail, such as the Real Time Clock or NeoPixel LEDs. This typically occurs when the Binary Clock Shield is not connected to the UNO board, so a visual indication is needed to alert the user.  
      To avoid looking like a blinky sketch is installed, a more sophisticated error handling mechanism is used, we use Morse code on the LED. While 99.999% of people won't be able to understand it directly, it does indicate to the user that a message is being conveyed.   
      This class is defined in the [MorseCodeLED.h](./lib/MorseCodeLED/MorseCodeLED.h) and [MorseCodeLED.cpp](./lib/MorseCodeLED/MorseCodeLED.cpp) files.

## **Note:**

This code uses a forked version of [Adafruit's RTClib library](https://github.com/adafruit/RTClib) (see the forked [RTClib README.md](./lib/RTClibPlus/README.md) file for more information).   
The forked library is called `RTCLibPlus` and is available on GitHub at [https://github.com/Chris-70/RTClibPlus](https://github.com/Chris-70/RTClibPlus). It has been modified to:

*   Make the inherited base class `RTC_I2C` public for the `RTC_DS3231` class (and all other child classes), e.g. `class RTC_DS3231 : public RTC_I2C`.
*   It removes the DS3231 interrupt enable check (register 0x0E, bit: 4) when setting alarms. This check has no reason to be there as setting the alarm time on the DS3231 chip is independent of the interrupt setting.  
    The Binary Clock makes use of the SQW pin for the 1 Hz signal, this is the same physical pin as the alarm interrupt pin.   
    The Binary Clock needs to set the alarm time values as the code checks for the alarm without needing the interrupt pin. This allows the rest of the code to set the alarm time registers.  
    In the methods: `bool RTC_DS3231::setAlarm1(const DateTime &dt, Ds3231Alarm1Mode alarm_mode)` and `bool RTC_DS3231::setAlarm2(const DateTime &dt, Ds3231Alarm1Mode alarm_mode)`, the removed code was:  
```cpp
//   if (!(ctrl & 0x04)) {
//     return false;
//   }
```

*   An additional `toString()` method was added to the `DateTime` class, `char* toString(char* buffer, size_t size, const char *format) const;`.  
    This method takes the `format` string and copies it to the `buffer` before calling `char *toString(char *buffer) const;` allowing the method to be used inline without the need to format the buffer first.  
    Example: 
    ```cpp
    Serial << time.toString(buffer, 31, "hh:mm AP on DDD. MMM. DD, YYYY");
    // e.g. For: "09:15 AM on Fri. Jul. 19, 2024"
    ```
    instead of needing two lines: 
    ```cpp
    strncpy(buffer, "HH:mm AP on DDD. MMM. DD, YYYY", 32); 
    Serial << time.toString(buffer);
    // e.g. For: " 1:30 PM on Fri. Aug. 05, 2022"
    ```
*   The string format was expanded to include:
    *   `hh` - hours with    a leading zero:` "01" - 12, "00" - 23 `   _(Original)_
    *   `HH` - hours without a leading zero:` " 1" - 12, " 0" - 23 `   _(New)_
    *   `AP` - AM/PM indicator: `"AM" or "PM"`                         _(New)_

Additional features were added to RTClibPlus:

*   Added full support for 12 hour mode on the `DS3231` and `DS1307` RTC chips. These chips support 12 hour mode but the RTClib library didn't implement it.  
    * The `DateTime` class was modified to support reading and writing the 12 hour mode to/from the RTC chips. The `DateTime` class was modified to support reading and writing the 12 hour mode to/from the RTC chips.  
    * The `DateTime` constructor was modified to accept a new parameter, `is12HourFormat`, which defaults to `false` (24 hour mode).  
    * The `RTC_DS3231::adjust()` and `RTC_DS1307::adjust()` methods were modified to write the 12 hour mode to the RTC chips if the `is12HourFormat()` returns `true` or if the optional parameter `use12HourFormat` is set to `true`.
*   Added full support to set the __starting day of the week__ in the `DateTime` class at compile time. The original implementation defined May 1st, 2000 as the epoch for calculating the weekday as this date was a Monday.   
    * This was extended this concept to match every day of the week with the first corresponding month in the year 2000 where the 1st fell on that weekday.  
      This matches the weekday with the RTC that doesn't define a starting weekday just that they are consecutive from 1 - 7 (0 - 6 in DateTime). 
   * The developer just needs to modify the `#define FIRST_WEEKDAY "Mon"` line in the [`RTClib.h`](./lib/RTClibPlus/src/RTClib.h) file to the desired starting weekday.  
   The choices are: `"Mon"`, `"Tue"`, `"Wed"`, `"Thu"`, `"Fri"`, `"Sat"`, and `"Sun"`. The default is `"Mon"` which matches the original implementation.

A fork of the `RTCLib`, `RTCLibPlus`, is available ([https://github.com/Chris-70/RTClibPlus](https://github.com/Chris-70/RTClibPlus)) while the pull request, # 313, for `RTClib` is pending.

### License

This WiFiBinaryClock software, Copyright (c) 2025 Chris-70 and Chris-80, is licensed under the GNU General Public License v3.0 (GPL-v3.0). You may obtain a copy of the License at: https://www.gnu.org/licenses/gpl-3.0.en.html (see [**LICENSE**](https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/src/LICENSE) file). Parts of the `BCSettings` class are also Copyright (c) 2018 Marcin Saj and released under the GPL-v3.0 license.

## **Acknowledgements:**
*   Marcin Saj, the original creator of the [Binary Clock Shield for Arduino](https://nixietester.com/product/binary-clock-shield-for-arduino/) and the author of the [Binary-Clock-Shield-for-Arduino GitHub repository](https://github.com/nixietester/Binary-Clock-Shield-for-Arduino).
*   [Adafruit](https://www.adafruit.com/) for their excellent [RTClib library](https://github.com/adafruit/RTClib), their commitment to open source hardware and software and the numerous libraries, [Adafruit Repositories](https://github.com/adafruit), and all the software they provide to the community.   
    Their contribution to education and teaching with [Adafruit Learning System](https://learn.adafruit.com/) needs to be acknowledged as well. They are a great example of how companies can to do open source hardware and software right.
*   FastLED community for their excellent [FastLED library](https://github.com/FastLED/FastLED).
*   Janelia for their excellent [Janelia Arduino Streaming library](https://github.com/janelia-arduino/Streaming).
*   Arduino community for their excellent [Arduino IDE](https://www.arduino.cc/en/software) and [Arduino CLI](https://arduino.github.io/arduino-cli/latest/) tools.
*   PlatformIO community for their excellent [PlatformIO IDE](https://platformio.org/) extension for VSCode.
*   Everyone in the open source community who contributes to the libraries and tools that make projects like this possible.

## **Contact Information:**
*   Chris-70:
    *   GitHub: [https://github.com/Chris-70](https://github.com/Chris-70)
*   Chris-80:
      *   GitHub: [https://github.com/Chris-80](https://github.com/Chris-80)