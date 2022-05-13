#include "Common.h"
#include "Hardware.h"
#include "States.h"

namespace States
{
  void Flight()
  {
    // Container Telemetry: BMP 308 @ 1Hz, GPS @ 1Hz
    Common::GPS_Data gps_data;
    Common::Sensor_Data sensor_data;
    Hardware::read_gps(gps_data);
    Hardware::read_sensors(sensor_data);

    // Build the packet with the data
    String packet;
    Common::build_packet(packet, "Flight", "N", gps_data, sensor_data);

    // Send the container packet down to the ground station
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue("0");
    Hardware::mtx.unlock();
    
    // If altitude drops below 400 meters, drop the payload
    if (Common::altitudes.itemCount() == 3)
    {
      float previous_altitude = Common::altitudes.dequeue();
      Common::altitudes.enqueue(current_altitude);
      float current_velocity = current_altitude - previous_altitude;

      // Now we'll actually switch the state
      if (current_velocity < 0)
      {
        States::EE_STATE = 3;
        EEPROM.put(Common::ST_ADDR, 3);
      }
    }
    else if (Common::altitudes.itemCount() < 3)
    {
      float current_altitude = gps_data.altitude);
      Common::altitudes.enqueue(current_altitude)
    }

  }
}
