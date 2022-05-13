#include "Common.h"
#include "Hardware.h"
#include "States.h"

namespace States
{
  void Slow()
  {
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
      Common::build_packet(packet, "Slow", "Y", gps_data, sensor_data);
    }
    else
    {
      Common::build_packet(packet, "Slow", "N", gps_data, sensor_data);
    }

    // Send the container packet down to the ground station
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue("0");
    Hardware::mtx.unlock();

    // Record with camera
    Hardware::update_camera(true);

    if (Common::altitudes.itemCount() == 3)
    {
      // If altitude drops below 400 meters, drop the payload
      float previous_altitude = Common::altitudes.dequeue();
      float current_altitude = gps_data.altitude);
      Common::altitudes.enqueue(current_altitude);
      
      if (current_altitude < 300 && !Common::payload_deployed)
      {
        // Deploy Payload here
        Common::payload_deployed = true;
      }
  
      if (Common::payload_deployed)
      {
        // Collect payload telemetry here
      }
  
      // Finally, we check to see if the altitude stops changing
      if ((current_altitude - previous_altitude) >= 0)
      {
        States::EE_STATE = 5;
        EEPROM.put(Common::ST_ADDR, 5);
      }
    }
    else if (Common::altitudes.itemCount() < 3)
    {
      float current_altitude = gps_data.altitude);
      Common::altitudes.enqueue(current_altitude);
    }
  }
}
