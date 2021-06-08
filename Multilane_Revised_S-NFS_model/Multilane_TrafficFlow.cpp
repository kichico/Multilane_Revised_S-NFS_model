#include "Multilane_TrafficFlow.h"

void Multilane_TrafficFlow::Calculation(int lanelength, int Numberofvehicle, int Numberoflane) {
	std::string FileName = "Result.csv";
	std::ofstream ofs(FileName);
	int Crate = 0;
	double Cp = (double)Crate / 100;
	int filenumber = 1;
	ofs << "Number of Vehicle,flux" << std::endl;
	int ave = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) private(ave) num_threads(4)
#endif // _OPENMP
	for (int i = 1; i <= Numberofvehicle; i++) {
		int CCar = (int)std::round(Cp * i);
		ave = 0;
#pragma omp critical
		{
			std::cout << i << ",";
		}
		Update_Position::Measuredinfomation AllResult;
		Update_Position* DoSim = new Update_Position(filenumber, lanelength, i, Numberoflane, CCar);
		for (int j = 0; j < 1800; ++j) {
			_DoSimulation(DoSim);
			AllResult += DoSim->Update_PreviousInformation();
			//std::cout << j << std::endl;
		}
		DoSim->DoMeasure = true;
		for (int j = 0; j < 300; ++j) {
			_DoSimulation(DoSim);
			AllResult += DoSim->Update_PreviousInformation();
			for (int ID = 0; ID < i; ++ID) {
				ave += DoSim->car.velocity.current[ID];
			}
			//std::cout << j + 1800 << std::endl;
		}

#ifdef _OPENMP
#pragma omp critical
#endif // _OPENMP
		{
			ofs << i << "," << AllResult.passed << std::endl;
			std::cout << AllResult.passed << std::endl;
			std::cout << "Average Velocity:" << (double)ave / (i * 300) << std::endl;
		}
		delete DoSim;

	}
	ofs.close();
}

void Multilane_TrafficFlow::_DoSimulation(Update_Position* DoSim) {
	DoSim->Decide_targetvelocity();
	DoSim->car.Fromcurrent_toprevious();
	DoSim->map.Fromcurrent_toprevious();
	DoSim->Update_EachVehiclePosition();
}