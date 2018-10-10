// Constant numbers and utilities in DigitalCurling
// TODO: いずれシミュレータライブラリ（CurlingSimulator.h）へ移す
#pragma once

//#include "CurlingSimulator.h"
#include "dcurling_simulator.h"

#include <Windows.h>

#include <iostream>
#include <string>

// TODO: シミュレータ側で GameState, ShotPos, ShotVecを定義してコメントアウトする
//typedef struct _GAMESTATE GameState;
//typedef struct _ShotPos ShotPos;
//typedef struct _ShotVec ShotVec;

namespace digital_curling {

	/*
	namespace constant_num {
		static const float kCenterX      =  2.375f;  // X coord of Center Line
		static const float kTeeY         =  4.880f;  // Y coord of Tee Line
		static const float kSideX        = 4.750f;   // X coord of Side Line
		static const float kHogY         = 11.280f;  // Y coord of Hog Line
		static const float kStoneR       =  0.145f;  // Radius of Stone
		static const float kHouseR       =  1.830f;  // Radius of House (12 foot circle)
		static const float kHouse8FootR  =  1.220f;  // Radius of House ( 8 foot circle)
		static const float kHouse4FootR  =  0.610f;  // Radius of House ( 4 foot circle)
	}
	*/

	inline void PrintState(const GameState &gs) {
		std::cerr << "ShotNum = " << gs.ShotNum << ", CurEnd = " << gs.CurEnd << "/" << gs.LastEnd << ", WhiteToMove = " << gs.WhiteToMove << std::endl;
		for (unsigned int i = 0; i < 16; i++) {
			std::cerr << "Position" << i << " = (" << gs.body[i][0] << "," << gs.body[i][1] << ")" << std::endl;
		}
		std::cerr << "Score = ";
		for (unsigned int i = 0; i < gs.LastEnd; i++) {
			std::cerr << gs.Score[i] << " ";
		}
		std::cerr << std::endl;
	}
}

/*
extern SIMULATION_FUNC Simulation;
extern SIMULATIONEX_FUNC SimulationEx;
extern CREATESHOT_FUNC CreateShot;
extern CREATEHITSHOT_FUNC CreateHitShot;
extern HMODULE hCSDLL;

// load curling
bool LoadFunction(std::string dll_path);
*/