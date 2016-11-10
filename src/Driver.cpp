#include <iostream>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Driver.h"

TurnControl::TurnControl(double ang, long long tick) {
  angle = ang;
  startTick = tick;
  tickSum = 0;
}

bool TurnControl::DoControl(RenderResult* rr, long long currentTick) {

}

Driver::Driver(RoboClaw* rc, uint8_t address) {
  roboclaw = rc;
  addr = address;

  currentSpeed = 0;
  status = DRIVER_STATUS_NORMAL;
  statusTick = 0;
}

Driver::~Driver() {

}

void Driver::DoDriving(RenderResult* rr, int arrivedAt, double angle, long long currentTick) {
  // object check

  // lane check


  if (arrivedAt >= 0)
  {
    printf("Next Angle = %f \n", angle);
    Sleep(3000);

    if (angle < -10)	//Right
    {
      //Turn Right
      roboclaw->BackwardM2(addr, 0); // Left Off
      roboclaw->ForwardM2(addr, 100); // Right On
      Sleep(1000 + -angle * 500 / 15);

      //Recovery
      roboclaw->ForwardM2(addr, 0); // Right Off
      roboclaw->BackwardM2(addr, 100); // Left On
      Sleep(400);

      //Straight
      roboclaw->ForwardM2(addr, 0); // Right Off
      roboclaw->BackwardM2(addr, 0); // Left On
      Sleep(100);

    }
    else if (angle > 10)	//Left
    {
      //Turn Left
      roboclaw->ForwardM2(addr, 0); // Left Off
      roboclaw->BackwardM2(addr, 100); // Right On
      Sleep(1000 + angle * 500 / 15);		// up to angle

      //Recovery
      roboclaw->BackwardM2(addr, 0); // Right Off
      roboclaw->ForwardM2(addr, 100); // Left On
      Sleep(400);

      //Straight
      roboclaw->ForwardM2(addr, 0); // Right Off
      roboclaw->BackwardM2(addr, 0); // Left On
      Sleep(100);
    }
  }
}

void Driver::StartDriving() {
  if (currentSpeed == 0) {
    currentSpeed = 80;
    roboclaw->ForwardM1(addr, currentSpeed); // Go Forward
  }
}

void Driver::StopDriving() {
  if (currentSpeed > 0) {
    currentSpeed = 0;
    roboclaw->ForwardM1(addr, currentSpeed);
  }
}
