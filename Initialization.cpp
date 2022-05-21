#include "Common.h"
#include "Hardware.h"
#include "States.h"

namespace States
{  
  uint16_t EE_STATE = 0;
  
  void Initialization()
  {
    Common::GPS_Data gps_data;
    Common::Sensor_Data sensor_data;
    
    Hardware::read_gps(gps_data);
    Hardware::read_sensors(sensor_data);

    /*if (millis() < 5000)
    {
        Hardware::update_camera(true);
    } else if (millis() > 10000)
    {
        Hardware::update_camera(false);
    }*/

    String packet;
    Hardware::build_packet(packet, "INITIALIZATION", "N", gps_data, sensor_data); // build new packet
    Hardware::ground_packets.enqueue(packet);
  }
}
