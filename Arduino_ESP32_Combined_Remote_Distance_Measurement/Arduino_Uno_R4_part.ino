#include <WiFiS3.h>
#include <WebSocketsClient.h>

// WiFi AP Access Point credentials (hosted by ESP32)
const char* ssid = "ESP32_CREED";
const char* password = "creedlab";
const char* host = "192.168.4.1";

WebSocketsClient webSocket;

// Hardware Pins
const int trigpin = 10; 
const int echopin = 11;
const int localAlertLed = 13; // Onboard LED for local feedback

int lastSentDistance = -1;
unsigned long lastSendTime = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
  pinMode(localAlertLed, OUTPUT);

  // Initiate non-blocking connection to ESP32 WiFi AP
  WiFi.begin(ssid, password);

  // Configure WebSocket Client to connect to ESP32 Hub
  webSocket.begin(host, 81, "/");
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(3000, 3000, 2);
}

void loop() {
  // 1. Maintain WiFi & WebSocket connections without halting execution
  if (WiFi.status() == WL_CONNECTED) {
    webSocket.loop();
  } else {
    WiFi.begin(ssid, password);
  }

  // 2. Trigger HC-SR04 Ultrasonic Sensor
  digitalWrite(trigpin, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigpin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigpin, LOW);
  
  // 25ms pulseIn timeout (~4.3m range) prevents execution hanging on lost echoes
  long duration = pulseIn(echopin, HIGH, 25000); 
  
  // If echo returns 0, explicitly set distance to 999 (out of range)
  int distance = (duration > 0) ? (int)((duration * 0.0343) / 2) : 999;

  // 3. Instant Local Alert (Pin 13 turns ON for close proximity < 50cm)
  if (distance > 0 && distance < 50) {
    digitalWrite(localAlertLed, HIGH);
  } else {
    digitalWrite(localAlertLed, LOW);
  }

  // 4. Send distance to ESP32 instantly if it changes OR as a 1-second heartbeat
  unsigned long now = millis();
  bool distanceChanged = (abs(distance - lastSentDistance) >= 1);
  bool heartbeatTime = (now - lastSendTime > 1000);

  if (webSocket.isConnected() && (distanceChanged || heartbeatTime)) {
    String distanceStr = String(distance); 
    webSocket.sendTXT(distanceStr);
    
    lastSentDistance = distance;
    lastSendTime = now;
  }
  
  // Short 50ms loop delay (20 Hz sample rate) for real-time responsiveness
  delay(50); 
}
