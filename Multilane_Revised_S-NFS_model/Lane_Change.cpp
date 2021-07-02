#include "Lane_Change.h"

void Lane_Change::TurnonLaneChangersSignal() {
	//std::cout << "-----------------------------------" << std::endl;
	Lanechanger = std::vector<LaneChangerInformation>(0);
	CanditateLeadingCar = std::vector<CanditateAroundVehicle::Detected>(constants.Numberoflane);
	MeasuredThisTime.NumberofLanechange = 0;
	for (int i = 0; i < constants.Numberoflane; ++i) {
		CanditateLeadingCar[i].ID = -1;
		CanditateLeadingCar[i].distance = -1;
	}
	LaneChangerInformation changer;
	for (int i = 0; i < car.List_Defector.size(); ++i) {
		changer.signal = Car_information::SignalKind::Non;
		int ID = car.List_Defector[i];
		InsentiveInformation right, left;
		left = _CheckInsentives(ID, Car_information::SignalKind::Left);
		right = _CheckInsentives(ID, Car_information::SignalKind::Right);
		if (left.on == false && right.on == false) continue;
		if (left.on == true || right.on == true) {
			if (left.distance > right.distance) changer.signal = Car_information::SignalKind::Left;
			else if (left.distance < right.distance) changer.signal = Car_information::SignalKind::Right;
			else {
				if (random->random(1.0) < 0.5)  if (left.on) changer.signal = Car_information::SignalKind::Left;
				else if (right.on) changer.signal = Car_information::SignalKind::Right;
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
}

bool Lane_Change::TryLaneChange() {
	bool isLaneChangeDone = false;
	if (Lanechanger.size() > 0) {
		Lanechanger = _DecideUpdateOrder();
		LaneChangerInformation LI;
		CanditateAroundVehicle around;
		std::vector<LaneChangerInformation> debug;
		//for (auto x : Lanechanger) std::cout << "pos:" << x.position << " velocity:" << car.velocity.current[x.ID] << std::endl;
		//getchar();
		for (int i = 0; i < Lanechanger.size(); ++i) {
			LI = Lanechanger[i];
			LI.position = car.position.current[LI.ID];
			int FocalLane = car.lanenumber.current[LI.ID];
			int NextLane = FocalLane + 1;
			if (LI.signal == Car_information::SignalKind::Left) NextLane -= 2;
			//If there is another vehicle at NextLane'S same position, LaneChange won't be done
			if (map.recorded.existence.current[NextLane][LI.position]) continue;
			around = _GetAroundInformation(LI.ID, NextLane);
			bool beforeLaneChange = true;
			if (around.following.distance >= car.velocity.current[around.following.ID] - car.velocity.current[LI.ID]) {
				isLaneChangeDone = true;
				++MeasuredThisTime.NumberofLanechange;
				--map.eachlanevehicle[FocalLane];
				map.recorded.existence.current[FocalLane][LI.position] = false;
				if (map.eachlanevehicle[FocalLane] > 0) _UpdateRelationship(LI, around, beforeLaneChange);
				else car.leadingvehicle[FocalLane].existence = false;
				beforeLaneChange = false;
				car.lanenumber.current[LI.ID] = NextLane;
				++map.eachlanevehicle[NextLane];
				map.recorded.existence.current[NextLane][LI.position] = true;
				map.recorded.ID.current[NextLane][LI.position] = LI.ID;
				_UpdateRelationship(LI, around, beforeLaneChange);
				debug.emplace_back(LI);
			}
		}
		for (int Lane = 0; Lane < constants.Numberoflane; ++Lane) {
			if (map.eachlanevehicle[Lane] == 0) car.leadingvehicle[Lane].existence = false;
			else {
				Car_information::LeadingVehicle Leading;
				Leading = car.leadingvehicle[Lane];
				int ID = car.around.following.current[Leading.ID];
				while (ID != Leading.ID) {
					if (Leading.distance < car.headway.current[ID]) {
						Leading.distance = car.headway.current[ID];
						Leading.ID = ID;
					}
					ID = car.around.following.current[ID];
				}
			}
		}
		return isLaneChangeDone;
	}
	else return isLaneChangeDone;
}

Lane_Change::InsentiveInformation Lane_Change::_CheckInsentives(int ID, int signal) {
	InsentiveInformation check;
	CanditateAroundVehicle Around;
	check.distance = 0;
	check.on = false;
	int FocalLane = car.lanenumber.current[ID];
	int precedingcarID = car.around.preceding.current[ID];
	if (FocalLane == 0 && signal == Car_information::SignalKind::Left) return check;
	else if (FocalLane == constants.Numberoflane - 1 && signal == Car_information::SignalKind::Right) return check;
	else {
		FocalLane++;
		if (signal == Car_information::SignalKind::Left) FocalLane -= 2;
		if (map.recorded.existence.current[FocalLane][car.position.current[ID]]) return check;
		if (car.headway.current[ID] > car.velocity.current[ID] - car.velocity.current[precedingcarID]) return check;
		Around = _GetAroundInformation(ID, FocalLane);
		if (Around.preceding.distance > car.velocity.current[ID] - car.velocity.current[Around.preceding.ID] && Around.preceding.distance > 0) {
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
					around.preceding.distance = PrecedingPosition - car.position.current[ID] - 1;
					if (around.preceding.distance < 0) around.preceding.distance += constants.lanelength;
					around.following.ID = car.around.following.current[around.preceding.ID];
					around.following.distance = car.position.current[ID] - car.position.current[around.following.ID] - 1;
					if (around.following.distance < 0) around.following.distance += constants.lanelength;
					break;
				}
				else {
					around.following.ID = map.recorded.ID.current[FocalLane][FollowingPosition];
					around.following.distance = car.position.current[ID] - FollowingPosition - 1;
					if (around.following.distance < 0) around.following.distance += constants.lanelength;
					around.preceding.ID = car.around.preceding.current[around.following.ID];
					around.preceding.distance = car.position.current[around.preceding.ID] - car.position.current[ID] - 1;
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
	std::vector<CanditateAroundVehicle::Detected> TrueCanditateLeadingVehicle;
	for (int i = 0; i < constants.Numberoflane; ++i) {
		if (CanditateLeadingCar[i].ID != -1) TrueCanditateLeadingVehicle.emplace_back(CanditateLeadingCar[i]);
	}
	int picker = random->random((int)TrueCanditateLeadingVehicle.size() - 1);
	int leaderID = TrueCanditateLeadingVehicle[picker].ID;
	int StartingPosition = car.position.current[leaderID];
	LaneChangerInformation info;
	for (int i = 0; i < Lanechanger.size(); ++i) {
		info = Lanechanger[i];
		info.position = car.position.current[info.ID];
		if (info.position <= StartingPosition) info.position += constants.lanelength - StartingPosition - 1;
		else info.position -= StartingPosition + 1;
		Order.emplace_back(info);
	}
	std::sort(Order.begin(), Order.end(), [](LaneChangerInformation const& fr, LaneChangerInformation const& se) { 
		return fr.position > se.position;
	});
	return Order;
}


void Lane_Change::_UpdateRelationship(LaneChangerInformation LI, CanditateAroundVehicle around, bool beforeLaneChange) {
	int FocalLane = car.lanenumber.current[LI.ID];
	if (beforeLaneChange) {
		int Preceding = car.around.preceding.current[LI.ID];
		int Following = car.around.following.current[LI.ID];
		int newHeadway = car.position.current[Preceding] - car.position.current[Following] + 1;
		if (newHeadway < 0) newHeadway += constants.lanelength;
		car.around.following.current[Preceding] = Following;
		car.around.preceding.current[Following] = Preceding;
		car.headway.current[Following] = newHeadway;
		if (LI.ID == car.leadingvehicle[FocalLane].ID)  car.leadingvehicle[FocalLane].ID = Following;
	}
	else {
		car.around.following.current[LI.ID] = around.following.ID;
		car.around.preceding.current[LI.ID] = around.preceding.ID;
		car.around.preceding.current[around.following.ID] = car.around.following.current[around.preceding.ID] = LI.ID;
		car.headway.current[LI.ID] = around.preceding.distance;
		car.headway.current[around.following.ID] = around.following.distance;
	}
	if (map.eachlanevehicle[FocalLane] == 1) {
		car.around.following.current[LI.ID] = car.around.preceding.current[LI.ID] = LI.ID;
		car.headway.current[LI.ID] = constants.lanelength - 1;
		car.leadingvehicle[FocalLane].existence = true;
		car.leadingvehicle[FocalLane].ID = LI.ID;
	}
}

Lane_Change::LaneChangerInformation::LaneChangerInformation() {
	this->ID = -1;
	this->position = -1;
	this->signal = Car_information::SignalKind::Non;
}