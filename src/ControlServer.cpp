#include <stdio.h>
#include <initguid.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <strsafe.h>
#include <intsafe.h>
#include <ws2tcpip.h>


#define AUTODRIVE_PROTOCL_HEADER_PATTERN	"autocar!"

#define AUTODRIVE_PROTOCOL_HEADER_PATTERN_OFFSET			0
#define AUTODRIVE_PROTOCOL_HEADER_PATTERN_SIZE				8

#define AUTODRIVE_PROTOCOL_HEADED_VERSION_OFFSET			AUTODRIVE_PROTOCOL_HEADER_PATTERN_SIZE
#define AUTODRIVE_PROTOCOL_HEADER_VERSION_SIZE				4

#define AUTODRIVE_PROTOCOL_HEADED_SEQUENCE_NUMBER_OFFSET	AUTODRIVE_PROTOCOL_HEADED_VERSION_OFFSET + AUTODRIVE_PROTOCOL_HEADER_VERSION_SIZE
#define AUTODRIVE_PROTOCOL_HEADER_SEQUENCE_NUMBER_SIZE		4

#define AUTODRIVE_PROTOCOL_HEADED_ACTION_CODE_OFFSET		AUTODRIVE_PROTOCOL_HEADED_SEQUENCE_NUMBER_OFFSET + AUTODRIVE_PROTOCOL_HEADER_SEQUENCE_NUMBER_SIZE
#define AUTODRIVE_PROTOCOL_HEADER_ACTION_CODE_SIZE			4

#define AUTODRIVE_PROTOCOL_HEADED_PAYLOAD_SIZE_OFFSET		AUTODRIVE_PROTOCOL_HEADED_ACTION_CODE_OFFSET + AUTODRIVE_PROTOCOL_HEADER_ACTION_CODE_SIZE
#define AUTODRIVE_PROTOCOL_HEADER_PAYLOAD_SIZE_SIZE			4

#define AUTODRIVE_HEADER_SIZE								AUTODRIVE_PROTOCOL_HEADED_PAYLOAD_SIZE_OFFSET + AUTODRIVE_PROTOCOL_HEADER_PAYLOAD_SIZE_SIZE

#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_PING 0x00000000
#define AUTODRIVE_PROTOCOL_ACTION_CODE_SERVER_TO_CLIENT_PING 0x00000001

#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_INITIALIZE 0x10000000
#define AUTODRIVE_PROTOCOL_ACTION_CODE_SERVER_TO_CLIENT_INITIALIZE 0x10000001
#define AUTODRIVE_PROTOCOL_ACTION_CODE_SERVER_TO_CLIENT_INITIALIZE_FAILED 0x10000002

#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_SEND_LOCATION_DATA 0x20000001

#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_SEND_SEMGENT_LIST	0x30000000
#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_SEGMENT_ARRIVED		0x30000001

#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_DESTINATION_ARRIVED		0x40000000

#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_START		0x50000000
#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_END		0x50000001

#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_STOP		0x60000000
#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_GO		0x60000001
#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_BACK		0x60000002
#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_LEFT		0x60000003
#define AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_RIGHT		0x60000004

// {B62C4E8D-62CC-404b-BBBF-BF3E3BBB1374}
DEFINE_GUID(g_guidServiceClass, 0x50038ec2, 0x6485, 0x4a54, 0xa8, 0xee, 0x99, 0x7d, 0x8e, 0x1e, 0xda, 0xa4);

#define CXN_READ_BUFFER_SIZE			  1024 * 1024


#define CXN_BDADDR_STR_LEN                17   // 6 two-digit hex values plus 5 colons
#define CXN_MAX_INQUIRY_RETRY             3
#define CXN_DELAY_NEXT_INQUIRY            15

#define CXN_SUCCESS                       0
#define CXN_ERROR                         1
#define CXN_DEFAULT_LISTEN_BACKLOG        4

#define CXN_INSTANCE_STRING L"Autodrive Bluetooth Server"

#include "ControlServer.h"

#include <iostream>

struct membuf : std::streambuf
{
	membuf(char* begin, char* end) {
		this->setg(begin, begin, end);
	}
};

struct AUTODRIVE_HEADER {
	char pattern[8];
	int version;
	int seq;
	int action;
	int size;
};

DWORD WINAPI ControlServerConnectionCheckRoutine(LPVOID param) {
	return 0;
}

DWORD WINAPI ControlServerWriteWorkRoutine(LPVOID parma) {
	return 0;
}

