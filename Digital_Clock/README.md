CREED Lab Smart Clock 

An ESP32-powered desktop smart clock equipped with an SH1106 OLED display, dynamic weather tracking, astronomical solar and lunar telemetry, precise Indian AQI calculation, capacitive touch interface, and tactile stopwatch controls. 

Features & Operational Modes:
The CREED Lab Smart Clock features five distinct operational modes, toggled via capacitive touch using an external wire connected to GPIO 4 of the ESP32: 

1. Main Interface (Standard Mode) 
Provides a comprehensive overview of the current environment, displaying Time, Day, Date, and Location. It features a dual-status icon system:
* Primary Icon: Dynamically switches between a Sun icon (Daylight) and a Star/Moon icon (Night) based on localized solar data.
* Secondary Icon: Displays real-time weather conditions, including cloud coverage, rain, or thunderstorms.
* Environmental Data: Provides live Temperature and the Indian Air Quality Index (AQI), calculated from raw $\text{PM}_{2.5}$ data for high precision. 

2. Digital Stopwatch 
A precision timer mode that utilizes the ESP32’s physical Boot button for tactile control:
* Short Press: Start or Stop the timer.
* Long Press: Reset the timer to zero.
* Feedback: High-resolution timing display including minutes, seconds, and tenths of a second.
   
3. Solar Path Visualization 
An astronomical tracking interface showing Sunrise and Sunset timings at the bottom corners. A semicircular arc represents the sky, with a sun icon that dynamically tracks the sun’s elevation. Status text updates dynamically:
* Daylight: Between sunrise and sunset.
* Evening: Between sunset and midnight.
* Pre-Dawn: Between midnight and sunrise (automatically updating for the new calendar day).
  
4. Lunar Path & Phase 
Designed for lunar tracking, this mode displays Moonrise and Moonset timings alongside the moon’s current position on the celestial arc. Includes a Lunar Illumination percentage to track the moon’s phase cycle throughout the month. 

5. Night / Minimalist Mode 
Optimized for bedside use, displaying time in a large-scale HH:MM:SS format alongside the date. To prevent sleep disruption, OLED intensity is programmatically dimmed to its lowest brightness level with minimal screen clutter.

Hardware Connections & Pinout
Component            ESP32 Pin              Function / Description
SH1106 OLED VCC      3.3V                    Display Power Supply
SH1106 OLED GND       GND                     Ground Reference
SH1106 OLED SDA     GPIO 21                    I2C Data Line
SH1106 OLED SCL     GPIO 22                    I2C Clock Line
Touch Wire          GPIO 4 (D4)                 Capacitive Touch (Mode Switching)
Boot Button         GPIO 0 (D0)               Integrated On-Board Button (Stopwatch Control) 


Key Libraries Used 
U8g2: OLED graphics library optimized for SH1106 monochrome displays.
ezTime: Precise NTP timekeeping with automated timezone and DST support.
ArduinoJson: Fast JSON parsing for API responses.
WiFiManager: Captive portal for seamless Wi-Fi credential provisioning. 

External APIs Integrated 
OpenWeatherMap API: Live weather conditions, temperature, and raw $\text{PM}_{2.5}$ concentrations.
Visual Crossing Weather API: Solar/Lunar rise, set, and illumination metadata.
