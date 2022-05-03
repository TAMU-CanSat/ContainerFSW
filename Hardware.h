#pragma once
#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "Common.h"
#include <Arduino.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_GPS.h>
#include <TeensyThreads.h>
#include <ArduinoQueue.h>
#include <Servo.h>

namespace Hardware
{
  extern Adafruit_BMP3XX bmp;
  extern Adafruit_GPS GPS;
  extern Servo para_servo;

  extern ArduinoQueue<String> payload_packets;
  extern ArduinoQueue<String> ground_packets;
  extern Threads::Mutex mtx;

  static elapsedMillis cameraHold;
  static bool cameraRecording;
  static bool firstCameraCall;

  void init();

  void deploy_chute();
  
  void buzzer_on();
  void buzzer_off();

  void update_camera(bool record);
  void start_recording();
  void stop_recording();
  
  void read_gps(Common::GPS_Data &data);
  void read_sensors(Common::Sensor_Data &data);

  void write_payload_radio(const String &data);
  bool read_payload_radio(String &data);
  
  void write_ground_radio(const String &data);
  bool read_ground_radio(String &data);

  void payload_radio_loop();
  void ground_radio_loop();
}
#endif