DWORD WINAPI ControlServerCoreRoutine(LPVOID param) {
	ControlServer* server = (ControlServer*)param;

	server->DoWork();

	return 0;
}

void ControlServer::DoWork() {
	WSADATA			WSAData = { 0 };
	ULONG           ulRetCode = CXN_SUCCESS;
	int             iAddrLen = sizeof(SOCKADDR_BTH);
	size_t          cbInstanceNameSize = 0;
	char *       pszInstanceName = NULL;
	char         szThisComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD           dwLenComputerName = MAX_COMPUTERNAME_LENGTH + 1;
	SOCKET          LocalSocket = INVALID_SOCKET;
	SOCKET          ClientSocket = INVALID_SOCKET;
	WSAQUERYSET     wsaQuerySet = { 0 };
	SOCKADDR_BTH    SockAddrBthLocal = { 0 };
	LPCSADDR_INFO   lpCSAddrInfo = NULL;
	HRESULT         res;

	//
	// Ask for Winsock version 2.2.
	//
	if (CXN_SUCCESS == ulRetCode) {
		ulRetCode = WSAStartup(MAKEWORD(2, 2), &WSAData);
		if (CXN_SUCCESS != ulRetCode) {
			wprintf(L"-FATAL- | Unable to initialize Winsock version 2.2\n");
		}
	}

	//
	// This fixed-size allocation can be on the stack assuming the
	// total doesn't cause a stack overflow (depends on your compiler settings)
	// However, they are shown here as dynamic to allow for easier expansion
	//
	/*
	lpCSAddrInfo = (LPCSADDR_INFO)HeapAlloc(GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		sizeof(CSADDR_INFO));
	if (NULL == lpCSAddrInfo) {
		wprintf(L"!ERROR! | Unable to allocate memory for CSADDR_INFO\n");
		ulRetCode = CXN_ERROR;
	}

	if (CXN_SUCCESS == ulRetCode) {

		if (!GetComputerName(szThisComputerName, &dwLenComputerName)) {
			wprintf(L"=CRITICAL= | GetComputerName() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
			ulRetCode = CXN_ERROR;
		}
	}

	//
	// Open a bluetooth socket using RFCOMM protocol
	//
	if (CXN_SUCCESS == ulRetCode) {
		LocalSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
		mLocalSocket = LocalSocket;
		if (INVALID_SOCKET == LocalSocket) {
			wprintf(L"=CRITICAL= | socket() call failed. WSAGetLastError = [%d]\n", WSAGetLastError());
			ulRetCode = CXN_ERROR;
		}
	}

	if (CXN_SUCCESS == ulRetCode) {

		//
		// Setting address family to AF_BTH indicates winsock2 to use Bluetooth port
		//
		SockAddrBthLocal.addressFamily = AF_BTH;
		SockAddrBthLocal.port = BT_PORT_ANY;

		//
		// bind() associates a local address and port combination
		// with the socket just created. This is most useful when
		// the application is a server that has a well-known port
		// that clients know about in advance.
		//
		if (SOCKET_ERROR == bind(LocalSocket,
			(struct sockaddr *) &SockAddrBthLocal,
			sizeof(SOCKADDR_BTH))) {
			wprintf(L"=CRITICAL= | bind() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", (ULONG64)LocalSocket, WSAGetLastError());
			ulRetCode = CXN_ERROR;
		}
	}

	if (CXN_SUCCESS == ulRetCode) {

		ulRetCode = getsockname(LocalSocket,
			(struct sockaddr *)&SockAddrBthLocal,
			&iAddrLen);
		if (SOCKET_ERROR == ulRetCode) {
			wprintf(L"=CRITICAL= | getsockname() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", (ULONG64)LocalSocket, WSAGetLastError());
			ulRetCode = CXN_ERROR;
		}
	}

	if (CXN_SUCCESS == ulRetCode) {
		//
		// CSADDR_INFO
		//
		lpCSAddrInfo[0].LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
		lpCSAddrInfo[0].LocalAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
		lpCSAddrInfo[0].RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
		lpCSAddrInfo[0].RemoteAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
		lpCSAddrInfo[0].iSocketType = SOCK_STREAM;
		lpCSAddrInfo[0].iProtocol = BTHPROTO_RFCOMM;

		//
		// If we got an address, go ahead and advertise it.
		//
		ZeroMemory(&wsaQuerySet, sizeof(WSAQUERYSET));
		wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
		wsaQuerySet.lpServiceClassId = (LPGUID)&g_guidServiceClass;

		//
		// Adding a byte to the size to account for the space in the
		// format string in the swprintf call. This will have to change if converted
		// to UNICODE
		//
		res = StringCchLength(szThisComputerName, sizeof(szThisComputerName), &cbInstanceNameSize);
		if (FAILED(res)) {
			wprintf(L"-FATAL- | ComputerName specified is too large\n");
			ulRetCode = CXN_ERROR;
		}
	}

	if (CXN_SUCCESS == ulRetCode) {
		cbInstanceNameSize += sizeof(CXN_INSTANCE_STRING)+1;
		pszInstanceName = (LPSTR)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			cbInstanceNameSize);
		if (NULL == pszInstanceName) {
			wprintf(L"-FATAL- | HeapAlloc failed | out of memory | gle = [%d] \n", GetLastError());
			ulRetCode = CXN_ERROR;
		}
	}

	if (CXN_SUCCESS == ulRetCode) {
		StringCbPrintf(pszInstanceName, cbInstanceNameSize, "%s %s", szThisComputerName, CXN_INSTANCE_STRING);
		wsaQuerySet.lpszServiceInstanceName = pszInstanceName;
		wsaQuerySet.lpszComment = "Example Service instance registered in the directory service through RnR";
		wsaQuerySet.dwNameSpace = NS_BTH;
		wsaQuerySet.dwNumberOfCsAddrs = 1;      // Must be 1.
		wsaQuerySet.lpcsaBuffer = lpCSAddrInfo; // Req'd.

		//
		// As long as we use a blocking accept(), we will have a race
		// between advertising the service and actually being ready to
		// accept connections.  If we use non-blocking accept, advertise
		// the service after accept has been called.
		//
		if (SOCKET_ERROR == WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0)) {
			wprintf(L"=CRITICAL= | WSASetService() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
			ulRetCode = CXN_ERROR;
		}
	}

	//
	// listen() call indicates winsock2 to listen on a given socket for any incoming connection.
	//
	if (CXN_SUCCESS == ulRetCode) {
		wprintf(L"Waiting for listen \n");
		if (SOCKET_ERROR == listen(LocalSocket, CXN_DEFAULT_LISTEN_BACKLOG)) {
			wprintf(L"=CRITICAL= | listen() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", (ULONG64)LocalSocket, WSAGetLastError());
			ulRetCode = CXN_ERROR;
		}
	}*/

	struct addrinfo* result = NULL, *ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	ulRetCode = getaddrinfo("40.74.121.179", "3000", &hints, &result);
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ClientSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return;
		}

	// Connect to server.
		int iResult = connect(ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}


	freeaddrinfo(result);


	if (ClientSocket != INVALID_SOCKET) {
		mClientSocket = ClientSocket;

		while ((ClientSocket != INVALID_SOCKET)) {

			//
			// accept() call indicates winsock2 to wait for any
			// incoming connection request from a remote socket.
			// If there are already some connection requests on the queue,
			// then accept() extracts the first request and creates a new socket and
			// returns the handle to this newly created socket. This newly created
			// socket represents the actual connection that connects the two sockets.
			//
			wprintf(L"Waiting for connection \n");
			/*mClientSocket = accept(LocalSocket, NULL, NULL);
			if (INVALID_SOCKET == mClientSocket) {
				wprintf(L"=CRITICAL= | accept() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
				ulRetCode = CXN_ERROR;
				break; // Break out of the for loop
			}*/

			wprintf(L"New Bluetooth connection established \n");
			//
			// Read data from the incoming stream
			//
			while (INVALID_SOCKET != mClientSocket) {
				int byteReceived = recv(mClientSocket, mReadBuffer + mReadPosition, CXN_READ_BUFFER_SIZE - mReadPosition, 0);
				if (byteReceived > 0) {
					mReadPosition += byteReceived;

					int pos = 0;
					while (mReadPosition - pos >= AUTODRIVE_HEADER_SIZE) {
						AUTODRIVE_HEADER header = { 0 };
						memcpy(&header, mReadBuffer + pos, sizeof(AUTODRIVE_HEADER));

						if (strncmp(header.pattern, AUTODRIVE_PROTOCL_HEADER_PATTERN, AUTODRIVE_PROTOCOL_HEADER_PATTERN_SIZE) == 0) {
							if (mReadPosition - (pos + AUTODRIVE_HEADER_SIZE) >= header.size) {
								// handle action
								HandleAction(header.seq, header.action, mReadBuffer + pos + AUTODRIVE_HEADER_SIZE, header.size);
								pos += AUTODRIVE_HEADER_SIZE + header.size;
							}
							else {
								break;
							}
						}
						else {
							// header failed
							int start = 0;
							for (int idx = pos + 1; idx < mReadPosition; idx++) {
								if (AUTODRIVE_PROTOCL_HEADER_PATTERN[start] == mReadBuffer[idx]) {
									start++;

									if (start >= AUTODRIVE_PROTOCOL_HEADER_PATTERN_SIZE || idx == mReadPosition - 1) {
										pos = idx - (start - 1);
										break;
									}
								}
								else {
									start = 0;
								}

								pos = idx + 1;
							}

							if (start >= AUTODRIVE_PROTOCOL_HEADER_PATTERN_SIZE) {
								continue;
							}
							else {
								break;
							}
						}

					}

					if (pos > 0) {
						if (mReadPosition > pos) {
							memmove(mReadBuffer, mReadBuffer + pos, mReadPosition - pos);
							mReadPosition -= pos;
						}
						else {
							mReadPosition = 0;
						}
					}
				}
				else {
					closesocket(mClientSocket);
					mClientSocket = NULL;

					WaitForSingleObject(mWriteWorkHandle, INFINITE);
					mWriteWorkHandle = NULL;
					break;
				}
			}
		}
	}

	if (NULL != lpCSAddrInfo) {
		HeapFree(GetProcessHeap(), 0, lpCSAddrInfo);
		lpCSAddrInfo = NULL;
	}
	if (NULL != pszInstanceName) {
		HeapFree(GetProcessHeap(), 0, pszInstanceName);
		pszInstanceName = NULL;
	}
}

