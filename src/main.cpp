//standard includes
#include <iostream>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "RoboClaw.h"
#include "Camera.h"
#include "ControlServer.h"
#include "MQ.h"

using namespace std;

double direction(Coord currentPoint, Coord nextPoint)
{
	double slope;
	double direction;

	if ((nextPoint.x - currentPoint.x) == 0) slope = 999999;
	else slope = ((nextPoint.y - currentPoint.y) / (nextPoint.x - currentPoint.x));

	direction = atan(slope) * 180 / M_PI + 90;

	if ((nextPoint.x - currentPoint.x) > 0)
	{
		direction += 180;
	}

	return direction; // degree (0~360)
}

double distanceCoords(Coord p1, Coord p2)
{
	double x = (p1.x - p2.x)*(p1.x - p2.x);
	double y = (p1.y - p2.y)*(p1.y - p2.y);
	double temp = sqrt(x + y);

	return temp * 100000;	// distance (meter)
}

bool sameCoord(Coord p1, Coord p2)
{
	double dist = distanceCoords(p1, p2);

	return dist < 7.0;
}

void print(FILE* fp, const char* log, ...) {
	char buf[512] = { 0 };
	va_list list;
	va_start(list, log);
	vsprintf(buf, log, list);
	va_end(list);

	printf(buf);
	fprintf(fp, buf);
}

