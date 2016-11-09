#include "Camera.h"

#undef MAX
#undef MIN

#define MAX(a,b) ((a)<(b)?(b):(a))
#define MIN(a,b) ((a)>(b)?(b):(a))

#define GREEN CV_RGB(0,255,0)
#define RED CV_RGB(255,0,0)
#define BLUE CV_RGB(255,0,255)
#define PURPLE CV_RGB(255,0,255)

#define K_VARY_FACTOR 0.2f
#define B_VARY_FACTOR 20
#define MAX_LOST_FRAMES 30

const double LPF_Beta = 0.025;

CvPoint2D32f sub(CvPoint2D32f b, CvPoint2D32f a) { return cvPoint2D32f(b.x - a.x, b.y - a.y); }
CvPoint2D32f mul(CvPoint2D32f b, CvPoint2D32f a) { return cvPoint2D32f(b.x*a.x, b.y*a.y); }
CvPoint2D32f add(CvPoint2D32f b, CvPoint2D32f a) { return cvPoint2D32f(b.x + a.x, b.y + a.y); }
CvPoint2D32f mul(CvPoint2D32f b, float t) { return cvPoint2D32f(b.x*t, b.y*t); }
float dot(CvPoint2D32f a, CvPoint2D32f b) { return (b.x*a.x + b.y*a.y); }
float dist(CvPoint2D32f v) { return sqrtf(v.x*v.x + v.y*v.y); }

CvPoint2D32f point_on_segment(CvPoint2D32f line0, CvPoint2D32f line1, CvPoint2D32f pt){

	CvPoint2D32f v = sub(pt, line0);
	CvPoint2D32f dir = sub(line1, line0);
	float len = dist(dir);
	float inv = 1.0f / (len + 1e-6f);

	dir.x *= inv;
	dir.y *= inv;

	float t = dot(dir, v);
	if (t >= len) return line1;
	else if (t <= 0) return line0;

	return add(line0, mul(dir, t));
}

float dist2line(CvPoint2D32f line0, CvPoint2D32f line1, CvPoint2D32f pt){
	return dist(sub(point_on_segment(line0, line1, pt), pt));
}

void crop(IplImage* src, IplImage* dest, CvRect rect) {
	cvSetImageROI(src, rect);
	cvCopy(src, dest);
	cvResetImageROI(src);
}

enum{
	SCAN_STEP = 5,			  // in pixels
	LINE_REJECT_DEGREES = 10, // in degrees
	BW_TRESHOLD = 250,		  // edge response strength to recognize for 'WHITE'
	BORDERX = 10,			  // px, skip this much from left & right borders
	MAX_RESPONSE_DIST = 5,	  // px

	CANNY_MIN_TRESHOLD = 1,	  // edge detector minimum hysteresis threshold
	CANNY_MAX_TRESHOLD = 100, // edge detector maximum hysteresis threshold

	HOUGH_TRESHOLD = 50,		// line approval vote threshold
	HOUGH_MIN_LINE_LENGTH = 50,	// remove lines shorter than this treshold
	HOUGH_MAX_LINE_GAP = 100,   // join lines to one with smaller than this gaps

	CAR_DETECT_LINES = 4,    // minimum lines for a region to pass validation as a 'CAR'
	CAR_H_LINE_LENGTH = 10,  // minimum horizontal line length from car body in px

	MAX_VEHICLE_SAMPLES = 30,      // max vehicle detection sampling history
	CAR_DETECT_POSITIVE_SAMPLES = MAX_VEHICLE_SAMPLES - 2, // probability positive matches for valid car
	MAX_VEHICLE_NO_UPDATE_FREQ = 15 // remove car after this much no update frames
};

void FindResponses(IplImage *img, int startX, int endX, int y, std::vector<int>& list)

