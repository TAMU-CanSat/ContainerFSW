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
#include <SD.h>
#include <SPI.h>

namespace Hardware
{
  extern bool SIM_ACTIVATE;
  extern bool SIM_ENABLE;
  extern int SIM_PRESSURE;
  extern float EE_BASE_ALTITUDE;
  extern uint16_t EE_PACKET_COUNT;
  extern int lastCheck;
  extern String lastCMD;
  
  extern Adafruit_BMP3XX bmp;
  extern Adafruit_GPS GPS;
  extern Servo para_servo;

  extern ArduinoQueue<String> payload_packets;
  extern ArduinoQueue<String> ground_packets;
  extern Threads::Mutex mtx;

  extern elapsedMillis cameraHold;
  extern bool cameraRecording;
  extern bool firstCameraCall;

  const int chipSelect = BUILTIN_SDCARD;
  static File telemetry;

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

  static int millisecond()
  {
    int elapse = millis() - lastCheck;
    return abs(elapse - ((elapse / 1000) * 1000));
  }

  static void build_packet(String& packet, const String& state, const String& tp_released, const Common::GPS_Data &gps, const Common::Sensor_Data &sensors)
  {
    packet = String(Common::TEAM_ID) + ","; //0
    packet += String(hour()) + ":" + String(minute()) + ":" + String(second()) + "." + String(millisecond()) + ",";
    packet += String(EE_PACKET_COUNT) + ",";
    packet += "C,";
    if (SIM_ACTIVATE && SIM_ENABLE)
      packet += "S,";
    else
      packet += "F,";
    packet += tp_released + ",";
    packet += String(sensors.altitude) + ","; 
    packet += String(sensors.temperature) + ",";
    packet += String(sensors.vbat) + ",";
    packet += String(gps.hours) + ":" + String(gps.minutes) + ":" + String(gps.seconds) + "." + String(gps.milliseconds) + ",";
    packet += String(gps.latitude) + ","; 
    packet += String(gps.longitude) + ","; 
    packet += String(gps.altitude) + ",";  
    packet += String(gps.sats) + ",";
    packet += state + ",";
    packet += lastCMD + "\r\n";
  }
}
#endif
