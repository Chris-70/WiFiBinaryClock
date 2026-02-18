# BinaryClock Library - Software to Control and Interface with the Hardware  

[![GitHub release](https://img.shields.io/github/release/Chris-70/WiFiBinaryClock.svg?style=flat-square)]
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?style=flat-square)]

The main software that interfaces with the Binary Clock Shield hardware, managing time display, alarms, button handling, and menu navigation.  
___
[![*Binary Clock Shield image*](../../assets/BinaryClockShield.jpg)][shield]   
The `BinaryClock` library provides functionality to display time on a binary clock shield using various microcontroller boards such as Arduino UNO, ESP32, and others. It supports features like time display in binary format, alarm settings, button handling, and more.  
This library is part of the larger [WiFiBinaryClock project][WiFiBinaryClock], which integrates Wi-Fi connectivity and NTP time synchronization. This library specifically focuses on the core functionalities of the binary clock itself.  
The library was designed for the [Binary Clock Shield for Arduino][shield]. If you build your own, it is expected to have the following components at a minimum:
- 17x RGB [NeoPixel][neopixel] LEDs (for binary time display)
   - 5x LEDs for hours (0-23)/(1-12 + AM/PM indicator)
   - 6x LEDs for minutes (0-59)
   - 6x LEDs for seconds (0-59)
- 3x Buttons (for user interaction)
   - S1 - Alarm set / increment
   - S2 - Mode / save
   - S3 - Time set / decrement
- 1x [DS3231 RTC][rtc] module (for accurate timekeeping)
   - The RTC module communicates over the I2C bus.
- 1x [KELIKING KLJ-1230][piezo] Piezo buzzer (for alarm melodies)
   - The piezo buzzer is used to play melodies for the alarm sound.

## Details
This library is designed to be modular and extensible, allowing for easy integration with different hardware configurations and additional features. The core functionalities include:
- **Time Display**: The library manages the display of time in binary format using the [NeoPixel][NeoPixels] LEDs.
- **Alarm Functionality**: Users can set alarms, and the library handles the alarm triggering and melody playback using the [KJL-1230 Piezo][piezo] buzzer.
- **Button Handling**: The library includes debouncing and event handling for the buttons, allowing users to interact with the clock settings.
- **RTC Integration**: The library interfaces with the [DS3231 RTC][rtc] module to ensure accurate timekeeping.
- **Menu System**: A menu system is implemented to allow users to navigate through different settings and options to set the alarm, time, and other preferences.

The WiFi functionality and NTP synchronization are handled in a separate library, namely [BinaryClockWiFi][BinaryClockWiFi_lib]. This separation allows for a clear distinction between the core binary clock functionalities and the network-related features, making it easier to maintain and extend each component independently. Boards that do not have WiFi capabilities can still use the `BinaryClock` library without any dependencies on WiFi features.

The library was written to support multiple boards making use of their capablilities. The Arduino UNO R3 board is the most restrictive board that is supported, as a result not all features are available on this board. The other supported boards are assumed to be capable of including the C++ RTL and supporting the FreeRTOS operating system features. Conditional compilation is used to include or exclude features based on the target board capabilities. Boards that are not specifically supported can be used by defining `CUSTOM_UNO` as `true` and defining the required pin numbers and capabilities in the [`board_select.h`][boardselect] file. Default values, parameters and definitions can all be overridden in the `board_select.h` file as required.

The only fundamental restrictions that the library has to display binary time are:
- The [NeoPixels][NeoPixels] data pins are arranged where the __LSB__ of the seconds is on the first display LED and the __MSB__ of the hours is on the last display LED. The LED rows are arranged in the order: Seconds, Minutes, Hours (with __AM/PM__ indicator as the last Hour LED), data flows from __LSB__ to __MSB__ on each row. Additional LEDs are permitted on each row as long as the physical offsets are adjusted accordingly. 
   - The code can accomidate NeoPixel arrays that are larger than required to display the time by adjusting the physical offsets to the start of each time section (i.e. seconds, minutes and hours). 
   - The __AM/PM__ indicator LED is assumed to be the last Hour LED, meaning the hour row can have 5 or 6 LEDs.
   - The NeoPixels must be connected in series, with a single data input pin.
   - The NeoPixels must be powered with a suitable 5V power supply and have a common ground with the microcontroller board. The microcontroller board can be powered with 5V or 3.3V as required.
