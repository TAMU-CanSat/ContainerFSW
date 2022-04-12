#include "Common.h"
#include "Hardware.h"
#include "States.h"

void setup() {  
  GROUND_XBEE_SERIAL.begin(9600);
  PAYLOAD_XBEE_SERIAL.begin(9600);
  
  std::thread ground(Hardware::ground_radio_loop);
  std::thread payload(Hardware::payload_radio_loop);
  
  ground.detach();
  payload.detach();
}

void loop() {
  switch (States::state)
  {
    case 0:
      States::Initialization();
      break;
    case 1:
      States::Standby();
      break;
    case 2:
      States::Flight();
      break;
    case 3:
      States::Rapid();
      break;
    case 4:
      States::Slow();
      break;
    case 5:
      States::Landing();
      break;
    default:
      States::Initialization();
      break;
  }
}