void ControlServer::HandleAction(int seq, int actionCode, char* payload, int length) {
	switch (actionCode)
	{
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_PING:
		// printf("ping with seq %d\n", seq);
		Write(seq, AUTODRIVE_PROTOCOL_ACTION_CODE_SERVER_TO_CLIENT_PING, 0, NULL);
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_INITIALIZE:
		Write(seq, AUTODRIVE_PROTOCOL_ACTION_CODE_SERVER_TO_CLIENT_INITIALIZE, 0, NULL);
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_SEND_LOCATION_DATA: {
		membuf buf(payload, payload + length);
		com::autodrive::message::LocationMessage* message = new com::autodrive::message::LocationMessage();
		std::istream stream(&buf);
		message->ParseFromIstream(&stream);

		// printf("message %f %f", message->latitude(), message->longitude());
	
		EnterCriticalSection(&mPositionCS);
		mCurrentPosition.x = message->latitude();
		mCurrentPosition.y = message->longitude();
		LeaveCriticalSection(&mPositionCS);
		delete message;
	}
		
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_SEND_SEMGENT_LIST:
	{
		membuf buf(payload, payload + length);
		com::autodrive::message::SegmentList* list = new com::autodrive::message::SegmentList();
		std::istream stream(&buf);
		list->ParseFromIstream(&stream);

		EnterCriticalSection(&mSegmentCS);
		mSegments.clear();

		const int size = list->locations_size();
		for (int idx = 0; idx < size; idx++) {
			com::autodrive::message::LocationMessage message = list->locations(idx);
			
			Coord coord;
			coord.x = message.latitude();
			coord.y = message.longitude();
			mSegments.push_back(coord);
			printf("message %f %f\n", message.latitude(), message.longitude());
		}
		LeaveCriticalSection(&mSegmentCS);
		delete list;
	}
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_SEGMENT_ARRIVED:
	{
		membuf buf(payload, payload + length);
		com::autodrive::message::SegmentArrived* arrived = new com::autodrive::message::SegmentArrived();
		std::istream stream(&buf);
		arrived->ParseFromIstream(&stream);

		ADRequest rq;
		rq.type = CONTROL_REQUEST_SEGMENT_ARRIVED;
		rq.data = arrived;
		mq->offer(rq);
	}
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_DESTINATION_ARRIVED:
	{
		ADRequest rq;
		rq.type = CONTROL_REQUEST_DESTINATION_ARRIVED;
		rq.data = NULL;
		mq->offer(rq);
	}
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_START:
	{
		ADRequest rq;
		rq.type = CONTROL_REQUEST_START;
		rq.data = NULL;
		mq->offer(rq);
	}
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_END:
	{
		ADRequest rq;
		rq.type = CONTROL_REQUEST_END;
		rq.data = NULL;
		mq->offer(rq);
	}
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_STOP:
	{
		ADRequest rq;
		rq.type = CONTROL_REQUEST_STOP;
		rq.data = NULL;
		mq->offer(rq);
	}
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_GO:
	{
		ADRequest rq;
		rq.type = CONTROL_REQUEST_GO;
		rq.data = NULL;
		mq->offer(rq);
	}
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_BACK:
	{
		ADRequest rq;
		rq.type = CONTROL_REQUEST_BACK;
		rq.data = NULL;
		mq->offer(rq);
	}
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_LEFT:
	{
		ADRequest rq;
		rq.type = CONTROL_REQUEST_LEFT;
		rq.data = NULL;
		mq->offer(rq);
	}
		break;
	case AUTODRIVE_PROTOCOL_ACTION_CODE_CLIENT_TO_SERVER_CONTROL_CONTROL_RIGHT:
	{
		ADRequest rq;
		rq.type = CONTROL_REQUEST_RIGHT;
		rq.data = NULL;
		mq->offer(rq);
	}
		break;
	}
}

