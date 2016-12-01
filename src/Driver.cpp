#include "Driver.h"
#include <Windows.h>

#define DRIVE_ACTION_TYPE_TURN	1
#define DRIVE_ACTION_TYPE_LANE	2
#define DRIVE_ACTION_TYPE_AVOID 3

#define TURN_VALUE	110

// 
#define DRIVER_SIMULATION

#define THRESHOLD_TURN_ANGLE	15.0
#define THRESHOLD_LANE_RATIO	0.1

AvoidAction::AvoidAction(bool c, bool l, bool r) {
	center = c;
	left = l;
	right = r;

	turn = false;
	recover = false;
}

int AvoidAction::GetType() {
	return DRIVE_ACTION_TYPE_AVOID;
}

bool AvoidAction::Drive(Driver* driver, RoboClaw* roboclaw, uint8_t addr, DriveInfo info) {
	if (center) {
		if (driver->currentSpeed > 0) {
			printf("object stop\n");
#ifndef DRIVER_SIMULATION
			roboclaw->ForwardM1(addr, 0);
#endif
			driver->currentSpeed = 0;
		}
		
		if (!info.center) {
#ifndef DRIVER_SIMULATION
			roboclaw->ForwardM1(addr, 50);
#endif
			driver->currentSpeed = 50;
			return true;
		}

		return false;
	}
	else {
		if (!turn) {

			startTick = info.tick;
			turn = true;
			// roboclaw->ForwardM1(addr, 50);
			if (left) {
				printf("object turn right\n");
#ifndef DRIVER_SIMULATION
				roboclaw->BackwardM2(addr, 0); // Left Off
				roboclaw->ForwardM2(addr, TURN_VALUE); // Right On
#endif
			}
			else {
				printf("object turn left %f\n");
#ifndef DRIVER_SIMULATION
				roboclaw->ForwardM2(addr, 0); // Right On
				roboclaw->BackwardM2(addr, TURN_VALUE); // Left Off
#endif
			}

			return false;
		}

		int64_t elapsed = info.tick - startTick;
		if (turn && !recover && elapsed > 750) {
			// printf("object recover\n");
			recover = true;
			startTick = info.tick;

			if (left) {
#ifndef DRIVER_SIMULATION
				roboclaw->ForwardM2(addr, 0); // Left On
				roboclaw->BackwardM2(addr, TURN_VALUE); // Right Off
#endif
			}
			else {
#ifndef DRIVER_SIMULATION
				roboclaw->BackwardM2(addr, 0); // Left On
				roboclaw->ForwardM2(addr, TURN_VALUE); // Right Off
#endif
			}

			return false;
		}

		if (turn && recover && elapsed > 400) {
			// roboclaw->ForwardM1(addr, 80);
			// printf("object finishing\n");
#ifndef DRIVER_SIMULATION
			roboclaw->ForwardM2(addr, 0);
			roboclaw->BackwardM2(addr, 0);
#endif
			return true;
		}

		return false;
	}

	

	return true;
}

TurnAction::TurnAction(double ang) {
  angle = ang;
  wait = false;
  turn = false;
  recover = false;
}

int TurnAction::GetType() {
  return DRIVE_ACTION_TYPE_TURN;
}

bool TurnAction::Drive(Driver* driver, RoboClaw* roboclaw, uint8_t addr, DriveInfo info) {
  /*if (!wait) {
	  printf("turn waiting 5 seconds\n");
    startTick = info.tick;
    wait = true;
    return false;
  }*/

  if (!turn) {
	  printf("turn turning\n");
    startTick = info.tick;
    turn = true;

    if (angle < 0) {
#ifndef DRIVER_SIMULATION
      roboclaw->BackwardM2(addr, 0); // Left Off
      roboclaw->ForwardM2(addr, 100); // Right On
#endif
    } else {
#ifndef DRIVER_SIMULATION
      roboclaw->ForwardM2(addr, 0); // Left Off
      roboclaw->BackwardM2(addr, 100); // Right On
#endif
    }

    return false;
  }

  int64_t elapsed = info.tick - startTick;
  if (turn && !recover && elapsed > 500 + abs(angle) * 500 / 20) {
	  // printf("turn recover\n");
    recover = true;
    startTick = info.tick;

    if (angle < 0) {
#ifndef DRIVER_SIMULATION
      roboclaw->ForwardM2(addr, 0); // Right Off
      roboclaw->BackwardM2(addr, 100); // Left On
#endif
	}
	else {
#ifndef DRIVER_SIMULATION
      roboclaw->BackwardM2(addr, 0); // Right Off
      roboclaw->ForwardM2(addr, 100); // Left On
#endif
    }

    return false;
  }

  if (turn && recover && elapsed > 400) {
	  // printf("turn finishing\n");
#ifndef DRIVER_SIMULATION
    roboclaw->ForwardM2(addr, 0);
    roboclaw->BackwardM2(addr, 0);
#endif
    return true;
  }

  return false;
}

