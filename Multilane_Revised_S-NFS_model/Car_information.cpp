#include "Car_information.h"

void Car_information::Initialize(int Numberofvehicle) {
	velocity.current = velocity.previous = canditate_velocity = std::vector<int>(Numberofvehicle, 0);
	position.current = position.previous = std::vector<int>(Numberofvehicle, 0);
	headway.current = headway.previous = std::vector<int>(Numberofvehicle, 0);
	lanenumber.current = lanenumber.previous = std::vector<int>(Numberofvehicle, 0);
	signal = std::vector<int>(Numberofvehicle, Car_information::SignalKind::Non);
	S = std::vector<int>(Numberofvehicle, 0);
}

void Car_information::Fromcurrent_toprevious() {
	velocity.previous = velocity.previous;
	position.previous = position.current;
	headway.previous = headway.current;
	lanenumber.previous = lanenumber.current;
	around.following.previous = around.following.current;
	around.preceding.previous = around.preceding.current;
}