#include "Driver.h"

#define DRIVE_ACTION_TYPE_TURN 1
#define DRIVE_ACTION_TYPE_LANE 2

// 
//#define DRIVER_SIMULATION

#define THRESHOLD_TURN_ANGLE	15.0
#define THRESHOLD_LANE_RATIO	0.10

TurnAction::TurnAction(double ang) {
  angle = ang;
  wait = false;
  turn = false;
  recover = false;
}

int TurnAction::GetType() {
  return DRIVE_ACTION_TYPE_TURN;
}

bool TurnAction::Drive(RoboClaw* roboclaw, uint8_t addr, DriveInfo info) {
  if (!wait) {
	  printf("turn waiting 3 seconds\n");
    startTick = info.tick;
    wait = true;
    return false;
  }

  int64_t elapsed = info.tick - startTick;
  if (wait && !turn && elapsed > 3000) {
	  printf("turn turn waiting\n");
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

  if (wait && turn && !recover && elapsed > 500 + abs(angle) * 500 / 20) {
	  printf("turn recover waiting\n");
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
	  printf("turn finishing\n");
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

bool LaneAction::Drive(RoboClaw* roboclaw, uint8_t addr, DriveInfo info) {
  if (!turn) {
	  
    startTick = info.tick;
    turn = true;

    if (ratio > 0.0) {
		printf("lane turn right %f\n", ratio);
#ifndef DRIVER_SIMULATION
      roboclaw->BackwardM2(addr, 0); // Left Off
      roboclaw->ForwardM2(addr, 90); // Right On
#endif
    } else {
		printf("lane turn left %f\n", ratio);
#ifndef DRIVER_SIMULATION
      roboclaw->ForwardM2(addr, 0); // Right On
      roboclaw->BackwardM2(addr, 90); // Left Off
#endif
    }

    return false;
  }

  int64_t elapsed = info.tick - startTick;
  if (turn && !recover && elapsed > 1000) {
	  printf("lane recover\n");
    recover = true;
    startTick = info.tick;

	if (ratio > 0.0) {
#ifndef DRIVER_SIMULATION
      roboclaw->ForwardM2(addr, 0); // Left On
      roboclaw->BackwardM2(addr, 90); // Right Off
#endif
    } else {
#ifndef DRIVER_SIMULATION
      roboclaw->BackwardM2(addr, 0); // Left On
      roboclaw->ForwardM2(addr, 90); // Right Off
#endif
    }

    return false;
  }

  if (turn && recover && elapsed > 400) {
	  printf("lane finishing\n");
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
  if (abs(info.lane) > THRESHOLD_LANE_RATIO) {
    bool insert = true;
    if (!actions.empty()) {
		DriveAction* action = actions.top();
		int lane = action->GetType();
		insert = action->GetType() != DRIVE_ACTION_TYPE_LANE;
    }

    if (insert) {
      actions.push((DriveAction*) new LaneAction(info.lane));
    }
  }

  if (info.arrived >= 0 && abs(info.angle) > THRESHOLD_TURN_ANGLE) {
	  printf("new turn %f\n", info.angle);
	  actions.push((DriveAction*) new TurnAction(info.angle));
  }

  if (!actions.empty()) {
    DriveAction* action = actions.top();
    if (action->Drive(roboclaw, addr, info)) {
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
	if (controlM2 >= 0) {
		roboclaw->BackwardM2(addr, 0);
		roboclaw->ForwardM2(addr, controlM2);
	}
	else {
		roboclaw->ForwardM2(addr, controlM2);
		roboclaw->BackwardM2(addr, -controlM2);
	}
#endif
}

void Driver::Right() {
	controlM2 += 90;
	printf("Right controlM2: %d\n", controlM2);
#ifndef DRIVER_SIMULATION
	if (controlM2 >= 0) {
		roboclaw->BackwardM2(addr, 0);
		roboclaw->ForwardM2(addr, controlM2);
	}
	else {
		roboclaw->ForwardM2(addr, controlM2);
		roboclaw->BackwardM2(addr, -controlM2);
	}
#endif
}
