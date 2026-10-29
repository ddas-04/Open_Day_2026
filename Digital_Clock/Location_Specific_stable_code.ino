//**************************************************************************//
//     Smart Desk Clock with Astro mode code. Author: Debasis Das           //
//     Steps:                                                               //
//     1. Get your OpenWeather (line 37) and moonphase (line 38) API.       //
//     2. Set the location (at line 31)                                     //
//     Enjoy.                                                               //
//**************************************************************************//


#include <WiFi.h>
#include <ezTime.h>
#include <U8g2lib.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h> // For non-volatile storage

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
Timezone myTZ;
Preferences prefs;

// --- GLOBALS ---
int displayMode = 0; 
unsigned long swStart = 0, swElapsed = 0, lastWeatherUpdate = 0;
bool swRunning = false, buttonActive = false, isNight = false;
unsigned long buttonPressTime = 0;

//default block for initialization of the variables
String displayLoc = "Searching..."; 
float currentTemp = 0.0;
float lat = 15.2450, lon = 73.9780; // default for Goa
float MoonPhase = 0.5, MoonIllum = 0.0;
int indiaAQI = 0, cloudPercent = 0, weatherCode = 800; 
unsigned long sunrise = 0, sunset = 0; 
int moonRiseMin = 0, moonSetMin = 0;

const String weatherKey = "*************************"; 
const String moonApiKey = "*************************"; 

// --- HELPER: UNIVERSAL TIME CONVERSION (HARD-WIRED IST/USER OFFSET) ---
int getLocalMin(unsigned long unixTime) {
  if (unixTime == 0) return 0;
  unsigned long utcSecondsInDay = unixTime % 86400; // 24 hours to seconds is 86400 second
  int utcMin = utcSecondsInDay / 60;
  
  // Apply the 330 min offset for IST (UTC+5:30)
  int localMin = utcMin + 330; // 330 for IST timezone, i.e. UTC+5:30. (5*60)+30=330
  if (localMin >= 1440) localMin -= 1440; // 24 hrs = (24*60)=1440 minutes
  if (localMin < 0) localMin += 1440;
  return localMin;
}

// based on your location, you can also remove this block
int calculateIndian_AQI(float pm25) {
  
  if (pm25 < 0.0)   return 0;
  if (pm25 <= 12.0)  return (int)((50.0 / 12.0) * pm25);
  if (pm25 <= 35.4)  return (int)(51.0 + ((100.0 - 51.0) / (35.4 - 12.1)) * (pm25 - 12.1));
  if (pm25 <= 55.4)  return (int)(101.0 + ((150.0 - 101.0) / (55.4 - 35.5)) * (pm25 - 35.5));
  if (pm25 <= 150.4) return (int)(151.0 + ((200.0 - 151.0) / (150.4 - 55.5)) * (pm25 - 55.5));
  if (pm25 <= 250.4) return (int)(201.0 + ((300.0 - 201.0) / (250.4 - 150.5)) * (pm25 - 150.5));
  return 301;

  
  //if (pm25 < 0)    return 0;
  //if (pm25 <= 30)  return (int)(pm25 * 50.0 / 30.0);
  //if (pm25 <= 60)  return (int)(51.0 + (pm25 - 30.0) * 49.0 / 30.0);
  //if (pm25 <= 90)  return (int)(101.0 + (pm25 - 60.0) * 99.0 / 30.0);
  //if (pm25 <= 120) return (int)(201.0 + (pm25 - 90.0) * 99.0 / 30.0);
  //if (pm25 <= 250) return (int)(301.0 + (pm25 - 120.0) * 99.0 / 130.0);
  //if (pm25 <= 380) return (int)(401.0 + (pm25 - 250.0) * 99.0 / 130.0);
  //return 500;
  
}
// Initial WiFi setup, if the information is not already in the non-volatile storage of the ESP32
void configModeCallback (WiFiManager *myWiFiManager) {
  u8g2.clearBuffer(); u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 15, "WiFi Configuration");
  u8g2.drawStr(0, 30, "Connect AP: CREED_Clock");
  u8g2.drawStr(0, 45, "IP: 192.168.4.1");
  u8g2.sendBuffer();
}