{

	// scans for single response: /^\_
	const int row = y * img->width * img->nChannels;
	unsigned char* ptr = (unsigned char*)img->imageData;

	int step = (endX < startX) ? -1 : 1;
	int range = (endX > startX) ? endX - startX + 1 : startX - endX + 1;

	for (int x = startX; range>0; x += step, range--) {
		if (ptr[row + x] <= BW_TRESHOLD) continue; // skip black: loop until white pixels show up

		// first response found
		int idx = x + step;

		// skip same response(white) pixels
		while (range > 0 && ptr[row + idx] > BW_TRESHOLD) {
			idx += step;
			range--;
		}

		// reached black again
		if (ptr[row + idx] <= BW_TRESHOLD) {
			list.push_back(x);
		}

		x = idx; // begin from new pos

	}

}



unsigned char pixel(IplImage* img, int x, int y) {
	return (unsigned char)img->imageData[(y*img->width + x)*img->nChannels];
}



int findSymmetryAxisX(IplImage* half_frame, CvPoint bmin, CvPoint bmax) {
	float value = 0;
	int axisX = -1; // not found
	int xmin = bmin.x;
	int ymin = bmin.y;
	int xmax = bmax.x;
	int ymax = bmax.y;
	int half_width = half_frame->width / 2;
	int maxi = 1;

	for (int x = xmin, j = 0; x<xmax; x++, j++) {
		float HS = 0;
		for (int y = ymin; y<ymax; y++) {
			int row = y*half_frame->width*half_frame->nChannels;
			for (int step = 1; step<half_width; step++) {
				int neg = x - step;
				int pos = x + step;
				unsigned char Gneg = (neg < xmin) ? 0 : (unsigned char)half_frame->imageData[row + neg*half_frame->nChannels];
				unsigned char Gpos = (pos >= xmax) ? 0 : (unsigned char)half_frame->imageData[row + pos*half_frame->nChannels];
				HS += abs(Gneg - Gpos);
			}
		}

		if (axisX == -1 || value > HS) { // find minimum
			axisX = x;
			value = HS;
		}
	}

	return axisX;

}



bool hasVertResponse(IplImage* edges, int x, int y, int ymin, int ymax) {
	bool has = (pixel(edges, x, y) > BW_TRESHOLD);
	if (y - 1 >= ymin) has &= (pixel(edges, x, y - 1) < BW_TRESHOLD);
	if (y + 1 < ymax) has &= (pixel(edges, x, y + 1) < BW_TRESHOLD);
	return has;
}



int horizLine(IplImage* edges, int x, int y, CvPoint bmin, CvPoint bmax, int maxHorzGap) {
	// scan to right
	int right = 0;
	int gap = maxHorzGap;

	for (int xx = x; xx<bmax.x; xx++) {
		if (hasVertResponse(edges, xx, y, bmin.y, bmax.y)) {
			right++;
			gap = maxHorzGap; // reset
		}
		else {
			gap--;

			if (gap <= 0) {
				break;
			}
		}
	}

	int left = 0;
	gap = maxHorzGap;
	for (int xx = x - 1; xx >= bmin.x; xx--) {
		if (hasVertResponse(edges, xx, y, bmin.y, bmax.y)) {
			left++;
			gap = maxHorzGap; // reset
		}
		else {
			gap--;
			if (gap <= 0) {
				break;
			}
		}
	}

	return left + right;

}

