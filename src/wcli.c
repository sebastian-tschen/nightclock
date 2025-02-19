
#include <main.h>

void setupWCLI(){

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

}