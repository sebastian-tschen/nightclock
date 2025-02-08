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

#define PIN 16

// Max is 255, 32 is a conservative value to not overload
// a USB power supply (500mA) for 12x12 pixels.
#define BRIGHTNESS 255
#define BRIGHTNESS_AVERAGE 10
#define MIN_BRIGHTNESS 32
// #define delay FastLED.delay

#define MATRIX_WIDTH 9
#define MATRIX_HEIGHT 12
#define MATRIX_TYPE         VERTICAL_ZIGZAG_MATRIX
#define NUMMATRIX (MATRIX_WIDTH*MATRIX_HEIGHT)
// Define matrix width and height.

#define FRAMES_PER_SECOND 30

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

// change these params via CLI:
const char * default_server = "pool.ntp.org";  
const char * default_tzone = "CET-1CEST,M3.5.0,M10.5.0/3";
const char * default_color = "ffffff";

bool wcli_setup_ready = false;

static uint8_t brightness_readings[BRIGHTNESS_AVERAGE] = {0};
static uint8_t rollingBrightnesIndex = 0;
static uint32_t sum = 0;
static uint8_t average_brightness = 0;


// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120



CRGB getColorForDay(struct tm *timeinfo) {
  // Calculate the total minutes since midnight
  int totalMinutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;

  // Calculate the hue value (0-255) based on the total minutes
  uint8_t hue = map(totalMinutes, 0, 1440, 0, 255);
  
  // Add an offset value to the hue
  uint8_t hueOffset = wcli.getInt(key_color,0);
  hue = (hue + hueOffset) % 256;

  // Convert the hue to an RGB value
  CRGB color = CHSV(hue, 255, average_brightness);
  Serial.printf("D HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, average_brightness, color.r, color.g, color.b);
  return color;
}

CRGB getColorForMinute(struct tm *timeinfo) {
  // Calculate the total minutes since midnight
  int totalMinutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;

  // Calculate the hue value (0-255) based on the total minutes
  uint8_t hue = map(timeinfo->tm_sec, 0, 59, 0, 255);
  
  // Add an offset value to the hue
  uint8_t hueOffset = wcli.getInt(key_color,0);
  hue = (hue + hueOffset) % 256;

  // Convert the hue to an RGB value
  CRGB color = CHSV(hue, 255, average_brightness);
  Serial.printf("M HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, average_brightness, color.r, color.g, color.b);
  return color;
}

CRGB getColorForHour(struct tm *timeinfo) {
  // Calculate the total minutes since midnight
  int totalSeconds = timeinfo->tm_min * 60 + timeinfo->tm_sec;

  // Calculate the hue value (0-255) based on the total minutes
  uint8_t hue = map(totalSeconds, 0, 3599, 0, 255);
  
  // Add an offset value to the hue
  uint8_t hueOffset = wcli.getInt(key_color,0);
  hue = (hue + hueOffset) % 256;

  // Convert the hue to an RGB value
  CRGB color = CHSV(hue, 255, average_brightness);
  Serial.printf("H HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, average_brightness, color.r, color.g, color.b);
  return color;
}

void displayTime(struct tm * info) {

    // matrix.DrawCircle(4,6,1,getColorForMinute(info));
    matrix.setFont(&Font3x5FixedNum);    
    matrix_clear();
    CRGB color = getColorForDay(info);
    // print hours
    matrix.DrawChar(1,5,info->tm_hour/10+'0',color,0,1);
    matrix.DrawChar(5,5,info->tm_hour%10+'0',color,0,1);

    // print minutes
    matrix.DrawChar(1,12,info->tm_min/10+'0',color,0,1);
    matrix.DrawChar(5,12,info->tm_min%10+'0',color,0,1);
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

  
    uint8_t brightness = map(analogRead(34),0,4095,MIN_BRIGHTNESS,255);

    // Subtract the oldest reading from the sum
    sum -= brightness_readings[rollingBrightnesIndex];
    // Add the new reading to the sum
    brightness_readings[rollingBrightnesIndex] = brightness;
    sum += brightness;

    // Move to the next index, wrapping around if necessary
    rollingBrightnesIndex = (rollingBrightnesIndex + 1) % BRIGHTNESS_AVERAGE;

    // Calculate the running average
    average_brightness = sum / BRIGHTNESS_AVERAGE;


    Serial.println(average_brightness);

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

  displayTime(&timeinfo);
  updateBrightness();
  while(!wcli_setup_ready) wcli.loop(); // only for fist setup
  wcli.loop();
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
    wcli.add("color", &setColor, "\tset color hue offset");
    
    // NTP init
    updateTimeSettings();
    wcli_setup_ready = wcli.isConfigured();
    wcli.begin("nightclock");

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;
    

    // FastLED.addLeds<NEOPIXEL, PIN>(matrix[0], matrix.Size()).setCorrection(0xFFC0F0);
    FastLED.addLeds<NEOPIXEL, PIN>(matrix[0], matrix.Size()).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
  

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
    
}

// vim:sts=4:sw=4
