// #include <Adafruit_GFX.h>
#include <FastLED.h>
// #include <FastLED_GFX.h>
#include <LEDMatrix.h>
#include <tinifont.h>
#include "OneButton.h"
#include "WiFi.h"
#include "esp_sntp.h"
#include "time.h"
#include <ESP32WifiCLI.hpp>
#include "pin_config.h"
#include <ArduinoOTA.h>

#define PIN 16

// Max is 255, 32 is a conservative value to not overload
// a USB power supply (500mA) for 12x12 pixels.
#define BRIGHTNESS_AVERAGE 10
#define MIN_BRIGHTNESS 30
#define MIN_DARK_BRIGHTNESS 10
#define MIN_COLOR CRGB(1,0,0)
// #define delay FastLED.delay

#define MATRIX_WIDTH 9
#define MATRIX_HEIGHT 12
#define MATRIX_TYPE         VERTICAL_ZIGZAG_MATRIX
#define NUMMATRIX (MATRIX_WIDTH*MATRIX_HEIGHT)
// Define matrix width and height.

#define FRAMES_PER_SECOND 30

#define TIME 0
#define FULLSCREEN 1
#define DAY_MODE 0
#define HOUR_MODE 1
#define MINUTE_MODE 2
#define CONSTANT_MODE 3
#define AUTO_MODE 0
#define FIXED_MODE 1


bool gReverseDirection = false;
cLEDMatrix<-MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE> matrix;


void matrix_show() {
    FastLED.show();
}
void matrix_clear() {
    FastLED.clear();
}

static const char* TAG = "nightclock";

u_int8_t r;
u_int8_t g;
u_int8_t b;


const char * key_ntp_server = "kntpserver";
const char * key_tzone = "ktzone";
const char * key_color = "kcolor";
const char * key_hue_offset = "khueoffset";

// change these params via CLI:
const char * default_server = "pool.ntp.org";  
const char * default_tzone = "CET-1CEST,M3.5.0,M10.5.0/3";
const int default_color = 0xffffff;
const char * default_display_mode = TIME;

bool wcli_setup_ready = false;

static uint8_t brightness_readings[BRIGHTNESS_AVERAGE] = {0};
static uint8_t rollingBrightnesIndex = 0;
static uint32_t sum = 0;
static uint8_t average_brightness = 0;
static uint8_t displayMode = TIME;
static uint8_t colorMode = DAY_MODE;
static uint8_t brightnessMode = AUTO_MODE;
static uint8_t fixed_brightness = 100;



