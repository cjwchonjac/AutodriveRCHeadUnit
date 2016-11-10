//standard includes
#include <iostream>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "RoboClaw.h"
#include "Camera.h"
#include "ControlServer.h"
#include "MQ.h"
#include "Driver.h"

using namespace std;
using namespace com::autodrive::message;

#define TEST_DRIVE

long long currentMillis() {
    static LARGE_INTEGER s_frequency;
    static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    } else {
        return GetTickCount();
    }
}

#ifdef TEST_DRIVE
void doTEST(RoboClaw* roboclaw) {
	// test

	while (1) {
		;
	}
}
#endif

//////////////////
int main() {
	// init message queue
	ADMQ* mq = new ADMQ();

	// init smart device connection
	ControlServer* server = new ControlServer(mq);
	server->Start();

	// RC Control
	RoboClaw* roboclaw = new RoboClaw();
	std::string port("Com3");	///////// COM port ///////

	if (!roboclaw->begin(port, 115200)) {
		cout << "RoboClaw not found" << endl;
		return;
	}

#ifdef TEST_DRIVE
	doTEST(roboclaw);
#endif

	Camera* c = new Camera();
	if (!c->Initialize()) {
		cout << "Camera not found" << endl;
		return;
	}

	Driver* driver = new Driver(roboclaw, 0x80);

	while (1) {
		ADRequest rq;
		SegmentArrived* arrived = NULL;

		do {
			rq = mq->poll();

			switch (rq.type) {
			case CONTROL_REQUEST_START:
				start = true;
				driver.StartDriving();
				break;
			case CONTROL_REQUEST_END:
				start = false;
				break;
			case CONTROL_REQUEST_SEGMENT_ARRIVED:
				arrived = reinterpret_cast<SegmentArrived*>(rq.data);
				break;
			case CONTROL_REQUEST_DESTINATION_ARRIVED:
				start = false;
				break;
			}
		} while (rq.type > 0);

		if (start) {
			RenderResult rr = c->Render();
			printf("left: %d, right: %d, center: %d, laneDirection: %f\n",
				rr.left, rr.right, rr.center, rr.laneDirection);

			int index = -1;
			double angle 0.0;
			if (arrived != NULL) {
				index = arrived->index();
				angle = arrived->angle();
			}

			driver.DoDriving(&rr, index, angle, currentMillis());
		}
		else {
			driver.StopDriving();
		}

		if (arrived != NULL) {
			delete arrived;
			arrived = NULL;
		}

		cv::waitKey(50);
	}

	delete mq;
	delete roboclaw;
	delete driver;
	delete camera;
	delete server;

	return 0;
}
