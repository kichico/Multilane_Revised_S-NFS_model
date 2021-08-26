#include "Multilane_TrafficFlow.h"

void Multilane_TrafficFlow::Calculation(int filenumber,int lanelength, int Numberofvehicle, int Numberoflane) {
	for (int Crate = 0; Crate <= 100; Crate += 10) {
		double Cp = (double)Crate / 100;
		Cp = 0.6;
		int ave = 0;
		std::string filename = "Result/Global" + std::to_string(filenumber) + "_Crate" + std::to_string(Crate) + "_.dat";
		std::string file_TS = "Result/time-space.csv";
		std::ofstream ofs(filename,std::ios::app);
		std::ifstream ifs(filename);
		std::vector<bool> NFinished((lanelength * Numberoflane) / 75, false);
		if (ifs) {
			std::string SLine, S;
			while (std::getline(ifs, SLine)) {
				std::stringstream SS;
				SS << SLine;
				std::getline(SS, S, ',');
				int Val = ((std::stoi(S)) / 75) - 1;
				if (Val < 0) continue;
				NFinished[Val] = true;
			}
		}
		ifs.close();
		std::vector<int> NLists;
		for (int i = 0; i < NFinished.size(); i++) if (!NFinished[i]) NLists.push_back((i + 1) * 75);

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif // _OPENMP
		for (int i = 0; i < NLists.size(); i++) {
			int N = NLists[i];
			int CCar = (int)std::round(Cp * N);
			Update_Position::Measuredinfomation AllResult;
			Update_Position::AverageVelocity AllAverageVelocity;
			AllAverageVelocity.init();
			Update_Position* DoSim = new Update_Position(filenumber, lanelength, N, Numberoflane, CCar);
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
				//for (int k = 0; k < DoSim->constants.N; ++k) ofsTS << j << "," << DoSim->car.position.current[k] << std::endl;
			}
#ifdef _OPENMP
#pragma omp critical
#endif // _OPENMP
			{
				ofs << N << "," << (double)N / Numberofvehicle << "," << (double)AllResult.passed / Numberoflane << "," << AllResult.Lanechange_original << "," << AllResult.Lanechange_Pushed << ",";
				ofs << (double)AllAverageVelocity.Cooperator / (DoSim->Cooperator * 300) << "," << (double)AllAverageVelocity.Defector / (DoSim->Defector * 300) << std::endl;
				std::cout << N << "," << (double)N / Numberofvehicle << "," << (double)AllResult.passed / Numberoflane << "," << AllResult.Lanechange_original << "," << AllResult.Lanechange_Pushed << ",";
				std::cout << (double)AllAverageVelocity.Cooperator / (DoSim->Cooperator * 300) << "," << (double)AllAverageVelocity.Defector / (DoSim->Defector * 300) << std::endl;
			}
			delete DoSim;
		}
	}
}

void Multilane_TrafficFlow::_DoSimulation(Update_Position* DoSim) {
	DoSim->AverageVelocityThisTime.init();
	DoSim->Decide_targetvelocity();
	DoSim->car.Fromcurrent_toprevious();
	DoSim->map.Fromcurrent_toprevious();
	DoSim->InitializeLaneChangerInfomation();
	DoSim->TurnonLaneChangersSignal();
	DoSim->PickUpPushed();
	DoSim->Update_EachVehiclePosition();
	if (DoSim->TryLaneChange()) DoSim->Update_EachVehiclePosition();
	//for (int i = 0; i < DoSim->constants.N; ++i) std::cout << i << "th v:" << DoSim->car.velocity.current[i] << std::endl;
}