CRGB getColorForDay(struct tm *timeinfo) {
  // Calculate the total minutes since midnight
  int totalMinutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;

  // Calculate the hue value (0-255) based on the total minutes
  uint8_t hue = map(totalMinutes, 0, 1440, 0, 255);
  
  // Add an offset value to the hue
  uint8_t hueOffset = wcli.getInt(key_hue_offset,0);
  hue = (hue + hueOffset) % 256;
  uint8_t brightness = fixed_brightness;
  if (brightnessMode == AUTO_MODE){
    brightness =  average_brightness;
  }
  CRGB color;
  if (brightness < MIN_BRIGHTNESS){
    // use a dark red when embiant light is low
    color = MIN_COLOR;
  } else{
    // Convert the hue to an RGB value
    color = CHSV(hue, 255, brightness);
  }
  ESP_LOGD(TAG,"D HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, brightness, color.r, color.g, color.b);
  return color;
}

CRGB getColorForMinute(struct tm *timeinfo) {
  // Calculate the total minutes since midnight
  int totalMinutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;

  // Calculate the hue value (0-255) based on the total minutes
  uint8_t hue = map(timeinfo->tm_sec, 0, 59, 0, 255);
  
  // Add an offset value to the hue
  uint8_t hueOffset = wcli.getInt(key_hue_offset,0);
  hue = (hue + hueOffset) % 256;

  uint8_t brightness = fixed_brightness;
  if (brightnessMode == AUTO_MODE){
    brightness =  average_brightness;
  }
  CRGB color;
  if (brightness < MIN_BRIGHTNESS){
    // use a dark red when embiant light is low
    color = MIN_COLOR;
  } else{
    // Convert the hue to an RGB value
    color = CHSV(hue, 255, brightness);
  }
  ESP_LOGD(TAG,"M HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, brightness, color.r, color.g, color.b);
  return color;
}

CRGB getColorForHour(struct tm *timeinfo) {
  // Calculate the total minutes since midnight
  int totalSeconds = timeinfo->tm_min * 60 + timeinfo->tm_sec;

  // Calculate the hue value (0-255) based on the total minutes
  uint8_t hue = map(totalSeconds, 0, 3599, 0, 255);
  
  // Add an offset value to the hue
  uint8_t hueOffset = wcli.getInt(key_hue_offset,0);
  hue = (hue + hueOffset) % 256;

  uint8_t brightness = fixed_brightness;
  if (brightnessMode == AUTO_MODE){
    brightness =  average_brightness;
  }
  CRGB color;
  if (brightness < MIN_BRIGHTNESS){
    // use a dark red when embiant light is low
    color = MIN_COLOR;
  } else{
    // Convert the hue to an RGB value
    color = CHSV(hue, 255, brightness);
  }
  ESP_LOGD(TAG,"H HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, brightness, color.r, color.g, color.b);
  return color;
}

CRGB getColor(struct tm *timeinfo) {
  // get the color depending on the setting. Either by day, hour or minute, or a constant color.
  if(colorMode == DAY_MODE){
    return getColorForDay(timeinfo);
  } else if(colorMode == HOUR_MODE){
    return getColorForHour(timeinfo);
  } else if(colorMode == MINUTE_MODE){
    return getColorForMinute(timeinfo);
  } else {
    return CRGB(wcli.getInt(key_color, default_color));
  }
}



void displayFullScreen(struct tm * info) {
  CRGB color = getColor(info);
  matrix.DrawFilledRectangle(0,0,MATRIX_WIDTH, MATRIX_HEIGHT, color);
  matrix_show();
}


void displayTime(struct tm * info) {

    // matrix.DrawCircle(4,6,1,getColorForMinute(info));
    matrix.setFont(&Font3x5FixedNum);    
    matrix_clear();
    CRGB color = getColor(info);
    // print hours
    matrix.DrawChar(1,5,info->tm_hour/10+'0',color,0,1);
    matrix.DrawChar(5,5,info->tm_hour%10+'0',color,0,1);

    // print minutes
    matrix.DrawChar(1,12,info->tm_min/10+'0',color,0,1);
    matrix.DrawChar(5,12,info->tm_min%10+'0',color,0,1);
    matrix_show();

    
}

void display(struct tm * info) {
  if(displayMode == TIME){
    displayTime(info);
  } else if(displayMode == FULLSCREEN){
    displayFullScreen(info);
  }
}

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {}
  void onHelpShow() {}
  void onNewWifi(String ssid, String passw) { wcli_setup_ready = wcli.isConfigured(); }
};

void updateTimeSettings() {
  String server = wcli.getString(key_ntp_server, default_server );
  String tzone = wcli.getString(key_tzone, default_tzone);
  Serial.printf("ntp server: \t%s\r\ntimezone: \t%s\r\n",server.c_str(),tzone.c_str());
  configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, server.c_str(), NTP_SERVER2);
  setenv("TZ", tzone.c_str(), 1);  
  tzset();
}


void setNTPServer(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String server = operands.first();
  if (server.isEmpty()) {
    Serial.println(wcli.getString(key_ntp_server, default_server));
    return;
  }
  wcli.setString(key_ntp_server, server);
  updateTimeSettings();
}

void setTimeZone(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String tzone = operands.first();
  if (tzone.isEmpty()) {
    Serial.println(wcli.getString(key_tzone, default_tzone));
    return;
  }
  wcli.setString(key_tzone, tzone);
  updateTimeSettings();
}

void setColor(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String colorHex = operands.first();

  uint32_t colorValue = strtol(colorHex.c_str(), NULL, 16);
  CRGB color = CRGB(colorValue);
  Serial.printf("Setting color to %d %d %d\r\n", color.r, color.g, color.b);
  wcli.setInt(key_color, colorValue);
}

void setBrightness(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String brightness = operands.first();
  if(brightness == "auto"){
    Serial.println("Setting brightness to auto");
    brightnessMode = AUTO_MODE;
  } else {
    uint8_t brightnessValue = strtol(brightness.c_str(), NULL, 10);
    Serial.printf("Setting brightness to %d\r\n", brightnessValue);
    brightnessMode = FIXED_MODE;
    fixed_brightness = brightnessValue;
  }
}

void setColorMode(char *args, Stream *response){
  Pair<String, String> operands = wcli.parseCommand(args);
  String mode = operands.first();
  if(mode == "D"){
    Serial.println("Setting color mode to day");
    colorMode = DAY_MODE;
  } else if(mode == "H"){
    Serial.println("Setting color mode to hour");
    colorMode = HOUR_MODE;
  } else if(mode == "M"){
    Serial.println("Setting color mode to minute");
    colorMode = MINUTE_MODE;
  } else if(mode == "C"){
    Serial.println("Setting color mode to constant");
    colorMode = CONSTANT_MODE;
  } else {
    Serial.println("Invalid color mode. Use D for day, H for hour, M for minute and C for constant color");
  }
}

void setDisplayMode(char *args, Stream *response){
  Pair<String, String> operands = wcli.parseCommand(args);
  String mode = operands.first();
  if(mode == "T"){
    Serial.println("Setting display mode to time");
    displayMode = TIME;
  } else if(mode == "F"){
    Serial.println("Setting display mode to full screen");
    displayMode = FULLSCREEN;
  } else {
    Serial.println("Invalid display mode. Use T for time or F for full screen");
  }
}

void setHueOffset(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String colorHueOffset = operands.first();
  uint8_t hueOffset = strtol(colorHueOffset.c_str(), NULL, 10);
  Serial.printf("Setting color hue offset to %d\r\n", hueOffset);
  wcli.setInt(key_hue_offset, hueOffset);
}

void printLocalTime(char *args, Stream *response) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void updateBrightness() {

  
    uint8_t brightness = map(analogRead(34),0,4095,MIN_DARK_BRIGHTNESS,255);

    // Subtract the oldest reading from the sum
    sum -= brightness_readings[rollingBrightnesIndex];
    // Add the new reading to the sum
    brightness_readings[rollingBrightnesIndex] = brightness;
    sum += brightness;

    // Move to the next index, wrapping around if necessary
    rollingBrightnesIndex = (rollingBrightnesIndex + 1) % BRIGHTNESS_AVERAGE;

    // Calculate the running average
    average_brightness = sum / BRIGHTNESS_AVERAGE;


    ESP_LOGD(TAG, "%d",average_brightness);

}

void rainbow_wave(uint8_t thisSpeed, uint8_t deltaHue) {     // The fill_rainbow call doesn't support brightness levels.
// uint8_t thisHue = beatsin8(thisSpeed,0,255);                // A simple rainbow wave.
 uint8_t thisHue = beat8(thisSpeed,255);                     // A simple rainbow march.
  
 fill_rainbow(matrix[0], NUMMATRIX, thisHue, deltaHue);            // Use FastLED's fill_rainbow routine.
 
}

void loop() {
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo,1000 / FRAMES_PER_SECOND)){
      updateBrightness();
      FastLED.setBrightness(average_brightness);
      rainbow_wave(10, 10);                                      // Speed, delta hue values.
      FastLED.show();
  }
  FastLED.setBrightness(255);

  display(&timeinfo);
  if (brightnessMode == AUTO_MODE){
    updateBrightness();
  }
  while(!wcli_setup_ready) wcli.loop(); // only for fist setup
  wcli.loop();
  ArduinoOTA.handle();
  delay(1000 / FRAMES_PER_SECOND);
}

