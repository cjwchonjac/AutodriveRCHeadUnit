#include "Driver.h"
#include "Camera.h"
#include "ControlServer.h"
#include "MQ.h"

#define ROBOCLAW_PORT 			"Com3"
#define ROBOCLOW_BAUDRATE 	115200

using namespace std;
using namespace com::autodrive::message;

int main() {
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;

	ADMQ* mq = new ADMQ();

	// Connection to start device
	ControlServer* server = new ControlServer(mq);
	server->Start();

	// Camera
	Camera* c = new Camera();
	if (!c.Initialize()) {
		cout << "Camera not found" << endl;
		return 0;
	}

	// RC Control
	RoboClaw* roboclaw = new RoboClaw();

	// RoboClaw Connection
	std::string port(ROBOCLAW_PORT);	///////// COM port ///////
	if (!roboclaw->begin(port, ROBOCLOW_BAUDRATE)) {
		cout << "RoboClaw not found" << endl;
		return 0;
	}

	Driver* driver = new Driver(roboclaw, 0x80);
	cout << "RoboClaw connected through " << ROBOCLAW_PORT << " with baudrate 115200" << endl;

	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	bool start = false;
	while (1) {
		ADRequest rq;
		SegmentArrived* arrived = NULL;

		do {
			rq = mq->poll();

			switch (rq.type) {
			case CONTROL_REQUEST_START:
				start = true;
				driver->Start();
				break;
			case CONTROL_REQUEST_END:
				start = false;
				driver->Stop();
				break;
			case CONTROL_REQUEST_SEGMENT_ARRIVED:
				arrived = reinterpret_cast<SegmentArrived*>(rq.data);
				break;
			case CONTROL_REQUEST_DESTINATION_ARRIVED:
				start = false;
				driver->Stop();
				break;
			}
		} while (rq.type > 0);

		if (start) {
			RenderResult rr = c.Render();
			printf("left: %d, right: %d, center: %d, laneDirection: %f\n",
				rr.left, rr.right, rr.center, rr.laneDirection);

			int index = -1;
			double angle = 0.0;

			if (arrived != NULL) {
				index = arrived->index();
				angle = arrived->angle();
			}

			QueryPerformanceCounter(&EndingTime);
			ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

			DriveInfo info;
			info.left = left;
			info.right = right;
			info.center = center;
			info.lane = laneDirection;
			info.arrived = index;
			info.angle = angle;
			info.tick = ElapsedMicroseconds.QuadPart;

			driver->Drive(DriveInfo);
		}

		if (arrived != NULL) {
			delete arrived;
			arrived = NULL;
		}

		cv::waitKey(30);
	}

	delete mq;
	delete server;
	delete roboclaw;
	delete camera;

	return 0;
}