- An I2C interface is required to connect to the DS3231 RTC module.
   - The I2C bus speed must be compatible with the DS3231 module (typically 100kHz or 400kHz), as defined by the [RTC data sheet][rtc].
   - The I2C bus can be shared with other devices as long as there are no address or speed conflicts.
- The piezo buzzer must be connected to a PWM-capable digital output pin for melody playback.
   - The buzzer should be able to handle the voltage and current levels provided by the microcontroller board.
   - The buzzer can be driven directly from the microcontroller pin or through a suitable driver circuit if required.
   - The buzzer plays melodies based on the tone melodies as described in the Arduino tutorials for [tone][tonetutorial] and [melody][tonemelody].
- A minimum of 3 buttons for user interaction.
Provided these hardware requirements are met, the library can be used to work with a wide range of microcontroller boards and binary display configurations.

## Software Details
The library is structured into several key components:
- **Interface Classes**: Located in the [BCGlobalDefines][BCGlobalDefines] library, these define the interfaces for the Binary Clock and its components, ensuring a consistent structure across implementations. ([IBinaryClock.h][IBinaryClock], [IBinaryClockBase.h][IBinaryClockBase], [IBCButtonsBase.h][IBCButtonBase])
- **BinaryClock Class**: The main class that manages the binary clock's functionality, including time display, alarm settings, and button handling. ([BinaryClock.h][BinaryClock], [BinaryClock.cpp][BinaryClock_cpp])
- **BCButtons Class**: Handles button inputs, including debouncing and event detection. ([BCButtons.h][BCButton], [BCButtons.cpp][BCButton_cpp])
- **BCMenu Class**: Manages the menu system for user interaction with the clock settings. ([BCMenu.h][BCMenu], [BCMenu.cpp][BCMenu_cpp])
- **board_select.h**: Contains all board-specific custom defines and pin definitions to ensure desired functionality for the specific implementation. ([board_select.h][boardselect])

The class diagram for the `BinaryClock` library is contained in [**CLASS_DIAGRAM.md**][CLASS_DIAGRAM] and shows the relationships between the classes and interfaces in the library. The diagram illustrates how the `BinaryClock` class implements the `IBinaryClock` interface, and how the `BCMenu` and `BCButtons` classes interact with the `BinaryClock` class through the defined interfaces.  

The classes in this library implement the interfaces defined in the [BCGlobalDefines][BCGlobalDefines] library. This ensures that the `BinaryClock` library adheres to a consistent structure and can be easily integrated with other components of the WiFiBinaryClock project. Interfaces are created when when two or more classes need to reference the class.
- [**IBinaryClock** interface class][IBinaryClock] defines the standard features and functionalities that other classes can expect from a Binary Clock implementation. This interface class inherits from the [`IBinaryClockBase` interface class][IBinaryClockBase] which limits functionality for boards such as the UNO_R3.
   - [**BCMenu** class][BCMenu] requires the [`IBinaryClockBase` interface class][IBinaryClockBase] to interact with the Binary Clock for menu operations. The selections made by the user need to be communicated back to the Binary Clock instance.
   - [**BinaryClockWAN** class][BinaryClockWAN] requires the [`IBinaryClock` interface class][IBinaryClock] to access the full range of Binary Clock functionalities, including time display and alarm settings, for network-related operations such as setting the time.
- [**IBCButtonBase** interface class][IBCButtonBase] defines the basic features and functionalities that other classes can expect from a Button implementation. This interface class is needed by other classes to interface with the button functionality.
   - [**BCMenu** class][BCMenu] requires the [`IBCButtonBase` interface class][IBCButtonBase] to handle button events for menu navigation and selection.
   - [**BinaryClock** class][BinaryClock] requires the [`IBCButtonBase` interface class][IBCButtonBase] to manage button inputs for setting the time and alarms.
