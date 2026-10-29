The CREED Lab Smart Clock features five distinct operational modes, toggled via capacitive touch using an external wire connected to GPIO 4 of the ESP32.

1. Main Interface (Standard Mode)
This mode provides a comprehensive overview of the current environment, displaying the Time, Day, Date, and Location. It features a dual-status icon system:

Primary Icon: Dynamically switches between a Sun icon (Daylight) and a Star icon (Night) based on localized solar data.

Secondary Icon: Displays real-time weather conditions, including cloud coverage, rain, or thunderstorms.

Environmental Data: Provides live Temperature and the Indian Air Quality Index (AQI), calculated using raw PM2.5 data for high precision.

2. Digital Stopwatch
A precision timer mode that utilizes the ESP32’s physical Boot button for tactile control.

Short Press: Start or Stop the timer.

Long Press: Reset the timer to zero.

The interface provides high-resolution feedback, including minutes, seconds, and tenths of a second.

3. Solar Path Visualization
This mode offers an astronomical tracking interface showing Sunrise and Sunset timings at the bottom corners. A semicircular arc represents the sky, with a sun icon that dynamically tracks the sun's elevation. The display intelligently updates its status text:

Daylight: Between sunrise and sunset.

Evening: Between sunset and midnight.

Pre-Dawn: Between midnight and sunrise (automatically updating for the new calendar day).

4. Lunar Path & Phase
Designed for lunar tracking, this mode displays Moonrise and Moonset timings and the moon’s current position on the celestial arc. It includes a specific Lunar Illumination percentage, allowing users to track the moon's phase cycle throughout the month.

5. Night/Minimalist Mode
Optimized for bedside use, this mode displays the time in a large-scale HH:MM:SS format alongside the date. To prevent sleep disruption, the OLED intensity is programmatically dimmed to its lowest level, providing essential information with zero light pollution.

