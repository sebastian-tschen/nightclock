
#ifdef ENABLE_WCLI
#ifndef WCLI_HPP
#define WCLI_HPP

#include <ESP32WifiCLI.hpp>

#define WIFI_CONNECT_WAIT_MAX        (30 * 1000)

#define NTP_SERVER1                  "pool.ntp.org"
#define NTP_SERVER2                  "time.nist.gov"
#define GMT_OFFSET_SEC               (3600 * 8)
#define DAY_LIGHT_OFFSET_SEC         0

void setupWCLI();
void updateTimeSettings();

#endif // WCLI_HPP
#endif