// Function for drawing two symbols at place A and place B
// place A is for Sun icon during daylight, and star icon after sunset
// place B is for showing cloud or rain based on weather

// void drawDualStatus(int xA, int xB, int y) {
//   u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
//   if (!isNight) u8g2.drawGlyph(xA, y, 69); 
//   else {
//     u8g2.setFont(u8g2_font_open_iconic_weather_1x_t); 
//     u8g2.drawGlyph(xA + 4, y - 4, 68);
//   }
//   if (weathercode>=500 || cloudPercent > 60) {
//     u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
//     if (weatherCode >= 500 && weatherCode < 600) u8g2.drawGlyph(xB, y, 67); 
//     else u8g2.drawGlyph(xB, y, 64); 
//   }
// }

void drawDualStatus(int xA, int xB, int y) {
  // --- PRIMARY ICON (A): SUN OR MOON ---
  if (!isNight) {
    u8g2.setFont(u8g2_font_open_iconic_weather_2x_t); 
    u8g2.drawGlyph(xA, y, 69); // Sun
  } else {
    // For the moon, we use 1x to keep the aesthetic clean at night
    u8g2.setFont(u8g2_font_open_iconic_weather_1x_t); 
    u8g2.drawGlyph(xA + 4, y - 4, 68); // Moon
  }

  // --- SECONDARY ICON (B): SPECIFIC WEATHER STATUS ---
  u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);

  // 1. THUNDERSTORM (2xx range)
  if (weatherCode >= 200 && weatherCode < 300) {
    u8g2.drawGlyph(xB, y, 65); // Flash icon
  }
  // 2. RAIN/DRIZZLE (3xx and 5xx range)
  else if (weatherCode >= 300 && weatherCode < 600) {
    u8g2.drawGlyph(xB, y, 67); // Rain icon
  }
  // 3. ATMOSPHERE/HAZE/CLOUDS (5xx range OR cloud % > 60)
  else if ((weatherCode >= 500 && weatherCode < 800) || cloudPercent > 60) {
    u8g2.drawGlyph(xB, y, 64); // Cloud icon
  }
  else if (weatherCode >= 803) {
    u8g2.drawGlyph(xB, y, 64); // Cloud icon
  }

}

// Fetches the moonphase from the API
void fetchMoonAPI() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; http.setTimeout(8000); 
    
    // SMART DATE SELECTION: If 00:00 - 07:00, use yesterday's lunar cycle
    String dateParam = "today";
    if (myTZ.hour() < 7) {
        time_t yesterday = myTZ.now() - 86400; 
        dateParam = dateTime(yesterday, "Y-m-d");
    }

    String url = "https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/" 
                 + String(lat) + "," + String(lon) + "/" + dateParam 
                 + "?unitGroup=metric&key=" + moonApiKey + "&include=days&elements=moonillumination,moonphase,moonrise,moonset&contentType=json";
    
    http.begin(url);
    if (http.GET() == HTTP_CODE_OK) {
      DynamicJsonDocument doc(3000); deserializeJson(doc, http.getString());
      JsonObject day = doc["days"][0];
      MoonPhase = day["moonphase"].as<float>();
      float apiI = day["moonillumination"].as<float>();
      String rS = day["moonrise"].as<String>(), sS = day["moonset"].as<String>();
      moonRiseMin = (rS.substring(0,2).toInt() * 60) + rS.substring(3,5).toInt();
      moonSetMin = (sS.substring(0,2).toInt() * 60) + sS.substring(3,5).toInt();
      MoonIllum = (apiI < 0.1 && MoonPhase > 0.01) ? (1.0 - cos(MoonPhase * 2.0 * PI)) * 50.0 : apiI;
    }
    http.end();
  }
}

