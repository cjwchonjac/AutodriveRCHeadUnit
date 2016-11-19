// #define TEST_RECORDED_VIDEO
// #define TEST_CAMERA_ONLY
// #define TEST_FROM_VIDEO

#ifndef TEST_RECORDED_VIDEO
#include "Driver.h"
#include "Camera.h"
#include "ControlServer.h"
#include "MQ.h"

#define ROBOCLAW_PORT 		"Com3"
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
	RoboClaw* roboclaw = NULL;
	// Camera
	Camera* c = new Camera();

	roboclaw = new RoboClaw(); std::string port(ROBOCLAW_PORT);
	///////// COM port ///////
	if (!roboclaw->begin(port, ROBOCLOW_BAUDRATE)) {
		cout << "RoboClaw not found" << endl;
		return 0;
	}

	if (!c->Initialize()) {
		cout << "Camera not found" << endl;
		return 0;
		
	}
	
	/*while (1) {
		c->Render();
		cv::waitKey(30);
	}*/

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
				c->StartRecording();
				break;
			case CONTROL_REQUEST_END:
				start = false;
				driver->End();
				c->StopRecording();
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
			RenderResult rr = c->Render();
			
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
			info.laneX = rr.laneDirectionX;
			info.laneY = rr.laneDirectionY;
			info.laneVPE = rr.laneVPE;
			info.laneLeft = rr.laneValidLeft;
			info.laneRight = rr.laneValidRight;
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
		CvCapture* capture = cvCaptureFromFile("road.mp4");
		CvVideoWriter* writer = NULL;

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
						writer = cvCreateVideoWriter("test.avi", 
							CV_FOURCC('D', 'I', 'V', '3'), 
							fps,
							CvSize(image->width, image->height), 
							1);
						init = true;
					}

					double result[2];
					int flags[3];
					// c.CheckLanes(image, result, flags);
					int value = cvWriteFrame(writer, image);
					printf("value: %d\n", value);
					cv::waitKey(30);
					// printf("(%f, %f) %d %d %d\n", result[0], result[1], flags[0], flags[1], flags[2]);
				}
				else {
					break;
				}

				// cv::waitKey(50);
			}
		}

		cvReleaseVideoWriter(&writer);
		return 0;
	}
#endif
