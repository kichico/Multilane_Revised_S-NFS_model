#ifndef UPDATE_POSITION.H
#define UPDATE_POSITION.H
#include "Decide_Velocity.h"

class Update_Position : public Decide_Velocity {
public:
	Update_Position(int filenumber, int lanelength, int Numberofvehicle, int Numberoflane);
	~Update_Position();
	struct Measuredinfomation {
		int NumberofLanechange = 0;
		int passed = 0;
		double average_velocity = 0.0;
	};
	Measuredinfomation measured;
	bool DoMeasure = false;
	void Update_EachVehiclePosition();
	Measuredinfomation Update_PreviousInformation();
	int MeasuringPoint = 100;
private:
	bool _UpdateEachLane(int leaderID);
	Measuredinfomation _Measure(Measuredinfomation& MeasuredThistime);
};


#endif // !UPDATE_POSITION.H