#include "Common.h"
#include "Hardware.h"
#include "States.h"

namespace States
{  
  void Initialization()
  {
    if (!Hardware::ready())
    {
      delay(500);
      return;
    }
    
    unsigned long start = millis();
  
    Downlink packet;
    packet.milliseconds = start;
    Hardware::read_gps(packet.gps_data);
    Hardware::read_sensors(packet.sensor_data);
  
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue(packet);
    Hardware::mtx.unlock();
  
    if (TELEMETRY_DELAY > (millis() - start))
      delay(TELEMETRY_DELAY - (millis() - start));
    }
}