LaneAction::LaneAction(double r) {
	ratio = r;
  turn = false;
  recover = false;
}

int LaneAction::GetType() {
  return DRIVE_ACTION_TYPE_LANE;
}

bool LaneAction::Drive(Driver* driver, RoboClaw* roboclaw, uint8_t addr, DriveInfo info) {
  if (!turn) {
	  
    startTick = info.tick;
    turn = true;
	// roboclaw->ForwardM1(addr, 50);
    if (ratio > 0.0) {
		printf("lane turn right %f\n", ratio);
#ifndef DRIVER_SIMULATION
      roboclaw->BackwardM2(addr, 0); // Left Off
	  roboclaw->ForwardM2(addr, TURN_VALUE); // Right On
#endif
    } else {
		printf("lane turn left %f\n", ratio);
#ifndef DRIVER_SIMULATION
      roboclaw->ForwardM2(addr, 0); // Right On
	  roboclaw->BackwardM2(addr, TURN_VALUE); // Left Off
#endif
    }

    return false;
  }

  int64_t elapsed = info.tick - startTick;
  if (turn && !recover && elapsed > 500) {
	  // printf("lane recover\n");
    recover = true;
    startTick = info.tick;

	if (ratio > 0.0) {
#ifndef DRIVER_SIMULATION
      roboclaw->ForwardM2(addr, 0); // Left On
	  roboclaw->BackwardM2(addr, TURN_VALUE); // Right Off
#endif
    } else {
#ifndef DRIVER_SIMULATION
      roboclaw->BackwardM2(addr, 0); // Left On
	  roboclaw->ForwardM2(addr, TURN_VALUE); // Right Off
#endif
    }

    return false;
  }

  if (turn && recover && elapsed > 400) {
	  // roboclaw->ForwardM1(addr, 80);
	  // printf("lane finishing\n");
#ifndef DRIVER_SIMULATION
    roboclaw->ForwardM2(addr, 0);
    roboclaw->BackwardM2(addr, 0);
#endif
    return true;
  }

  return false;
}

Driver::Driver(RoboClaw* r, uint8_t address) {
  roboclaw = r;
  addr = address;
  currentSpeed = 0;
  
  controlM1 = 0;
  controlM2 = 0;

  angle = 0.0;
  seg = -1;
  lastLaneX = 0;
}

Driver::~Driver() {

}

void Driver::Start() {
  if (currentSpeed == 0) {
	  printf("Drive Start\n");
    currentSpeed = 50;
#ifndef DRIVER_SIMULATION
    roboclaw->ForwardM1(addr, currentSpeed);
#endif
  }
}

