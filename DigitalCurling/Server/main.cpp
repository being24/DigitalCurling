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

			// Open config file w/ json
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