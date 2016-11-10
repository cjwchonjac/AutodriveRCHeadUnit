#include <stack>
#include "RoboClaw.h"

#define DRIVER_STATUS_NORMAL      0
#define DRIVER_STATUS_TURN_LEFT   1
#define DRIVER_STATUS_TURN_RIGHT  2

using namespace std;

class DriveControl {
  virtual bool DoControl(RenderResult* rr, long long currentTick) = 0;
};

class TurnControl : DrivelControl {
public:
  double angle;
  long long tickSum;
  long long startTick;
  TurnControl(double angle, long long tick);
  virtual bool DoControl(RenderResult* rr, long long currentTick);
}

class LaneControl : DriveControl {
  LaneControl();
  virtual bool DoControl(RenderResult* rr, long long currentTick);
}

class Driver {
private:
  RoboClaw* roboclaw;
	uint8_t addr;

  int status;
	int currentSpeed;

  long long statusTick;
  stack<DriveControl*> ctrls;

public:
  Driver(RoboClaw* roboclaw, uint8_t addr);
  ~Driver();

  void DoDriving(RenderResult* rr, int arrivedAt, double withAngle, long long currentTick);
  void StartDriving();
  void StopDriving();
};
