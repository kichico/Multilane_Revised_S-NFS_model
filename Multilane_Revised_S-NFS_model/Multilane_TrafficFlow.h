#ifndef MULTILANE_TRAFFICFLOW.H
#define MULTILANE_TRAFFICFLOW.H
#include "Update_Position.h"

class Multilane_TrafficFlow :public Update_Position{
private:
	Measuredinfomation AllResult;
	void _DoSimulation();
public:
	void Calculation(int lanelength, int Numberofvehivle, int Numberoflane);
};


#endif // !MULTILANE_TRAFFICFLOW.H