void setup() {
  pinMode(0,OUTPUT);
  digitalWrite(0,LOW);

  Serial.begin(115200);
  
  // button1.attachClick([]() { shutdown(); });
  // button2.attachClick([]() { ui_switch_page(); });


    wcli.setCallback(new mESP32WifiCLICallbacks());
    wcli.setSilentMode(true);
    // CLI config  
    wcli.add("ntpserver", &setNTPServer, "\tset NTP server. Default: pool.ntp.org");
    wcli.add("ntpzone", &setTimeZone, "\tset TZONE. https://tinyurl.com/4s44uyzn");
    wcli.add("time", &printLocalTime, "\t\tprint the current time");
    wcli.add("color", &setColor, "\tset constant color");
    wcli.add("hueoffset", &setHueOffset, "\tset color hue offset");
    wcli.add("displaymode", &setDisplayMode, "\tset display mode. T for time, F for full screen");
    wcli.add("colormode", &setColorMode, "\tset Color mode. D for Day, H for Hour, M for Minute and C for constant color");
    wcli.add("brightness", &setBrightness, "\tset brightness. can be 0-255 or auto");

    // NTP init
    updateTimeSettings();
    wcli_setup_ready = wcli.isConfigured();
    wcli.begin("nightclock");
    wcli.setInt(key_hue_offset,0);

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;
    

    // FastLED.addLeds<NEOPIXEL, PIN>(matrix[0], matrix.Size()).setCorrection(0xFFC0F0);
    FastLED.addLeds<NEOPIXEL, PIN>(matrix[0], matrix.Size()).setCorrection(TypicalLEDStrip);
    // FastLED.addLeds<WS2811, PIN, GRB>(matrix[0], matrix.Size()).setCorrection(UncorrectedColor);
    FastLED.setBrightness(255);
  

    FastLED.clear();
    FastLED.show();

    // Time for serial port to work?
    Serial.print("Init on pin: ");
    Serial.println(PIN);
    Serial.print("Matrix Size: ");
    Serial.print(MATRIX_WIDTH);
    Serial.print(" ");
    Serial.print(MATRIX_HEIGHT);
    Serial.print(" ");
    Serial.println(NUMMATRIX);

    esp_log_level_set(TAG, ESP_LOG_WARN); 
    esp_log_level_set("*", ESP_LOG_WARN);



  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

ArduinoOTA.begin();

}

// vim:sts=4:sw=4
