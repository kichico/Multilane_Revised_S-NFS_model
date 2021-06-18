#pragma once
#include "Lane_Change.h"

class Update_Position : public Lane_Change {
public:
	Update_Position(int filenumber, int lanelength, int Numberofvehicle, int Numberoflane, int Cooperator);
	~Update_Position();
	Measuredinfomation measured;
	bool DoMeasure = false;
	void Update_EachVehiclePosition();
	Measuredinfomation AddThisTimeResult();
private:
	int lanevelocity = 0;
	int MeasuringPoint = 100;
	bool _UpdateEachLane(int leaderID);
	Measuredinfomation _Measure(Measuredinfomation& MeasuredThistime);
};