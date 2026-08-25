# Smart RFID Audio Player

This project lets a child make a small museum guide. When an RFID card is placed near the RC522 reader, the ESP32 checks the card's ID and tells the DFPlayer Mini to play a matching MP3 file through the amplifier and speaker.

The program in [esp32-rfid-audio-player](esp32-rfid-audio-player.ino) supports two cards:

| Card | Audio file |
| --- | --- |
| Card 1 | `mp3/0001.mp3` |
| Card 2 | `mp3/0002.mp3` |


## How the project works

1. The RC522 sends the RFID card's UID to the ESP32.
2. The ESP32 compares the UID with `card1` and `card2` in the sketch.
3. A matching card starts its MP3 file on the DFPlayer Mini.
4. The DFPlayer sends audio to the LM386 amplifier.
5. The amplifier makes the signal strong enough for the speaker.
6. An unknown card prints `Unknown card` in the Serial Monitor and does not play audio.

## Parts list

- ESP32 development board
- RC522 RFID reader
- At least two 13.56 MHz RFID cards or tags
- DFPlayer Mini MP3 player
- microSD card, 32 GB or smaller recommended, formatted as FAT32
- LM386 amplifier module
- 4 ohm speaker, about 3 W
- 1 k ohm resistor
- Breadboard and jumper wires
- USB data cable for the ESP32
- Regulated 5 V power source for the DFPlayer and amplifier

## Safety first

- Ask an adult to check the circuit before powering it.
- Disconnect power before changing wires.
- The RC522 is a **3.3 V device**. Connect its power to ESP32 `3.3V`, never `5V`.
- The ESP32, RC522, DFPlayer, and amplifier must share a common `GND`.
- Never connect the speaker directly to an ESP32 pin or to a DFPlayer power pin.
- The resistor protects the DFPlayer serial input. Do not skip it.
- Keep the speaker wires and power wires tidy so they cannot touch accidentally.

## Wiring

### RC522 to ESP32

| RC522 pin | ESP32 pin | Purpose |
| --- | --- | --- |
| `SDA` or `SS` | `GPIO 5` | Chip select |
| `RST` | `GPIO 27` | Reset |
| `SCK` | `GPIO 18` | SPI clock |
| `MISO` | `GPIO 19` | Data from RC522 |
| `MOSI` | `GPIO 23` | Data to RC522 |
| `3.3V` | `3.3V` | Power |
| `GND` | `GND` | Common ground |

Some RC522 boards label `SDA` as `SS`. They are the same connection for this project.

### DFPlayer Mini to ESP32

| DFPlayer pin | Connection | Purpose |
| --- | --- | --- |
| `TX` | ESP32 `GPIO 16` | DFPlayer sends data to ESP32 |
| `RX` | ESP32 `GPIO 17` through a `1 k ohm` resistor | ESP32 sends commands to DFPlayer |
| `VCC` | Regulated `5V` | Power |
| `GND` | Common `GND` | Ground |
| `DAC_L` or `DAC_R` | LM386 audio input | Audio signal |

Put the resistor in the signal wire like this:

`ESP32 GPIO 17 -> 1 k ohm resistor -> DFPlayer RX`

### LM386 to speaker and power

| LM386 connection | Connect to |
| --- | --- |
| `VCC` | Regulated `5V` |
| `GND` | Common `GND` |
| Audio input | DFPlayer `DAC_L` or `DAC_R` |
| Speaker output | Speaker positive terminal |
| Speaker other terminal | LM386 speaker return/output terminal |

Use the exact input and output labels printed on your LM386 module. Different LM386 boards may arrange their pins differently. The included [hardware diagram](Smart%20Museum%20Artifact%20Information%20Telling%20Robot%20Hardware%20Diagram.png) shows the intended circuit.

## Prepare the microSD card

1. Copy any important files off the microSD card.
2. Format the card as `FAT32`.
3. Create a folder named exactly `mp3` using lowercase letters.
4. Put the first recording in the folder and name it exactly `0001.mp3`.
5. Put the second recording in the folder and name it exactly `0002.mp3`.
6. Safely eject the card and insert it into the DFPlayer Mini before testing.

The required paths are:

```text
microSD/
└── mp3/
		├── 0001.mp3
		└── 0002.mp3
```

Use ordinary MP3 files with short filenames. Do not name them `track1.mp3`, `story.mp3`, or `1.mp3`, because this code asks the DFPlayer for track numbers 1 and 2 in the `mp3` folder.

## Install the Arduino software

### 1. Install Arduino IDE

Download and install Arduino IDE 2 from the official Arduino website. Start Arduino IDE after installation.

### 2. Add ESP32 board support

1. Open **File > Preferences**.
2. Find **Additional boards manager URLs**.
3. Add this URL:

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

4. Select **OK**.
5. Open **Tools > Board > Boards Manager**.
6. Search for `esp32`.
7. Install **esp32 by Espressif Systems**.

### 3. Install the two libraries

Open **Sketch > Include Library > Manage Libraries**, search for each library, and install:

