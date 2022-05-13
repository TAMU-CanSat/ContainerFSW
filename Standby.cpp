
#include "Common.h"
#include "Hardware.h"
#include "States.h"

namespace States
{
  void Standby()
  {
    // Offset altitude reading by average calculated

    // Container Telemetry: BMP 308 @ 1Hz, GPS @ 1Hz
    Common::GPS_Data gps_data;
    Common::Sensor_Data sensor_data;
    Hardware::read_gps(gps_data);
    Hardware::read_sensors(sensor_data);

    // Build the packet with the data
    String packet;
    Common::build_packet(packet, "Standby", "N", gps_data, sensor_data);

    // Send the container packet down to the ground station
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue("0");
    Hardware::mtx.unlock();

    // We make sure we always have a queue of the correct length, and if we do, we proceed to check for the next state
    if (Common::altitudes.itemCount() == 3)
    {
      float previous_altitude = Common::altitudes.dequeue();
      float current_altitude = gps_data.altitude);
      Common::altitudes.enqueue(current_altitude);
      
      // Now we'll actually switch the state
      if (current_altitude >= 10)
      {
        States::EE_STATE = 2;
        EEPROM.put(Common::ST_ADDR, 2);
      }  
    }

    // If the queue isn't the correct length, then it is lower than the correct length, so we will skip the dequeue
    else if (Common::altitudes.itemCount() < 3)
    {
      float current_altitude = gps_data.altitude);
      Common::altitudes.enqueue(current_altitude);
    }
    
    // I'm guessing that this part is to account for delay of sending???
    if (Common::TELEMETRY_DELAY > (millis() - start))
    {
      delay(Common::TELEMETRY_DELAY - (millis() - start));
    }
  }
}
