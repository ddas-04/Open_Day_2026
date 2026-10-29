#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <U8g2lib.h>
#include <Wire.h>

// Page-Buffer Mode (_1_) prevents I2C hardware timeouts from blocking WebSockets
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// STABLE PINS: GPIO 21 (SDA) and GPIO 22 (SCL) reserved for OLED
const int ledCount = 8;
int ledPins[] = {4, 5, 18, 19, 23, 27, 13, 12}; 

unsigned long lastMessageTime = 0;
unsigned long lastOLEDDrawTime = 0;

int lastDisplayLevel = -1;
int currentDistance = -1;
int renderedDistance = -999;

const char* htmlPage = R"rawliteral(
<!DOCTYPE html><html><head>
  <title>CREED Lab Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { text-align: center; font-family: sans-serif; transition: background-color 0.2s; padding-top: 50px; background-color: #f4f4f9; color: #222; }
    .container { padding: 40px; border-radius: 20px; background-color: #ffffff !important; display: inline-block; box-shadow: 0 10px 30px rgba(0,0,0,0.2); min-width: 250px; border: 2px solid #ddd; }
    #dist-val { font-size: 100px; font-weight: bold; color: #000000 !important; margin: 0; }
    .unit { font-size: 30px; color: #444; }
    body.alert-active { background-color: #ff0000 !important; }
    #warn-msg { color: #ff0000; font-weight: bold; font-size: 24px; display: none; margin-top: 20px; }
  </style>
</head>
<body>
  <div class="container">
    <h2>CREED Security Hub</h2>
    <div id="dist-val">--</div><span class="unit">cm</span>
    <div id="warn-msg">&#9888; PROXIMITY ALERT &#9888;</div>
  </div>
  <script>
    var gateway = `ws://${window.location.hostname}:81/`;
    var websocket = new WebSocket(gateway);
    websocket.onmessage = function(event) {
      var d = parseInt(event.data);
      if (!isNaN(d) && d > 0 && d <= 200) {
        document.getElementById('dist-val').innerHTML = d;
        if(d < 50) {
          document.body.classList.add('alert-active');
          document.getElementById('warn-msg').style.display = 'block';
        } else {
          document.body.classList.remove('alert-active');
          document.getElementById('warn-msg').style.display = 'none';
        }
      } else {
        document.getElementById('dist-val').innerHTML = '--';
        document.body.classList.remove('alert-active');
        document.getElementById('warn-msg').style.display = 'none';
      }
    };
  </script>
</body></html>)rawliteral";

void drawOLEDPage(int dist) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "CREED SECURITY HUB");
  u8g2.drawHLine(0, 12, 128);
  
  if (dist > 0 && dist <= 200) {
    u8g2.setFont(u8g2_font_logisoso32_tn);
    String dStr = String(dist);
    u8g2.drawStr(20, 50, dStr.c_str());
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(95, 50, "cm");
    
    int barWidth = map(constrain(dist, 0, 200), 0, 200, 128, 0);
    u8g2.drawFrame(0, 58, 128, 6);
    u8g2.drawBox(0, 58, barWidth, 6);
  } else {
    u8g2.setFont(u8g2_font_logisoso32_tn);
    u8g2.drawStr(30, 50, "---");
  }
}

// Low-overhead page rendering loop
void renderOLED(int dist) {
  u8g2.firstPage();
  do {
    drawOLEDPage(dist);
  } while (u8g2.nextPage());
  renderedDistance = dist;
}

void updateLEDArray(int dist) {
  int displayLevel = (dist > 200 || dist <= 0) ? 0 : map(200 - dist, 0, 200, 0, ledCount + 1);
  displayLevel = constrain(displayLevel, 0, ledCount);

  if (displayLevel != lastDisplayLevel) {
    for (int i = 0; i < ledCount; i++) {
      digitalWrite(ledPins[i], (i < displayLevel) ? HIGH : LOW);
    }
    lastDisplayLevel = displayLevel;
  }
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT && length > 0 && payload != NULL) {
    lastMessageTime = millis();
    int dist = atoi((char*)payload);
    
    // 1. INSTANT Hardware LED & Web Broadcast
    updateLEDArray(dist);
    webSocket.broadcastTXT(payload, length);
    
    // 2. Queue new distance for background OLED rendering
    currentDistance = dist;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Set I2C Clock to 400kHz (Fast Mode) on Pins 21 (SDA) & 22 (SCL)
  Wire.begin(21, 22);
  Wire.setClock(400000);
  
  u8g2.begin();
  
  for (int i = 0; i < ledCount; i++) { 
    pinMode(ledPins[i], OUTPUT); 
    digitalWrite(ledPins[i], LOW);
  }

  WiFi.softAP("ESP32_CREED", "creedlab");
  WiFi.setSleep(false); // Disable WiFi power saving mode

  server.on("/", []() { server.send(200, "text/html", htmlPage); });
  server.begin();
  
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  renderOLED(-1); // Display initial state "---"
}

void loop() {
  webSocket.loop();
  server.handleClient();

  unsigned long now = millis();

  // NON-BLOCKING DISPLAY THROTTLE: Render screen only if distance changed AND >=150ms has elapsed
  if (currentDistance != renderedDistance && (now - lastOLEDDrawTime > 150)) {
    renderOLED(currentDistance);
    lastOLEDDrawTime = now;
  }
  
  // Backup safety net: clear screen & LEDs if R4 disconnects for > 1 second
  if (now - lastMessageTime > 1000 && lastDisplayLevel != 0) { 
    updateLEDArray(999); 
    if (renderedDistance != -1) {
      renderOLED(-1);
    }
  }
}
