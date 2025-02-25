#include <PS4Controller.h>
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "main.hpp"
#include "esp_err.h"
#ifdef ENABLE_REMOTE_DEBUG
#include "RemoteDebug.h"
#endif

unsigned long lastTimeStamp = 0;
#define EVENTS 0
#define BUTTONS 0
#define JOYSTICKS 0
#define SENSORS 0

void removePairedDevices();
void onConnect();
void notify();
void onDisConnect();

void setupDS4() {
  PS4.attach(notify);
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisConnect);
  PS4.begin();
  removePairedDevices(); // This helps to solve connection issues
}

void removePairedDevices() {
  uint8_t pairedDeviceBtAddr[20][6];
  int count = esp_bt_gap_get_bond_device_num();
  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
  for (int i = 0; i < count; i++) {
    esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
  }
}

void onConnect() {
  Serial.println("Connected!");
}

void notify() {
  #if EVENTS
    boolean sqd = PS4.event.button_down.square,
            squ = PS4.event.button_up.square,
            trd = PS4.event.button_down.triangle,
            tru = PS4.event.button_up.triangle,
            upd = PS4.event.button_up.down,
            upu = PS4.event.button_up.up,
            dou = PS4.event.button_down.up,
            dod = PS4.event.button_down.down;
    if (PS4.event.button_down.up)
      Serial.println("^");
    else if (PS4.event.button_down.down)
      Serial.println("|");
    else if (PS4.event.button_down.right)
      Serial.println(">");
    else if (PS4.event.button_down.left)
      Serial.println("<");
  #endif

#if BUTTONS
  boolean sq = PS4.Square(),
          tr = PS4.Triangle();
  if (sq)
    Serial.print(" SQUARE pressed");
  if (tr)
    Serial.print(" TRIANGLE pressed");
  if (sq | tr)
    Serial.println();
#endif

  //Only needed to print the message properly on serial monitor. Else we dont need it.
  if (millis() - lastTimeStamp > 50) {
#if JOYSTICKS
    Serial.printf("lx:%4d,ly:%4d,rx:%4d,ry:%4d\n",
                  PS4.LStickX(),
                  PS4.LStickY(),
                  PS4.RStickX(),
                  PS4.RStickY());
#endif
#if SENSORS
    Serial.printf("gx:%5d,gy:%5d,gz:%5d,ax:%5d,ay:%5d,az:%5d\n",
                  PS4.GyrX(),
                  PS4.GyrY(),
                  PS4.GyrZ(),
                  PS4.AccX(),
                  PS4.AccY(),
                  PS4.AccZ());
#endif
    lastTimeStamp = millis();
  }

  if (PS4.event.button_down.cross){
    hueConfigMode = true;
  }
  if (hueConfigMode){
    int8_t x = PS4.LStickX();
    int8_t y = PS4.LStickY();
    float r = sqrt(x * x + y * y);
    float theta = atan2(y, x);
    hueOffsetSetting = map(theta * 180 / PI, -180, 180, 0, 255);
    Serial.printf("hueOffset: %d\n", hueOffsetSetting);
    if (PS4.event.button_up.cross){
      hueConfigMode = false;
    }
  }



}

void onDisConnect() {
  Serial.println("Disconnected!");
}