The [BinaryClock][BinaryClock] class implements the [`IBinaryClock` interface class][IBinaryClock], it is a large class that interfaces and controls the shield hardware. The main sections of the class are:
- **Time Management**: Functions to manage the time display, including updating the NeoPixels based on the current time and handling time changes.
- **Alarm Management**: Functions to set, check, and trigger alarms, including playing melodies on the piezo buzzer when an alarm goes off.
- **Button Handling**: Functions to read button states, handle debouncing, and trigger events based on button presses.
- **Menu System**: Functions to manage the menu system, allowing users to navigate through settings and options using the buttons.
- **RTC Integration**: Functions to interface with the DS3231 RTC module for accurate timekeeping and synchronization.
- **Serial Output**: Functions to manage serial output for debugging and user interaction through the serial monitor.
- **Configuration and Customization**: Functions to allow for configuration of various parameters such as brightness, alarm melodies, and more.
- **Board-Specific Implementations**: Conditional compilation is used to include or exclude features based on the target board capabilities, ensuring that the library can run on a wide range of hardware configurations.
- **Display Management**: Functions to manage the NeoPixel display, including brightness control, color settings, and animations for time changes or alarm notifications. Display of the menu screen patterns such as: On; oFF; OK; Fail/cancel, etc. in addition to the time display.  

The `setup()` function initializes the hardware components, sets up the RTC, and prepares the clock for operation. The main components of the `setup()` function are:
- The `SetupRTC()` method initializes the RTC module, registers an interrupt handler for the 1 Hz square wave output from the RTC, reads the current time format from the RTC (i.e. AM/PM or 24 Hr. mode) to set the display mode and finally reads the current time. If the `SetupRTC()` method fails, the `BinaryClock` instance is sent to `Purgatory()` and remains there until an RTC modeule is detected.
- The `SetupFastLED()` method initializes the FastLED library for controlling the NeoPixel LEDs, setting the appropriate data pin and LED count based on the board configuration as well as the initial brightness. The array size of `CRGB` LED elements is the value defined by `TOTAL_LEDS` and initially set to `CRGB::Black`. The display only uses `NUM_LEDS` (e.g. 17) LEDs for the binary time and screen patterns.
   - The `LedPattern::rainbow` pattern is displayed on the the LEDs after `FastLED` initialization to confirm the LEDs are working and to indicate that the clock is in the setup phase.  
   - Depending on the parameter value, all patterns except the `LedPattern::off` pattern are displayed in sequence to the user ending with the rainbow pattern before exiting the setup. Boards that support `FreeRTOS` features run the pattern display in a separate task to avoid blocking the setup process allowing other components, suxh as WiFi, to continue to setup.
- The `SetupAlarm()` method initializes the alarm settings, including setting default alarm times and melodies. The alarm settings are read from the RTC module and the values and state are stored in the `BinaryClock` instance. The alarm settings can also displayed on the serial monitor for user reference.
- Boards that run `FreeRTOS` setup a task, `TimeTask()`, to handle the time and alarm display updates, removing this work from the main loop to focus on handling user input and other tasks. If the task can't be created, the execution is sent to `Purgatory()` indicating a critical failure that needs to be resolved before the clock can operate. The task only runs when the RTC signals a time change, i.e. every second.  

The `loop()` function primarily handles user input from the buttons and manages the menu system. The binary time display as well as the alarm sound are done from within the `loop()` to accomidate boards that do not support `FreeRTOS` features. The main components are:
- **ProcessMenu()**: This function manages the menu system, allowing users to navigate through different settings and options using the buttons. It checks for button presses, updates the menu state, and applies any changes made by the user.
- **DisplayBinaryTime()**: This function updates the NeoPixel display based on the current time, showing the hours, minutes, and seconds in binary format as well as the AM/PM indicators when in 12 hour mode.
- **PlayAlarm()**: This method plays the selected alarm melody on the piezo buzzer when an alarm goes off. Currently this is a blocking call so the time display will freeze while the melody is playing. 
- **CheckHardwareDebugPin()**: This method is used for debugging purposes to check the state of a hardware debug pin, allowing for testing and troubleshooting of the clock's operation. This method only exists when `HARDWARE_DEBUG` is defined as `true` for the development boards only.  

