#include "Multilane_TrafficFlow.h"

void Multilane_TrafficFlow::Calculation(int filenumber,int lanelength, int Numberofvehicle, int Numberoflane) {
	int Crate = 10;
	double Cp = (double)Crate / 100;
	int ave = 0;
	std::string filename = "Result/Global" + std::to_string(filenumber) + "_Crate" + std::to_string(Crate) + "_.dat";
	std::ofstream ofs(filename);
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif // _OPENMP
	for (int i = 75; i <= Numberofvehicle; i += 75) {
		int CCar = (int)std::round(Cp * i);
		Update_Position::Measuredinfomation AllResult;
		Update_Position::AverageVelocity AllAverageVelocity;
		AllAverageVelocity.init();
		Update_Position* DoSim = new Update_Position(filenumber, lanelength, i, Numberoflane, CCar);
		for (int j = 0; j < 1800; ++j) {
			//std::cout << j << "th step" << std::endl;
			
			_DoSimulation(DoSim);
			AllResult += DoSim->AddThisTimeResult();
		}
		DoSim->DoMeasure = true;
		for (int j = 0; j < 300; ++j) {
			
			_DoSimulation(DoSim);
			AllResult += DoSim->AddThisTimeResult();
			AllAverageVelocity += DoSim->AverageVelocityThisTime;
		}
#ifdef _OPENMP
#pragma omp critical
#endif // _OPENMP
		{
			ofs << i << "," << (double)i / Numberofvehicle << "," << (double)AllResult.passed / 3 << "," << AllResult.Lanechange_original << "," << AllResult.Lanechange_Pushed << ",";
			ofs << (double)AllAverageVelocity.Cooperator / (DoSim->Cooperator * 300) << "," << (double)AllAverageVelocity.Defector / (DoSim->Defector * 300) << std::endl;
			std::cout << i << "," << (double)i / Numberofvehicle << "," << (double)AllResult.passed / 3 << "," << AllResult.Lanechange_original << "," << AllResult.Lanechange_Pushed << ",";
			std::cout << (double)AllAverageVelocity.Cooperator / (DoSim->Cooperator * 300) << "," << (double)AllAverageVelocity.Defector / (DoSim->Defector * 300) << std::endl;
		}
		delete DoSim;
	}
}

void Multilane_TrafficFlow::_DoSimulation(Update_Position* DoSim) {
	DoSim->AverageVelocityThisTime.init();
	DoSim->Decide_targetvelocity();
	DoSim->car.Fromcurrent_toprevious();
	DoSim->map.Fromcurrent_toprevious();
	DoSim->InitializeLaneChangerInfomation();
	DoSim->PickUpPushed();
	DoSim->TurnonLaneChangersSignal();
	DoSim->Update_EachVehiclePosition();
	if (DoSim->TryLaneChange()) DoSim->Update_EachVehiclePosition();
	//for (int i = 0; i < DoSim->constants.N; ++i) std::cout << i << "th v:" << DoSim->car.velocity.current[i] << std::endl;
}
