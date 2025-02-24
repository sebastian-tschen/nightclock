
#include <main.hpp>
#include <wcli.hpp>

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks
{
  void onWifiStatus(bool isConnected) {}
  void onHelpShow() {}
  void onNewWifi(String ssid, String passw) { wcli_setup_ready = wcli.isConfigured(); }
};

void setupWCLI()
{

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
  wcli.setInt(key_hue_offset, 0);
}

void updateTimeSettings()
{
  String server = wcli.getString(key_ntp_server, default_server);
  String tzone = wcli.getString(key_tzone, default_tzone);
  Serial.printf("ntp server: \t%s\r\ntimezone: \t%s\r\n", server.c_str(), tzone.c_str());
  configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, server.c_str(), NTP_SERVER2);
  setenv("TZ", tzone.c_str(), 1);
  tzset();
}

void setNTPServer(char *args, Stream *response)
{
  Pair<String, String> operands = wcli.parseCommand(args);
  String server = operands.first();
  if (server.isEmpty())
  {
    Serial.println(wcli.getString(key_ntp_server, default_server));
    return;
  }
  wcli.setString(key_ntp_server, server);
  updateTimeSettings();
}

void setTimeZone(char *args, Stream *response)
{
  Pair<String, String> operands = wcli.parseCommand(args);
  String tzone = operands.first();
  if (tzone.isEmpty())
  {
    Serial.println(wcli.getString(key_tzone, default_tzone));
    return;
  }
  wcli.setString(key_tzone, tzone);
  updateTimeSettings();
}

void setColor(char *args, Stream *response)
{
  Pair<String, String> operands = wcli.parseCommand(args);
  String colorHex = operands.first();

  uint32_t colorValue = strtol(colorHex.c_str(), NULL, 16);
  CRGB color = CRGB(colorValue);
  Serial.printf("Setting color to %d %d %d\r\n", color.r, color.g, color.b);
  wcli.setInt(key_color, colorValue);
}

void setBrightness(char *args, Stream *response)
{
  Pair<String, String> operands = wcli.parseCommand(args);
  String brightness = operands.first();
  if (brightness == "auto")
  {
    Serial.println("Setting brightness to auto");
    brightnessMode = AUTO_MODE;
  }
  else
  {
    uint8_t brightnessValue = strtol(brightness.c_str(), NULL, 10);
    Serial.printf("Setting brightness to %d\r\n", brightnessValue);
    brightnessMode = FIXED_MODE;
    fixed_brightness = brightnessValue;
  }
}

void setColorMode(char *args, Stream *response)
{
  Pair<String, String> operands = wcli.parseCommand(args);
  String mode = operands.first();
  if (mode == "D")
  {
    Serial.println("Setting color mode to day");
    colorMode = DAY_MODE;
  }
  else if (mode == "H")
  {
    Serial.println("Setting color mode to hour");
    colorMode = HOUR_MODE;
  }
  else if (mode == "M")
  {
    Serial.println("Setting color mode to minute");
    colorMode = MINUTE_MODE;
  }
  else if (mode == "C")
  {
    Serial.println("Setting color mode to constant");
    colorMode = CONSTANT_MODE;
  }
  else
  {
    Serial.println("Invalid color mode. Use D for day, H for hour, M for minute and C for constant color");
  }
}

void setDisplayMode(char *args, Stream *response)
{
  Pair<String, String> operands = wcli.parseCommand(args);
  String mode = operands.first();
  if (mode == "T")
  {
    Serial.println("Setting display mode to time");
    displayMode = TIME;
  }
  else if (mode == "F")
  {
    Serial.println("Setting display mode to full screen");
    displayMode = FULLSCREEN;
  }
  else
  {
    Serial.println("Invalid display mode. Use T for time or F for full screen");
  }
}

void setHueOffset(char *args, Stream *response)
{
  Pair<String, String> operands = wcli.parseCommand(args);
  String colorHueOffset = operands.first();
  uint8_t hueOffset = strtol(colorHueOffset.c_str(), NULL, 10);
  Serial.printf("Setting color hue offset to %d\r\n", hueOffset);
  wcli.setInt(key_hue_offset, hueOffset);
}

void printLocalTime(char *args, Stream *response)
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("No time available (yet)");
    return;
  }
  response->println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
