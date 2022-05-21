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
#include <SD.h>
#include <SPI.h>

namespace Hardware
{  
  bool SIM_ACTIVATE = false;
  bool SIM_ENABLE = false;
  int SIM_PRESSURE = 0;
  float EE_BASE_ALTITUDE = 0;
  uint16_t EE_PACKET_COUNT = 0;
  int lastCheck = 4;
  String lastCMD = "None";
  elapsedMillis cameraHold = 0;
  bool cameraRecording = false;
  bool firstCameraCall = true;
    
  Adafruit_BMP3XX bmp;
  Adafruit_GPS GPS(&GPS_SERIAL);
  Servo para_servo;

  ArduinoQueue<String> payload_packets(20);
  ArduinoQueue<String> ground_packets(20);
  Threads::Mutex mtx;
  
  void init()
  {
    pinMode(Common::CAMERA_PIN, OUTPUT);
    digitalWrite(Common::CAMERA_PIN, HIGH);
    cameraHold = 0;
    cameraRecording = false;
    firstCameraCall = true;
    
    para_servo.attach(Common::PARA_SERVO_PIN);
    para_servo.write(75);
    Wire2.begin();
    bmp.begin_I2C(0x77, &Wire2);

    buzzer_on();

    if (SD.begin(chipSelect))
    {
      //setup GPS
      GPS.begin(9600);                   // 9600 is the default baud rate
      GPS.sendCommand("$PMTK251,38400*27");
      GPS_SERIAL.end();
      delay(200);
      buzzer_off();
      delay(800);
      GPS.begin(38400);
      GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    }

    //buzzer_off();
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
    para_servo.write(0);
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
      pinMode(Common::CAMERA_PIN, OUTPUT);
      digitalWrite(Common::CAMERA_PIN, LOW);
    } else
    {
      pinMode(Common::CAMERA_PIN, OUTPUT);
      digitalWrite(Common::CAMERA_PIN, HIGH);
      cameraRecording = true;
      firstCameraCall = true;
    }
  }

  void stop_recording()
  {
    if (cameraHold < 550)
    {
      pinMode(Common::CAMERA_PIN, OUTPUT);
      digitalWrite(Common::CAMERA_PIN, LOW);
    } else
    {
      pinMode(Common::CAMERA_PIN, OUTPUT);
      digitalWrite(Common::CAMERA_PIN, HIGH);
      cameraRecording = false;
      firstCameraCall = true;
    }
  }

  void read_gps(Common::GPS_Data &data)
  {
    bool newData = false;

    while (!GPS.newNMEAreceived())
    {
      char c = GPS.read();
    }
    newData = GPS.parse(GPS.lastNMEA());
         
    if (newData)
    {
      //Serial.println(String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + "." + String(GPS.milliseconds));
      setTime(GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
      lastCheck = GPS.milliseconds + millis();
    }

    data.hours = GPS.hour;
    data.minutes = GPS.minute;
    data.seconds = GPS.seconds;
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
      data = GROUND_XBEE_SERIAL.readStringUntil('\r\n');
      return true;
    } else
      return false;
  }

  bool read_payload_radio(String &data)
  {
    if (PAYLOAD_XBEE_SERIAL.available())
    {
      data = PAYLOAD_XBEE_SERIAL.readStringUntil('&');
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
        EE_PACKET_COUNT += 1;
        EEPROM.put(Common::PC_ADDR, EE_PACKET_COUNT);
      }

      String received;
      if (read_payload_radio(received))
      {
        String header = String(Common::TEAM_ID + 5000) + ",";
        header += String(hour()) + ":" + String(minute()) + ":" + String(second()) + "." + String(millisecond()) + ",";
        header += String(EE_PACKET_COUNT) + ",";
        header += "T,";
        header += received + "\r\n";

        ground_packets.enqueue(header);
      }
      mtx.unlock();
      threads.delay(250);
    }
  }
  
  void ground_radio_loop()
  {
    while(1)
    { 
      mtx.lock();
      while (!ground_packets.isEmpty())
      {
        String packet = ground_packets.dequeue();
        
        telemetry = SD.open("container_telemetry.csv", FILE_WRITE);
        telemetry.print(packet);
        telemetry.close();
        
        GROUND_XBEE_SERIAL.println(packet);
        EE_PACKET_COUNT += 1;
        EEPROM.put(Common::PC_ADDR, EE_PACKET_COUNT);
      }

      String received;
      if (read_ground_radio(received))
      {
        if (received.substring(4, 8).equals(String(Common::TEAM_ID)))
        {
          String data = received.substring(9);
          int comma = data.indexOf(',');
          String cmd = data.substring(0, comma);
          String params = data.substring(data.indexOf(','));

          lastCMD = cmd;

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
              SIM_ENABLE = true;
            } else if (params.equals("DISABLE"))
            {
              SIM_ENABLE = false;
              SIM_ACTIVATE = false;
            } else if (params.equals("ACTIVATE"))
            {
              if (SIM_ENABLE) SIM_ACTIVATE = true;
            }
          } else if (cmd.equals("SIMP"))
          {
            SIM_PRESSURE = params.toInt();
          }
        }
      }
      mtx.unlock();
      threads.delay(200);
    }
  }
}
