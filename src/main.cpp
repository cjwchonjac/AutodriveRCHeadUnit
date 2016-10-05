//standard includes
#include <iostream>

#include "RoboClaw.h"
#include "Camera.h"
#include "ControlServer.h"

using namespace std;

int main() {
	// init BT Control server
	ControlServer server;
	server.Start();

	// RC Control TEST
	RoboClaw* roboclaw = new RoboClaw();
	std::string port("Com4");
	if (roboclaw->begin(port, 115200)) {
		std::cout << "RoboClaw connected through " << port << " with baudrate 115200" << std::endl;
		// write test
		uint8_t addr = 0x80;
		Sleep(2000);
		roboclaw->ForwardM1(addr, 16);
		Sleep(1000);
		roboclaw->ForwardM1(addr, 0);
		Sleep(1000);
		roboclaw->BackwardM1(addr, 16);
		Sleep(1000);
		roboclaw->BackwardM1(addr, 0);
		// roboclaw.BackwardM2(addr, 64);
	}
	else {
		std::cout << "RoboClaw not found" << std::endl;
	}

	Camera* camera = new Camera();

	/*cout << errcode2str(err) << endl;

	if (err != sl::zed::ERRCODE::SUCCESS) {
		delete zed;
		printf("ZED Initialization failed\n");
		return -1;
	}

	int width = zed->getImageSize().width;
	int height = zed->getImageSize().height;
	cv::Size size(width, height);
	cv::Mat depthDisplay(size, CV_8UC1);

	int depth_clamp = 5000;
	zed->setDepthClampValue(depth_clamp);*/

	camera->DoWork();

	delete roboclaw;
	delete camera;

	return 0;
}