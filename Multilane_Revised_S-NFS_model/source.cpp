#include "Multilane_TrafficFlow.h"

int main() {
	Multilane_TrafficFlow mt;
	int NumberofLane = 3;
	int Length = 1000;
	int N = NumberofLane * Length;
	mt.Calculation(Length, N, NumberofLane);
	return 0;
}