#include "Lane_Change.h"

void Lane_Change::TurnonLaneChangersSignal() {
	Lanechanger = std::vector<LaneChangerInformation>(0);
	for (int i = 0; i < car.List_Defector.size(); ++i) {
		int ID = car.List_Defector[i];
		InsentiveInformation right, left;
		LaneChangerInformation changer;
		left = _CheckInsentives(ID, Car_information::SignalKind::Left);
		right = _CheckInsentives(ID, Car_information::SignalKind::Right);
		if (left.distance > right.distance) changer.signal = Car_information::SignalKind::Left;
		else if (left.distance < right.distance) changer.signal = Car_information::SignalKind::Right;
		else {
			if (random->random(1.0) < 0.5)  changer.signal = Car_information::SignalKind::Left;
			else changer.signal = Car_information::SignalKind::Right;
		}
		if (changer.signal != Car_information::SignalKind::Non) Lanechanger.emplace_back(changer);
	}
}

bool Lane_Change::TryLaneChange() {

}

Lane_Change::InsentiveInformation Lane_Change::_CheckInsentives(int ID, int signal) {
	InsentiveInformation check;
	CanditateAroundVehicle Around;
	check.distance = 0;
	check.on = false;
	int FocalLane = car.lanenumber.current[ID];
	int precedingcarID = car.around.preceding.current[ID];
	if (FocalLane == 0 && signal == Car_information::SignalKind::Left) return check;
	else if (FocalLane == constants.Numberoflane - 1 && Car_information::SignalKind::Right) return check;
	else {
		FocalLane++;
		if (signal == Car_information::SignalKind::Left) FocalLane -= 2;
		if (map.recorded.existence.current[FocalLane][car.position.current[ID]]) return check;
		if (car.headway.current[ID] > car.velocity.current[ID] - car.velocity.current[precedingcarID]) return check;
		Around = _GetAroundInformation(ID, FocalLane);
		if (Around.preceding.distance > car.velocity.current[ID] - car.velocity.current[Around.preceding.ID]) {
			check.on = true;
			check.distance = Around.preceding.distance;
		}
	}
	return check;
}

Lane_Change::CanditateAroundVehicle Lane_Change::_GetAroundInformation(int ID, int FocalLane) {

}