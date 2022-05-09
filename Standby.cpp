
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
    Common::build_packet(packet, "Standby", "N", mission_start_time, sensor_data);

    // Send the container packet down to the ground station
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue("0");
    Hardware::mtx.unlock();

    // Detect Acceleration
    // The plan to detect acceleration is to keep a queue of maybe 20 altitude readings,
    // derive it once for velocity, and derive it again for acceleration
    int altitude_length = sizeof(Common::altitudes) / sizeof(Common::altitudes[0]);

    // We'll check to see if there are enough altitudes stored to calculate acceleration:
    if (altitude_length >= 3)
    {
      for (int i = 0, i < 2, i++)
      {
        Common::velocities.push_back(Common::altitudes[i + 1] - Common::altitudes[i]);
        if (i > 0)
        {
          Common::acceleration = Common::velocities[i + 1] - Common::velocities[i];
        }
      }
      Common::altitudes.pop_back();
    }
    Common::altitudes.push_back(gps_data.altitude);

    // Now we'll actually switch the state
    if (Common::acceleration >= 18 || Common::altitudes[altitude_length - 1] >= 10)
    {
      States::EE_STATE = 2;
      EEPROM.put(Common::ST_ADDR, 2);
    }

    // I'm guessing that this part is to account for delay of sending???
    if (Common::TELEMETRY_DELAY > (millis() - start))
    {
      delay(Common::TELEMETRY_DELAY - (millis() - start));
    }
  }
}
