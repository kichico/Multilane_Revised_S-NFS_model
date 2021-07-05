#include "Multilane_TrafficFlow.h"

void Multilane_TrafficFlow::Calculation(int filenumber,int lanelength, int Numberofvehicle, int Numberoflane) {
	int Crate = 0;
	double Cp = (double)Crate / 100;
	int ave = 0;
	std::string filename = "Global" + std::to_string(filenumber) + "_Crate" + std::to_string(Crate) + "_.dat";
	std::ofstream ofs(filename);
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) num_threads(3)
#endif // _OPENMP
	for (int i = 75; i <= Numberofvehicle; i += 75) {
		int CCar = (int)std::round(Cp * i);
		Update_Position::Measuredinfomation AllResult;
		Update_Position* DoSim = new Update_Position(filenumber, lanelength, i, Numberoflane, CCar);
		for (int j = 0; j < 1800; ++j) {
			_DoSimulation(DoSim);
			AllResult += DoSim->AddThisTimeResult();
		}
		//std::cout << "1:" << DoSim->map.eachlanevehicle[0] << ",2:" << DoSim->map.eachlanevehicle[1] << ",3:" << DoSim->map.eachlanevehicle[2] << std::endl;
		DoSim->DoMeasure = true;
		for (int j = 0; j < 300; ++j) {
			_DoSimulation(DoSim);
			AllResult += DoSim->AddThisTimeResult();
			//std::cout << "Number of Passed:" << AllResult.passed << std::endl;
		}
		//std::cout << "1:" << DoSim->map.eachlanevehicle[0] << ",2:" << DoSim->map.eachlanevehicle[1] << ",3:" << DoSim->map.eachlanevehicle[2] << std::endl;
#ifdef _OPENMP
#pragma omp critical
#endif // _OPENMP
		{
			//std::cout << (double)i / (Numberoflane * lanelength) << "=>" << AllResult.passed << "," << AllResult.NumberofLanechange << std::endl;
			ofs << i << "," << (double)i / Numberofvehicle << "," << (double)AllResult.passed / 3 << std::endl;
			std::cout << i << "," << (double)i / Numberofvehicle << "," << (double)AllResult.passed / 3 << std::endl;
		}
		delete DoSim;
	}
}

void Multilane_TrafficFlow::_DoSimulation(Update_Position* DoSim) {
	DoSim->Decide_targetvelocity();
	DoSim->car.Fromcurrent_toprevious();
	DoSim->map.Fromcurrent_toprevious();
	//DoSim->TurnonLaneChangersSignal();
	DoSim->CheckPushed();
	DoSim->Update_EachVehiclePosition();
	//if (DoSim->TryLaneChange()) DoSim->Update_EachVehiclePosition();
	//for (int i = 0; i < DoSim->constants.N; ++i) std::cout << i << "th v:" << DoSim->car.velocity.current[i] << std::endl;
}