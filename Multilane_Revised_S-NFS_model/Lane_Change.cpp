#include "Lane_Change.h"

void Lane_Change::CheckPushed() {
	Pushed = std::vector<PushedVehicleInformation>(0);
	//At first, checking every vehicle about whether backward pushing criteria is met or not
	PushedVehicleInformation PushedVehicle;
	BothLaneChange both;
	for (int i = 0; i < constants.N; ++i) {
		int followerID = car.around.following.current[i];
		int distance = car.headway.current[followerID];
		bool isFocalVehiclePushed = false;
		if (car.velocity.current[i] - car.velocity.current[followerID] < 0) isFocalVehiclePushed = true;
		//If Following Vehicle is approaching focal vehicle and he is faster than focal, focakl vehicle feel "Pushed"
		if (distance <= constants.G && isFocalVehiclePushed) {
			PushedVehicle.info.ID = i;
			PushedVehicle.info.position = car.position.current[i];
			PushedVehicle.info.signal = Car_information::SignalKind::Non;
			int FocalLane = car.lanenumber.current[i];
			CanditateAroundVehicle right, left;
			if (0 < FocalLane && FocalLane < constants.Numberoflane - 1) {
				int NextLane = FocalLane + 1;
				right = _GetAroundInformation(i, NextLane);
				NextLane = FocalLane - 1;
				left = _GetAroundInformation(i, NextLane);
				if (right.following.distance > left.following.distance) PushedVehicle.info.signal = Car_information::SignalKind::Right;
				else if (right.following.distance < left.following.distance) PushedVehicle.info.signal = Car_information::SignalKind::Left;
				else {
					if (random->random(1.0) < 0.5) PushedVehicle.info.signal = Car_information::SignalKind::Left;
					else PushedVehicle.info.signal = Car_information::SignalKind::Right;
				}
			}
			else if (FocalLane == 0) {
				int NextLane = FocalLane + 1;
				right = _GetAroundInformation(i, NextLane);
				if (right.following.distance > 0) PushedVehicle.info.signal = Car_information::SignalKind::Right;
				else PushedVehicle.info.signal = Car_information::SignalKind::Non;
			}
			else {
				int NextLane = FocalLane - 1;
				left = _GetAroundInformation(i, NextLane);
				if (left.following.distance > 0) PushedVehicle.info.signal = Car_information::SignalKind::Left;
				else PushedVehicle.info.signal = Car_information::SignalKind::Non;
			}
			if (PushedVehicle.info.signal != Car_information::SignalKind::Non) {
				if (PushedVehicle.info.signal == Car_information::SignalKind::Left) PushedVehicle.around = left;
				if (PushedVehicle.info.signal == Car_information::SignalKind::Right) PushedVehicle.around = right;
				Pushed.emplace_back(PushedVehicle);
			}
		}
	}
	if (Pushed.size() > 0) {
		PushedVehicleInformation LI;
		for (int i = 0; i < Pushed.size(); ++i) {
			LI = Pushed[i];
			LI.info.position = car.position.current[LI.info.ID];
			int FocalLane = car.lanenumber.current[LI.info.ID];
			int NextLane = FocalLane + 1;
			if (LI.info.signal == Car_information::SignalKind::Left) NextLane -= 2;
			//If there is another vehicle at NextLane'S same position, LaneChange won't be done
			if (map.recorded.existence.current[NextLane][LI.info.position]) continue;
			bool beforeLaneChange = true;
			LI.around = _GetAroundInformation(LI.info.ID, NextLane);
			if (LI.around.following.distance >= car.canditate_velocity[LI.around.following.ID] - car.canditate_velocity[LI.info.ID]
				&& LI.around.preceding.distance >= car.canditate_velocity[LI.info.ID] - car.canditate_velocity[LI.around.preceding.ID]) {
				++MeasuredThisTime.Lanechange_Pushed;
				--map.eachlanevehicle[FocalLane];
				map.recorded.existence.current[FocalLane][LI.info.position] = false;
				if (map.eachlanevehicle[FocalLane] > 0) _UpdateRelationship(LI.info, LI.around, beforeLaneChange);
				else car.leadingvehicle[FocalLane].existence = false;
				beforeLaneChange = false;
				car.lanenumber.current[LI.info.ID] = NextLane;
				++map.eachlanevehicle[NextLane];
				map.recorded.existence.current[NextLane][LI.info.position] = true;
				map.recorded.ID.current[NextLane][LI.info.position] = LI.info.ID;
				_UpdateRelationship(LI.info, LI.around, beforeLaneChange);
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
	}
}

void Lane_Change::PickUpPushed() {
	TotalLaneChanger = std::vector<BothLaneChange>(0);
	Pushed = std::vector<PushedVehicleInformation>(0);
	AlreadyPicked.clear();
	PushedVehicleInformation PushedVehicle;
	BothLaneChange both;
	for (int i = 0; i < constants.N; ++i) {
		int followerID = car.around.following.current[i];
		int distance = car.headway.current[followerID];
		bool isFocalVehiclePushed = false;
		if (car.velocity.current[i] - car.velocity.current[followerID] < 0) isFocalVehiclePushed = true;
		//If Following Vehicle is approaching focal vehicle and he is faster than focal, focakl vehicle feel "Pushed"
		if (distance <= constants.G && isFocalVehiclePushed) {
			PushedVehicle.info.ID = i;
			PushedVehicle.info.position = car.position.current[i];
			PushedVehicle.info.signal = Car_information::SignalKind::Non;
			int FocalLane = car.lanenumber.current[i];
			CanditateAroundVehicle right, left;
			if (0 < FocalLane && FocalLane < constants.Numberoflane - 1) {
				int NextLane = FocalLane + 1;
				right = _GetAroundInformation(i, NextLane);
				NextLane = FocalLane - 1;
				left = _GetAroundInformation(i, NextLane);
				if (right.following.distance > left.following.distance) PushedVehicle.info.signal = Car_information::SignalKind::Right;
				else if (right.following.distance < left.following.distance) PushedVehicle.info.signal = Car_information::SignalKind::Left;
				else {
					if (random->random(1.0) < 0.5) PushedVehicle.info.signal = Car_information::SignalKind::Left;
					else PushedVehicle.info.signal = Car_information::SignalKind::Right;
				}
			}
			else if (FocalLane == 0) {
				int NextLane = FocalLane + 1;
				right = _GetAroundInformation(i, NextLane);
				if (right.following.distance > 0) PushedVehicle.info.signal = Car_information::SignalKind::Right;
				else PushedVehicle.info.signal = Car_information::SignalKind::Non;
			}
			else {
				int NextLane = FocalLane - 1;
				left = _GetAroundInformation(i, NextLane);
				if (left.following.distance > 0) PushedVehicle.info.signal = Car_information::SignalKind::Left;
				else PushedVehicle.info.signal = Car_information::SignalKind::Non;
			}
			if (PushedVehicle.info.signal != Car_information::SignalKind::Non) {
				if (PushedVehicle.info.signal == Car_information::SignalKind::Left) PushedVehicle.around = left;
				if (PushedVehicle.info.signal == Car_information::SignalKind::Right) PushedVehicle.around = right;
				both.info = PushedVehicle.info;
				both.isPushed = true;
				Pushed.emplace_back(PushedVehicle);
				AlreadyPicked.insert(both.info.ID);
				TotalLaneChanger.emplace_back(both);
			}
		}
	}
}

void Lane_Change::TurnonLaneChangersSignal() {
	//std::cout << "-----------------------------------" << std::endl;
	Lanechanger = std::vector<LaneChangerInformation>(0);
	CanditateLeadingCar = std::vector<CanditateAroundVehicle::Detected>(constants.Numberoflane);
	MeasuredThisTime.Lanechange_original = 0;
	MeasuredThisTime.Lanechange_Pushed = 0;
	for (int i = 0; i < constants.Numberoflane; ++i) {
		CanditateLeadingCar[i].ID = -1;
		CanditateLeadingCar[i].distance = -1;
	}
	LaneChangerInformation changer;
	BothLaneChange both;
	for (int i = 0; i < car.List_Defector.size(); ++i) {
		changer.signal = Car_information::SignalKind::Non;
		int ID = car.List_Defector[i];
		if (AlreadyPicked.count(ID) != 0) continue;
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
				int v = car.lanenumber.current[changer.ID];
				Lanechanger.emplace_back(changer);
				both.info = changer;
				both.isPushed = false;
				TotalLaneChanger.emplace_back(both);
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
				++MeasuredThisTime.Lanechange_original;
				--map.eachlanevehicle[FocalLane];
				map.recorded.existence.current[FocalLane][LI.position] = false;
				map.recorded.ID.current[FocalLane][LI.position] = -1;
				if (map.eachlanevehicle[FocalLane] > 0) _UpdateRelationship(LI, around, beforeLaneChange);
				else car.leadingvehicle[FocalLane].existence = false;
				beforeLaneChange = false;
				car.lanenumber.current[LI.ID] = NextLane;
				++map.eachlanevehicle[NextLane];
				map.recorded.existence.current[NextLane][LI.position] = true;
				map.recorded.ID.current[NextLane][LI.position] = LI.ID;
				_UpdateRelationship(LI, around, beforeLaneChange);
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

bool Lane_Change::TryBothLaneChangeRule() {
	bool isLaneChangeDone = false;
	if (TotalLaneChanger.size() > 0) {
		//Lanechanger = _DecideUpdateOrder();
		std::vector<BothLaneChange> Order;
		std::vector<CanditateAroundVehicle::Detected> TrueCanditateLeadingVehicle;
		for (int i = 0; i < constants.Numberoflane; ++i) {
			if (CanditateLeadingCar[i].ID != -1) TrueCanditateLeadingVehicle.emplace_back(CanditateLeadingCar[i]);
		}
		int picker;
		int leaderID;
		if (TrueCanditateLeadingVehicle.empty()) {
			picker = 0;
			leaderID = TotalLaneChanger[picker].info.ID;
		}
		else {
			picker = random->random((int)TrueCanditateLeadingVehicle.size() - 1);
			leaderID = TrueCanditateLeadingVehicle[picker].ID;
		}
		int StartingPosition = car.position.current[leaderID];
		BothLaneChange changer;
		for (int i = 0; i < TotalLaneChanger.size(); ++i) {
			changer = TotalLaneChanger[i];
			changer.info.position = car.position.current[changer.info.ID];
			if (changer.info.position <= StartingPosition) changer.info.position += constants.lanelength - StartingPosition - 1;
			else changer.info.position -= StartingPosition + 1;
			Order.emplace_back(changer);
		}
		std::sort(Order.begin(), Order.end(), [](BothLaneChange const& fr, BothLaneChange const& se) {
			return fr.info.position > se.info.position;
		});
		TotalLaneChanger = Order;
		BothLaneChange LI;
		CanditateAroundVehicle around;
		for (int i = 0; i < TotalLaneChanger.size(); ++i) {
			LI = TotalLaneChanger[i];
			LI.info.position = car.position.current[LI.info.ID];
			int FocalLane = car.lanenumber.current[LI.info.ID];
			int NextLane = FocalLane + 1;
			if (LI.info.signal == Car_information::SignalKind::Left) NextLane -= 2;
			if (map.recorded.existence.current[NextLane][LI.info.position]) continue;
			around = _GetAroundInformation(LI.info.ID, NextLane);
			bool beforeLaneChange = true;
			if (around.following.distance >= car.velocity.current[around.following.ID] - car.velocity.current[LI.info.ID]) {
				isLaneChangeDone = true;
				if (!LI.isPushed)++MeasuredThisTime.Lanechange_original;
				else ++MeasuredThisTime.Lanechange_Pushed;
				--map.eachlanevehicle[FocalLane];
				map.recorded.existence.current[FocalLane][LI.info.position] = false;
				if (map.eachlanevehicle[FocalLane] > 0) _UpdateRelationship(LI.info, around, beforeLaneChange);
				else car.leadingvehicle[FocalLane].existence = false;
				beforeLaneChange = false;
				car.lanenumber.current[LI.info.ID] = NextLane;
				++map.eachlanevehicle[NextLane];
				map.recorded.existence.current[NextLane][LI.info.position] = true;
				map.recorded.ID.current[NextLane][LI.info.position] = LI.info.ID;
				_UpdateRelationship(LI.info, around, beforeLaneChange);
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
		int newHeadway = car.position.current[Preceding] - car.position.current[Following] - 1;
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