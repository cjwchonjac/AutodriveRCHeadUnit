#include <iostream>
#include <zed/Camera.hpp>

#include <opencv2/highgui\/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define DEPTH_CLAMP		1000
#define DIFFERENCE		20
#define DISPLAY_WIDTH	720
#define DISPLAY_HEIGHT	404

class Camera {
public:
	sl::zed::Camera* zed;
	int imageWidth;
	int imageHeight;

	cv::RNG* rng;

	cv::Mat* imageLeft;
	cv::Mat* imageRight;
	cv::Mat* depth;

	std::vector<std::vector<cv::Point>> *contours;
	std::vector<cv::Vec4i> *hierachy;

	cv::Size *displaySize;
	cv::Mat *imageLeftDisplay;
	cv::Mat *imageRightDisplay;
	cv::Mat *depthDisplay;
	cv::Mat *imageThreasholded;
	cv::Mat *drawing;

	bool Initialize();
	void Render();
	void CheckObjects();
};