//standard includes
#include <iostream>
//ZED includes
#include <zed/Camera.hpp>
#include "RoboClaw.h"

#include "ControlServer.h"

using namespace std;

int main() {
	// init BT Control server
	ControlServer server;
	server.Start();

	// RC Control TEST
	RoboClaw roboclaw;

	if (roboclaw.begin("com3", 115200)) {
		std::cout << "RoboClaw connected through com3 with baudrate 115200" << std::endl;
		// write test
		uint8_t addr = 0x80;
		roboclaw.ForwardM1(addr, 64);
		roboclaw.BackwardM2(addr, 64);
	}
	else {
		std::cout << "RoboClaw not found" << std::endl;
	}

	// init ZED Camera
	/*sl::zed::Camera * zed = 
		new sl::zed::Camera(static_cast<sl::zed::ZEDResolution_mode> (sl::zed::ZEDResolution_mode::HD1080));

	sl::zed::InitParams parameters;
	parameters.mode = sl::zed::MODE::PERFORMANCE;
	parameters.unit = sl::zed::UNIT::MILLIMETER;
	parameters.verbose = 1;
	
	sl::zed::ERRCODE err = zed->init(parameters);*/

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

	while (1) {
		
	}
	return 0;
}