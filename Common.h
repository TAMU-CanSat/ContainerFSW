#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino.h>
#include <TeensyThreads.h>
#include <ArduinoQueue.h>
#include <TimeLib.h>

#define GPS_SERIAL Serial1
#define GROUND_XBEE_SERIAL Serial4
#define PAYLOAD_XBEE_SERIAL Serial5

namespace Common {
  const unsigned long TELEMETRY_DELAY = 1000; //1hz
  const byte VOLTAGE_PIN = 38;
  const byte BMP_SCL = 24;
  const byte BMP_SDA = 25;
  const byte PARA_SERVO_PIN = 41;
  const byte CAMERA_PIN = 36;
  const byte AUDIO_BEACON_PIN = 37;
  const float SEA_LEVEL = 1014.6f; //update this before launch
  const uint16_t TEAM_ID = 1051;
  const uint16_t BA_ADDR = 0;
  const uint16_t PC_ADDR = 4;
  const uint16_t ST_ADDR = 6;
  
  struct GPS_Data
  {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint16_t milliseconds;
    float latitude;
    float longitude;
    float altitude;
    byte sats;
  };
  
  struct Sensor_Data
  {
    float vbat;
    float altitude;
    float temperature;
  };
}
#endif
