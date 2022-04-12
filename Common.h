#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino.h>
#include <TeensyThreads.h>
#include <ArduinoQueue.h>
#define GROUND_XBEE_SERIAL Serial1
#define PAYLOAD_XBEE_SERIAL Serial2

const unsigned long TELEMETRY_DELAY = 500;
const byte VOLTAGE_PIN = 14;
const byte GPS_FIX_PIN = 12;
const float SEA_LEVEL = 1014.6f;

struct GPS_Data // 13 bytes
{
  float latitude; // 4 bytes
  float longitude; // 4 bytes
  float altitude; // 4 bytes
  byte sats; // 1 byte
};

struct Sensor_Data // 36 bytes
{
  float vbat; // 4 bytes
  float altitude; // 4 bytes
  float temperature; // 4 bytes
  float gyro[3]; // 12 bytes
  float acceleration[3]; // 12 bytes
};

struct Downlink // 54 bytes
{
  unsigned long milliseconds; //4 bytes
  bool gps_fix; //1 byte 
  GPS_Data gps_data; // 13 bytes 
  Sensor_Data sensor_data; // 36 bytes
};
#endif
