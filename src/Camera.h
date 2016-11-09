#include <iostream>
#include <zed/Camera.hpp>

#include <stdio.h>
#include "utils.h"
// #include "opencv2/imgproc/imgproc.hpp"
// #include "opencv2/highgui/highgui.hpp"
// #include <opencv2\opencv.hpp>
#include <math.h>

// #include <opencv2/highgui\/highgui.hpp>
// #include <opencv2/imgproc/imgproc.hpp>

#define DEPTH_CLAMP		2000
#define DIFFERENCE		20
#define DISPLAY_WIDTH	720
#define DISPLAY_HEIGHT	404

struct Lane {
	Lane(){}
	Lane(CvPoint a, CvPoint b, float angle, float kl, float bl) : p0(a), p1(b), angle(angle),
		votes(0), visited(false), found(false), k(kl), b(bl) { }

	CvPoint p0, p1;
	int votes;
	bool visited, found;
	float angle, k, b;
};

struct Status {

	Status() :reset(true), lost(0){}
	ExpMovingAverage k, b;
	bool reset;
	int lost;
};


struct RenderResult {
	bool center;
	bool left;
	bool right;
	double laneDirection;
	double leftObjectVelocity;
	double rightObjectVelocity;
};

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
	
	CvVideoWriter* writer;

	SYSTEMTIME lastLeftObjectTime;
	int lastLeftSideRightmost;

	SYSTEMTIME lastRightObjectTime;
	int lastRightSideLeftmost;

	Status laneR, laneL;
	int center = -1;

	CvMemStorage* houghStorage;

	CvSize frameSize;
	IplImage * tempFrame;
	IplImage *grey;
	IplImage *edges;
	IplImage *halfFrame;

	bool Initialize();
	RenderResult Render();
	RenderResult CheckObjects();
	double CheckLanes(IplImage* image);

	bool processSide(std::vector<Lane> lanes, IplImage *edges, bool right);
	int processLanes(CvSeq* lines, IplImage* edges, IplImage* temp_frame);
};