bool Camera::processSide(std::vector<Lane> lanes, IplImage *edges, bool right) {

	Status* side = right ? &laneR : &laneL;
	// response search
	int w = edges->width;
	int h = edges->height;
	const int BEGINY = 0;
	const int ENDY = h - 1;
	const int ENDX = right ? (w - BORDERX) : BORDERX;
	int midx = w / 2;
	int midy = edges->height / 2;
	unsigned char* ptr = (unsigned char*)edges->imageData;

	// show responses

	int* votes = new int[lanes.size()];
	for (int i = 0; i<lanes.size(); i++) votes[i++] = 0;

	for (int y = ENDY; y >= BEGINY; y -= SCAN_STEP) {
		std::vector<int> rsp;
		FindResponses(edges, midx, ENDX, y, rsp);

		if (rsp.size() > 0) {
			int response_x = rsp[0]; // use first reponse (closest to screen center)
			float dmin = 9999999;
			float xmin = 9999999;
			int match = -1;

			for (int j = 0; j<lanes.size(); j++) {

				// compute response point distance to current line
				float d = dist2line(
					cvPoint2D32f(lanes[j].p0.x, lanes[j].p0.y),
					cvPoint2D32f(lanes[j].p1.x, lanes[j].p1.y),
					cvPoint2D32f(response_x, y));

				// point on line at current y line
				int xline = (y - lanes[j].b) / lanes[j].k;
				int dist_mid = abs(midx - xline); // distance to midpoint

				// pick the best closest match to line & to screen center
				if (match == -1 || (d <= dmin && dist_mid < xmin)) {
					dmin = d;
					match = j;
					xmin = dist_mid;
					break;
				}
			}

			// vote for each line
			if (match != -1) {
				votes[match] += 1;
			}
		}
	}

	int bestMatch = -1;
	int mini = 9999999;

	for (int i = 0; i<lanes.size(); i++) {
		int xline = (midy - lanes[i].b) / lanes[i].k;
		int dist = abs(midx - xline); // distance to midpoint

		if (bestMatch == -1 || (votes[i] > votes[bestMatch] && dist < mini)) {
			bestMatch = i;
			mini = dist;
		}
	}

	if (bestMatch != -1) {
		Lane* best = &lanes[bestMatch];
		float k_diff = fabs(best->k - side->k.get());
		float b_diff = fabs(best->b - side->b.get());

		bool update_ok = (k_diff <= K_VARY_FACTOR && b_diff <= B_VARY_FACTOR) || side->reset;

		// printf("side: %s, k vary: %.4f, b vary: %.4f, lost: %s\n",
		// 	(right ? "RIGHT" : "LEFT"), k_diff, b_diff, (update_ok ? "no" : "yes"));

		if (update_ok) {
			// update is in valid bounds
			side->k.add(best->k);
			side->b.add(best->b);
			side->reset = false;
			side->lost = 0;
		}
		else {

			// can't update, lanes flicker periodically, start counter for partial reset!
			side->lost++;

			if (side->lost >= MAX_LOST_FRAMES && !side->reset) {
				side->reset = true;
			}
		}
	}
	else {
		// printf("no lanes detected - lane tracking lost! counter increased\n");
		side->lost++;

		if (side->lost >= MAX_LOST_FRAMES && !side->reset) {
			// do full reset when lost for more than N frames

			side->reset = true;
			side->k.clear();
			side->b.clear();
		}

		return false;
	}

	delete[] votes;
	return true;
}



