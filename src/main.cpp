//standard includes
#include <iostream>
#include <zed/Camera.hpp>

#include <opencv2/highgui\/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// #include "RoboClaw.h"
// #include "ControlServer.h"

#define DEPTH_CLAMP		2000
#define DIFFERENCE		20

using namespace std;

sl::zed::Camera* zed;
int imageWidth;
int imageHeight;

cv::RNG rng(12345);

// Input from keyboard
char keyboard = ' ';

void LeftImageClicked(int event, int x, int y, int flags, void* userdata) {
	y -= 20;
	if (y < 0) {
		return;
	}

	cv::Mat* image = reinterpret_cast<cv::Mat*>(userdata);
	int depth = static_cast<int>(image->at<uchar>(y, x));
	printf("LeftImage event %d, %d depth : %d\n", y, x, depth);
}

void RightImageClicked(int event, int x, int y, int flags, void* userdata) {
	cv::Mat* image = reinterpret_cast<cv::Mat*>(userdata);
	int depth = static_cast<int>(image->at<uchar>(y, x));
	printf("RightImage event %d, %d depth : %d\n", y, x, depth);
}

int main() {
	// init BT Control server
	//ControlServer server;
	//server.Start();

	zed = new sl::zed::Camera(static_cast<sl::zed::ZEDResolution_mode> (sl::zed::ZEDResolution_mode::HD720));

	sl::zed::InitParams parameters;
	parameters.mode = sl::zed::MODE::PERFORMANCE;
	parameters.unit = sl::zed::UNIT::MILLIMETER;
	parameters.verbose = 1;

	sl::zed::ERRCODE err = zed->init(parameters);
	if (err != sl::zed::ERRCODE::SUCCESS) {
		delete zed;
		zed = NULL;
		return -1;
	}

	zed->setDepthClampValue(DEPTH_CLAMP);

	// Initialize color image and depth
	int width = zed->getImageSize().width;
	int height = zed->getImageSize().height;
	cv::Mat imageLeft(height, width, CV_8UC4, 1);
	cv::Mat imageRight(height, width, CV_8UC4, 1);
	cv::Mat depth(height, width, CV_8UC4, 1);

	cv::namedWindow("ImageLeft", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageRight", cv::WINDOW_AUTOSIZE);
	// cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);

	cv::Size displaySize(720, 404);
	cv::Mat imageLeftDisplay(displaySize, CV_8UC4);
	cv::Mat imageRightDisplay(displaySize, CV_8UC4);
	cv::Mat depthDisplay(displaySize, CV_8UC4);
	cv::Mat imageThreasholded(displaySize, CV_8UC4);

	cv::setMouseCallback("ImageLeft", LeftImageClicked, &depthDisplay);
	cv::setMouseCallback("ImageRight", RightImageClicked, &depthDisplay);

	// Loop until 'q' is pressed
	while (keyboard != 'q') {

		// Grab frame and compute depth in FULL sensing mode
		if (!zed->grab(sl::zed::SENSING_MODE::FILL))
		{
			// Retrieve left color image
			sl::zed::Mat left = zed->retrieveImage(sl::zed::SIDE::LEFT);
			memcpy(imageLeft.data, left.data, width*height * 4 * sizeof(uchar));

			// retrieve right color image
			sl::zed::Mat right = zed->retrieveImage(sl::zed::SIDE::RIGHT);
			memcpy(imageRight.data, right.data, width*height * 4 * sizeof(uchar));

			// Retrieve depth map
			sl::zed::Mat depthmap = zed->normalizeMeasure(sl::zed::MEASURE::DEPTH);
			memcpy(depth.data, depthmap.data, width*height * 4 * sizeof(uchar));
			cv::inRange(depth, cv::Scalar(192, 192, 192, 255), cv::Scalar(255, 255, 255, 255), imageThreasholded);

			// Display left image in OpenCV window
			// cv::resize(imageThreasholded, imageLeftDisplay, displaySize);
			// cv::imshow("ImageLeft", imageLeftDisplay);
			// Display right image in OpenCV window
			cv::resize(depth, imageRightDisplay, displaySize);
			cv::resize(depth, depthDisplay, displaySize);
			
			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierachy;

			cv::Mat gray, canny;
			cv::cvtColor(imageRightDisplay, gray, CV_BGR2GRAY);
			cv::blur(gray, gray, cv::Size(3, 3));
			cv::Canny(gray, canny, 160, 200, 3);
			cv::findContours(gray, contours, hierachy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

			cv::Mat drawing = cv::Mat::zeros(imageRightDisplay.size(), CV_8UC3);
			for (int i = 0; i < contours.size(); i++) {
				std::vector<cv::Point> points = contours.at(i);

				int min = 255, max = 0, avg = 0, count = 0, cX = 0, cY = 0, maxDist = 0;
				for (int j = 0; j < points.size(); j++) {
					int x = points.at(j).x;
					int y = points.at(j).y;

					cX += x;
					cY += y;

					if (x < 0) {
						break;
					}

					int depth = static_cast<int>(depthDisplay.at<uchar>(y, x));
					if (depth == 255) {
						continue;
					}

					if (depth < min) {
						min = depth;
					}

					if (depth > max) {
						max = depth;
					}

					count++;
					avg += depth;
				}

				cX /= points.size();
				cY /= points.size();

				for (int j = 0; j < points.size(); j++) {
					int x = points.at(j).x;
					int y = points.at(j).y;

					int dist = (x - cX) * (x - cX) + (y - cY) * (y - cY);
					if (dist > maxDist) {
						maxDist = dist;
					}
				}

				if (count > 0 && maxDist > 400) {
					avg /= count;

					if (max > 50) {
						
					}
				}

				cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
				cv::drawContours(drawing, contours, i, color, 2, 8, hierachy, 0, cv::Point());
			}

			cv::imshow("ImageLeft", drawing);
			cv::imshow("ImageRight", depthDisplay);
		}

		keyboard = cv::waitKey(30);

	}

	return 0;
}