#pragma once
#include "Decide_Velocity.h"

class Update_Position : public Decide_Velocity {
public:
	Update_Position(int filenumber, int lanelength, int Numberofvehicle, int Numberoflane, int Cooperator);
	~Update_Position();
	struct Measuredinfomation {
		int NumberofLanechange = 0;
		int passed = 0;
		double average_velocity = 0.0;
		Measuredinfomation operator+=(const Measuredinfomation &other) {
			this->average_velocity += other.average_velocity;
			this->NumberofLanechange += other.NumberofLanechange;
			this->passed += other.passed;
			return *this;
		}
	};
	Measuredinfomation measured;
	bool DoMeasure = false;
	void Update_EachVehiclePosition();
	Measuredinfomation Update_PreviousInformation();
	int MeasuringPoint = 100;
private:
	int lanevelocity = 0;
	bool _UpdateEachLane(int leaderID);
	Measuredinfomation _Measure(Measuredinfomation& MeasuredThistime);
};