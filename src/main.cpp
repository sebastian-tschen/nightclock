// #include <Adafruit_GFX.h>
// #include <FastLED_GFX.h>
#include <LEDMatrix.h>
#include <tinifont.h>

#include "WiFi.h"
#include "esp_sntp.h"
#include "time.h"
#include "main.hpp"
#include <ota.hpp>
#include <color.h>
#include <ArduinoOTA.h>
#include <ds4.h>

bool gReverseDirection = false;
cLEDMatrix<-MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE> matrix;


void matrix_show() {
    FastLED.show();
}
void matrix_clear() {
    FastLED.clear();
}

static const char* TAG = "nightclock";

bool foo;


const char *key_ntp_server = "kntpserver";
const char *key_tzone = "ktzone";
const char *key_color = "kcolor";
const char *key_hue_offset = "khueoffset";

const char *default_server = "pool.ntp.org";
const char *default_tzone = "CET-1CEST,M3.5.0,M10.5.0/3";
const int default_color = 0xffffff;
const char *default_display_mode = TIME;

bool hueConfigMode = false;

uint16_t ambientBrightnessReadings[BRIGHTNESS_AVERAGE_COUNT] = {0};
uint8_t rollingBrightnesIndex = 0;
uint32_t sum = 0;
uint16_t averageAmbientBrightness = 0;
uint8_t displayMode = TIME;
uint8_t colorMode = DAY_MODE;
uint8_t brightnessMode = AUTO_MODE;
uint8_t fixed_brightness = 100;
uint8_t hueOffsetSetting = 0;

struct tm timeinfo;


#ifdef ENABLE_REMOTE_DEBUG
RemoteDebug Debug;
#endif



void displayFullScreen() {
  CRGB color = getTextColor();
  matrix.DrawFilledRectangle(0,0,MATRIX_WIDTH, MATRIX_HEIGHT, color);
  matrix_show();
}


void displayTime() {

    // matrix.DrawCircle(4,6,1,getColorForMinute(info));
    matrix.setFont(&Font3x5FixedNum);
    matrix_clear();
    CRGB color = getTextColor();
    // print hours
    matrix.DrawChar(1,5,timeinfo.tm_hour/10+'0',color,0,1);
    matrix.DrawChar(5,5,timeinfo.tm_hour%10+'0',color,0,1);

    // print minutes
    matrix.DrawChar(1,12,timeinfo.tm_min/10+'0',color,0,1);
    matrix.DrawChar(5,12,timeinfo.tm_min%10+'0',color,0,1);
    matrix_show();


}

void updateDisplay() {
  if(displayMode == TIME){
    displayTime();
  } else if(displayMode == FULLSCREEN){
    displayFullScreen();
  }
}

void updateAmbientLight() {

    uint16_t brightness = analogRead(34);

    // Subtract the oldest reading from the sum
    sum -= ambientBrightnessReadings[rollingBrightnesIndex];
    // Add the new reading to the sum
    ambientBrightnessReadings[rollingBrightnesIndex] = brightness;
    sum += brightness;

    // Move to the next index, wrapping around if necessary
    rollingBrightnesIndex = (rollingBrightnesIndex + 1) % BRIGHTNESS_AVERAGE_COUNT;

    // Calculate the running average
    averageAmbientBrightness = sum / BRIGHTNESS_AVERAGE_COUNT;


    ESP_LOGD(TAG, "%d",averageAmbientBrightness);

}

void rainbow_wave(uint8_t thisSpeed, uint8_t deltaHue) {
  FastLED.setBrightness(map(averageAmbientBrightness,0,4095,MIN_DARK_BRIGHTNESS,255));
  uint8_t thisHue = beat8(thisSpeed,255);                     // A simple rainbow march.
  fill_rainbow(matrix[0], NUMMATRIX, thisHue, deltaHue);            // Use FastLED's fill_rainbow routine.
  FastLED.show();
}

void delayForFPS(){
  static uint32_t lastFrame = 0;
  uint32_t currentFrame = millis();
  uint32_t frameTime = 1000 / FRAMES_PER_SECOND;
  if (currentFrame - lastFrame < frameTime){
    delay(frameTime - (currentFrame - lastFrame));
  }
  lastFrame = millis();
}

void setupFastLED(){
  
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

}

void setupSimpleWifi()
{
  WiFi.begin("SSid", "passwd");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  configTime(3600*8, 0, default_server, "pool.ntp.org");
  setenv("TZ", default_tzone, 1);
  tzset();
}

void loop() {
  getLocalTime(&timeinfo, 1000 / FRAMES_PER_SECOND);
  updateDisplay();
  updateAmbientLight();
  ArduinoOTA.handle();
  delayForFPS();
}

void setup()
{
  pinMode(0, OUTPUT); // make sure Boot pin is kept low by software
  digitalWrite(0, LOW);

  Serial.begin(115200);

  setupSimpleWifi();
  
  setupFastLED();
  setupOTA();
  setupDS4();

  while (!getLocalTime(&timeinfo, 1000 / FRAMES_PER_SECOND))
  {
    updateAmbientLight();
    rainbow_wave(40, 2);
  }
  FastLED.setBrightness(255); // fix brightness to max. most modes will sett brightness via color
  #ifdef ENABLE_REMOTE_DEBUG
  Debug.begin(TAG);
  Debug.setResetCmdEnabled(true); // Enable the reset command
  #endif // ENABLE_REMOTE_DEBUG
}
// vim:sts=4:sw=4
