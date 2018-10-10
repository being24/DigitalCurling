#include <windows.h>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include "game_process.h"

#ifdef _DEBUG
#pragma comment( lib, "../x64/Debug/Simulator.lib" )
#endif
#ifndef _DEBUG
#pragma comment( lib, "../x64/Release/Simulator.lib" )
#endif

using std::cerr;
using std::endl;
using std::string;

namespace digital_curling {

	namespace server {

		// Initialize player from params[3]
		Player* SetPlayer(std::vector<string> params) {
			Player *player = new digital_curling::LocalPlayer(
				params[1],
				atoi(params[2].c_str()),
				(float)atof(params[3].c_str()),
				(float)atof(params[4].c_str()));
			return player;
		}

		// Simple server for DigitalCurling
		int SimpleServer() {
			// Open config file
			const string config_file_path = "config.txt";
			//const string config_file_path = "C:\\Data\\Research\\programs\\DCServer-master\\x64\\Debug\\config.txt";
			std::vector<string> config_params[3];
			//                               [0][0] : type of rules (0: standard, 1:mix doubles)
			//                               [0][1] : number of ends (1 - 10)
			//                               [0][2] : size of random X (default 0.145)
			//                               [0][3] : size of random Y (default 0.145)
			//                               [1][0] : type of player1 (0: Local AI, 1: Network AI)
			//                               [1][1] : path of .exe file of player1
			//                               [1][2] : timelimit of player1 (msec, 0:infinite)
			//                               [1][3] : size of random X (default 0.145)
			//                               [1][4] : size of random Y (default 0.145)
			//                               [2][0] : type of player1 (0: Local AI, 1: Network AI)
			//                               [2][1] : path of .exe file of player1
			//                               [2][2] : timelimit
			//                               [2][3] : size of random X (default 0.145)
			//                               [2][4] : size of random Y (default 0.145)
			std::ifstream config_file(config_file_path);
			if (!config_file.is_open()) {
				cerr << "failed to open " << config_file_path << endl;
				return 0;
			}
			// Read config file
			string str_in;
			int i = 0;
			while (std::getline(config_file, str_in)) {  // read a line from config_file
				std::stringstream sstring(str_in);
				string param;
				while (std::getline(sstring, param, ',')) {  // sprit as token by delimiter ','
															 //cerr << "param[" << i << "][" << config_params[i].size() << "] = " << param << endl;
					config_params[i].push_back(param);
				}
				i++;
			}

			// Initialise LocalPlayer 1
			digital_curling::Player *p1;
			p1 = SetPlayer(config_params[1]);
			if (p1->InitProcess() == 0) {
				cerr << "failed to create process for player 1" << endl;
				return 0;
			}
	
			digital_curling::Player *p2;
			p2 = SetPlayer(config_params[2]);
			if (p2->InitProcess() == 0) {
				cerr << "failed to create process for player 2" << endl;
				return 0;
			}

			Sleep(10);  // MAGIC NUMBER: wait for process created

			cerr << "creating game process" << endl;

			// Create GameProcess
			digital_curling::GameProcess game_process(
				p1,
				p2,
				atoi(config_params[0][1].c_str()),
				(float)atof(config_params[0][2].c_str()),
				atoi(config_params[0][0].c_str())
			);

			// Send "ISREADY" to player1
			if (!game_process.IsReady(game_process.player1_, 5000)) {
				cerr << "failed to recieve ISREADY from player 1" << endl;
				return 0;
			}
			if (!game_process.IsReady(game_process.player2_, 5000)) {
				cerr << "failed to recieve ISREADY from player 2" << endl;
				return 0;
			}

			// Send "NEWGAME" to players
			game_process.NewGame();

			int status;
			while (game_process.gs_.CurEnd < game_process.gs_.LastEnd) {

				// Prepare for End
				cerr << "preparing for end" << endl;
				game_process.PrepareEnd(500);

				//do {
				while (game_process.gs_.ShotNum < 16) {
					// Send "SETSTATE" and "POSITION" to players
					game_process.SendState();
					cerr << "sending gamestate" << endl;

					// Send "GO" to player
					status = game_process.Go();
					if (status != GameProcess::BESTSHOT) {
						goto EXIT_PROCESS;
					}
					cerr << "bestshot recieved" << endl;

					// Simulation
					cerr << "run simulation" << endl;
					game_process.RunSimulation();
				}

				// Send 'SCORE' to players
				game_process.SendScore();
				cerr << "send score" << endl;
				Sleep(100);  // MAGIC NUMBER: wait for SendScore;

				
			}

		EXIT_PROCESS:
			// Exit Game
			game_process.Exit();
			cerr << "game_end" << endl;

			return 1;
		}

	}
}

int main(void)
{
	// run single game
	digital_curling::server::SimpleServer();

	return 0;
}