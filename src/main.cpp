
// #define TEST_RECORDED_VIDEO

#define TEST_CAMERA_ONLY

#ifndef TEST_RECORDED_VIDEO
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

	// RC Control
	RoboClaw* roboclaw = NULL;// new RoboClaw();

	// RoboClaw Connection
	/*std::string port(ROBOCLAW_PORT);	///////// COM port ///////
	if (!roboclaw->begin(port, ROBOCLOW_BAUDRATE)) {
		cout << "RoboClaw not found" << endl;
		return 0;
	}*/

	// Camera
	Camera* c = new Camera();
	/*if (!c->Initialize()) {
		cout << "Camera not found" << endl;
		return 0;
	}*/

	Driver* driver = new Driver(roboclaw, 0x80);
	cout << "RoboClaw connected through " << ROBOCLAW_PORT << " with baudrate 115200" << endl;

	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

#ifdef TEST_CAMERA_ONLY
	Sleep(3000);
	bool start = true;
	driver->Start();
#else
	bool start = false;
#endif

	bool init = false;
	CvCapture* capture = cvCaptureFromFile("road.mp4");
	if (capture) {
		double fps = cvGetCaptureProperty(
			capture,
			CV_CAP_PROP_FPS
			);
	}

	while (1) {
		ADRequest rq;
		SegmentArrived* arrived = NULL;
		IplImage* image = cvQueryFrame(capture);

		do {
			rq = mq->poll();

			switch (rq.type) {
			case CONTROL_REQUEST_START:
				start = true;
				driver->Start();
				break;
			case CONTROL_REQUEST_END:
				start = false;
				driver->End();
				break;
			case CONTROL_REQUEST_SEGMENT_ARRIVED:
				arrived = reinterpret_cast<SegmentArrived*>(rq.data);
				break;
			case CONTROL_REQUEST_DESTINATION_ARRIVED:
				start = false;
				driver->End();
				break;
			case CONTROL_REQUEST_GO:
				start = false;
				driver->Go();
				break;
			case CONTROL_REQUEST_STOP:
				driver->Stop();
				break;
			case CONTROL_REQUEST_BACK:
				driver->Back();
				break;
			case CONTROL_REQUEST_LEFT:
				driver->Left();
				break;
			case CONTROL_REQUEST_RIGHT:
				driver->Right();
				break;
			}
		} while (rq.type > 0);

		if (start) {
			RenderResult rr = { 0 }; //c->Render();
			if (image) {
				if (!init) {
					c->InitLaneOnly(image->width, image->height, image->nChannels);
				}

				double value = c->CheckLanes(image);
				rr.laneDirection = value;
				// printf("lane value: %f\n", value);
			}

			int index = -1;
			double angle = 0.0;

			if (arrived != NULL) {
				index = arrived->index();
				angle = arrived->angle();
			}

			QueryPerformanceCounter(&EndingTime);
			ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
			ElapsedMicroseconds.QuadPart *= 1000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

			DriveInfo info;
			info.left = rr.left;
			info.right = rr.right;
			info.center = rr.center;
			info.lane = rr.laneDirection;
			info.arrived = index;
			info.angle = angle;
			info.tick = ElapsedMicroseconds.QuadPart;

			driver->Drive(info);
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
	delete c;
	return 0;
}

#else
	#include "Camera.h"

	int main(void) {
		Camera c;
		bool init = false;
		CvCapture* capture = cvCaptureFromFile("output.avi");

		if (capture) {
			double fps = cvGetCaptureProperty(
			            capture,
			            CV_CAP_PROP_FPS
			            );
			while (1) {
				IplImage* image = cvQueryFrame(capture);

				if (image) {
					if (!init) {
						c.InitLaneOnly(image->width, image->height, image->nChannels);
					}

					double value = c.CheckLanes(image);
					printf("lane value: %f\n", value);
				}
			}
		}

		return 0;
	}
#endif