void Driver::Drive(DriveInfo info) {
	double laneDelta = info.laneX - lastLaneX;
	// printf("-- DriveInfo X: %.4f, Y: %.4f, %d %d %d laneVPE: %d, LeftLane: %d, RightLane: %d, %.4f --\n",
	// info.laneX, info.laneY, info.center, info.left, info.right, info.laneVPE, info.laneLeft, info.laneRight, laneDelta);

	
	 if (info.arrived >= 0) {
		 seg = info.arrived;
		 angle = info.angle;
		 arrivedTick = info.tick;
	 }

	 if (seg >= 0 && info.tick - arrivedTick > 10000) {
		 seg = -1;
		 angle = 0.0;
	 }

  if (abs(info.laneX) > THRESHOLD_LANE_RATIO) {
    bool insert = true;
    if (!actions.empty()) {
		DriveAction* action = actions.top();
		int lane = action->GetType();
		insert = action->GetType() != DRIVE_ACTION_TYPE_LANE && action->GetType() != DRIVE_ACTION_TYPE_TURN && action->GetType() != DRIVE_ACTION_TYPE_AVOID;
    }

    if (insert) {
		actions.push((DriveAction*) new LaneAction(info.laneX));
    }
  }

  if (laneDelta > 0.05 || laneDelta < -0.01) {
	  bool insert = true;
	  if (!actions.empty()) {
		  DriveAction* action = actions.top();
		  int lane = action->GetType();
		  insert = action->GetType() != DRIVE_ACTION_TYPE_LANE && action->GetType() != DRIVE_ACTION_TYPE_TURN && action->GetType() != DRIVE_ACTION_TYPE_AVOID;
	  }

	  if (insert) {
		  actions.push((DriveAction*) new LaneAction(laneDelta));
	  }
  }

  lastLaneX = info.laneX;

  if (seg >= 0 && abs(angle) > THRESHOLD_TURN_ANGLE) {
	  printf("new turn %f\n", info.angle);
	  if ((angle < 0 && !info.laneRight) || (angle > 0 && !info.laneLeft)) {
		  actions.push((DriveAction*) new TurnAction(angle));
		  seg = -1;
		  angle = 0.0;
	  }
  }

  // Avoid Object
  if (info.center || info.left || info.right) {
	  bool insert = true;
	  if (!actions.empty()) {
		  DriveAction* action = actions.top();
		  insert = action->GetType() != DRIVE_ACTION_TYPE_AVOID;
	  }
	  
	  if (insert) {
		  actions.push((DriveAction*) new AvoidAction(info.center, info.left, info.right));
	  }
  }

  if (!actions.empty()) {
    DriveAction* action = actions.top();
    if (action->Drive(this, roboclaw, addr, info)) {
      actions.pop();
      delete action;
    }
  }
}

void Driver::End() {
  if (currentSpeed > 0) {
	  printf("Drive End\n");
    currentSpeed = 0;
#ifndef DRIVER_SIMULATION
    roboclaw->ForwardM1(addr, currentSpeed);
#endif
  }
}

void Driver::Go() {
	controlM1 += 40;
	printf("Go controlM1: %d\n", controlM1);
#ifndef DRIVER_SIMULATION
	if (controlM1 >= 0) {
		roboclaw->ForwardM1(addr, controlM1);
	}
	else {
		roboclaw->BackwardM1(addr, -controlM1);
	}
#endif
}

void Driver::Stop() {
	controlM1 = 0;
	controlM2 = 0;
	printf("Stop\n");
#ifndef DRIVER_SIMULATION
	roboclaw->ForwardM1(addr, controlM1);
	roboclaw->ForwardM2(addr, controlM2);
	roboclaw->BackwardM2(addr, controlM2);
#endif
}

void Driver::Back() {
	controlM1 -= 40;
	printf("Back controlM1: %d\n", controlM1);
#ifndef DRIVER_SIMULATION
	if (controlM1 >= 0) {
		roboclaw->ForwardM1(addr, controlM1);
	}
	else {
		roboclaw->BackwardM1(addr, -controlM1);
	}
#endif
}

void Driver::Left() {
	controlM2 -= 90;
	printf("Left controlM2: %d\n", controlM2);
#ifndef DRIVER_SIMULATION
	roboclaw->ForwardM2(addr, 0);
	roboclaw->BackwardM2(addr, TURN_VALUE);

	Sleep(1000);
	roboclaw->BackwardM2(addr, 0);
	roboclaw->ForwardM2(addr, TURN_VALUE);

	Sleep(400);
	roboclaw->ForwardM2(addr, 0);
	roboclaw->BackwardM2(addr, 0);
#endif
}

void Driver::Right() {
	controlM2 += 90;
	printf("Right controlM2: %d\n", controlM2);
#ifndef DRIVER_SIMULATION
	roboclaw->BackwardM2(addr, 0);
	roboclaw->ForwardM2(addr, TURN_VALUE);

	Sleep(1000);
	roboclaw->ForwardM2(addr, 0);
	roboclaw->BackwardM2(addr, TURN_VALUE);

	Sleep(400);
	roboclaw->ForwardM2(addr, 0);
	roboclaw->BackwardM2(addr, 0);
#endif
}