Note: The `BinaryClock` library needs to work with an Arduino UNO R3 board as a minimum (2 KB RAM and 32 KB Flash), this requires some features to be removed from the library when compiling for this board. The library only implements the [`IBinaryClockBase`][IBinaryClockBase] interface class when compiling for the UNO R3 board so that the code can fit on the board. When compiling for other boards with more resources, the full [`IBinaryClock`][IBinaryClock] interface class is implemented, providing additional standard features and functionalities.


## Dependencies
### Local Libraries
The `BinaryClock` library depends on several other local libraries for its functionality:
- [BCGlobalDefines][BCGlobalDefines]: Provides the interface class definitions and global definitions and interfaces used across all the Binary Clock related libraries. ([BinaryClock.Defines.h][BinaryClockDefines], [BinaryClock.Structs.h][BinaryClockStructs])
- [MorseCodeLED][MorseCodeLED_lib]: Used for generating Morse code signals for alarms. ([MorseCodeLED.h][MorseCodeLED], [MorseCodeLED.cpp][MorseCodeLED_cpp])
- [RTClibPlus][RTClibPlus]: An enhanced version of the RTClib library for RTC functionality. ([DateTime.h][datetime_h], [RTClib.h][rtclib_h], [RTClibPlus_lib][RTClibPlus_lib])

### External Libraries
The `BinaryClock` library also relies on several external libraries: 
- [FastLED][fastled]: Used for controlling the [NeoPixel LEDs][NeoPixels].
- [Streaming][streaming]: Provides streaming operators for easier serial output.
- [Adafruit BusIO][busio]: Used for I2C communication with the RTC module.

## Installation and Usage

For complete installation instructions, API reference, usage examples, and troubleshooting, see:

**📚 [BinaryClock Installation & Usage Guide](BinaryClock_LibInstallUseage.md)**

The guide includes:
- Installation instructions for PlatformIO and Arduino IDE
- Quick start examples
- Complete API reference for BinaryClock, BCButton, and BCMenu classes
- Advanced usage patterns (custom colors, RTC sync, smart alarms, periodic updates)
- Configuration (board-specific setup, custom board support)
- Troubleshooting (RTC detection, LEDs, buttons, alarms, time drift, memory issues)
- Performance considerations
- Best practices

## Quick Start

Here's a minimal example to get started. For complete examples and detailed documentation, see the [Installation & Usage Guide (`BinaryClock_LibInstallUseage.md`)](BinaryClock_LibInstallUseage.md).

```cpp
#include <BinaryClock.h>

BinaryClock& clock = BinaryClock::get_Instance();

void setup() {
    Serial.begin(115200);
    
    // Initialize clock with LED test patterns
    clock.setup(true);  // true = show test patterns on startup
    
    Serial.println("Binary Clock Ready!");
}

void loop() {
    clock.loop();  // Handles button input and menu system
}
```

## Version History

- **v0.9.4** - Current version
  - Full clock, alarm, menu, and button integration
  - FreeRTOS task support for capable boards
  - Callback system for time and alarm events
  - Extensive melody registration system
  - Support for UNO R3, UNO R4, ESP32 boards

## Authors

**Chris-70** (2025, 2026)  
**Chris-80** (2025)  
**Marcin Saj** (2018) - Original creator of the [**Binary Clock Shield for Arduino**][shield] and the [example 11][example_11] code.

Created for the Binary Clock Shield for Arduino and WiFi Binary Clock project.

## License

This library is part of the WiFi Binary Clock project.  
Licensed under [GNU General Public License v3.0][gpl_3].

See [LICENSE][license] file for details.

## Links

