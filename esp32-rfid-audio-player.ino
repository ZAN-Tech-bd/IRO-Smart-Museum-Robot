#include <SPI.h>
#include <MFRC522.h>
#include <DFRobotDFPlayerMini.h>

// ---------- PINS (from your wiring diagram) ----------
#define RC522_SS   5    // RC522 SDA pin
#define RC522_RST  27   // RC522 RST pin
#define RC522_SCK  18
#define RC522_MISO 19
#define RC522_MOSI 23

#define DFPLAYER_RX 16  // ESP32 receives from DFPlayer TX
#define DFPLAYER_TX 17  // ESP32 sends to DFPlayer RX (through 1k resistor)

MFRC522 rfid(RC522_SS, RC522_RST);       // the RFID reader
HardwareSerial dfSerial(2);              // second serial channel, just for the DFPlayer
DFRobotDFPlayerMini dfPlayer;            // the audio player

// ---------- YOUR 2 CARD IDs ----------
// Replace these with the UIDs you already scanned from the other code
String card1 = "PUT_CARD_1_UID_HERE";   // this card plays /mp3/0001.mp3
String card2 = "PUT_CARD_2_UID_HERE";   // this card plays /mp3/0002.mp3

// ---------- SD CARD FILES ----------
// Put these two files inside a folder named "mp3" on the microSD card:
//   /mp3/0001.mp3   <- audio for card1
//   /mp3/0002.mp3   <- audio for card2
// (Replace 0001.mp3 and 0002.mp3 with your own recorded audio files,
// but keep the same file names/numbers so the code below still finds them.)

String lastUid = "";              // remembers the last card we saw
unsigned long lastScanAt = 0;     // remembers when we last scanned a card

void setup() {
  Serial.begin(115200); // for debug messages on Serial Monitor

  // Start the RFID reader
  SPI.begin(RC522_SCK, RC522_MISO, RC522_MOSI, RC522_SS);
  rfid.PCD_Init();

  // Start talking to the DFPlayer
  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  if (dfPlayer.begin(dfSerial, true, true)) {
    dfPlayer.volume(24); // volume: 0 (silent) to 30 (loudest)
  } else {
    Serial.println("DFPlayer not found! Check wiring and SD card.");
  }
}

void loop() {
  // Step 1: if no new card is present, do nothing and check again
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  // Step 2: read the card's ID and turn it into readable text
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (i) uid += ":";
    if (rfid.uid.uidByte[i] < 0x10) uid += "0"; // add leading zero if needed
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  // Step 3: only react if it's a new card, or 3 seconds passed since last scan
  // (stops the same card from replaying audio over and over)
  if (uid != lastUid || millis() - lastScanAt > 3000) {
    lastUid = uid;
    lastScanAt = millis();

    if (uid == card1) {
      dfPlayer.playMp3Folder(1);   // plays /mp3/0001.mp3
    } else if (uid == card2) {
      dfPlayer.playMp3Folder(2);   // plays /mp3/0002.mp3
    } else {
      Serial.println("Unknown card"); // card not in our list
    }
  }

  // Step 4: release the card, ready to scan the next one
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}