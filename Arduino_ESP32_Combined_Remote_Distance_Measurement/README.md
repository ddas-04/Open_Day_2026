
CREED Security Hub: Distance & Proximity Monitor 

An IoT distance monitoring system using an Arduino Uno R4 WiFi sensing node and an ESP32 central hub. 
The system reads distances from an ultrasonic sensor, transmits updates wirelessly over WebSockets, and controls an 8-LED transistor bar array, a 1.3-inch OLED screen, and a local Web Dashboard. 
 
Features:
Wireless Monitoring: Fast WebSocket communication between Arduino Uno R4 and ESP32.
Visual Bar Array: 8 LEDs controlled via 2N2222 NPN transistor switches.
OLED Display: Real-time distance output on a 1.3-inch SH1106 display.
Web Dashboard: Mobile-friendly dashboard served locally at http://192.168.4.1/.
Low Latency: Non-blocking code execution with fast page updates. 


Components Required 
1. ESP32 Development Board
2.Arduino Uno R4 WiFi
3. HC-SR04 Ultrasonic Sensor
4. 1.3-inch I2C OLED Display (SH1106)
5. 8x 2N2222 NPN Transistors
6. 8x LEDs
7. 8x 1k ohm Resistors (Base Current Limiters)
8. 8x 220 ohm Resistors (LED Current Limiters)
9. Breadboard and Jumper Wires 

Wiring & Pin Connections 
ESP32 Central Hub 
OLED SDA: GPIO 21
OLED SCL: GPIO 22
OLED VCC: 3.3V (Do not connect to 5V)
OLED GND: GND
LED Channels (1 to 8): GPIO 4, 5, 18, 19, 23, 27, 13, 12 

Arduino Uno R4 WiFi Sensing Node 
HC-SR04 Trig: Pin 10
HC-SR04 Echo: Pin 11
HC-SR04 VCC: 5V
HC-SR04 GND: GND
Local Alert LED: Pin 13 (Onboard LED) 
 
Transistor Switch Circuit 
For each LED channel: 1. Connect ESP32 GPIO pin to a 1k ohm resistor, then to the Transistor Base. 2. Connect Transistor Emitter to Common Ground (GND). 3. Connect Transistor Collector to the LED Cathode (-). 4. Connect LED Anode (+) through a 220 ohm resistor to 5V. 

Software Libraries Required 

ESP32 Libraries 
  WebSockets by Markus Sattler
  U8g2 by oliver
  WiFi and WebServer (Included with ESP32 board package) 

Arduino Uno R4 Libraries 
  WiFiS3 (Included with Arduino Uno R4 board package)WebSockets by Markus Sattler 


How to Run 
Flash the ESP32 code to the ESP32 board.
Flash the Arduino Uno R4 code to the Uno R4 board.
Power on the ESP32. It will create a Wi-Fi network called ESP32_CREED.
Power on the Arduino Uno R4. It will automatically connect to the ESP32.
Connect your phone or laptop to ESP32_CREED (Password: creedlab).
Open your browser and go to http://192.168.4.1/ to view the web dashboard.
