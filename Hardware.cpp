#include "Common.h"
#include "Hardware.h"
#include "States.h"

#include <Wire.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_GPS.h>
#include <TeensyThreads.h>
#include <ArduinoQueue.h>
#include <EEPROM.h>
#include <TimeLib.h>

namespace Hardware
{   
  Adafruit_BMP3XX bmp;
  Adafruit_GPS GPS(&Serial1);

  ArduinoQueue<String> payload_packets(20);
  ArduinoQueue<String> ground_packets(20);
  Threads::Mutex mtx;
  
  void init()
  {
    Wire.setSCL(Common::BMP_SCL);
    Wire.setSDA(Common::BMP_SDA);
    Wire.begin();
    bmp.begin_I2C();
    GPS.begin(9600);
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

    setTime(GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);

    data.hours = GPS.hour;
    data.minutes = GPS.minute;
    data.seconds = GPS.seconds + Common::LEAP_SECONDS;
    data.milliseconds = GPS.milliseconds;
    data.latitude = GPS.latitude;
    data.longitude = GPS.longitude;
    data.altitude = GPS.altitude;
    data.sats = (byte)(unsigned int)GPS.satellites;  // We do this double conversion to avoid signing issues
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
    data.vbat = map(analogRead(Common::VOLTAGE_PIN), 0, 1023, 0, 5.5);
    data.altitude = bmp.readAltitude(Common::SEA_LEVEL);
    data.temperature = bmp.readTemperature();
  }

  void payload_radio_loop()
  {
    while(1)
    {
      mtx.lock();
      while (!payload_packets.isEmpty())
      {
        PAYLOAD_XBEE_SERIAL.println(payload_packets.dequeue());
        Common::EE_PACKET_COUNT++;
        EEPROM.put(Common::PC_ADDR, Common::EE_PACKET_COUNT);
      }
      mtx.unlock();
  
      String received;
      while (read_payload_radio(received))
      {
        String header = String(Common::TEAM_ID + 5000) + ",";
        header += String(hour()) + ":" + String(minute()) + ":" + String(second()) + "." + String(elapsedMillis()) + ",";
        header += String(Common::EE_PACKET_COUNT) + ",";
        header += "T,";

        mtx.lock();
        ground_packets.enqueue(header + received);
        mtx.unlock();
      }
      delay(10);
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
        Common::EE_PACKET_COUNT++;
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
              EEPROM.put(Common::ST_ADDR, 0);
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
      delay(10);
    }
  }
}