- `MFRC522` by GithubCommunity or the library shown by Arduino IDE
- `DFRobotDFPlayerMini` by DFRobot

The sketch already includes these libraries:

```cpp
#include <SPI.h>
#include <MFRC522.h>
#include <DFRobotDFPlayerMini.h>
```

## Set up the RFID cards

The sketch starts with placeholders, so you must replace them before uploading:

```cpp
String card1 = "PUT_CARD_1_UID_HERE";
String card2 = "PUT_CARD_2_UID_HERE";
```

### Read a card UID

The current sketch does not print the UID. To discover a card's UID, temporarily add this line immediately after `uid.toUpperCase();` in `SmartMuseumRobot.ino`:

```cpp
Serial.println(uid);
```

Then:

1. Upload the sketch as described below.
2. Open **Tools > Serial Monitor**.
3. Set the speed to `115200 baud`.
4. Hold one card flat over the RC522 antenna.
5. Write down the UID shown in the Serial Monitor, such as `04:A1:B2:C3`.
6. Repeat with the second card.
7. Put the two UIDs into `card1` and `card2`.

The UID must use uppercase hexadecimal letters and colon separators, matching the format printed by the code:

```cpp
String card1 = "04:A1:B2:C3";
String card2 = "93:10:AB:42";
```

The example UIDs above are only examples. Every card has its own UID. Remove the temporary `Serial.println(uid);` line after recording the IDs, or leave it in if you want to see the card ID while testing.

## Upload the program

1. Open `SmartMuseumRobot.ino` in Arduino IDE.
2. Connect the ESP32 to the computer with a USB **data** cable.
3. Select **Tools > Board** and choose your ESP32 board. `ESP32 Dev Module` is often correct for a generic ESP32 board.
4. Select the ESP32's port in **Tools > Port**.
5. Click the checkmark button to verify or compile the sketch.
6. Click the right-arrow **Upload** button.
7. If the upload pauses at `Connecting...`, hold the ESP32 `BOOT` button until uploading begins, then release it.
8. Wait for the message that the upload finished.

Disconnect USB power before attaching or changing the external 5 V circuit. After all wiring is checked, power the project and wait a few seconds for the DFPlayer to start.

## Test the museum guide

1. Insert the prepared microSD card into the DFPlayer.
2. Turn the volume down on the amplifier before the first test.
3. Power the ESP32, DFPlayer, and amplifier.
4. Place Card 1 near the RC522 reader. `0001.mp3` should play.
5. Move Card 1 away, then place Card 2 near the reader. `0002.mp3` should play.
6. Try an unregistered card. The speaker should stay quiet and the Serial Monitor should show `Unknown card`.
7. Adjust the DFPlayer volume in the code if needed:

```cpp
dfPlayer.volume(24);
```

The allowed DFPlayer volume range is `0` to `30`. Upload the sketch again after changing the number.

The same card is prevented from replaying continuously. It can play again after it has been removed and scanned as a new card, or after more than three seconds have passed.

## Troubleshooting

### Nothing happens when a card is tapped

- Check that the RC522 uses `3.3V`, not `5V`.
- Check `SDA/SS`, `RST`, `SCK`, `MISO`, and `MOSI` against the table.
- Make sure every module has the same ground.
- Hold the card directly over the RC522 antenna, close to the reader.
- Confirm that the UID in `card1` or `card2` exactly matches the scanned UID.

### Serial Monitor says `Unknown card`

The reader is working, but that UID is not in the program. Scan the card again and copy the complete UID, including colons and leading zeroes.

### The Serial Monitor says `DFPlayer not found!`

- Check the DFPlayer's `VCC` and `GND`.
- Check `DFPlayer TX -> GPIO 16`.
- Check `GPIO 17 -> 1 k ohm resistor -> DFPlayer RX`.
- Confirm the microSD card is inserted and FAT32 formatted.
- Try a stable regulated 5 V supply.

### The card works but there is no sound

- Confirm the file is exactly `mp3/0001.mp3` or `mp3/0002.mp3`.
- Check the DFPlayer audio wire goes from `DAC_L` or `DAC_R` to the LM386 input.
- Check the speaker is connected to the LM386 output, not to the ESP32.
- Increase `dfPlayer.volume(24)` only a little at a time.
- Turn the amplifier's volume control slowly.

### The sound is noisy or the ESP32 resets

- Use a regulated power supply with enough current.
- Keep all grounds connected together.
- Keep speaker wires short and away from serial signal wires.
- Check for loose breadboard connections.
- Test first with the amplifier volume low.

## Changing the project

To add another card, the current sketch needs another card variable and another condition in `loop()`. The new audio file must use the next number, for example `mp3/0003.mp3`. The important matching parts look like this:

```cpp
String card3 = "YOUR_CARD_3_UID";

// Inside loop(), after the card1 and card2 checks:
else if (uid == card3) {
	dfPlayer.playMp3Folder(3);
}
```

For a classroom activity, children can record short museum stories, make two exhibit labels, and assign one story to each card. Always test the wiring with an adult before presenting the finished project.
