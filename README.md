# WiFiBinaryClock

## Binary Clock for Arduino ESP32 UNO

![Binary Clock Shield](./assets/Binary-Clock-Shield-for-Arduino-UNO.png)

The Binary Clock SHield for Arduino UNO (`https://nixietester.com/product/binary-clock-shield-for-arduino/`) is the best Binary Clock I've seen.

This project was created to unlock the full potential of this great Binary Clock. Being an Arduino Shield means that we can substitute the UNO R3 for something much more powerful. I wanted to get my Wemos D1 R32 ESP32 UNO board to work with this shield, then connecting to a NTP server over WiFi keep the time synced whenever we switch to/from daylight savings time. In addition we could change the colors of the LEDs and upload new alarm melodies from a phone or computer.

The Wemos D1 R32 ESP32 based UNO type board seemed like the ideal candidate however it had a hardware limitation. The shield uses UNO pin A3 for the ED data out pin, this pin corresponds to the ESP32 GPIO 34 pin which is input only. In order to get this to work with the shield, the corresponding pin on the shield needs to be connected to an output pin such as GPIO 15. To do this you need to physically remove the A3/GPIO34 socket from the ESP32 UNO board (cut the plastic and desolder the pin) then connect the corresponding shield pin to GPIO 15.
![Wemos D1 R32 ESP32 UNO](./assets/Pinout_Wemos_D1_R32.png)