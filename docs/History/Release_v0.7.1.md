# WiFi Binary Clock - Release v0.7.1

**Release Date:** September 18, 2025  
---
Note: **The Current Version is:** 
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/Chris-70/WiFiBinaryClock?style=flat-square)]
[![GitHub license](https://img.shields.io/github/license/Chris-70/WiFiBinaryClock?style=flat-square)]

---

## Release Highlights
Binary Cock base
This is the base Binary Clock software working on the UNO_R3 (as well as all other supported boards).  
This is a standalone version, no WiFi, and compact enough to fit on the Arduino UNO R3.  
All non-WiFi functionality has been implemented. The Arduino UNO R3 and R4 Minima will remain at this level while development continues with the WiFi and other communications supported by the ESP32 family of chips.

1. Fully supports 12 hour mode with AM/PM indicators (on Hour bit 5), in addition to the standard 24 hour mode.
2. Alarm time setting is displayed in the selected mode.
3. Cancel settings and return is available for both Time and Alarm. This is the first menu item along with 12/24 mode selection for Time or ON/OFF for Alarm.
4. Added feedback during settings:
    - Green checkmark, [✅], is displayed when the settings are saved, or
    - Red X, [❌], is displayed when the changes are aborted.
    - rainbow screen is displayed to indicate the end of the settings.
5. The user can change, through property calls:
  5.1 Individual display colors for:  
        - Hours
        - Minutes
        - Seconds
        - AM and PM indicators.    
  5.2 Users can upload different alarm melodies
6. Users can define their own UNO board in the optional [board_select.h](https://github.com/Chris-70/WiFiBinaryClock/blob/UNO_R3_v0.7.0/lib/BinaryClock/src/board_select.h) file. To use the custom UNO board, just define CUSTOM_UNO true at the start of the file, before the definition section.

- The code is fully commented.
- Doxygen files will be published in a future release.