// Fetch Weather info
void fetchWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; http.setTimeout(4000);
    http.begin("http://api.openweathermap.org/data/2.5/weather?lat=" + String(lat) + "&lon=" + String(lon) + "&units=metric&appid=" + weatherKey);
    if (http.GET() == HTTP_CODE_OK) {
      StaticJsonDocument<1024> doc; deserializeJson(doc, http.getString());
      currentTemp = doc["main"]["temp"]; sunrise = doc["sys"]["sunrise"]; sunset = doc["sys"]["sunset"];
      cloudPercent = doc["clouds"]["all"]; weatherCode = doc["weather"][0]["id"];
    }
    http.end();
    http.begin("http://api.openweathermap.org/data/2.5/air_pollution?lat=" + String(lat) + "&lon=" + String(lon) + "&appid=" + weatherKey);
    if (http.GET() == HTTP_CODE_OK) {
      StaticJsonDocument<512> doc; deserializeJson(doc, http.getString());
      float pm25 = (int)doc["list"][0]["components"]["pm2_5"]; 
      float calibratedPM25 = pm25 * 0.33;
      indiaAQI = (int) calculateIndian_AQI(calibratedPM25);
    }
    http.end();
    fetchMoonAPI();
  }
}

// functions for handling the Astro mode
//********* Sun info function ********//
void drawSunPath() {
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(0, 7, "Solar Path");
  u8g2.drawCircle(64, 55, 45, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);

  int currentMin = (myTZ.hour() * 60) + myTZ.minute(); 
  int sr = getLocalMin(sunrise), ss = getLocalMin(sunset);
  
  // Labels
  char rB[10], sB[10];
  sprintf(rB, "R:%02d:%02d", sr/60, sr%60);
  sprintf(sB, "S:%02d:%02d", ss/60, ss%60);
  u8g2.setFont(u8g2_font_4x6_tr); u8g2.drawStr(0, 64, rB); u8g2.drawStr(128 - u8g2.getStrWidth(sB), 64, sB);

  // Solar Icon Logic
  if (currentMin >= sr && currentMin <= ss && ss > sr) {
    float ratio = (float)(currentMin - sr) / (float)(ss - sr);
    float angle = PI - (ratio * PI); 
    int px = 64 + (int)(45 * cos(angle)), py = 55 - (int)(45 * sin(angle));
    u8g2.setFont(u8g2_font_open_iconic_weather_1x_t); 
    u8g2.drawGlyph(px - 4, py + 4, 69); // Sun

    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(42, 42, "Daylight");
  }
  else {
    // --- AFTER SUNSET / BEFORE SUNRISE ---
    u8g2.setFont(u8g2_font_6x12_tr);
    if (currentMin > ss) {
      u8g2.drawStr(42, 42, "Evening");
    } else {
      u8g2.drawStr(32, 42, "Pre-Dawn");
    }
    // No Sun icon is drawn here because the Sun is below the horizon
  }
  
  
  u8g2.drawStr(64 - (u8g2.getStrWidth(myTZ.dateTime("d M Y").c_str())/2), 64, myTZ.dateTime("d M Y").c_str());
}

