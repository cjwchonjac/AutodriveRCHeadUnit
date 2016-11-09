#include <Windows.h>
#include <vector>
#include "MQ.h"
#include "autodrive.pb.h"

#define CONTROL_REQUEST_START 1
#define CONTROL_REQUEST_END 2
#define CONTROL_REQUEST_SEGMENT_ARRIVED 3
#define CONTROL_REQUEST_DESTINATION_ARRIVED 4

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