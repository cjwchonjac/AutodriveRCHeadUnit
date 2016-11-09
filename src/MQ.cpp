#include "MQ.h"

ADMQ::ADMQ() {
	InitializeCriticalSection(&cs);
}

ADMQ::~ADMQ() {
	DeleteCriticalSection(&cs);
}

bool ADMQ::empty() {
	bool empty;
	EnterCriticalSection(&cs);
	empty = Q.empty();
	LeaveCriticalSection(&cs);
	return empty;
}

void ADMQ::offer(ADRequest rq) {
	EnterCriticalSection(&cs);
	Q.push(rq);
	LeaveCriticalSection(&cs);
}

ADRequest ADMQ::poll() {
	ADRequest rq = { 0 };
	EnterCriticalSection(&cs);
	if (!Q.empty()) {
		rq = Q.front();
		Q.pop();
	}
	LeaveCriticalSection(&cs);
	return rq;
}