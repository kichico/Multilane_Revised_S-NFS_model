#include "Multilane_TrafficFlow.h"

void Multilane_TrafficFlow::Calculation(int lanelength, int Numberofvehicle, int Numberoflane) {
	int Crate = 0;
	double Cp = (double)Crate / 100;
	int filenumber = 1;
	int ave = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) num_threads(4)
#endif // _OPENMP
	for (int i = 75; i <= Numberofvehicle; i+=75) {
		int CCar = (int)std::round(Cp * i);
		Update_Position::Measuredinfomation AllResult;
		Update_Position* DoSim = new Update_Position(filenumber, lanelength, i, Numberoflane, CCar);
		for (int j = 0; j < 1800; ++j) {
			_DoSimulation(DoSim);
			AllResult += DoSim->AddThisTimeResult();
		}
		DoSim->DoMeasure = true;
		for (int j = 0; j < 300; ++j) {
			_DoSimulation(DoSim);
			AllResult += DoSim->AddThisTimeResult();
		}
#ifdef _OPENMP
#pragma omp critical
#endif // _OPENMP
		{
			std::cout << (double)i / (Numberoflane * lanelength) << "=>" << AllResult.passed << "," << AllResult.NumberofLanechange << std::endl;
		}
		delete DoSim;
	}
}

void Multilane_TrafficFlow::_DoSimulation(Update_Position* DoSim) {
	DoSim->Decide_targetvelocity();
	DoSim->car.Fromcurrent_toprevious();
	DoSim->map.Fromcurrent_toprevious();
	DoSim->TurnonLaneChangersSignal();
	DoSim->Update_EachVehiclePosition();
	if (DoSim->TryLaneChange()) DoSim->Update_EachVehiclePosition();
}