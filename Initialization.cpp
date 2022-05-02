#include "Common.h"
#include "Hardware.h"
#include "States.h"

namespace States
{  
  void Initialization()
  {
    unsigned long start = millis();

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
  
    if (Common::TELEMETRY_DELAY > (millis() - start))
      delay(Common::TELEMETRY_DELAY - (millis() - start));
    }
}
