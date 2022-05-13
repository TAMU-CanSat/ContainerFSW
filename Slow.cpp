#include "Common.h"
#include "Hardware.h"
#include "States.h"

namespace States
{
  void Slow()
  {
    float sum = 0;

    // Release drogue chute
    if (!Common::chute_deployed)
    {
      Hardware::deploy_chute();
      Common::chute_deployed = true;
    }

    // Take telemetry
    // Container Telemetry: BMP 308 @ 1Hz, GPS @ 1Hz
    Common::GPS_Data gps_data;
    Common::Sensor_Data sensor_data;
    Hardware::read_gps(gps_data);
    Hardware::read_sensors(sensor_data);

    // Build the packet with the data and check if payload is deployed
    String packet;
    if (Common::payload_deployed)
    {
      Common::build_packet(packet, "Slow", "Y", mission_start_time, sensor_data);
    }
    else
    {
      Common::build_packet(packet, "Slow", "N", mission_start_time, sensor_data);
    }

    // Send the container packet down to the ground station
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue("0");
    Hardware::mtx.unlock();

    // Now we need to figure out how long our altitude queue is
    int altitude_length = sizeof(Common::altitudes) / sizeof(Common::altitudes[0]);

    // Record with camera
    Hardware::update_camera();

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

    if ((sum / 3) < 300 && !Common::payload_deployed)
    {
      // Deploy Payload here
      Common::payload_deployed = false;
    }

    if (Common::payload_deployed)
    {
      // Collect payload telemetry here
    }

    // Finally, we check to see if the altitude stops changing
    if (altitude_length == 3 && (Common::altitudes[2] - Common::altitudes[1]) >= 0)
    {
      States::EE_STATE = 5;
      EEPROM.put(Common::ST_ADDR, 5);
    }
  }
}

