#include "main.h"
#include <FastLED.h>

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
  