#include "Camera.h"
#include <gl/GL.h>
#include <GL/GLU.h>
#include <GL/glut.h>

Camera::Camera() {
	// init ZED Camera
	zed = new sl::zed::Camera(static_cast<sl::zed::ZEDResolution_mode> (sl::zed::ZEDResolution_mode::HD1080));

	sl::zed::InitParams parameters;
	parameters.mode = sl::zed::MODE::PERFORMANCE;
	parameters.unit = sl::zed::UNIT::MILLIMETER;
	parameters.verbose = 1;

	sl::zed::ERRCODE err = zed->init(parameters);
	if (err != sl::zed::ERRCODE::SUCCESS) {
		delete zed;
		zed = NULL;
	}

	char *argv[1];
	int argc = 1;
	argv[0] = _strdup("ZED View");


	glutInit(&argc, argv);
	glutInitWindowSize(600, 600);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("ZED 3D Viewer");

}

bool Camera::Initialized() {
	return zed != NULL;
}

void Camera::DoWork() {
	while (1) {
		;
	}
}