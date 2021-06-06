#include "Update_Position.h"

Update_Position::Update_Position(int filenumber, int lanelength, int Numberofvehicle, int Numberoflane) {
	this->constants.setConstants(lanelength, Numberoflane, Numberofvehicle);
	this->InitializeEachSettings(Numberofvehicle, Numberoflane, lanelength);
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
		int position = car.position.current[focalcarID];
		updated.existence[lane][position] = false;
		updated.ID[lane][position] = NULL;
		int RemainCelltoMove = car.canditate_velocity[focalcarID];
		for (int i = 0; i < RemainCelltoMove; ++i) {
			position++;
			if (position >= constants.lanelength) position -= constants.lanelength;
			if (updated.existence[lane][position]) {
				position--;
				break;
			}
		}
		car.canditate_velocity[focalcarID] = position - car.position.current[focalcarID];
		if (car.canditate_velocity[focalcarID] != 0) isUpdateComplete = false;
		car.position.current[focalcarID] = position;
		car.velocity.current[focalcarID] = position - car.position.previous[focalcarID];
		updated.existence[lane][position] = true;
		updated.ID[lane][position] = focalcarID;
		focalcarID = car.around.preceding.current[focalcarID];
		if (focalcarID == leaderID) break;
	}
	map.recorded.existence.current = updated.existence;
	map.recorded.ID.current = updated.ID;
	return isUpdateComplete;
}

Update_Position::Measuredinfomation Update_Position::Update_PreviousInformation() {
	Measuredinfomation MeasuredThistime;
	MeasuredThistime.average_velocity = 0;
	MeasuredThistime.NumberofLanechange = 0;
	MeasuredThistime.passed = 0;
	car.Fromcurrent_toprevious();
	map.Fromcurrent_toprevious();
	if (DoMeasure) return _Measure(MeasuredThistime);
	else return MeasuredThistime;
}

Update_Position::Measuredinfomation Update_Position::_Measure(Measuredinfomation& MeasuredThistime) {
	for (int lanenumber = 0; lanenumber < constants.Numberoflane; ++lanenumber) for (int i = 1; i <= 5; ++i) {
		if (map.recorded.existence.current[lanenumber][MeasuringPoint + i]) {
			int ID = map.recorded.ID.current[lanenumber][MeasuringPoint + i];
			if (car.position.current[ID] >= MeasuringPoint && car.position.previous[ID] < MeasuringPoint) {
				MeasuredThistime.average_velocity += car.velocity.current[ID];
				MeasuredThistime.passed++;
				if (car.lanenumber.current[ID] != car.lanenumber.previous[ID]) MeasuredThistime.NumberofLanechange++;
			}
		}
	}
	return MeasuredThistime;
}

