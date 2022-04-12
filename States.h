#pragma once
#ifndef __STATES_H__
#define __STATES_H__

#include <Arduino.h>
namespace States
{
  static int state = 0;
  
  void Initialization();
  void Standby();
  void Flight();
  void Rapid();
  void Slow();
  void Landing();
}
#endif
