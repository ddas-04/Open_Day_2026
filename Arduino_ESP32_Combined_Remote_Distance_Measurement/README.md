CREED Security Hub: Edge IoT Proximity & Distance Monitoring
System 
An end-to-end, real-time IoT security and proximity monitoring system using an Arduino Uno R4 WiFi sensing node paired with an
ESP32 hub. 
The system measures distance via an HC-SR04 ultrasonic sensor, wirelessly transmits data over WebSockets, and renders real-time
visual output across three distinct interfaces: 1. An 8-LED progress bar driven via 2N2222 NPN transistor switches. 2. A 1.3-inch
SH1106 OLED display driven over fast hardware I2C ($400\text{ kHz}$). 3. A live Web Dashboard served directly from the ESP32
Access Point. 
 🛠 System Architecture 
+--------------------------+ Wi-Fi (SoftAP) +------------------------------------+
| Arduino Uno R4 WiFi | WebSocket Port 81 | ESP32 |
| (Distance Sensing Node) | ====================> | (Security Hub) |
+--------------------------+ +------------------------------------+
| | | |
+-- HC-SR04 Sensor +-- 8x LED Array +-- 1.3" OLED +-- Web Dashboard
+-- Onboard Alert LED (Pin 13) (via 2N2222 BJTs) (SH1106 I2C) (Port 80)

The system operates as a master-slave network: * Distance Sensing Node (Arduino Uno R4 WiFi): Continuously pulses the HC
SR04 ultrasonic sensor and computes target distance in centimeters. It triggers a local warning LED on Pin 13 if an object enters
close proximity ($< 50\text{ cm}$) and wirelessly transmits distance data to the ESP32 via a persistent WebSocket connection
(ws://192.168.4.1:81/). * Central Hub (ESP32): Hosts a standalone Wi-Fi SoftAP network (ESP32_CREED) and acts as a central
hub. It receives WebSocket frames from the sensor node, updates the physical 8-LED proximity array, renders live metrics on a local
SH1106 OLED display, and broadcasts real-time telemetry to connected web clients over an HTTP dashboard. 
 ⚡ Technical Highlights & Performance Tuning 
Non-blocking Ultrasonic Processing: The Arduino Uno R4 uses a strict $25\text{ ms}$ pulseIn() timeout (~$4.3\text{ m}$
max range) to prevent execution hangs or loop delays during missed echo signals.
Out-of-Range Handling: The Uno R4 explicitly transmits a 999 distance packet the instant an object moves out of range,
allowing the ESP32 to clear physical LEDs and web UI states immediately rather than waiting for an inactivity timeout.
Throttled OLED Rendering: Display redraws on the ESP32 are throttled ($150\text{ ms}$) in Page-Buffer mode (_1_HW_I2C)
so that heavy I2C transfer cycles do not block network packet queues or delay physical transistor switching.
Hardware-Level Decoupling: Incoming WebSocket packets trigger the GPIO-driven LED array instantly prior to queueing
display redraw tasks, guaranteeing sub-millisecond physical output response times.
Zero-Sleep Radio Mode: WiFi.setSleep(false) is configured on the ESP32 to prevent Wi-Fi modem sleep states and
eliminate network packet latency spikes. 
 🧰 Hardware Bill of Materials (BOM)Component Quantity PurposeESP32 Dev Module 1 Access Point host, WebSocket server, OLED & LED array driverArduino Uno R4 WiFi 1 Ultrasonic measurement node & WebSocket clientHC-SR04 Sensor 1 Distance measuring sensor1.3" I2C OLED Display 1 SH1106 controller ($128 \times 64$ monochrome display)2N2222 NPN Transistors 8 Transistor switches for driving the LED bar graphLEDs (Red/Yellow/Green) 8 Physical proximity bar graph display$1\,\text{k}\Omega$ Resistors 8 Transistor Base current limiters$220\,\Omega$ Resistors 8 LED forward current limitersBreadboard & Jumpers — Circuit wiring and interconnects 
 🔌 Circuit & Pinout Specifications 
ESP32 Central Hub PinoutFunction ESP32 PinOLED SDA GPIO 21OLED SCLConnection & Circuit DescriptionHardware I2C Data lineOLED VCC 3.3V Power display via 3.3V to keep I2C bus logic levels safe for ESP32OLED GND GND System Common GroundLED Channels 1–8 GPIO 4, 5, 18, 19, 23, 27, 13, 12 Active-HIGH outputs driving $1\,\text{k}\Omega$ resistors to 2N2222 Bases 
Transistor Driving Circuit (Per LED Channel): - ESP32 GPIO Pin $\to 1\,\text{k}\Omega$ Resistor $\to$ Base (B) of
2N2222 - Emitter (E) $\to$ Common Ground (GND) - Collector © $\to$ LED Cathode ($-$) - LED Anode ($+$) $\to
220\,\Omega$ Resistor $\to$ 5V Supply 
Arduino Uno R4 Sensing Node PinoutFunction Uno R4 Pin Connection DetailsHC-SR04 Trig Pin 10 Digital Output trigger pulseHC-SR04 Echo Pin 11 Digital Input echo returnLocal Alert LED Pin 13 Onboard debug LED (Glows when distance $< 50\text{ cm}$)HC-SR04 Power 5V / GND Ultrasonic sensor requires standard $5\text{V}$ rail 
 📦 Required Software Libraries 
Install the following dependencies via the Arduino IDE Library Manager: 
For ESP32 Hub: 
WebSockets by Markus Sattler
U8g2 by oliver
WiFi & WebServer (Included in standard ESP32 board support package) 
For Arduino Uno R4 WiFi: 
WiFiS3 (Included in standard Arduino UNO R4 board support package)
WebSockets by Markus Sattler 
 🚀 Deployment Instructions 
Upload Firmware: 
Flash the ESP32 Hub code onto the ESP32 Dev Module.
Flash the Sensor Node code onto the Arduino Uno R4 WiFi.
Hardware Assembly: 
Wire the HC-SR04 to the Uno R4 (Pins 10 & 11) using $5\text{V}$ power.
Connect the OLED display to ESP32 (SDA $\to$ 21, SCL $\to$ 22) powered by $3.3\text{V}$.
Connect the 8-channel transistor driver array to ESP32 GPIOs 4, 5, 18, 19, 23, 27, 13, and 12. Connect all grounds
together.
System Startup: 
Power up the ESP32 first. It will initiate the ESP32_CREED Access Point.
Power up the Uno R4. It automatically joins the network and establishes the WebSocket connection.
Web Dashboard Access: 
Connect a smartphone, tablet, or PC to Wi-Fi SSID: ESP32_CREED (Password: creedlab).
Open any browser and navigate to http://192.168.4.1/ to access the real-time proximity UI.
