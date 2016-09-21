//standard includes
#include <iostream>
//ZED includes
#include <zed/Camera.hpp>

#include "ControlServer.h"

using namespace std;

int main() {
	ControlServer server;
	server.Start();

	sl::zed::Camera * zed = 
		new sl::zed::Camera(static_cast<sl::zed::ZEDResolution_mode> (sl::zed::ZEDResolution_mode::HD1080));

	sl::zed::InitParams parameters;
	parameters.mode = sl::zed::MODE::PERFORMANCE;
	parameters.unit = sl::zed::UNIT::MILLIMETER;
	parameters.verbose = 1;
	
	sl::zed::ERRCODE err = zed->init(parameters);

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