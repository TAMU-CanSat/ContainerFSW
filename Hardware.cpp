#include "Common.h"
#include "Hardware.h"

#include <Wire.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_GPS.h>
#include <TeensyThreads.h>
#include <ArduinoQueue.h>

namespace Hardware
{   
  Adafruit_BMP3XX bmp;
  Adafruit_MPU6050 mpu;
  Adafruit_GPS GPS(&Serial3);

  ArduinoQueue<Downlink> payload_packets(20);
  ArduinoQueue<Downlink> ground_packets(20);
  Threads::Mutex mtx;
  
  bool initialized = false;

  bool ready()
  {
    return initialized;
  }
  
  void init()
  {
    Wire.begin();
    bmp.begin_I2C();
    mpu.begin();
    GPS.begin(9600);

    initialized = true;
  }

  void read_gps(GPS_Data &data)
  {
    // Loop until we have a full NMEA sentence and it parses successfully
    do {
      GPS.read();
      while (!GPS.newNMEAreceived()) {
        GPS.read();
      }
    } while (!GPS.parse(GPS.lastNMEA()));

    data.latitude = GPS.latitude;
    data.longitude = GPS.longitude;
    data.altitude = GPS.altitude;
    data.sats = (byte)(unsigned int)GPS.satellites;  // We do this double conversion to avoid signing issues
  }

  void write_ground_radio(Downlink data)
  {
    String packet = String(data.milliseconds) + ","; //0
    packet += String(data.gps_fix) + ",";            //1
    packet += String(data.gps_data.latitude) + ",";  //2
    packet += String(data.gps_data.longitude) + ","; //3
    packet += String(data.gps_data.altitude) + ",";  //4
    packet += String(data.gps_data.sats) + ",";      //5
    packet += String(data.sensor_data.vbat) + ",";   //6
    packet += String(data.sensor_data.altitude) + ","; //7
    packet += String(data.sensor_data.temperature) + ","; //8
    packet += String(data.sensor_data.gyro[0]) + ","; //9
    packet += String(data.sensor_data.gyro[1]) + ","; //10
    packet += String(data.sensor_data.gyro[2]) + ","; //11
    packet += String(data.sensor_data.acceleration[0]) + ","; //12
    packet += String(data.sensor_data.acceleration[1]) + ","; //13
    packet += String(data.sensor_data.acceleration[2]) + ","; //14
    GROUND_XBEE_SERIAL.println(packet);
  }

  void write_payload_radio(Downlink data)
  {
    String packet = String(data.milliseconds) + ","; //0
    packet += String(data.gps_fix) + ",";            //1
    packet += String(data.gps_data.latitude) + ",";  //2
    packet += String(data.gps_data.longitude) + ","; //3
    packet += String(data.gps_data.altitude) + ",";  //4
    packet += String(data.gps_data.sats) + ",";      //5
    packet += String(data.sensor_data.vbat) + ",";   //6
    packet += String(data.sensor_data.altitude) + ","; //7
    packet += String(data.sensor_data.temperature) + ","; //8
    packet += String(data.sensor_data.gyro[0]) + ","; //9
    packet += String(data.sensor_data.gyro[1]) + ","; //10
    packet += String(data.sensor_data.gyro[2]) + ","; //11
    packet += String(data.sensor_data.acceleration[0]) + ","; //12
    packet += String(data.sensor_data.acceleration[1]) + ","; //13
    packet += String(data.sensor_data.acceleration[2]) + ","; //14
    PAYLOAD_XBEE_SERIAL.println(packet);
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

  void read_sensors(Sensor_Data &data)
  {
    data.vbat = map(analogRead(VOLTAGE_PIN), 0, 1023, 0, 5.5);
    data.altitude = bmp.readAltitude(SEA_LEVEL);
    data.temperature = bmp.readTemperature();

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    data.gyro[0] = g.gyro.x;
    data.gyro[1] = g.gyro.y;
    data.gyro[2] = g.gyro.z;

    data.acceleration[0] = a.acceleration.x;
    data.acceleration[1] = a.acceleration.y;
    data.acceleration[2] = a.acceleration.z;
  }

  void payload_radio_loop()
  {
    while(1)
    {
      mtx.lock();
      while (!payload_packets.isEmpty())
      {
        write_payload_radio(payload_packets.dequeue());
      }
      mtx.unlock();
  
      String received;
      if (read_payload_radio(received))
      {
        for(unsigned int i = 0; i < received.length(); i++)
        {
          // do something with received[i]
        }
      }
      delay(100);
    }
  }
  
  void ground_radio_loop()
  {
    while(1)
    {
      mtx.lock();
      while (!ground_packets.isEmpty())
      {
        write_ground_radio(ground_packets.dequeue());
      }
      mtx.unlock();
  
      String received;
      if (read_ground_radio(received))
      {
        for(unsigned int i = 0; i < received.length(); i++)
        {
          // do something with received[i]
        }
      }
      delay(100);
    }
  }
}
