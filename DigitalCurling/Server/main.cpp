#include <windows.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include "lib/picojson.h"
#include "game_process.h"

#ifdef _DEBUG
#pragma comment( lib, "../x64/Debug/Simulator.lib" )
#endif
#ifndef _DEBUG
#pragma comment( lib, "../x64/Release/Simulator.lib" )
#endif

using std::cout;
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

		void PrintScoreBoard(const GameProcess* const gp) {
			// Print for first player
			int s1 = 0;
			cout << "|";
			for (unsigned int i = 0; i < gp->gs_.LastEnd; i++) {
				if (i < gp->gs_.CurEnd) {
					if (gp->gs_.Score[i] < 0) {
						cout << " 0";
					}
					else {
						cout << " " << gp->gs_.Score[i];
						s1 += gp->gs_.Score[i];
					}
				}
				else {
					cout << " -";
				}
			}
			cout << " | " << std::setw(2) << s1 << " | " << gp->player1_->name_ << endl;

			// Print for second player
			int s2 = 0;
			cout << "|";
			for (unsigned int i = 0; i < gp->gs_.LastEnd; i++) {
				if (i < gp->gs_.CurEnd) {
					if (gp->gs_.Score[i] > 0) {
						cout << " 0";
					}
					else {
						cout << " " << -gp->gs_.Score[i];
						s2 -= gp->gs_.Score[i];
					}
				}
				else {
					cout << " -";
				}
			}
			cout << " | " << std::setw(2) << s2 << " | " << gp->player2_->name_ << endl;
		}

		void PrintState(const GameProcess* const gp) {
			cout << "Shot:" << std::setw(2) << gp->gs_.ShotNum + 1 << 
				", End:" << std::setw(2) << gp->gs_.CurEnd + 1 << "/" << std::setw(2) << gp->gs_.LastEnd << ", Next shot: ";

			if (gp->gs_.WhiteToMove) {
				cout << gp->player2_->name_ << endl;
			}
			else {
				cout << gp->player1_->name_ << endl;
			}
		}

		// Simple server for DigitalCurling
		int SimpleServer() {
			// Open config file
			/*
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

			cerr << "creating game process..." << endl;

			// Create GameProcess
			digital_curling::GameProcess game_process(
				p1,
				p2,
				atoi(config_params[0][1].c_str()),
				(float)atof(config_params[0][2].c_str()),
				atoi(config_params[0][0].c_str())
			);
			*/

			// Open config file w/ json
			//std::ifstream config_file("C:\\Users\\vista\\Source\\Repos\\digitalcurling\\DigitalCurling\\DigitalCurling\\x64\\Debug\\config.json");
			std::ifstream config_file("config.json");
			if (!config_file.is_open()) {
				cerr << "failed to open config.json" << endl;
				return 0;
			}

			// Perse json
			picojson::value val;
			config_file >> val;
			config_file.close();

			// Get ofjects
			picojson::object obj_match = val.get<picojson::object>()["match_default"].get<picojson::object>();
			picojson::object obj_p1 = obj_match["player_1"].get<picojson::object>();
			picojson::array params_p1 = obj_p1["params"].get<picojson::array>();
			picojson::object obj_p2 = obj_match["player_2"].get<picojson::object>();
			picojson::array params_p2 = obj_p2["params"].get<picojson::array>();
			picojson::object obj_server = val.get<picojson::object>()["server"].get<picojson::object>();
			picojson::object obj_sim = val.get<picojson::object>()["simulator"].get<picojson::object>();

			// Initialize LocalPlayer 1
			digital_curling::LocalPlayer *p1;
			p1 = new LocalPlayer(
				obj_p1["path"].get<string>(), 
				(int)obj_p1["timelimit"].get<double>(),
				(float)params_p1.begin()->get<picojson::object>()["random_1"].get<double>(), 
				(float)params_p1.begin()->get<picojson::object>()["random_2"].get<double>());
			if (p1->InitProcess() == 0) {
				cerr << "failed to create process for player 1" << endl;
				return 0;
			}

			// Initialize LocalPlayer 2 
			digital_curling::LocalPlayer *p2;
			p2 = new LocalPlayer(
				obj_p2["path"].get<string>(),
				(int)obj_p2["timelimit"].get<double>(),
				(float)params_p2.begin()->get<picojson::object>()["random_1"].get<double>(),
				(float)params_p2.begin()->get<picojson::object>()["random_2"].get<double>());
			if (p2->InitProcess() == 0) {
				cerr << "failed to create process for player 2" << endl;
				return 0;
			}

			// Get other parameters from json
			int timeout_isready = (int)obj_server["timeout_isready"].get<double>();
			int timeout_preend = (int)obj_server["timeout_preend"].get<double>();
			//bool output_dcl = obj_server["output_dcl"].get<bool>();
			//bool output_json = obj_server["output_json"].get<bool>();
			//bool output_server_log = obj_server["output_server_log"].get<bool>();
			SimulatorParams sim_params;
			sim_params.friction = (float)obj_sim["friction"].get<double>();
			sim_params.random_generator = (obj_sim["rand_type"].get<string>() == "RECTANGULAR") ? b2simulator::RECTANGULAR : b2simulator::POLAR;

			// Create GameProcess
			int rule_type;
			string str = obj_match["rule_type"].get<string>();
			if (str == "normal") {
				rule_type = 0;
			}
			else if (str == "mix_doubles") {
				rule_type = 1;
			}
			else {
				rule_type = 0;
			}
			digital_curling::GameProcess game_process(
				p1,
				p2,
				(int)obj_match["ends"].get<double>(),
				rule_type,
				sim_params
			);

			// Send "ISREADY" to player1
			if (!game_process.IsReady(game_process.player1_, timeout_isready)) {
				cerr << "failed to recieve ISREADY from player 1" << endl;
				return 0;
			}
			if (!game_process.IsReady(game_process.player2_, timeout_isready)) {
				cerr << "failed to recieve ISREADY from player 2" << endl;
				return 0;
			}

			// Send "NEWGAME" to players
			game_process.NewGame();

			int status;
			while (game_process.gs_.CurEnd < game_process.gs_.LastEnd) {

				// Prepare for End
				cerr << "Preparing for end..." << endl;
				game_process.PrepareEnd(timeout_preend);

				//do {
				while (game_process.gs_.ShotNum < 16) {
					// Send "SETSTATE" and "POSITION" to players
					game_process.SendState();
					PrintState(&game_process);

					// Send "GO" to player
					status = game_process.Go();
					if (status != GameProcess::BESTSHOT) {
						goto EXIT_PROCESS;
					}
					cerr << "BESTSHOT: (" << 
						game_process.best_shot_.x << ", " << game_process.best_shot_.y << ", " << 
						game_process.best_shot_.angle << ")" << endl;

					// Simulation
					game_process.RunSimulation();
				}

				// Send 'SCORE' to players
				game_process.SendScore();
				PrintScoreBoard(&game_process);
				//Sleep(100);  // MAGIC NUMBER: wait for SendScore;


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