int Camera::processLanes(CvSeq* lines, IplImage* edges, IplImage* temp_frame) {
	// classify lines to left/right side

	std::vector<Lane> left, right;

	for (int i = 0; i < lines->total; i++)
	{
		CvPoint* line = (CvPoint*)cvGetSeqElem(lines, i);
		int dx = line[1].x - line[0].x;
		int dy = line[1].y - line[0].y;
		float angle = atan2f(dy, dx) * 180 / CV_PI;

		if (fabs(angle) <= LINE_REJECT_DEGREES) { // reject near horizontal lines
			continue;
		}

		// assume that vanishing point is close to the image horizontal center
		// calculate line parameters: y = kx + b;
		dx = (dx == 0) ? 1 : dx; // prevent DIV/0!  
		float k = dy / (float)dx;
		float b = line[0].y - k*line[0].x;

		// assign lane's side based by its midpoint position 
		int midx = (line[0].x + line[1].x) / 2;
		if (midx < temp_frame->width / 2) {
			left.push_back(Lane(line[0], line[1], angle, k, b));
		}
		else if (midx > temp_frame->width / 2) {
			right.push_back(Lane(line[0], line[1], angle, k, b));
		}
	}

	// show Hough lines
	for (int i = 0; i<right.size(); i++) {
		cvLine(temp_frame, right[i].p0, right[i].p1, CV_RGB(0, 0, 255), 2);
	}

	for (int i = 0; i<left.size(); i++) {
		cvLine(temp_frame, left[i].p0, left[i].p1, CV_RGB(255, 0, 0), 2);
	}

	bool lValid = processSide(left, edges, false);
	bool rValid = processSide(right, edges, true);

	// show computed lanes
	int x = temp_frame->width * 0.55f;
	int x2 = temp_frame->width;

	CvPoint l1(x, laneR.k.get()*x + laneR.b.get());
	CvPoint l2(x2, laneR.k.get() * x2 + laneR.b.get());
	cvLine(temp_frame, l1, l2, CV_RGB(255, 0, 255), 2);

	x = temp_frame->width * 0;
	x2 = temp_frame->width * 0.45f;

	CvPoint r1(x, laneL.k.get()*x + laneL.b.get());
	CvPoint r2(x2, laneL.k.get() * x2 + laneL.b.get());
	cvLine(temp_frame, r1, r2, CV_RGB(255, 0, 255), 2);


	if (!laneR.reset && !laneL.reset && lValid && rValid) {
		// parellel case
		int det = ((l1.x - l2.x) * (r1.y - r2.y) - (l1.y - l2.y) * (r1.x - r2.x));
		if (det == 0) {
			return -1;
		}

		double lx = l2.x - l1.x;
		double ly = l2.y - l1.y;
		double rx = r2.x - r1.x;
		double ry = r2.y - r1.y;
		double lSlope = lx == 0 ? 90.0 : atan2(ly, lx) * 180.0 / CV_PI;
		double rSlope = rx == 0 ? 90.0 : atan2(ry, rx) * 180.0 / CV_PI;

		// printf("lSlope : %f, rSlope: %f\n", lSlope, rSlope);

		if (lSlope <= 0) {
			return -1;
		}

		if (rSlope >= 0) {
			return -1;
		}


		int cx = ((l1.x * l2.y - l1.y * l2.x) * (r1.x - r2.x) - (r1.x * r2.y - r1.y * r2.x) * (l1.x - l2.x)) /
			det;
		int cy = ((l1.x * l2.y - l1.y * l2.x) * (r1.y - r2.y) - (r1.x * r2.y - r1.y * r2.x) * (l1.y - l2.y)) /
			det;

		return cx;

	}

	return -1;
}


