//standard includes
#include <iostream>
#include <stack>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "RoboClaw.h"

using namespace std;

struct DriveInfo {
  bool left;
  bool right;
  bool center;
  double lane;
  int arrived;
  double angle;
  int64_t tick;
};

class DriveAction {
public:
  virtual int GetType() = 0;
  virtual bool Drive(DriveInfo info) = 0;
};

class TurnAction {
public:
  TurnAction(double ang);
  virtual int GetType();
  virtual bool Drive(DriveInfo info);
private:
  bool wait;
  bool turn;
  bool recover;
  double angle;
  int64_t startTick;
};

class LaneAction {
public:
  LaneAction();
  virtual int GetType();
  virtual bool Drive(DriveInfo info);
private:
  bool turn;
  bool recover;
  int64_t startTick;
}

class Driver {
private:
  uint8_t addr;
  RoboClaw* roboclaw;
  int currentSpeed;
  stack<DriveAction*> actions;

public:
  Driver(RoboClaw* r, uint8_t address);
  ~Driver();

  Start();
  Drive(DriveInfo info);
  End();
};
