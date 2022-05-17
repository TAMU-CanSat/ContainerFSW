#include "Common.h"
#include "Hardware.h"
#include "States.h"
#include <EEPROM.h>

namespace States
{
  void Rapid()
  {
    float sum = 0;

    // Take telemetry
    // Container Telemetry: BMP 308 @ 1Hz, GPS @ 1Hz
    Common::GPS_Data gps_data;
    Common::Sensor_Data sensor_data;
    Hardware::read_gps(gps_data);
    Hardware::read_sensors(sensor_data);

    // Build the packet with the data
    String packet;
    Common::build_packet(packet, "Rapid", "N", gps_data, sensor_data);

    // Send the container packet down to the ground station
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue("0");
    Hardware::mtx.unlock();
    
    int item_count = Hardware::altitudes.itemCount();
    if (item_count == 3)
    {
      // If altitude drops below 400 meters, drop the payload
      float previous_altitude = Hardware::altitudes.dequeue();
      float current_altitude = gps_data.altitude;
      Hardware::altitudes.enqueue(current_altitude);
  
      // Record with camera
      Hardware::update_camera(true);
  
      // If altitude drops below 400 meters, switch states
      if (current_altitude <= 400)
      {
        States::EE_STATE = 4;
        EEPROM.put(Common::ST_ADDR, 4);
      }
    }
    else if (item_count < 3)
    {
      float current_altitude = gps_data.altitude;
      Hardware::altitudes.enqueue(current_altitude);
    }
  }
}
