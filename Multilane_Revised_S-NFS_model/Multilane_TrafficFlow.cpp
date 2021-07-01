#include "Multilane_TrafficFlow.h"

void Multilane_TrafficFlow::Calculation(int lanelength, int Numberofvehicle, int Numberoflane) {
	int Crate = 0;
	double Cp = (double)Crate / 100;
	int filenumber = 1;
	int ave = 0;
	std::string filename = "check.csv";
	std::ofstream ofs(filename);
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) num_threads(4)
#endif // _OPENMP
	//for (int i = 300; i <= Numberofvehicle; i += 75) {
	int i = 675;
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
		std::cout << "Number of Passed:" << AllResult.passed << std::endl;
		if (i >= 300) {
			double ave = 0;
			for (int ID = 0; ID < DoSim->constants.N; ++ID) {
				//std::cout << ID << "th current:" << DoSim->car.position.current[ID] << ", previous:" << DoSim->car.position.previous[ID] << std::endl;
				ave += DoSim->car.velocity.current[ID];
				if (j >= 200 && ID >=50) ofs << j << "," << DoSim->car.position.current[ID] << std::endl;
			}
			std::cout << "ave:" << ave / DoSim->constants.N << std::endl;
		}
	}
	//std::cout << "1:" << DoSim->map.eachlanevehicle[0] << ",2:" << DoSim->map.eachlanevehicle[1] << ",3:" << DoSim->map.eachlanevehicle[2] << std::endl;
#ifdef _OPENMP
#pragma omp critical
#endif // _OPENMP
	{
		std::cout << (double)i / (Numberoflane * lanelength) << "=>" << AllResult.passed << "," << AllResult.NumberofLanechange << std::endl;
	}
	delete DoSim;
}

void Multilane_TrafficFlow::_DoSimulation(Update_Position* DoSim) {
	DoSim->Decide_targetvelocity();
	DoSim->car.Fromcurrent_toprevious();
	DoSim->map.Fromcurrent_toprevious();
	DoSim->TurnonLaneChangersSignal();
	DoSim->Update_EachVehiclePosition();
	if (DoSim->TryLaneChange()) DoSim->Update_EachVehiclePosition();
	//for (int i = 0; i < DoSim->constants.N; ++i) std::cout << i << "th v:" << DoSim->car.velocity.current[i] << std::endl;
}