//ZED includes
#include <zed/Camera.hpp>

class Camera {
public:
	Camera();
	bool Initialized();
	void DoWork();
private:
	sl::zed::Camera* zed;
};