 #include "Common.h"
#include "Hardware.h"
#include "States.h"
#include <TeensyThreads.h>

#include <EEPROM.h>

void setup() {
  Hardware::init();
  Serial.begin(115200);
  GROUND_XBEE_SERIAL.begin(115200); //xbees must be preconfigured for this
  PAYLOAD_XBEE_SERIAL.begin(115200); //default baud is 9600

  std::thread ground_thread(Hardware::ground_radio_loop);
  std::thread payload_thread(Hardware::payload_radio_loop);

  //load recovery params
  EEPROM.get(Common::BA_ADDR, Hardware::EE_BASE_ALTITUDE);  
  EEPROM.get(Common::PC_ADDR, Hardware::EE_PACKET_COUNT); 
  EEPROM.get(Common::ST_ADDR, States::EE_STATE);   
  
  ground_thread.detach();
  payload_thread.detach();
}

void loop() {
  unsigned long start = millis();
  Hardware::mtx.lock();
  switch (States::EE_STATE)
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
  Hardware::mtx.unlock();
    
  if (Common::TELEMETRY_DELAY > (millis() - start))
    threads.delay(Common::TELEMETRY_DELAY - (millis() - start));
}
