#include "Update_Position.h"

Update_Position::Update_Position(int filenumber, int lanelength, int Numberofvehicle, int Numberoflane, int Cooperator) {
	this->constants.setConstants(lanelength, Numberoflane, Numberofvehicle);
	this->InitializeEachSettings(Numberofvehicle, Numberoflane, lanelength, Cooperator);
	random = new Random();
}

Update_Position::~Update_Position() {
	delete random;
}

void Update_Position::Update_EachVehiclePosition() {
	for (int lanenumber = 0; lanenumber < constants.Numberoflane; ++lanenumber) {
		int leaderID;
		bool isThereLeadingvehicle = car.leadingvehicle[lanenumber].existence;
		if (!isThereLeadingvehicle) continue;
		else if (isThereLeadingvehicle) {
			lanevelocity = 0;
			leaderID = car.leadingvehicle[lanenumber].ID;
			bool isUpdateComplete = false;
			while (!isUpdateComplete) isUpdateComplete = _UpdateEachLane(leaderID);
		}
	}
}

bool Update_Position::_UpdateEachLane(int leaderID) {
	int focalcarID = leaderID;
	int lane = car.lanenumber.current[focalcarID];
	Map_information::update_information updated;
	updated.existence = map.recorded.existence.current;
	updated.ID = map.recorded.ID.current;
	bool isUpdateComplete = true;
	while (true) {
		int precedingcarID = car.around.preceding.current[focalcarID];
		int position = car.position.current[focalcarID];
		updated.existence[lane][position] = false;
		updated.ID[lane][position] = NULL;
		int RemainCelltoMove = car.canditate_velocity[focalcarID];
		for (int i = 0; i < RemainCelltoMove; ++i) {
			position++;
			if (position >= constants.lanelength) position -= constants.lanelength;
			if (updated.existence[lane][position]) {
				position--;
				if (position < 0) position += constants.lanelength;
				break;
			}
		}
		int moved = position - car.position.current[focalcarID];
		if (moved < 0) moved += constants.lanelength;
		car.canditate_velocity[focalcarID] -= moved;
		car.position.current[focalcarID] = position;
		car.velocity.current[focalcarID] = position - car.position.previous[focalcarID];
		if (car.velocity.current[focalcarID] < 0) car.velocity.current[focalcarID] += constants.lanelength;
		int  headway = car.position.current[precedingcarID] - car.position.current[focalcarID];
		if (headway < 0) headway += constants.lanelength;
		car.headway.current[focalcarID] = headway;
		map.lanevelocity += car.velocity.current[focalcarID];
		updated.existence[lane][position] = true;
		updated.ID[lane][position] = focalcarID;
		focalcarID = car.around.preceding.current[focalcarID];
		if (focalcarID == leaderID) break;
	}
	map.recorded.existence.current = updated.existence;
	map.recorded.ID.current = updated.ID;
	if (lanevelocity != map.lanevelocity) {
		lanevelocity = map.lanevelocity;
		map.lanevelocity = 0;
		isUpdateComplete = false;
	}
	else isUpdateComplete = true;
	return isUpdateComplete;
}

Update_Position::Measuredinfomation Update_Position::AddThisTimeResult() {
	MeasuredThisTime.average_velocity = 0;
	MeasuredThisTime.passed = 0;
	if (DoMeasure) MeasuredThisTime = _Measure(MeasuredThisTime);
	return MeasuredThisTime;
}

Update_Position::Measuredinfomation Update_Position::_Measure(Measuredinfomation& MeasuredThisTime) {
	for (int lanenumber = 0; lanenumber < constants.Numberoflane; ++lanenumber) for (int i = 0; i <= 6; ++i) {
		if (map.recorded.existence.current[lanenumber][MeasuringPoint + i]) {
			int ID = map.recorded.ID.current[lanenumber][MeasuringPoint + i];
			if (car.position.current[ID] >= MeasuringPoint && car.position.previous[ID] < MeasuringPoint) {
				MeasuredThisTime.average_velocity += car.velocity.current[ID];
				MeasuredThisTime.passed++;
				//if (car.lanenumber.current[ID] != car.lanenumber.previous[ID]) MeasuredThisTime.NumberofLanechange++;
			}
		}
	}
	return MeasuredThisTime;
}

