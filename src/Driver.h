//standard includes
#include <iostream>
#include <stack>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "RoboClaw.h"

using namespace std;

class Driver;

struct DriveInfo {
  bool left;
  bool right;
  bool center;
  double laneX;
  double laneY;
  int laneVPE;
  int laneLeft;
  int laneRight;
  int arrived;
  double angle;
  int64_t tick;
};

class DriveAction {
public:
  virtual int GetType() = 0;
  virtual bool Drive(Driver* driver, RoboClaw* roboclaw, uint8_t addr, DriveInfo info) = 0;
};

class TurnAction {
public:
  TurnAction(double ang);
  virtual int GetType();
  virtual bool Drive(Driver* driver, RoboClaw* roboclaw, uint8_t addr, DriveInfo info);
private:
  bool wait;
  bool turn;
  bool recover;
  double angle;
  int64_t startTick;
};

class LaneAction {
public:
	double ratio;
	LaneAction(double ratio);
  virtual int GetType();
  virtual bool Drive(Driver* driver, RoboClaw* roboclaw, uint8_t addr, DriveInfo info);
private:
	
  bool turn;
  bool recover;
  int64_t startTick;
};

class AvoidAction {
public:
	AvoidAction(bool center, bool left, bool right);
	virtual int GetType();
	virtual bool Drive(Driver* driver, RoboClaw* roboclaw, uint8_t addr, DriveInfo info);
private:
	bool center;
	bool left;
	bool right;

	bool turn;
	bool recover;
	int64_t startTick;
};

class Driver {
private:
  uint8_t addr;
  RoboClaw* roboclaw;

  stack<DriveAction*> actions;

  int controlM1;
  int controlM2;

  int seg;
  double angle;
  int64_t arrivedTick;
  double lastLaneX;

public:
	int currentSpeed;

  Driver(RoboClaw* r, uint8_t address);
  ~Driver();

  void Start();
  void Drive(DriveInfo info);
  void End();

  void Go();
  void Stop();
  void Left();
  void Right();
  void Back();


};