RenderResult Camera::CheckObjects() {
	cv::Mat* image = imageLeftDisplay;
	cv::imshow("ImageRight", *drawing);

	std::vector<std::vector<cv::Point>> newContours;
	RenderResult result = { 0 };

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

			if (y < DISPLAY_HEIGHT * 1 / 2) {
				filtered.push_back(cv::Point(x, y));

				if (x > DISPLAY_WIDTH * 4 / 5) {
					if (rightSideLeftmost < 0 || x < rightSideLeftmost) {
						rightSideLeftmost = x;
						result.right = true;
					}
				}
				else if (x < DISPLAY_WIDTH * 1 / 5) {
					if (leftSideRightmost < 0 || x > leftSideRightmost) {
						leftSideRightmost = x;
						result.left = true;
					}
				}
				else {
					result.center = true;
					// printf("Object in center position \n");
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
	return result;
}

void LeftImageClicked(int event, int x, int y, int flags, void* userdata) {
	Camera* camera = reinterpret_cast<Camera*>(userdata);
	if (event == CV_EVENT_LBUTTONUP) {
		// camera->CheckObjects();
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

	center = imageWidth / 2;

	cv::namedWindow("ImageLeft", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageRight", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);

	cv::setMouseCallback("ImageLeft", LeftImageClicked, this);
	cv::setMouseCallback("ImageRight", RightImageClicked, this);

	rng = new cv::RNG(12345);

	

	contours = new std::vector<std::vector<cv::Point>>();
	hierachy = new std::vector<cv::Vec4i>();

	displaySize = new cv::Size(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	imageLeftDisplay = new cv::Mat(*displaySize, CV_8UC4);
	imageRightDisplay = new cv::Mat(*displaySize, CV_8UC4);
	depthDisplay = new cv::Mat(*displaySize, CV_8UC4);
	imageThreasholded = new cv::Mat(*displaySize, CV_8UC4);
	drawing = new cv::Mat(*displaySize, CV_8UC4);

	frameSize = cvSize(imageWidth, imageHeight / 2);
	tempFrame = cvCreateImage(frameSize, IPL_DEPTH_8U, 4);
	grey = cvCreateImage(frameSize, IPL_DEPTH_8U, 1);
	edges = cvCreateImage(frameSize, IPL_DEPTH_8U, 1);
	halfFrame = cvCreateImage(cvSize(imageWidth / 2, imageHeight / 2), IPL_DEPTH_8U, 4);

	writer = cvCreateVideoWriter("output.avi", CV_FOURCC('M', 'J', 'P', 'G'), 15, CvSize(imageWidth, imageHeight), 1);
	houghStorage = cvCreateMemStorage(0);
	return true;
}

double Camera::CheckLanes(IplImage* frame) {
	cvPyrDown(frame, halfFrame, CV_GAUSSIAN_5x5); // Reduce the image by 2	 
	//cvCvtColor(temp_frame, grey, CV_BGR2GRAY); // convert to grayscale
	// we're interested only in road below horizont - so crop top image portion off
	crop(frame, tempFrame, cvRect(0, frameSize.height, frameSize.width, frameSize.height));
	cvCvtColor(tempFrame, grey, CV_BGR2GRAY); // convert to grayscale
	// Perform a Gaussian blur ( Convolving with 5 X 5 Gaussian) & detect edges

	cvSmooth(grey, grey, CV_GAUSSIAN, 5, 5);
	cvCanny(grey, edges, CANNY_MIN_TRESHOLD, CANNY_MAX_TRESHOLD);

	// do Hough transform to find lanes
	double rho = 1;
	double theta = CV_PI / 180;
	CvSeq* lines = cvHoughLines2(edges, houghStorage, CV_HOUGH_PROBABILISTIC,
		rho, theta, HOUGH_TRESHOLD, HOUGH_MIN_LINE_LENGTH, HOUGH_MAX_LINE_GAP);

	int cx = processLanes(lines, edges, tempFrame);
	if (cx < 0) {
		cx = frameSize.width / 2;
	}

	center = center - (LPF_Beta * (center - cx));
	cvDrawCircle(tempFrame, CvPoint(center, frameSize.height / 2), 5, CV_RGB(255, 0, 255), 3);

	double ratio = (center - (frameSize.width / 2)) / (frameSize.width / 2);

	// process vehicles

	//vehicleDetection(half_frame, cascade, haarStorage);
	//drawVehicles(half_frame);
	// cvShowImage("Half-frame", halfFrame);

	// show middle line
	cvLine(tempFrame, cvPoint(frameSize.width / 2, 0),
		cvPoint(frameSize.width / 2, frameSize.height), CV_RGB(255, 255, 0), 1);

	// cvShowImage("Grey", grey);
	// cvShowImage("Edges", edges);
	cvShowImage("Color", tempFrame);
	return center;
}

RenderResult Camera::Render() {


	if (!zed->grab(sl::zed::SENSING_MODE::FILL))
	{

		int width = zed->getImageSize().width;
		int height = zed->getImageSize().height;
		// Retrieve left color image
		// sl::zed::Mat left = zed->retrieveImage(sl::zed::SIDE::LEFT);
		// memcpy(imageLeft.data, left.data, width*height * 4 * sizeof(uchar));

		// retrieve right color image
		sl::zed::Mat right = zed->retrieveImage(sl::zed::SIDE::RIGHT);
		memcpy((*imageRight).data, right.data, width*height * 4 * sizeof(uchar));

		

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
		RenderResult rr = CheckObjects();

		IplImage frame(*imageRight);
		cvWriteFrame(writer, &frame);
		rr.laneDirection = CheckLanes(&frame);
		return CheckObjects();
		// cv::imshow("ImageRight", depthDisplay);
	}

	RenderResult reuslt = { 0 };
	return reuslt;
}