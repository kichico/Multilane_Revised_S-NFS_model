#pragma once
#include "Decide_Velocity.h"
#include <algorithm>
#include <set>

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
	struct PushedVehicleInformation {
		CanditateAroundVehicle around;
		LaneChangerInformation info;
	};
	struct BothLaneChange {
		LaneChangerInformation info;
		bool isPushed;
	};
	CanditateAroundVehicle _GetAroundInformation(int ID, int FocalLane);
	InsentiveInformation _CheckInsentives(int ID, int signal);
	std::vector<LaneChangerInformation> _DecideUpdateOrder();
	std::vector<CanditateAroundVehicle::Detected> CanditateLeadingCar;
	void _UpdateRelationship(LaneChangerInformation LI,CanditateAroundVehicle around, bool beforeLaneChange);
	std::vector<PushedVehicleInformation> Pushed;
	std::vector<BothLaneChange> TotalLaneChanger;
	std::set<int> AlreadyPicked;
public:
	std::vector<LaneChangerInformation> Lanechanger;
	void TurnonLaneChangersSignal();
	bool TryLaneChange();
	void CheckPushed();
	void PickUpPushed();
	bool TryBothLaneChangeRule();
};

