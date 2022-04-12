#pragma once
#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "Common.h"
#include <Arduino.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_GPS.h>
#include <TeensyThreads.h>
#include <ArduinoQueue.h>

namespace Hardware
{
  extern Adafruit_BMP3XX bmp;
  extern Adafruit_MPU6050 mpu;
  extern Adafruit_GPS GPS;

  extern ArduinoQueue<Downlink> payload_packets;
  extern ArduinoQueue<Downlink> ground_packets;
  extern Threads::Mutex mtx;

  bool ready();
  void init();
  void read_gps(GPS_Data &data);
  void read_sensors(Sensor_Data &data);

  void write_payload_radio(Downlink data);
  bool read_payload_radio(String &data);
  
  void write_ground_radio(Downlink data);
  bool read_ground_radio(String &data);

  void payload_radio_loop();
  void ground_radio_loop();
}
#endif