ControlServer::ControlServer(ADMQ* q) {
	mq = q;
	mLocalSocket = NULL;
	mClientSocket = NULL;
	
	mReadPosition = 0;
	mReadBuffer = (char *)HeapAlloc(GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		CXN_READ_BUFFER_SIZE);

	mWriteBuffer = (char *)HeapAlloc(GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		AUTODRIVE_HEADER_SIZE);

	mCoreHandle = NULL;
	mWriteWorkHandle = NULL;
	mConnectionCheckHandle = NULL;

	mCurrentPosition.x = 0.0;
	mCurrentPosition.y = 0.0;

	InitializeCriticalSection(&mPositionCS);
	InitializeCriticalSection(&mSegmentCS);
}

ControlServer::~ControlServer() {
	if (!IsDestroyed()) {
		Destroy();
	}

	if (NULL != mReadBuffer) {
		HeapFree(GetProcessHeap(), 0, mReadBuffer);
		mReadBuffer = NULL;
	}

	if (NULL != mWriteBuffer) {
		HeapFree(GetProcessHeap(), 0, mReadBuffer);
		mReadBuffer = NULL;
	}

	DeleteCriticalSection(&mSegmentCS);
	DeleteCriticalSection(&mPositionCS);
}

void ControlServer::Start() {
	if (mCoreHandle == NULL) {
		mCoreHandle = CreateThread(NULL, 0, ControlServerCoreRoutine, this, 0, NULL);
	}
}

