#include "dc_util.h"

/*
SIMULATION_FUNC Simulation = NULL;
SIMULATIONEX_FUNC SimulationEx = NULL;
CREATESHOT_FUNC CreateShot = NULL;
CREATEHITSHOT_FUNC CreateHitShot = NULL;

HMODULE hCSDLL;

bool LoadFunction(std::string dll_path)
{
	hCSDLL = LoadLibrary(dll_path.c_str());

	if (hCSDLL != NULL) {
		Simulation = (SIMULATION_FUNC)GetProcAddress(hCSDLL, "Simulation");
		SimulationEx = (SIMULATIONEX_FUNC)GetProcAddress(hCSDLL, "SimulationEx");
		CreateShot = (CREATESHOT_FUNC)GetProcAddress(hCSDLL, "CreateShot");
		CreateHitShot = (CREATEHITSHOT_FUNC)GetProcAddress(hCSDLL, "CreateHitShot");
	}
	else {
		return false;
	}

	return true;
}
*/