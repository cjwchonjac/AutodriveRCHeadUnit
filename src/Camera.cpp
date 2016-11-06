#include "Camera.h"

void Camera::CheckObjects() {
	cv::Mat* image = imageLeftDisplay;
	cv::imshow("ImageRight", *drawing);

	std::vector<std::vector<cv::Point>> newContours;

	for (int idx = 0; idx < (*contours).size(); idx++) {
		double leftSideRightmost = -1.0;
		double rightSideLeftmost = -1.0;
		double leftSideX = 0.0;
		double rightSideX = 0.0;
		int leftSideCnt = 0;
		int rightSideCnt = 0;

		std::vector<cv::Point> filtered;
		// printf("%d contour\n", idx + 1);
		for (int idx2 = 0; idx2 < (*contours).at(idx).size(); idx2++) {
			std::vector<cv::Point> points = (*contours).at(idx);
			// printf("%d, %d\n", ctx->contours->at(idx).at(idx2).x, ctx->contours->at(idx).at(idx2).y);

			int x = (*contours).at(idx).at(idx2).x;
			int y = (*contours).at(idx).at(idx2).y;

			bool left = false;
			bool right = false;
			bool center = false;

			if (y < DISPLAY_HEIGHT * 2 / 3) {
				filtered.push_back(cv::Point(x, y));

				if (x > DISPLAY_WIDTH * 4 / 5) {
					if (rightSideLeftmost < 0 || x < rightSideLeftmost) {
						rightSideLeftmost = x;
					}
				}
				else if (x < DISPLAY_WIDTH * 1 / 5) {
					if (leftSideRightmost < 0 || x > leftSideRightmost) {
						leftSideRightmost = x;
					}
				}
				else {
					printf("Object in center position \n");
				}
			}
		}

		newContours.push_back(filtered);

		// printf("%f %f\n", leftSideRightmost, rightSideLeftmost);
	}

	image->setTo(cv::Scalar(0, 0, 0));
	for (int idx = 0; idx < newContours.size(); idx++) {
		cv::Scalar color = cv::Scalar((*rng).uniform(0, 255), (*rng).uniform(0, 255), (*rng).uniform(0, 255));
		cv::drawContours(*image, newContours, idx, color, 2, 8, *hierachy, 0, cv::Point());
	}


	cv::imshow("Depth", *image);
}

void LeftImageClicked(int event, int x, int y, int flags, void* userdata) {
	Camera* camera = reinterpret_cast<Camera*>(userdata);
	if (event == CV_EVENT_LBUTTONUP) {
		camera->CheckObjects();
	}

}

void RightImageClicked(int event, int x, int y, int flags, void* userdata) {
	//CContext* ctx = reinterpret_cast<CContext*>(userdata);
	//cv::Mat* image = ctx->imageRightDisplay;
	//int depth = static_cast<int>(image->at<uchar>(y, x));
	//printf("RightImage event %d, %d depth : %d\n", y, x, depth);
}

bool Camera::Initialize() {
	zed = new sl::zed::Camera(static_cast<sl::zed::ZEDResolution_mode> (sl::zed::ZEDResolution_mode::HD720));

	sl::zed::InitParams parameters;
	parameters.mode = sl::zed::MODE::PERFORMANCE;
	parameters.unit = sl::zed::UNIT::MILLIMETER;
	parameters.verbose = 1;

	sl::zed::ERRCODE err = zed->init(parameters);
	if (err != sl::zed::ERRCODE::SUCCESS) {
		delete zed;
		zed = NULL;
		return false;
	}

	zed->setDepthClampValue(DEPTH_CLAMP);

	// Initialize color image and depth
	imageWidth = zed->getImageSize().width;
	imageHeight = zed->getImageSize().height;
	imageLeft = new cv::Mat(imageHeight, imageWidth, CV_8UC4, 1);
	imageRight = new cv::Mat(imageHeight, imageWidth, CV_8UC4, 1);
	depth = new cv::Mat(imageHeight, imageWidth, CV_8UC4, 1);

	cv::namedWindow("ImageLeft", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageRight", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);

	cv::setMouseCallback("ImageLeft", LeftImageClicked, this);
	cv::setMouseCallback("ImageRight", RightImageClicked, this);

	int imageWidth;
	int imageHeight;

	rng = new cv::RNG(12345);

	contours = new std::vector<std::vector<cv::Point>>();
	hierachy = new std::vector<cv::Vec4i>();

	displaySize = new cv::Size(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	imageLeftDisplay = new cv::Mat(*displaySize, CV_8UC4);
	imageRightDisplay = new cv::Mat(*displaySize, CV_8UC4);
	depthDisplay = new cv::Mat(*displaySize, CV_8UC4);
	imageThreasholded = new cv::Mat(*displaySize, CV_8UC4);
	drawing = new cv::Mat(*displaySize, CV_8UC4);

	return true;
}

void Camera::Render() {
	

	if (!zed->grab(sl::zed::SENSING_MODE::FILL))
	{

		int width = zed->getImageSize().width;
		int height = zed->getImageSize().height;
		// Retrieve left color image
		// sl::zed::Mat left = zed->retrieveImage(sl::zed::SIDE::LEFT);
		// memcpy(imageLeft.data, left.data, width*height * 4 * sizeof(uchar));

		// retrieve right color image
		// sl::zed::Mat right = zed->retrieveImage(sl::zed::SIDE::RIGHT);
		// memcpy(imageRight.data, right.data, width*height * 4 * sizeof(uchar));

		// Retrieve depth map
		sl::zed::Mat depthmap = zed->normalizeMeasure(sl::zed::MEASURE::DEPTH);
		memcpy((*depth).data, depthmap.data, width*height * 4 * sizeof(uchar));
		// cv::inRange(*depth, cv::Scalar(192, 192, 192, 255), cv::Scalar(255, 255, 255, 255), imageThreasholded);

		// Display left image in OpenCV window
		// cv::resize(imageThreasholded, imageLeftDisplay, displaySize);
		// cv::imshow("ImageLeft", imageLeftDisplay);
		// Display right image in OpenCV window
		cv::resize(*depth, *imageRightDisplay, *displaySize);
		cv::resize(*depth, *depthDisplay, *displaySize);

		(*contours).clear();
		(*hierachy).clear();

		cv::Mat gray, canny;
		cv::cvtColor(*imageRightDisplay, gray, CV_BGR2GRAY);
		cv::blur(gray, gray, cv::Size(3, 3));
		cv::Canny(gray, canny, 160, 200, 3);
		cv::findContours(gray, *contours, *hierachy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

		(*drawing).setTo(cv::Scalar(0, 0, 0));

		for (int i = 0; i < (*contours).size(); i++) {
			cv::Scalar color = cv::Scalar((*rng).uniform(0, 255), (*rng).uniform(0, 255), (*rng).uniform(0, 255));
			cv::drawContours(*drawing, *contours, i, color, 2, 8, *hierachy, 0, cv::Point());

		}

		cv::imshow("ImageLeft", *drawing);
		CheckObjects();
		// cv::imshow("ImageRight", depthDisplay);
	}
}