//********* Moon info function ********//
void drawMoonPath() {
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(0, 7, "Lunar Path");
  u8g2.drawCircle(64, 55, 45, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);

  int currentMin = (myTZ.hour() * 60) + myTZ.minute(); 
  
  // Lunar Icon Logic (Using your stabilized crossing-midnight math)
  if (moonSetMin < moonRiseMin) { // Crosses Midnight
      if (currentMin >= moonRiseMin || currentMin <= moonSetMin) {
         int total = (moonSetMin + 1440) - moonRiseMin;
         int elap = (currentMin >= moonRiseMin) ? (currentMin - moonRiseMin) : (currentMin + 1440 - moonRiseMin);
         float ratio = (float)elap / total;
         float angle = PI - (ratio * PI); 
         int px = 64 + (int)(45 * cos(angle)), py = 55 - (int)(45 * sin(angle));
         u8g2.drawDisc(px, py, 3);
      }
  } else { // Single Day
      if (currentMin >= moonRiseMin && currentMin <= moonSetMin) {
         float ratio = (float)(currentMin - moonRiseMin) / (float)(moonSetMin - moonRiseMin);
         float angle = PI - (ratio * PI); 
         int px = 64 + (int)(45 * cos(angle)), py = 55 - (int)(45 * sin(angle));
         u8g2.drawDisc(px, py, 3);
      }
  }

  // Labels
  char mrB[10], msB[10];
  sprintf(mrB, "R:%02d:%02d", moonRiseMin/60, moonRiseMin%60);
  sprintf(msB, "S:%02d:%02d", moonSetMin/60, moonSetMin%60);
  u8g2.setFont(u8g2_font_4x6_tr); u8g2.drawStr(0, 64, mrB); u8g2.drawStr(128 - u8g2.getStrWidth(msB), 64, msB);

  u8g2.setFont(u8g2_font_6x12_tr);
  String s = "Illum: " + String(MoonIllum, 1) + "%";
  u8g2.drawStr(64 - (u8g2.getStrWidth(s.c_str())/2), 42, s.c_str());
  u8g2.drawStr(64 - (u8g2.getStrWidth(myTZ.dateTime("d M Y").c_str())/2), 64, myTZ.dateTime("d M Y").c_str());
}

void setup() {
  Serial.begin(115200); delay(1000);
  u8g2.begin(); u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_9x15_tf); u8g2.drawStr(22, 30, "CREED Lab");
  u8g2.setFont(u8g2_font_7x14_tf); u8g2.drawStr(15, 50, "... by Debasis"); 
  u8g2.sendBuffer(); delay(2000); 

  u8g2.clearBuffer(); u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 10, "System Booting...");
  u8g2.drawStr(0, 25, "WiFi: Connecting..."); u8g2.sendBuffer();

  WiFiManager wm;
  wm.setAPCallback(configModeCallback);
  if (!wm.autoConnect("CREED_Clock_Setup")) { ESP.restart(); }

  u8g2.drawStr(100, 25, "OK"); delay(3000); 
  u8g2.drawStr(0, 35, "Sync: Geo-Loc..."); u8g2.sendBuffer();

  HTTPClient http; http.begin("http://ip-api.com/json/");
  if (http.GET() == HTTP_CODE_OK) {
    StaticJsonDocument<1024> doc; deserializeJson(doc, http.getString());
    // lat = doc["lat"]; lon = doc["lon"];
    // displayLoc = doc["city"].as<String>() + ", " + doc["countryCode"].as<String>();
    // --- BYPASS AUTO-LOCATION FOR PRECISION ---
    lat = 15.2450; // Precise Latitude for Davorlim/Margao, Goa, India
    lon = 73.9780; // Precise Longitude for Davorlim/Margao, Goa, India
    displayLoc = "Goa, India"; 
      
    
    myTZ.setLocation(doc["timezone"].as<String>()); 
    u8g2.drawStr(100, 35, "OK");
  } else { u8g2.drawStr(100, 35, "FAIL"); }
  http.end();

  myTZ.setPosix("IST-5:30"); // set for India
  u8g2.drawStr(0, 45, "Time: Syncing..."); u8g2.sendBuffer();
  waitForSync(30); 
  if (timeStatus() == timeSet) {
      u8g2.drawStr(100, 45, "OK");
  } else { u8g2.drawStr(100, 45, "FAIL"); }
  
  u8g2.drawStr(0, 55, "Data: Weather..."); u8g2.sendBuffer();
  fetchWeather(); lastWeatherUpdate = millis();
  u8g2.drawStr(100, 55, "OK"); u8g2.sendBuffer(); delay(1500); 
  pinMode(0, INPUT_PULLUP);
}

