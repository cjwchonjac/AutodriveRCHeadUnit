#include <Windows.h>
#include <vector>
#include "MQ.h"
#include "autodrive.pb.h"

#define CONTROL_REQUEST_START 1
#define CONTROL_REQUEST_END 2
#define CONTROL_REQUEST_SEGMENT_ARRIVED 3
#define CONTROL_REQUEST_DESTINATION_ARRIVED 4

#define CONTROL_REQUEST_STOP		10
#define CONTROL_REQUEST_GO			11
#define CONTROL_REQUEST_BACK		12
#define CONTROL_REQUEST_LEFT		13
#define CONTROL_REQUEST_RIGHT		14

struct Coord {
	double x;
	double y;
};

class ControlServer {
public:
	ControlServer(ADMQ* mq);
	~ControlServer();

	void Start();
	void Destroy();
	bool IsDestroyed();
	void HandleAction(int seq, int actionCode, char* payload, int length);
	void DoWork();
	void Write(int seq, int action, int size, char* payload);

	std::vector<Coord> GetSegments();
	Coord GetCurrentPosition();

	SOCKET mLocalSocket;
	SOCKET mClientSocket;

	int mReadPosition;
	char* mReadBuffer;
	char* mWriteBuffer;

	HANDLE mCoreHandle;
	HANDLE mWriteWorkHandle;
	HANDLE mConnectionCheckHandle;

	CRITICAL_SECTION mSegmentCS;
	CRITICAL_SECTION mPositionCS;
	
	bool mDestroyed;
private:
	std::vector<Coord> mSegments;
	Coord mCurrentPosition;
	ADMQ* mq;
};