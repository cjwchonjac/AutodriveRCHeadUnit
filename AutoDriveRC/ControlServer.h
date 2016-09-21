#include <Windows.h>

class ControlServer {
public:
	ControlServer();
	~ControlServer();

	void Start();
	void Destroy();
	bool IsDestroyed();
	void HandleAction(int seq, int actionCode, char* payload, int length);

	SOCKET mLocalSocket;
	SOCKET mClientSocket;

	int mReadPosition;
	char* mReadBuffer;
	char* mWriteBuffer;

	HANDLE mCoreHandle;
	HANDLE mWriteWorkHandle;
	HANDLE mConnectionCheckHandle;
private:
	
	bool mDestroyed;

	
};