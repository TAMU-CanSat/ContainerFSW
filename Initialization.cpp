#include "Common.h"
#include "Hardware.h"
#include "States.h"
#include <EEPROM.h>

namespace States
{  
  void Initialization()
  {
    Common::GPS_Data gps_data;
    Common::Sensor_Data sensor_data;
    Hardware::read_gps(gps_data);
    Hardware::read_sensors(sensor_data);

    String packet;
    Common::build_packet(packet, "INITIALIZATION", 'N', gps_data, sensor_data);
  
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue("0");
    Hardware::mtx.unlock();

    int item_count = Hardware::altitudes.itemCount();
    if (item_count == 3)
    {
      float previous_altitude = Hardware::altitudes.dequeue();
      float current_altitude = gps_data.altitude;
      Hardware::altitudes.enqueue(current_altitude);
    }

    else if (item_count < 3)
    {
      float current_altitude = gps_data.altitude;
      Hardware::altitudes.enqueue(current_altitude);
    }
  }
}
