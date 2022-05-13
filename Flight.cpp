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
    Common::build_packet(packet, "Flight", "N", mission_start_time, sensor_data);

    // Send the container packet down to the ground station
    Hardware::mtx.lock();
    Hardware::ground_packets.enqueue(packet);
    Hardware::payload_packets.enqueue("0");
    Hardware::mtx.unlock();

    // Now we're going to monitor the altitude readings and detect when altitude begins to decrease
    int altitude_length = sizeof(Common::altitudes) / sizeof(Common::altitudes[0]);

    // We'll check to see if there are enough altitudes stored to calculate velocity
    // NOTE: time isn't taken into account for the measurement, so technically there are no units
    // The following was my logic for getting the altitude vector down to a length of two. It looks ugly but I think it works.

    if (altitude_length > 2)
    {
      do
      {
        Common::altitudes.erase(0)
            altitude_length = sizeof(Common::altitudes) / sizeof(Common::altitudes[0])
      } while (altitude_length > 1)
    }
    Common::altitudes.push_back(gps_data.altitude);

    if (altitude_length == 2)
    {
      Common::vertical_velocity = Common::altitudes[1] - Common::altitudes[0];
      Common::altitudes.pop_back();
    }

    // Now we'll actually switch the state
    if (Common::vertical_velocity < 0)
    {
      States::EE_STATE = 3;
      EEPROM.put(Common::ST_ADDR, 3);
    }
  }
}
