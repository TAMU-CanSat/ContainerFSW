#pragma once
#ifndef __STATES_H__
#define __STATES_H__

namespace States
{
  extern uint16_t EE_STATE;
  
  void Initialization();
  void Standby();
  void Flight();
  void Rapid();
  void Slow();
  void Landing();
}
#endif
