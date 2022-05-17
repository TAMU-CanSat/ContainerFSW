#include "Common.h"
#include "Hardware.h"
#include "States.h"
#include <EEPROM.h>

void setup() {
  Hardware::init();
  GROUND_XBEE_SERIAL.begin(115200); //xbees must be preconfigured for this
  PAYLOAD_XBEE_SERIAL.begin(115200); //default baud is 9600
  
  std::thread ground(Hardware::ground_radio_loop);
  std::thread payload(Hardware::payload_radio_loop);

  //load recovery params
  EEPROM.get(Common::BA_ADDR, Common::EE_BASE_ALTITUDE);  
  EEPROM.get(Common::PC_ADDR, Common::EE_PACKET_COUNT); 
  EEPROM.get(Common::ST_ADDR, States::EE_STATE);   
  
  ground.detach();
  
  payload.detach();
}

void loop() {
  switch (States::EE_STATE)
  {
    case 0:
      //reset recovery params
      EEPROM.put(Common::BA_ADDR, 0.0f);
      EEPROM.put(Common::PC_ADDR, 0);
      EEPROM.put(Common::ST_ADDR, 0);
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
