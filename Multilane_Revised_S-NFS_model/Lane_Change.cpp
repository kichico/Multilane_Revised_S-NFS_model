#include "Lane_Change.h"

void Lane_Change::TurnonLaneChangersSignal() {
	Lanechanger = std::vector<LaneChangerInformation>(0);
	CanditateLeadingCar = std::vector<CanditateAroundVehicle::Detected>(constants.lanelength);
	MeasuredThisTime.NumberofLanechange = 0;
	for (int i = 0; i < constants.lanelength; ++i) {
		CanditateLeadingCar[i].ID = -1;
		CanditateLeadingCar[i].distance = -1;
	}
	for (int i = 0; i < car.List_Defector.size(); ++i) {
		int ID = car.List_Defector[i];
		InsentiveInformation right, left;
		LaneChangerInformation changer;
		left = _CheckInsentives(ID, Car_information::SignalKind::Left);
		right = _CheckInsentives(ID, Car_information::SignalKind::Right);
		if (left.distance == 0 && right.distance == 0) continue;
		if (left.distance > right.distance) changer.signal = Car_information::SignalKind::Left;
		else if (left.distance < right.distance) changer.signal = Car_information::SignalKind::Right;
		else {
			if (random->random(1.0) < 0.5)  changer.signal = Car_information::SignalKind::Left;
			else changer.signal = Car_information::SignalKind::Right;
		}
		if (changer.signal != Car_information::SignalKind::Non) {
			changer.ID = ID;
			changer.position = car.position.current[ID];
			Lanechanger.emplace_back(changer);
			if (CanditateLeadingCar[car.lanenumber.current[changer.ID]].distance < car.headway.current[changer.ID]) {
				CanditateLeadingCar[car.lanenumber.current[changer.ID]].ID = changer.ID;
				CanditateLeadingCar[car.lanenumber.current[changer.ID]].distance = car.headway.current[changer.ID];
			}
		}
	}
}

Lane_Change::LaneChangeThisTime Lane_Change::TryLaneChange() {
	if (Lanechanger.size() > 0) {
		Lanechanger = _DecideUpdateOrder();
		LaneChangerInformation LI;
		CanditateAroundVehicle around;
		for (int i = 0; i < Lanechanger.size(); ++i) {
			LI.ID = Lanechanger[i].ID;
			LI.position = car.position.current[LI.ID];
			LI.signal = Lanechanger[i].signal;
			int FocalLane = car.lanenumber.current[LI.ID];
			int NextLane = FocalLane + 1;
			if (LI.signal == Car_information::SignalKind::Left) FocalLane -= 2;
			if (map.recorded.existence.current[NextLane][LI.position]) continue;
			around = _GetAroundInformation(LI.ID, NextLane);
			if (around.following.distance >= car.velocity.current[around.following.ID] - car.velocity.current[LI.ID]) {
				
			}
		}
	}
	else return false;
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
	CanditateAroundVehicle around;
	if (map.eachlanevehicle[FocalLane] > 0) {
		int PrecedingPosition = car.position.current[ID] + 1;
		int FollowingPosition = car.position.current[ID] - 1;
		while (true) {
			if (PrecedingPosition >= constants.lanelength) PrecedingPosition -= constants.lanelength;
			if (FollowingPosition < 0) FollowingPosition += constants.lanelength;
			bool isPrecedingisDetected = map.recorded.existence.current[FocalLane][PrecedingPosition];
			bool isFollowingisDetected = map.recorded.existence.current[FocalLane][FollowingPosition];
			if (isPrecedingisDetected or isFollowingisDetected) {
				if (isPrecedingisDetected) {
					around.preceding.ID = map.recorded.ID.current[FocalLane][PrecedingPosition];
					around.preceding.distance = PrecedingPosition - car.position.current[ID];
					if (around.preceding.distance < 0) around.preceding.distance += constants.lanelength;
					around.following.ID = car.around.following.current[around.preceding.ID];
					around.following.distance = car.position.current[ID] - FollowingPosition;
					if (around.following.distance < 0) around.following.distance += constants.lanelength;
					break;
				}
				else {
					around.following.ID = map.recorded.ID.current[FocalLane][FollowingPosition];
					around.following.distance = car.position.current[ID] - FollowingPosition;
					if (around.following.distance < 0) around.following.distance += constants.lanelength;
					around.preceding.ID = car.around.preceding.current[around.following.ID];
					around.preceding.distance = PrecedingPosition - car.position.current[ID];
					if (around.preceding.distance < 0) around.preceding.distance += constants.lanelength;
					break;
				}
			}
			else {
				++PrecedingPosition;
				--FollowingPosition;
			}
		}
	}
	else {
		around.following.distance = constants.lanelength - 1;
		around.preceding.distance = constants.lanelength - 1;
		around.preceding.ID = around.following.ID = ID;
	}
	return around;
}

std::vector<Lane_Change::LaneChangerInformation> Lane_Change::_DecideUpdateOrder() {
	std::vector<LaneChangerInformation> Order;
	sort(CanditateLeadingCar.begin(), CanditateLeadingCar.end(), _CompareHeadway);
	int leaderID = CanditateLeadingCar[0].ID;
	int StartingPosition = car.position.current[leaderID];
	LaneChangerInformation info;
	for (int i = 0; i < Lanechanger.size(); ++i) {
		info.ID = Lanechanger[i].ID;
		info.position = car.position.current[info.ID];
		if (info.position <= StartingPosition) info.position += constants.lanelength - StartingPosition - 1;
		else info.position -= StartingPosition + 1;
		info.signal = Lanechanger[i].signal;
		Order.emplace_back(info);
	}
	sort(Order.begin(), Order.end(), _ComparePosition);
	return Order;
}

bool Lane_Change::_CompareHeadway(CanditateAroundVehicle::Detected& fr, CanditateAroundVehicle::Detected& se) {
	if (fr.distance != se.distance) return fr.distance > se.distance;
	else {
		if (random->random(1.0) < 0.5) return true;
		else false;
	}
}

bool Lane_Change::_ComparePosition(LaneChangerInformation& fr, LaneChangerInformation& se) {
	if (fr.position != se.position) return fr.position > se.position;
	else {
		if (random->random(1.0) < 0.5) return true;
		else false;
	}
}