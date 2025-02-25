#include "main.hpp"
#include "time.h"
#include "color.h"
#ifdef ENABLE_WCLI
#include "wcli.hpp"
#endif

#ifdef ENABLE_REMOTE_DEBUG
#include "RemoteDebug.h"
#endif

int getHueForTimeInterval(uint8_t interval_mode, struct tm *timeinfo)
{

  int interval, total_time;

  switch (interval_mode)
  {
  case DAY_MODE:
    interval = 1440 - 1; // Total minutes in a day
    total_time = timeinfo->tm_hour * 60 + timeinfo->tm_min;
    break;
  case HOUR_MODE:
    interval = 60 - 1; // Total minutes in an hour
    total_time = timeinfo->tm_min;
    break;
  case MINUTE_MODE:
    interval = 60 - 1; // Total seconds in a minute
    total_time = timeinfo->tm_sec;
    break;
  default:
    interval = 1;
    total_time = 0;
    break;
  }

  return map(total_time, 0, interval, 0, 255);
}

CRGB getColorForTime(uint8_t interval_mode, struct tm *timeinfo, uint8_t hueOffset)
{
  // Calculate the hue value (0-255) based on the current time mode
  uint8_t hue = getHueForTimeInterval(interval_mode, timeinfo);

  uint8_t brightness = fixed_brightness;
  if (brightnessMode == AUTO_MODE)
  {
    brightness = map(averageAmbientBrightness, 0, 4095, MIN_DARK_BRIGHTNESS, 255);
  }
  CRGB color;
  if (brightness < MIN_BRIGHTNESS)
  {
    // use a dark red when embiant light is low
    color = MIN_COLOR;
  }
  else
  {
    // Convert the hue to an RGB value
    color = CHSV(hue, 255, brightness);
  }

  LOG_V("H HSV: %d %d %d RGB: %d %d %d\r\n", hue, 255, brightness, color.r, color.g, color.b);
  return color;
}

CRGB getTextColor()
{
  // get the color depending on the setting. Either by day, hour or minute, or a constant color.

  if (colorMode == CONSTANT_MODE)
  {
    return CRGB(CONFIG_GET_INT(key_color, default_color));
  }
  else
  {
    return getColorForTime(colorMode, &timeinfo, CONFIG_GET_INT(key_hue_offset, 0));
  }
}