void ControlServer::Destroy() {
	mDestroyed = true;

	if (INVALID_SOCKET != mClientSocket) {
		closesocket(mClientSocket);
		mClientSocket = INVALID_SOCKET;
	}

	if (INVALID_SOCKET != mLocalSocket) {
		closesocket(mLocalSocket);
		mLocalSocket = INVALID_SOCKET;
	}
}

bool ControlServer::IsDestroyed() {
	return mDestroyed;
}

void ControlServer::Write(int seq, int action, int size, char* payload) {
	if (INVALID_SOCKET != mClientSocket) {
		int version = 1;
		send(mClientSocket, (const char*)AUTODRIVE_PROTOCL_HEADER_PATTERN, AUTODRIVE_PROTOCOL_HEADER_PATTERN_SIZE, 0);
		send(mClientSocket, (const char*)&version, AUTODRIVE_PROTOCOL_HEADER_VERSION_SIZE, 0);
		send(mClientSocket, (const char*)&seq, AUTODRIVE_PROTOCOL_HEADER_SEQUENCE_NUMBER_SIZE, 0);
		send(mClientSocket, (const char*)&action, AUTODRIVE_PROTOCOL_HEADER_ACTION_CODE_SIZE, 0);
		send(mClientSocket, (const char*)&size, AUTODRIVE_PROTOCOL_HEADER_PAYLOAD_SIZE_SIZE, 0);

		if (size > 0 && payload > 0) {
			send(mClientSocket, (const char*)payload, size, 0);
		}
	}
}

Coord ControlServer::GetCurrentPosition() {
	Coord coord;
	EnterCriticalSection(&mPositionCS);
	memcpy(&coord, &mCurrentPosition, sizeof(coord));
	LeaveCriticalSection(&mPositionCS);

	return coord;
}

std::vector<Coord> ControlServer::GetSegments() {
	std::vector<Coord> coords;
	EnterCriticalSection(&mSegmentCS);
	for (int idx = 0; idx < mSegments.size(); idx++) {
		coords.push_back(mSegments.at(idx));
	}
	
	LeaveCriticalSection(&mSegmentCS);
	return coords;
}