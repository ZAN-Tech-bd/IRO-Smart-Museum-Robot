# Smart Museum Artifact Information Telling Robot

Upload [SmartMuseumRobot.ino](SmartMuseumRobot.ino) with the Arduino IDE using an ESP32 board. It creates a local Wi-Fi network named **Museum-Guide**. Visitors join it with password **museum123**, then open `http://192.168.4.1`.

## Components required

- ESP32 development board
- RC522 RFID/NFC card reader module and RFID/NFC cards
- DFPlayer Mini MP3 player module
- LM386 audio amplifier module
- 3 W, 4 Ω speaker
- FAT32-formatted microSD card (for the narration MP3 files)
- 1 kΩ resistor (between ESP32 GPIO 17 and DFPlayer RX)
- Regulated external 5 V power supply
- Jumper wires and a USB cable for programming the ESP32

## Wiring

| Module | ESP32 connection |
| --- | --- |
| RC522 SDA/SS | GPIO 5 |
| RC522 RST | GPIO 27 |
| RC522 SCK / MISO / MOSI | GPIO 18 / GPIO 19 / GPIO 23 |
| RC522 3.3V / GND | 3.3V / common GND |
| DFPlayer TX | GPIO 16 (ESP32 RX2) |
| DFPlayer RX | GPIO 17 through the supplied 1 kΩ resistor |
| DFPlayer VCC / GND | 5V / common GND |
| DFPlayer DAC_L or DAC_R | LM386 audio input |
| LM386 output | 3 W, 4 Ω speaker |
| LM386 VCC / GND | 5V / common GND |

Power the ESP32 from USB or a suitable regulated supply. All modules must share ground. The RC522 must use **3.3 V**, never 5 V.

## Audio and cards

1. Format the microSD card FAT32 and create a folder named `mp3`.
2. Copy narration files as `0001.mp3`, `0002.mp3`, etc.
3. Open Serial Monitor at 115200 baud and tap each RFID/NFC card. Copy its displayed UID.
4. Replace the sample UIDs and artifact wording in the sketch's `ARTIFACTS` table. Set `audioTrack` to the matching filename number.
5. Install **MFRC522** and **DFRobotDFPlayerMini** from Arduino Library Manager before compiling.
