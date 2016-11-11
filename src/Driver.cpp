#include "Driver.h"

#define DRIVE_ACTION_TYPE_TURN 1
#define DRIVE_ACTION_TYPE_LANE 2

TurnAction::TurnAction(double ang) {
  angle = ang;
  wait = false;
  turn = false;
  recover = false;
}

int TurnAction::GetType() {
  return DRIVE_ACTION_TYPE_TURN;
}

bool TurnAction::Drive(DriveInfo info) {
  if (!wait) {
    startTick = info.tick;
    wait = true;
    return false;
  }

  int64_t elapsed = info.tick - startTick;
  if (wait && !turn && elapsed > 3000) {
    startTick = info.tick;
    turn = true;

    if (angle < 0) {
      roboclaw->BackwardM2(addr, 0); // Left Off
      roboclaw->ForwardM2(addr, 100); // Right On
    } else {
      roboclaw->ForwardM2(addr, 0); // Left Off
      roboclaw->BackwardM2(addr, 100); // Right On
    }

    return false;
  }

  if (wait && turn && !recover && elapsed > 1000 + abs(angle) * 500 / 15) {
    recover = true;
    startTick = info.tick;

    if (angle < 0) {
      roboclaw->ForwardM2(addr, 0); // Right Off
      roboclaw->BackwardM2(addr, 100); // Left On
    } else {
      roboclaw->BackwardM2(addr, 0); // Right Off
      roboclaw->ForwardM2(addr, 100); // Left On
    }

    return false;
  }

  if (turn && recover) {
    roboclaw->ForwardM2(addr, 0);
    roboclaw->BackwardM2(addr, 0);
    return true;
  }

  return false;
}

LaneAction::LaneAction() {
  turn = false;
  recover = false;
}

int LaneAction::GetType() {
  return DRIVE_ACTION_TYPE_LANE;
}

bool LaneAction::Drive(DriveInfo info) {
  if (!turn) {
    startTick = info.tick;
    turn = true;

    if (info.lane > 0.0) {
      roboclaw->BackwardM2(addr, 0); // Left Off
      roboclaw->ForwardM2(addr, 20); // Right On
    } else {
      roboclaw->ForwardM2(addr, 0); // Right On
      roboclaw->BackwardM2(addr, 20); // Left Off
    }

    return false;
  }

  int64_t elapsed = info.tick - startTick;
  if (turn && !recover && elapsed > 1000) {
    recover = true;
    startTick = info.tick;

    if (info.lane > 0.0) {
      roboclaw->ForwardM2(addr, 0); // Left On
      roboclaw->BackwardM2(addr, 20); // Right Off
    } else {
      roboclaw->BackwardM2(addr, 0); // Left On
      roboclaw->ForwardM2(addr, 20); // Right Off
    }

    return false;
  }

  if (turn && recover) {
    roboclaw->ForwardM2(addr, 0);
    roboclaw->BackwardM2(addr, 0);
    return true;
  }

  return false;
}

Driver::Driver(RoboClaw* r, uint8_t address) {
  roboclaw = r;
  addr = address;
  currentSpeed = 0;
}

Driver::~Driver() {

}

Driver::Start() {
  if (currentSpeed == 0) {
    currentSpeed = 80;
    roboclaw->ForwardM1(addr, currentSpeed);
  }
}

Driver::Drive(DriveInfo info) {
  if (abs(info.lane) > 20.0) {
    bool insert = true;
    if (!actions.empty()) {
      DriveAction* aciton = actions.back();
      insert = action->GetType() != DRIVE_ACTION_TYPE_LANE;
    }

    if (insert) {
      actions.push_back(new LaneAction());
    }
  }

  if (arrived >= 0) {
    actions->push_back(new TurnAction(angle));
  }

  if (!actions.empty()) {
    DriveAction* action = actions.back();
    if (action->Drive(info)) {
      action = actions.pop_back();
      delete action;
    }
  }
}

Drive::End() {
  if (currentSpeed > 0) {
    currentSpeed = 0;
    roboclaw->ForwardM1(addr, currentSpeed);
  }
}
