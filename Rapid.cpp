#include "Common.h"
#include "Hardware.h"
#include "States.h"

namespace States
{
  void Rapid()
  {
    float sum = 0;
    // Release drogue chute
    if (!Common::drogue_chute_deployed)
    {
      Hardware::deploy_chute();
      Common::drogue_chute_deployed = true;
    }

    // Take telemetry
    // Container Telemetry: BMP 308 @ 1Hz, GPS @ 1Hz
    Common::GPS_Data gps_data;
    Common::Sensor_Data sensor_data;
    Hardware::read_gps(gps_data);
    Hardware::read_sensors(sensor_data);

    // Build the packet with the data
    String packet;
    Common::build_packet(packet, "Rapid", "N", mission_start_time, sensor_data);

    // Send the container packet down to the ground station
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue("0");
    Hardware::mtx.unlock();

    // Record with camera
    Hardware::update_camera()

        // If altitude drops below 400 meters, switch states
        if (altitude_length >= 3)
    {
      for (int i = 0, i < 2, i++)
      {
        sum = sum + Common::altitudes[i];
      }
      Common::altitudes.erase(0);
    }
    Common::altitudes.push_back(gps_data.altitude);

    if ((sum / 3) < 400)
    {
      States::EE_STATE = 4;
      EEPROM.put(Common::ST_ADDR, 4);
    }
  }
}