- **WiFi Binary Clock Project**: [https://github.com/Chris-70/WiFiBinaryClock][WiFiBinaryClock]
- **BinaryClock Library**: [https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock][BinaryClock_lib]
- **Installation & Usage Guide**: [InstallUseage.md](InstallUseage.md)
- **Class Diagram**: [ClassDiagram.md](ClassDiagram.md)
- **Binary Clock Shield**: [https://nixietester.com/product/binary-clock-shield-for-arduino/][shield]
- **Shield GitHub**: [https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino][shield_github]
- **Dependencies**:
  - [BCGlobalDefines][BCGlobalDefines]
  - [MorseCodeLED][MorseCodeLED_lib]
  - [RTClibPlus][RTClibPlus]

---

**Keep time in binary. ⏰✨**

---

<!-- Reference Links -->
[BasicBinaryClock]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/src/BinaryClock_ESP32.cpp
[BCButton]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BCButtons.h
[BCButton_cpp]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BCButtons.cpp
[BCMenu]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BCMenu.h
[BCMenu_cpp]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BCMenu.cpp
[BinaryClock_lib]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src
[BinaryClock]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BinaryClock.h
[BinaryClock_cpp]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BinaryClock.cpp
[BinaryClockDefines]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines/src/BinaryClock.Defines.h
[BinaryClockStructs]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines/src/BinaryClock.Structs.h
[BCGlobalDefines]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines/src
[BinaryClockWiFi_lib]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi/src
[BinaryClockNTP]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWAN/src/BinaryClockNTP.h
[BinaryClockNTP_cpp]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWAN/src/BinaryClockNTP.cpp
[BinaryClockSettings]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BinaryClockSettings.h
[BinaryClockSettings_cpp]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/BinaryClockSettings.cpp
[BinaryClockWAN]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWAN/src/BinaryClockWAN.h
[BinaryClockWAN_cpp]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWAN/src/BinaryClockWAN.cpp
[BinaryClockWPS]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWAN/src/BinaryClockWPS.h
[BinaryClockWPS_cpp]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWAN/src/BinaryClockWPS.cpp
[boardselect]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/board_select.h
[busio]: https://github.com/adafruit/Adafruit_BusIO
[Chris-70]: https://github.com/Chris-70
[Chris-80]: https://github.com/Chris-80
[CLASS_DIAGRAM]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/CLASS_DIAGRAM.md
[datetime_h]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/RTClibPlus/src/DateTime.h
[example_11]: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino/tree/master/example/11-BinaryClockRTCInterruptAlarmButtons
[fastled]: https://github.com/FastLED/FastLED
[fastled]: https://github.com/FastLED/FastLED
[gpl_3]: https://www.gnu.org/licenses/gpl-3.0.en.html
[IBinaryClock]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines/src/IBinaryClock.h
[IBinaryClockBase]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines/src/IBinaryClockBase.h
[IBCButtonBase]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines/src/IBCButtonsBase.h
[LibInstallUseage]: BinaryClock_LibInstallUseage.md
[license]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClock/src/LICENSE
[metro]: https://www.adafruit.com/product/5500
[MorseCodeLED_lib]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/MorseCodeLED/src
[MorseCodeLED]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/MorseCodeLED/src/MorseCodeLED.h
[MorseCodeLED_cpp]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/MorseCodeLED/MorseCodeLED.cpp
[neopixel]: https://www.adafruit.com/product/1655
[NeoPixels]: https://www.adafruit.com/category/275
[piezo]: https://www.keliking.com/KLJ-1230-SMD-Piezo-Buzzer-pd6204765.html
[readme_rtc]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/RTClibPlus/README.md
[rtc]: https://www.analog.com/en/products/ds3231.html
[rtclib]: https://github.com/adafruit/RTClib
[rtclib_h]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/RTClibPlus/src/RTClib.h
[RTClibPlus_lib]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/RTClibPlus/src
[RTClibPlus]: https://github.com/Chris-70/RTClibPlus/tree/RTClibPlus
[SerialOutputDefines]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClock/src/SerialOutput.Defines.h
[settings]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWAN/src/BinaryClockSettings.h
[settings_cpp]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWAN/src/BinaryClockSettings.cpp
[shield_github]: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino
[shield]: https://nixietester.com/product/binary-clock-shield-for-arduino/
[ShieldExamples]: https://github.com/marcinsaj/Binary-Clock-Shield-for-Arduino/tree/master/example
[streaming]: https://github.com/janelia-arduino/Streaming
[TaskWrapper]: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BCGlobalDefines/src/TaskWrapper.h
[tonetutorial]: https://www.arduino.cc/en/Tutorial/Tone
[tonemelody]: https://www.arduino.cc/en/Tutorial/ToneMelody
[WiFiBinaryClock]: https://github.com/Chris-70/WiFiBinaryClock