void loop() {
  events();
  unsigned long nowUTC = UTC.now(); 
  int hr = myTZ.hour();
  isNight = (sunrise > 0 && sunset > 0) ? (nowUTC < sunrise || nowUTC > sunset) : (hr >= 18 || hr < 7);
  
  if (touchRead(4) < 600) { 
  delay(50); 
  if (touchRead(4) < 600) { 
    displayMode = (displayMode + 1) % 5; // Updated to 5 modes
    // Set contrast only for the Big Clock mode (now mode 4)
    if (displayMode == 4) u8g2.setContrast(5); else u8g2.setContrast(255);
    u8g2.clearBuffer(); u8g2.sendBuffer(); 
    while(touchRead(4) < 600); 
  }
}

  if (digitalRead(0) == LOW && !buttonActive) { buttonActive = true; buttonPressTime = millis(); } 
  else if (digitalRead(0) == HIGH && buttonActive) {
    long dur = millis() - buttonPressTime;
    if (dur > 1500) { swRunning = false; swElapsed = 0; } 
    else { if (!swRunning) { swStart = millis() - swElapsed; swRunning = true; } else { swRunning = false; swElapsed = millis() - swStart; } }
    buttonActive = false;
  }

  u8g2.clearBuffer();
  if (displayMode == 0) { 
    if (millis() - lastWeatherUpdate > 900000) { fetchWeather(); lastWeatherUpdate = millis(); }

    // HAZARD ICON: Filled Triangle with XOR '!'
    if (millis() - lastWeatherUpdate > 900000 || WiFi.status() != WL_CONNECTED) {
      u8g2.setDrawColor(1); 
      
      u8g2.drawVLine(2, 2, 4); u8g2.drawPixel(2, 7); 
      u8g2.setDrawColor(1);
    }

    u8g2.setFont(u8g2_font_logisoso24_tn); u8g2.drawStr(0, 32, myTZ.dateTime("H:i").c_str());
    drawDualStatus(88, 108, 32); 
    u8g2.setFont(u8g2_font_5x7_tr); u8g2.drawStr(128 - u8g2.getStrWidth(displayLoc.c_str()), 8, displayLoc.c_str());
    u8g2.setFont(u8g2_font_6x12_tf); u8g2.drawStr(110 - (u8g2.getStrWidth((String(currentTemp, 1) + "\xb0" + "C").c_str())/2), 46, (String(currentTemp, 1) + "\xb0" + "C").c_str());
    u8g2.setFont(u8g2_font_7x14_tr); u8g2.drawStr(0, 48, myTZ.dateTime("l").c_str()); 
    u8g2.setFont(u8g2_font_6x12_tr); u8g2.drawStr(0, 62, myTZ.dateTime("d M Y").c_str()); 
    u8g2.drawStr(128 - u8g2.getStrWidth(("AQI:" + String(indiaAQI)).c_str()), 62, ("AQI:" + String(indiaAQI)).c_str());
  } 
  else if (displayMode == 1) { 
    u8g2.setFont(u8g2_font_6x12_tr); u8g2.drawStr(34, 10, "STOPWATCH");
    unsigned long cur = swRunning ? (millis() - swStart) : swElapsed;
    char buf[20]; sprintf(buf, "%02d:%02d.%d", (cur/60000)%60, (cur/1000)%60, (cur%1000)/100);
    u8g2.setFont(u8g2_font_logisoso32_tn); u8g2.drawStr(5, 52, buf);
  } 
  else if (displayMode == 2) { 
    drawSunPath(); 
  } 
  else if (displayMode ==3) {
    drawMoonPath();
  }
  else if (displayMode == 4) { 
    u8g2.setFont(u8g2_font_fur25_tn); 
    char bB[12]; sprintf(bB, "%02d:%02d:%02d", myTZ.hour(), myTZ.minute(), myTZ.second());
    u8g2.drawStr(64 - (u8g2.getStrWidth(bB) / 2), 48, bB); 
    u8g2.setFont(u8g2_font_6x12_tr); u8g2.drawStr(64 - (u8g2.getStrWidth(myTZ.dateTime("d M Y").c_str())/2), 62, myTZ.dateTime("d M Y").c_str());
    
  
  }
  u8g2.sendBuffer();
  yield();
}
