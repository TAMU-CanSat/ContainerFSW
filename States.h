#pragma once
#ifndef __STATES_H__
#define __STATES_H__

namespace States
{
  static uint16_t EE_STATE = 0;
  
  void Initialization();
  void Standby();
  void Flight();
  void Rapid();
  void Slow();
  void Landing();
}
#endif
