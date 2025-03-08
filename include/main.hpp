#include <stdint.h>
#include <stdbool.h>
#include <LEDMatrix.h>
#include <FastLED.h>


#define PIN 33

#define BRIGHTNESS_AVERAGE_COUNT 10
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

#define DISPLAY_MODE_COUNT 2
#define TIME 0
#define SNAKE 1



#define DAY_MODE 0
#define HOUR_MODE 1
#define MINUTE_MODE 2
#define CONSTANT_MODE 3
#define AUTO_MODE 0
#define FIXED_MODE 1

extern const char *key_ntp_server;
extern const char *key_tzone;
extern const char *key_color;
extern const char *key_hue_offset;

extern const char *default_server;
extern const char *default_tzone;
extern const int default_color;
extern const char *default_display_mode;

extern bool hueConfigMode;

extern uint16_t ambientBrightnessReadings[BRIGHTNESS_AVERAGE_COUNT];
extern uint8_t rollingBrightnesIndex;
extern uint32_t sum;
extern uint16_t averageAmbientBrightness;
extern uint8_t displayMode;
extern uint8_t colorMode;
extern uint8_t brightnessMode;
extern uint8_t fixed_brightness;
extern uint8_t hueOffsetSetting;

extern struct tm timeinfo;

extern cLEDMatrix<-MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE> matrix;

void setBrightness();
void matrix_show();
void matrix_clear();