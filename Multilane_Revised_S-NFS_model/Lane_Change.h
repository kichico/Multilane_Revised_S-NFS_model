#pragma once
#include "Decide_Velocity.h"
#include <algorithm>

class Lane_Change :public Decide_Velocity{
private:
	struct InsentiveInformation {
		bool on;
		int distance;
	};
	struct LaneChangerInformation {
		int signal;
		int ID;
		int position;
		LaneChangerInformation();
	};
	struct CanditateAroundVehicle {
		struct Detected{
			int ID;
			int distance;
		};
		Detected preceding, following;
	};
	std::vector<LaneChangerInformation> Lanechanger;
	CanditateAroundVehicle _GetAroundInformation(int ID, int FocalLane);
	InsentiveInformation _CheckInsentives(int ID, int signal);
	std::vector<LaneChangerInformation> _DecideUpdateOrder();
	std::vector<CanditateAroundVehicle::Detected> CanditateLeadingCar;
	void _UpdateRelationship(LaneChangerInformation LI,CanditateAroundVehicle around, bool beforeLaneChange);
public:
	void TurnonLaneChangersSignal();
	bool TryLaneChange();
};

