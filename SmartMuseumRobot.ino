/*
  Smart Museum Artifact Information Robot
  ESP32 + RC522 + DFPlayer Mini + LM386 amplifier
  Required libraries: MFRC522, DFRobotDFPlayerMini
  microSD files: /mp3/0001.mp3, /mp3/0002.mp3, ...
*/
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DFRobotDFPlayerMini.h>

constexpr uint8_t RC522_SS = 5, RC522_RST = 27;
constexpr uint8_t RC522_SCK = 18, RC522_MISO = 19, RC522_MOSI = 23;
// ESP32 GPIO17 -> 1k resistor -> DFPlayer RX. DFPlayer TX -> GPIO16.
constexpr uint8_t DFPLAYER_RX = 16, DFPLAYER_TX = 17;
const char *AP_NAME = "Museum-Guide";
const char *AP_PASSWORD = "museum123"; // Change this before public use.

struct Artifact {
  const char *uid; const char *title; const char *period;
  const char *description; uint16_t audioTrack;
};

// Tap each card while Serial Monitor is open, then replace its sample UID here.
const Artifact ARTIFACTS[] = {
  {"DE:AD:BE:EF", "Terracotta Warrior", "Qin dynasty, c. 210 BCE",
   "A life-sized clay soldier from the army created to guard China's first emperor.", 1},
  {"12:34:56:78", "Bronze Age Axe", "c. 1800 BCE",
   "This cast bronze axe shows the technical skill of early metalworking communities.", 2},
  {"04:A1:B2:C3:D4:E5:80", "Painted Clay Vessel", "c. 500 BCE",
   "Its geometric decoration records a visual language used in ceremonial life.", 3}
};
constexpr size_t ARTIFACT_COUNT = sizeof(ARTIFACTS) / sizeof(ARTIFACTS[0]);

MFRC522 rfid(RC522_SS, RC522_RST);
HardwareSerial dfSerial(2);
DFRobotDFPlayerMini dfPlayer;
WebServer server(80);
String lastUid = "Waiting for a card";
int lastArtifact = -1;
bool playerReady = false;
unsigned long lastScanAt = 0;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Museum Guide</title><style>
*{box-sizing:border-box}body{margin:0;background:#101c22;color:#eaf4f4;font:16px system-ui,sans-serif}
main{max-width:760px;margin:auto;padding:36px 20px}h1{color:#f4c76c;margin:0 0 7px}.sub{color:#9bb6ba}
.card{margin-top:24px;background:#193039;border:1px solid #315159;border-radius:16px;padding:25px;min-height:250px}
.tag{display:inline-block;background:#f4c76c;color:#1d282a;padding:4px 10px;border-radius:20px;font-size:.8rem;font-weight:700}
h2{font-size:2rem;margin:17px 0 6px}.period{color:#f4c76c;font-weight:600}.empty{color:#b7ced0;font-size:1.1rem;padding-top:50px}
button{border:0;border-radius:9px;background:#f4c76c;color:#152326;padding:12px 16px;font-weight:750;cursor:pointer;margin-top:15px}
footer{margin-top:24px;color:#9bb6ba;font-size:.9rem}code{color:#f4c76c}</style></head><body><main>
<h1>Smart Museum Guide</h1><div class="sub">Tap an artifact card on the reader to hear its story.</div>
<section class="card" id="card"><div class="empty">Ready to scan an RFID/NFC card…</div></section>
<footer>Reader status: <span id="uid">Connecting…</span><br>Connected to <code>Museum-Guide</code></footer>
</main><script>
let shown='';function esc(s){const d=document.createElement('div');d.textContent=s;return d.innerHTML}
async function refresh(){try{const r=await fetch('/api/status'),d=await r.json();document.querySelector('#uid').textContent=d.uid;
if(d.found&&d.uid!==shown){shown=d.uid;document.querySelector('#card').innerHTML='<span class="tag">ARTIFACT FOUND</span><h2>'+esc(d.title)+'</h2><div class="period">'+esc(d.period)+'</div><p>'+esc(d.description)+'</p><button onclick="play()">Play narration again</button>'}
else if(!d.found&&d.uid!==shown){shown=d.uid;document.querySelector('#card').innerHTML='<div class="empty">This card is not in the museum collection.<br><small>Ask a curator for assistance.</small></div>'}}catch(e){document.querySelector('#uid').textContent='Connection lost — reconnect to Museum-Guide'}}
async function play(){await fetch('/api/play',{method:'POST'})}refresh();setInterval(refresh,1000);
</script></body></html>
)rawliteral";

String uidToString() {
  String value;
  for (byte i = 0; i < rfid.uid.size; ++i) {
    if (i) value += ':';
    if (rfid.uid.uidByte[i] < 0x10) value += '0';
    value += String(rfid.uid.uidByte[i], HEX);
  }
  value.toUpperCase(); return value;
}
int findArtifact(const String &uid) {
  for (size_t i = 0; i < ARTIFACT_COUNT; ++i)
    if (uid.equalsIgnoreCase(ARTIFACTS[i].uid)) return i;
  return -1;
}
void playCurrentArtifact() {
  if (playerReady && lastArtifact >= 0) dfPlayer.playMp3Folder(ARTIFACTS[lastArtifact].audioTrack);
}
void sendStatus() {
  String json = "{\"uid\":\"" + lastUid + "\",\"found\":" + (lastArtifact >= 0 ? "true" : "false");
  if (lastArtifact >= 0) {
    const Artifact &a = ARTIFACTS[lastArtifact];
    json += ",\"title\":\"" + String(a.title) + "\",\"period\":\"" + String(a.period) +
            "\",\"description\":\"" + String(a.description) + "\"";
  }
  server.send(200, "application/json", json + "}");
}
void setup() {
  Serial.begin(115200);
  SPI.begin(RC522_SCK, RC522_MISO, RC522_MOSI, RC522_SS); rfid.PCD_Init();
  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  playerReady = dfPlayer.begin(dfSerial, true, true);
  if (playerReady) dfPlayer.volume(24); else Serial.println("DFPlayer unavailable: check power, SD card, and wiring.");
  WiFi.mode(WIFI_AP); WiFi.softAP(AP_NAME, AP_PASSWORD);
  Serial.print("Open http://"); Serial.println(WiFi.softAPIP());
  server.on("/", [] { server.send_P(200, "text/html", INDEX_HTML); });
  server.on("/api/status", HTTP_GET, sendStatus);
  server.on("/api/play", HTTP_POST, [] { playCurrentArtifact(); server.send(204); });
  server.begin();
}
void loop() {
  server.handleClient();
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  String scannedUid = uidToString();
  if (scannedUid != lastUid || millis() - lastScanAt > 3000) {
    lastUid = scannedUid; lastArtifact = findArtifact(lastUid); lastScanAt = millis();
    Serial.printf("Scanned UID: %s\n", lastUid.c_str());
    if (lastArtifact >= 0) playCurrentArtifact(); else Serial.println("Card is not registered.");
  }
  rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();
}
