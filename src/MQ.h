#pragma once

#include <Windows.h>
#include <queue>



using namespace std;

struct ADRequest {
	int type;
	void* data;
};


class ADMQ {
private:
	queue<ADRequest> Q;
	CRITICAL_SECTION cs;
public:
	ADMQ();
	~ADMQ();
	bool empty();
	void offer(ADRequest rq);
	ADRequest poll();
};