//////////////////
int main() {
	FILE* fp = NULL;
	fopen_s(&fp, "log.txt", "w");
	// init BT Control server

	ADMQ* mq = new ADMQ();

	ControlServer server(mq);
	server.Start();

	Camera c;

	// RC Control
	RoboClaw* roboclaw = new RoboClaw();

	// Motor Command Set
	int MotorCommand[2];
	int MotorVelocity;		// 0 ~ 90
	int MotorRight;		//Motor2Forward
	int MotorLeft;		//Motor2Backward
	bool MotorRecover = false;
	bool motor1change = false;
	bool motor2change = false;

	int nextPoint = 0;		//Current Segment index
	double nextDirection;		// Direction to nextPoint (degree)
	double currentDirection;	// Current Direction (degree)	
	double nextDistance;		// Distance left to NextPoint;
	double errorRange;			// between currentDirection and nextDirection;


//// RoboClaw Connection
	std::string port("Com3");	///////// COM port ///////
	if (roboclaw->begin(port, 115200)) {
		std::cout << "RoboClaw connected through " << port << " with baudrate 115200" << std::endl;
		// write test
		uint8_t addr = 0x80;


		/*
////////////////////////////////////////////////////////
// Angle Test
		roboclaw->ForwardM1(addr, 50); // Go Forward
		Sleep(1000);

	//Turn Right
		roboclaw->BackwardM2(addr, 0); // Left Off
		roboclaw->ForwardM2(addr, 100); // Right On	
		Sleep(1500);

	//Recovery
		roboclaw->ForwardM2(addr, 0); // Right Off
		roboclaw->BackwardM2(addr, 100); // Left On
		Sleep(400);

	//Straight
		roboclaw->ForwardM2(addr, 0); // Right Off
		roboclaw->BackwardM2(addr, 0); // Left On
		Sleep(100);

	//Stop
		roboclaw->ForwardM1(addr, 0); // Go Forward
		Sleep(2000);

while (1){ }*/

//////////////////////////////////////////////////////

		bool result = c.Initialize();
		if (!result) {
			printf("camera error\n");
		}


		// Motor Initialization
		MotorCommand[0] = 0;
		MotorCommand[1] = 0;
		MotorVelocity = 0;
		MotorRight = 0;
		MotorLeft = 0;

		Coord newCurrentPos;
		Coord oldCurrentPos;

		int	speedUp = 0;

		//test
//		MotorCommand[0] = 2;
		
		// Sleep(1000);

		 

		bool start = false;
		std::vector<Coord> list;
		while (1) {
			ADRequest rq;
			com::autodrive::message::SegmentArrived* arrived = NULL;

			do {
				rq = mq->poll();

				switch (rq.type) {
				case CONTROL_REQUEST_START:
					start = true;
					list = server.GetSegments();
					roboclaw->ForwardM1(addr, 80); // Go Forward
					break;
				case CONTROL_REQUEST_END:
					start = false;
					list.clear();
					break;
				case CONTROL_REQUEST_SEGMENT_ARRIVED:
					arrived = reinterpret_cast<com::autodrive::message::SegmentArrived*>(rq.data);
					break;
				case CONTROL_REQUEST_DESTINATION_ARRIVED:
					start = false;
					list.clear();
					break;
				}
			} while (rq.type > 0);

			if (start) {
				RenderResult rr = c.Render();
				printf("left: %d, right: %d, center: %d, laneDirection: %f\n",
					rr.left, rr.right, rr.center, rr.laneDirection);

				if (arrived != NULL)
				{
					double angle = arrived->angle();
					printf("Next Angle = %f \n", arrived->angle());
					Sleep(3000);

					if (angle < -10)	//Right
					{
						//Turn Right
						roboclaw->BackwardM2(addr, 0); // Left Off
						roboclaw->ForwardM2(addr, 100); // Right On	
						Sleep(1000 + -angle * 500 / 15);

						//Recovery
						roboclaw->ForwardM2(addr, 0); // Right Off
						roboclaw->BackwardM2(addr, 100); // Left On
						Sleep(400);

						//Straight
						roboclaw->ForwardM2(addr, 0); // Right Off
						roboclaw->BackwardM2(addr, 0); // Left On
						Sleep(100);

					}
					else if (angle > 10)	//Left
					{
						//Turn Left
						roboclaw->ForwardM2(addr, 0); // Left Off
						roboclaw->BackwardM2(addr, 100); // Right On	
						Sleep(1000 + angle * 500 / 15);		// up to angle

						//Recovery
						roboclaw->BackwardM2(addr, 0); // Right Off
						roboclaw->ForwardM2(addr, 100); // Left On
						Sleep(400);

						//Straight
						roboclaw->ForwardM2(addr, 0); // Right Off
						roboclaw->BackwardM2(addr, 0); // Left On
						Sleep(100);
					}
				}
			}
			else {
				// stop
				if (MotorVelocity != 0) {
					MotorVelocity = 0;
					roboclaw->ForwardM1(addr, MotorVelocity);
				}
			}

			if (arrived != NULL) {
				delete arrived;
				arrived = NULL;
			}

			cv::waitKey(30);
		}


			/*
		// Getting Route Segments
		while (1)
		{
			std::vector<Coord> list = server.GetSegments();
			if (list.size() > 0) break;
			Sleep(1000);
		}
		print(fp, "Get Route Segments!!!\n");
		std::vector<Coord> list = server.GetSegments();


		//Position Initialization
		newCurrentPos = server.GetCurrentPosition();
		oldCurrentPos = newCurrentPos;
		print(fp, "newCurrentPos (x,y) : %f, %f \n", newCurrentPos.x, newCurrentPos.y);

		//Direction Initialization
		nextDirection = direction(newCurrentPos, list[nextPoint]);
		print(fp, "Direction to Next Segment (degree, 0~360) : %f \n", nextDirection);

		// MotorCommand[0] 
		// -1 : Emergency Stop
		// 0 : Stop
		// 1 : Go Forward Level 1
		// 2 : Go Forward Level 2
		// 3 : Go Forward level 3
		// 4 : wait for new command
		//
		// MotorCommand[1]
		// -1: wait for new command
		// 0 : Straight
		// 1 : Turn Left
		// 2 : --------
		// 3 : Turn Right
		// 4 : --------

		//test
		MotorCommand[0] = 2;

		//////////////////////////////////////////////////////////////
		////////////////////   RC car Controler   ////////////////////
		//////////////////////////////////////////////////////////////

		while (1) {
			
			if (motor1change)
			{
				roboclaw->ForwardM1(addr, MotorVelocity); // Go Forward
				motor1change = false;
				speedUp = 0;
			}
//			else
//			{
//				speedUp++;
//				if (speedUp > 20)
//				{
//					MotorCommand[0]++;
//					if (MotorCommand[0] == 4) MotorCommand[0] = 3;
//				}
//			}
			if (motor2change)
			{
				speedUp = 0;
				if (MotorRight > MotorLeft)	//Turn Right
				{
					roboclaw->BackwardM2(addr, MotorLeft); // Left Off
					roboclaw->ForwardM2(addr, MotorRight); // Right On
				}
				else						//Turn Left
				{
					roboclaw->ForwardM2(addr, MotorRight); // Right Off
					roboclaw->BackwardM2(addr, MotorLeft); // Left On
				}
				motor2change = false;
			}
			Sleep(500);		// "need to find the good number after a few trial."


			/////////// Control Command  /////////////
			// 1. GPS information
			newCurrentPos = server.GetCurrentPosition();
			print(fp, "newCurrentPos (x,y) : %f, %f \n", newCurrentPos.x, newCurrentPos.y);

			nextDistance = distanceCoords(newCurrentPos, list[nextPoint]);
			print(fp, "%f m left to Next Point \n", nextDistance);


			if (!sameCoord(newCurrentPos, oldCurrentPos))
			{
				currentDirection = direction(oldCurrentPos, newCurrentPos);
				print(fp, "Current Direction (degree, 0~360) : %f \n", currentDirection);

				oldCurrentPos = newCurrentPos;	//refresh currentPos

				errorRange = currentDirection - nextDirection;
				if (errorRange > 180) { errorRange = 360 - errorRange; }
				if (errorRange <-180) { errorRange = 360 + errorRange; }

				// error range = 45 degree
				if (errorRange > 45)
				{
					MotorCommand[1] = 3;		//turn right
					motor2change = true;
					print(fp, "Command : Turn RIGHT \n");
				}
				else if (errorRange < -45)
				{
					MotorCommand[1] = 1;		//turn left
					motor2change = true;
					print(fp, "Command : Turn LEFT \n");
				}
				else
				{
					MotorCommand[1] = 0;		//go straight
					motor2change = true;
					print(fp, "Command : Go Straight \n");
				}
			}

			// 2. ZED camera information
			RenderResult rr = c.Render();
			if (rr.center) {
				printf("Object in center position \n");
			}



			// 3. Generate Final Command






			// 4. Move on to the NextPoint
			while (nextDistance < 10)
			{
				nextPoint++;
				//motor1change = true;
				print(fp, " !!!!!!!!!!!! GET TO THE NEXT SEGMENT NOW!!!!!!!!!!!!!!!!!!\n");

				nextDirection = direction(newCurrentPos, list[nextPoint]);
				print(fp, "New Direction to Next Segment (degree, 0~360) : %f \n", nextDirection);

				nextDistance = distanceCoords(newCurrentPos, list[nextPoint]);
				print(fp, "%f m left to Next Point \n", nextDistance);
			}

////////// Apply MotorCommand /////////////
			////////////// Go Forward ////////////
			switch (MotorCommand[0])
			{
				case -1:		//Velocity = 0
					MotorVelocity = 0;
					motor1change = true;
					MotorCommand[0] = 0;
					break;

				case 0:		//velocity to 0
					if (MotorVelocity > 0) { MotorVelocity -= 5; motor1change = true;}
					if (MotorVelocity < 0) { MotorVelocity = 0; motor1change = true; }
					break;

				case 1:		//velocity to 40			
					if (MotorVelocity > 40) { MotorVelocity -= 5;  motor1change = true; }
					if (MotorVelocity < 40) { MotorVelocity += 5;  motor1change = true; }
					break;

				case 2:		//velocity to 70
					if (MotorVelocity > 70) { MotorVelocity -= 5;  motor1change = true; }
					if (MotorVelocity < 70) { MotorVelocity += 5;  motor1change = true; }
					break;

				case 3:		//velocity to 100
					if (MotorVelocity > 100) { MotorVelocity -= 5;  motor1change = true; }
					if (MotorVelocity < 100) { MotorVelocity += 5;  motor1change = true; }
					break;
			}

			////////////// Direction //////////////
			switch (MotorCommand[1])
			{
				case -1:		//wait for the next Command
					motor2change = false;
					break;

				case 0:		//Go Straight
					MotorRight = 0;
					MotorLeft = 0;
					motor2change = true;
					MotorCommand[1] = -1;
					break;

				case 1:		//Turn Left
					MotorLeft = 65;
					MotorRight = 0;
					MotorCommand[1] = 2;
					motor2change = true;
					break;

				case 2:		//Turn Left and go straight again
					MotorLeft = 0;
					MotorRight = 60;
					MotorCommand[1] = 0;
					motor2change = true;
					break;

				case 3:		//Turn Right
					MotorLeft = 0;
					MotorRight = 65;
					MotorCommand[1] = 4;
					motor2change = true;
					break;

				case 4:		//Turn Right and go straight again
					MotorLeft = 60;
					MotorRight = 0;
					MotorCommand[1] = 0;
					motor2change = true;
					break;
			}
			//cv::waitKey(30);
		}//END of While Loop
	*/
	}
	else {
		std::cout << "RoboClaw not found" << std::endl;
	}

	delete mq;
	delete roboclaw;
	// delete camera;

	return 0;
}