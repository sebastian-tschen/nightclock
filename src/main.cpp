#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include <tinifont.h>
#include "OneButton.h"
#include "WiFi.h"
#include "esp_sntp.h"
#include "time.h"
#include <ESP32WifiCLI.hpp>
#include "pin_config.h"

#define PIN 16

// Max is 255, 32 is a conservative value to not overload
// a USB power supply (500mA) for 12x12 pixels.
#define BRIGHTNESS 255

// #define delay FastLED.delay

#define mw 9
#define mh 12
#define NUMMATRIX (mw*mh)
CRGB leds[NUMMATRIX];
// Define matrix width and height.
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, mw, mh, 
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);


void matrix_show() {
    matrix->show();
}

static const char* TAG = "nightclock";

u_int8_t r;
u_int8_t g;
u_int8_t b;


OneButton button1(14, true);
OneButton button2(15, true);


const char * key_ntp_server = "kntpserver";
const char * key_tzone = "ktzone";
const char * key_color = "kcolor";

// change these params via CLI:
const char * default_server = "pool.ntp.org";  
const char * default_tzone = "CET-1CEST,M3.5.0,M10.5.0/3";
const char * default_color = "ffffff";

bool wcli_setup_ready = false;

static uint8_t brightness_readings[10] = {0};
static uint8_t rollingBrightnesIndex = 0;
static uint32_t sum = 0;
static uint8_t average_brightness = 0;


void matrix_clear() {
    // clear does not work properly with multiple matrices connected via parallel inputs
    memset(leds, 0, sizeof(leds));
}

uint16_t getColorForDay(struct tm *timeinfo) {
  // Calculate the total minutes since midnight
  int totalMinutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;

  // Calculate the hue value (0-255) based on the total minutes
  uint8_t hue = map(timeinfo->tm_min, 0, 59, 0, 255);
  
  // Add an offset value to the hue
  uint8_t hueOffset = wcli.getInt(key_color,0);
  hue = (hue + hueOffset) % 256;

  // Convert the hue to an RGB value
  CRGB color = CHSV(hue, 255, average_brightness);
  Serial.printf("HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, average_brightness, color.r, color.g, color.b);
  return matrix->Color(color.r, color.g, color.b);
}

uint16_t getColorForMinute(struct tm *timeinfo) {
  // Calculate the total minutes since midnight
  int totalMinutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;

  // Calculate the hue value (0-255) based on the total minutes
  uint8_t hue = map(timeinfo->tm_sec, 0, 59, 0, 255);
  
  // Add an offset value to the hue
  uint8_t hueOffset = wcli.getInt(key_color,0);
  hue = (hue + hueOffset) % 256;

  // Convert the hue to an RGB value
  CRGB color = CHSV(hue, 255, average_brightness);
  Serial.printf("HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, average_brightness, color.r, color.g, color.b);
  return matrix->Color(color.r, color.g, color.b);
}

uint16_t getColorForHour(struct tm *timeinfo) {
  // Calculate the total minutes since midnight
  int totalSeconds = timeinfo->tm_min * 60 + timeinfo->tm_sec;

  // Calculate the hue value (0-255) based on the total minutes
  uint8_t hue = map(totalSeconds, 0, 3599, 0, 255);
  
  // Add an offset value to the hue
  uint8_t hueOffset = wcli.getInt(key_color,0);
  hue = (hue + hueOffset) % 256;

  // Convert the hue to an RGB value
  CRGB color = CHSV(hue, 255, average_brightness);
  Serial.printf("HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, average_brightness, color.r, color.g, color.b);
  return matrix->Color(color.r, color.g, color.b);
}

void displayTime(struct tm * info) {


    matrix->setTextSize(1);
    matrix->setFont(&Font3x5FixedNum);    
    matrix->setTextColor(getColorForHour(info));
    // matrix->setTextColor(matrix->Color(255,255,255));
    matrix_clear();

    // print hours
    matrix->setCursor(1, 5);
    matrix->print(info->tm_hour/10);
    matrix->setCursor(5, 5);
    matrix->print(info->tm_hour%10);

    // print minutes
    matrix->setCursor(1, 12);
    matrix->print(info->tm_min/10);
    matrix->setCursor(5, 12);
    matrix->print(info->tm_min%10);
    matrix_show();

    
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
  String colorHueOffset = operands.first();

  uint8_t hueOffset = strtol(colorHueOffset.c_str(), NULL, 10);
  Serial.printf("Setting color hue offset to %d\r\n", hueOffset);
  wcli.setInt(key_color, hueOffset);
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
    uint8_t brightness = map(analogRead(34),0,4095,50,255);

    // Subtract the oldest reading from the sum
    sum -= brightness_readings[rollingBrightnesIndex];
    // Add the new reading to the sum
    brightness_readings[rollingBrightnesIndex] = brightness;
    sum += brightness;

    // Move to the next index, wrapping around if necessary
    rollingBrightnesIndex = (rollingBrightnesIndex + 1) % 10;

    // Calculate the running average
    average_brightness = sum / 10;

    Serial.println(average_brightness);

}

void loop() {
  button1.tick();
  button2.tick();
  delay(3);
  static uint32_t last_tick;
  if (millis() - last_tick > 1000) {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    displayTime(&timeinfo);
    last_tick = millis();
    updateBrightness();
  }
  while(!wcli_setup_ready) wcli.loop(); // only for fist setup
  wcli.loop();
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
    wcli.add("color", &setColor, "\tset color hue offset");
    
    // NTP init
    updateTimeSettings();
    wcli_setup_ready = wcli.isConfigured();
    wcli.begin("nightclock");

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    FastLED.addLeds<NEOPIXEL,PIN>(  leds, NUMMATRIX  ).setCorrection(0xFFC0F0);
    
    // Time for serial port to work?
    Serial.print("Init on pin: ");
    Serial.println(PIN);
    Serial.print("Matrix Size: ");
    Serial.print(mw);
    Serial.print(" ");
    Serial.print(mh);
    Serial.print(" ");
    Serial.println(NUMMATRIX);
    matrix->begin();
    matrix->setTextWrap(false);
    matrix->setBrightness(BRIGHTNESS);
}

// vim:sts=4:sw=4
