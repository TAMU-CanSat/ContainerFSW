#include "Common.h"
#include "Hardware.h"
#include "States.h"

#include <Wire.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_GPS.h>
#include <TeensyThreads.h>
#include <ArduinoQueue.h>
#include <EEPROM.h>
#include <TimeLib.h>
#include <Servo.h>

namespace Hardware
{   
  Adafruit_BMP3XX bmp;
  Adafruit_GPS GPS(&GPS_SERIAL);
  Servo para_servo;

  ArduinoQueue<String> payload_packets(20);
  ArduinoQueue<String> ground_packets(20);
  Threads::Mutex mtx;
  
  void init()
  {
    cameraHold = 0;
    cameraRecording = false;
    firstCameraCall = true;
    
    para_servo.attach(Common::PARA_SERVO_PIN);
    Wire2.begin();
    bmp.begin_I2C(0x77, &Wire2);
    GPS.begin(9600);
  }

  void buzzer_on()
  {
    analogWriteFrequency(Common::AUDIO_BEACON_PIN, 4000);
    analogWrite(Common::AUDIO_BEACON_PIN, 128);
  }

  void buzzer_off()
  {
    analogWriteFrequency(Common::AUDIO_BEACON_PIN, 0);
    analogWrite(Common::AUDIO_BEACON_PIN, 0);
  }

  void deploy_chute()
  {
    para_servo.write(30);
  }

  void update_camera(bool record)
  {
    if (record && !cameraRecording)
    {
      if (firstCameraCall)
      { 
        cameraHold = 0;
        firstCameraCall = false;
      }
      
      start_recording();
    } else if (!record && cameraRecording)
    {
      if (firstCameraCall)
      {
        cameraHold = 0;
        firstCameraCall = false;
      }
      
      stop_recording();
    }
  }

  void start_recording()
  {
    if (cameraHold < 150)
    {
      digitalWrite(Common::CAMERA_PIN, 1);
    } else
    {
      digitalWrite(Common::CAMERA_PIN, 0);
      cameraRecording = true;
      firstCameraCall = true;
    }
  }

  void stop_recording()
  {
    if (cameraHold < 550)
    {
      digitalWrite(Common::CAMERA_PIN, 1);
    } else
    {
      digitalWrite(Common::CAMERA_PIN, 0);
      cameraRecording = true;
      firstCameraCall = true;
    }
  }

  void read_gps(Common::GPS_Data &data)
  {
    // Loop until we have a full NMEA sentence and it parses successfully
    do {
      GPS.read();
      while (!GPS.newNMEAreceived()) {
        GPS.read();
      }
    } while (!GPS.parse(GPS.lastNMEA()));

    mtx.lock();
    setTime(GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
    adjustTime(Common::LEAP_SECONDS);

    data.hours = GPS.hour;
    data.minutes = GPS.minute;
    data.seconds = GPS.seconds;
    data.milliseconds = GPS.milliseconds;
    Common::milli = GPS.milliseconds;
    data.latitude = GPS.latitude;
    data.longitude = GPS.longitude;
    data.altitude = GPS.altitude;
    data.sats = (byte)(unsigned int)GPS.satellites;  // We do this double conversion to avoid signing issues
    mtx.unlock();
  }

  bool read_ground_radio(String &data)
  {
    if (GROUND_XBEE_SERIAL.available())
    {
      data = GROUND_XBEE_SERIAL.readStringUntil('\n');
      return true;
    } else
      return false;
  }

  bool read_payload_radio(String &data)
  {
    if (PAYLOAD_XBEE_SERIAL.available())
    {
      data = PAYLOAD_XBEE_SERIAL.readStringUntil('\n');
      return true;
    } else
      return false;
  }

  void read_sensors(Common::Sensor_Data &data)
  {
    data.vbat = ((analogRead(Common::VOLTAGE_PIN) / 1023.0) * 4.2) + 0.35;
    bmp.performReading();
    data.altitude = bmp.readAltitude(Common::SEA_LEVEL);
    data.temperature = bmp.temperature;
  }

  void payload_radio_loop()
  {
    while(1)
    {
      mtx.lock();
      while (!payload_packets.isEmpty())
      {
        PAYLOAD_XBEE_SERIAL.println(payload_packets.dequeue());
        Common::EE_PACKET_COUNT += 1;
        EEPROM.put(Common::PC_ADDR, Common::EE_PACKET_COUNT);
      }
      mtx.unlock();
  
      String received;
      while (read_payload_radio(received))
      {
        String header = String(Common::TEAM_ID + 5000) + ",";
        header += String(hour()) + ":" + String(minute()) + ":" + String(second()) + "." + String(Common::milli) + ",";
        header += String(Common::EE_PACKET_COUNT) + ",";
        header += "T,";

        mtx.lock();
        ground_packets.enqueue(header + received);
        mtx.unlock();
      }
      delay(250);
    }
  }
  
  void ground_radio_loop()
  {
    while(1)
    {
      mtx.lock();
      while (!ground_packets.isEmpty())
      {
        GROUND_XBEE_SERIAL.println(ground_packets.dequeue());
        Common::EE_PACKET_COUNT += 1;
        EEPROM.put(Common::PC_ADDR, Common::EE_PACKET_COUNT);
      }
      mtx.unlock();
  
      String received;
      if (read_ground_radio(received))
      {
        if (received.substring(4, 8).equals(String(Common::TEAM_ID)))
        {
          String data = received.substring(9);
          int comma = data.indexOf(',');
          String cmd = data.substring(0, comma);
          String params = data.substring(data.indexOf(','));

          Common::lastCMD = cmd;

          if (cmd.equals("CX"))
          {
            if (params.equals("ON"))
            {
              States::EE_STATE = 1;
              EEPROM.put(Common::ST_ADDR, 1);
            } else if (params.equals("OFF"))
            {
              States::EE_STATE = 0;
              //reset recovery params
              //EEPROM.put(Common::BA_ADDR, 0.0f);
              //EEPROM.put(Common::PC_ADDR, 0);
              //EEPROM.put(Common::ST_ADDR, 0);
            }
          } else if (cmd.equals("ST"))
          {
            // GPS does this could implement if a must
          } else if (cmd.equals("SIM"))
          {
            if (params.equals("ENABLE"))
            {
              Common::SIM_ENABLE = true;
            } else if (params.equals("DISABLE"))
            {
              Common::SIM_ENABLE = false;
              Common::SIM_ACTIVATE = false;
            } else if (params.equals("ACTIVATE"))
            {
              if (Common::SIM_ENABLE) Common::SIM_ACTIVATE = true;
            }
          } else if (cmd.equals("SIMP"))
          {
            Common::SIM_PRESSURE = params.toInt();
          }
        }
      }
      delay(500);
    }
  }
}
