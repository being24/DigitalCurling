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

		// Server options
		struct Options {
			int timeout_isready;     // timeout for "ISREADY" command
			int timeout_preend;      // timeout for "PUTSTONE" command
			int timeout_setorder;    // timeout for "SETORDER" command
			bool output_dcl;         // output log (.dcl) or not
			bool output_json;        // output log (.json) *not implemented
			bool output_server_log;  // output server log *notimplemented

			int view_board_delay;    // interval for display board [msec]
		};

		// Print score board on console
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
			cout << " | " << std::setw(2) << s1 << " | " << gp->player1_->name_ << " ÅZ" << endl;

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
			cout << " | " << std::setw(2) << s2 << " | " << gp->player2_->name_ << " Å~" << endl;
		}

		// Improvised board viewer on console
		void PrintBoard(const GameState* const gs) {
			const float discrete_size = kStoneR * 2;
			const int X_SIZE = 17;
			const int Y_SIZE = 39;
			const int X_CENTER = 8;
			const int Y_CENTER = 16;
			string s[X_SIZE][Y_SIZE];

			// init char table
			for (int y = 0; y < Y_SIZE; y++) {
				for (int x = 0; x < X_SIZE; x++) {
					if ((pow((x - X_CENTER) * discrete_size, 2) + pow((y - Y_CENTER) * discrete_size, 2)) < pow(kHouseR, 2)) {
						s[x][y] = "ÅE";  // in house
					}
					else {
						s[x][y] = "Å@";  // out of house
					}

					if (y == Y_CENTER) {
						s[x][y] = "Å\";  // hog line
					}
					if (x == X_CENTER) {
						s[x][y] = "Åb";  // center line
					}
				}
			}

			for (int i = 0; i < 16; i++) {
				if (!(gs->body[i][0] == 0.0f && gs->body[i][0] == 0.0f)) {
					if (gs->WhiteToMove == (1 & (gs->ShotNum % 2))) {
						if ((gs->ShotNum - 1) == i) {
							s[(int)(gs->body[i][0] / discrete_size)][(int)(gs->body[i][1] / discrete_size)] = (i % 2) ? "Å¶" : "Åú";
						}
						else {
							s[(int)(gs->body[i][0] / discrete_size)][(int)(gs->body[i][1] / discrete_size)] = (i % 2) ? "Å~" : "ÅZ";
						}
					}
					else {
						if ((gs->ShotNum - 1) == i) {
							s[(int)(gs->body[i][0] / discrete_size)][(int)(gs->body[i][1] / discrete_size)] = (i % 2) ? "Åú" : "Å¶";
						}
						else {
							s[(int)(gs->body[i][0] / discrete_size)][(int)(gs->body[i][1] / discrete_size)] = (i % 2) ? "ÅZ" : "Å~";
						}
					}
				}

			}

			// print table
			std::stringstream strstr;
			for (int y = 0; y < Y_SIZE; y++) {
				for (int x = 0; x < X_SIZE; x++) {
					strstr << s[x][y];
				}

				strstr << endl;
			}

			cout << strstr.str();
		}

		void PrintState(const GameProcess* const gp) {
			cout << "Shot:" << std::setw(2) << gp->gs_.ShotNum + 1 <<
				", End:" << std::setw(2) << gp->gs_.CurEnd + 1 << "/" << std::setw(2) << gp->gs_.LastEnd << ", Next shot: ";

			if (gp->gs_.WhiteToMove) {
				cout << gp->player2_->name_ << " ( " << gp->player2_->time_remain_ / 1000 << " [sec] left)" << endl;
			}
			else {
				cout << gp->player1_->name_ << " ( " << gp->player1_->time_remain_ / 1000 << " [sec] left)" << endl;
			}
		}

		// Initialize player from params via picojson
		Player* SetPlayer(picojson::object obj) {
			picojson::array params = obj["params"].get<picojson::array>();

			PlayerInfo pinfo(int(params.size()));
			
			int i = 0;

			// Set player parameters
			for (picojson::array::iterator it = params.begin(); it != params.end(); it++) {
				pinfo.params[i].random_1 = (float)it->get<picojson::object>()["random_1"].get<double>();
				pinfo.params[i].random_2 = (float)it->get<picojson::object>()["random_2"].get<double>();
				pinfo.params[i].shot_max = (float)it->get<picojson::object>()["weight_max"].get<double>();
				pinfo.order[i] = i;
				i++;
			}

			// Fill rest of players
			for (i = pinfo.nplayers; i < 4; i++) {
				pinfo.params[i].random_1 = pinfo.params[0].random_1;
				pinfo.params[i].random_2 = pinfo.params[0].random_2;
				pinfo.params[i].shot_max = pinfo.params[0].shot_max;
			}

			// Initialize player as LocalPlayer
			Player *p = new LocalPlayer(
				obj["path"].get<string>(),
				(int)obj["timelimit"].get<double>(),
				pinfo);
			if (p->InitProcess() == 0) {
				cerr << "failed to create process for player" << endl;
				return 0;
			}
			p->mix_doubles = obj["md"].get<bool>();

			/*  for debug
			for (int i = 0; i < 4; i++) {
				cerr << "params[" << i << "] = " << pinfo.params[i].random_1 << " " << pinfo.params[i].random_2 << " " << pinfo.params[i].shot_max << endl;
			}
			cerr << "order = " << p->pinfo_.order[0] << " " << p->pinfo_.order[1] << " " << p->pinfo_.order[2] << " " << p->pinfo_.order[3] << endl;
			*/

			return p;
		}

		// Load config and initialize game process
		GameProcess* Init(std::string config_path, std::string match_name, Options &options) {
			std::ifstream config_file(config_path);
			if (!config_file.is_open()) {
				cerr << "failed to open " << config_path << endl;
				return 0;
			}

			// Perse json via picojson
			picojson::value val;
			config_file >> val;
			config_file.close();

			// Get objects
			picojson::object obj_server = val.get<picojson::object>()["server"].get<picojson::object>();  // Simulator
			picojson::object obj_sim = val.get<picojson::object>()["simulator"].get<picojson::object>();  // Server

			if (!val.get<picojson::object>()[match_name].is<picojson::object>()) {
				cerr << "failed to find match '" << match_name << "' in " << config_path << "." << endl;
				return 0;
			}
			picojson::object obj_match = val.get<picojson::object>()[match_name].get<picojson::object>();  // Match
			picojson::object obj_p1 = obj_match["player_1"].get<picojson::object>();  // player 1
			picojson::object obj_p2 = obj_match["player_2"].get<picojson::object>();  // player 2

			// Initialize Player 1
			digital_curling::Player *p1;
			p1 = SetPlayer(obj_p1);

			// Initialize Player 2 
			digital_curling::Player *p2;
			p2 = SetPlayer(obj_p2);

			// Get server parameters
			options.timeout_isready = (int)obj_server["timeout_isready"].get<double>();
			options.timeout_preend = (int)obj_server["timeout_preend"].get<double>();
			options.output_dcl = obj_server["output_dcl"].get<bool>();
			options.output_json = obj_server["output_json"].get<bool>();
			options.output_server_log = obj_server["output_server_log"].get<bool>();
			options.view_board_delay = (int)obj_server["view_board_delay"].get<double>();

			// Get simulator parameters
			SimulatorParams sim_params;
			sim_params.friction = (float)obj_sim["friction"].get<double>();
			sim_params.friction_stone = (float)obj_sim["friction_stone"].get<double>();
			sim_params.random_generator = (obj_sim["rand_type"].get<string>() == "RECTANGULAR") ? b2simulator::RECTANGULAR : b2simulator::POLAR;
			sim_params.freeguard_num = (unsigned int)obj_sim["freeguard_num"].get<double>();

			// Get rule type
			int rule_type;
			string str = obj_match["rule_type"].get<string>();
			if (str == "normal") {
				rule_type = 0;
			}
			else if (str == "mix_doubles") {
				rule_type = 1;
			}
			else {
				cerr << "invalid rule_type from " << config_path << ", set rule_type_ = 0" << endl;
				rule_type = 0;
			}

			// Get extended end
			bool extended_end = obj_match["extended_end"].get<bool>();

			// Create GameProcess
			digital_curling::GameProcess *game_process = new GameProcess(
				p1,
				p2,
				(int)obj_match["ends"].get<double>(),
				rule_type,
				(int)obj_match["repetition"].get<double>(),
				extended_end,
				sim_params
			);

			return game_process;
		}

		// Run a match
		int RunMatch(GameProcess &game_process, Options opt) {
			// Send "NEWGAME" to players
			game_process.NewGame();

			int status = 0;
			while (game_process.gs_.CurEnd < game_process.gs_.LastEnd) {

				// Prepare for End
				cerr << "Preparing for end..." << endl;
				game_process.PrepareEnd(opt.timeout_preend);

				//do {
				while (game_process.gs_.ShotNum < 16) {
					// Send "SETSTATE" and "POSITION" to players
					game_process.SendState();
					Sleep(50);  // wait for
					cerr << "==========================================" << endl;
					PrintState(&game_process);
					PrintBoard(&game_process.gs_);
					PrintScoreBoard(&game_process);
					cerr << "==========================================" << endl;
					Sleep(opt.view_board_delay + 50);  // wait for

								// Send "GO" to player
					status = game_process.Go();
					if (status != GameProcess::BESTSHOT) {
						cerr << "status = " << status << endl;
						return status;
					}
					cerr << "BESTSHOT: (" <<
						game_process.best_shot_.x << ", " << game_process.best_shot_.y << ", " <<
						game_process.best_shot_.angle << ")" << endl;

					// Simulation
					game_process.RunSimulation();
				}

				// Send 'SCORE' to players
				game_process.SendScore();
				Sleep(50);  // wait for
				cerr << "==========================================" << endl;
				PrintBoard(&game_process.gs_);
				PrintScoreBoard(&game_process);
				cerr << "==========================================" << endl;
				Sleep(50);  // wait for
			}

			// Exit Game
			game_process.Exit();
			cerr << "game_end" << endl;

			return status;
		}

		// Simple game server for DigitalCurling
		int SimpleServer(std::string match_name) {

			// Open config file w/ json
			std::string config_path = "config.json";
			Options opt;
			
			GameProcess *gp = Init(config_path, match_name, opt);
			GameProcess game_process = *gp;

			// Send "ISREADY" to both players
			if (!game_process.IsReady(game_process.player1_, opt.timeout_isready)) {
				cerr << "failed to recieve ISREADY from player 1" << endl;
				return 0;
			}
			if (!game_process.IsReady(game_process.player2_, opt.timeout_isready)) {
				cerr << "failed to recieve ISREADY from player 2" << endl;
				return 0;
			}

			// Run match(es)
			for (unsigned int i = 0; i < game_process.repetition_; i++) {
				if (i > 0) {
					// Create new log file
					game_process.log_file_.Create(game_process.player1_, game_process.player2_);
					// Reset GameState
					game_process.gs_.ClearAll();
				}

				// Run a match
				RunMatch(game_process, opt);
			}

			// Exit Player Process
			if (game_process.player1_->ExitProcess() == 0) {
				cerr << "failed to player1_->ExitProcess()" << endl;
				return  0;
			}
			
			if (game_process.player2_->ExitProcess() == 0) {
				cerr << "failed to player2_->ExitProcess()" << endl;
				return 0;
			}

			return 1;
		}
	
		int CuiServer() {
			std::stringstream ss_help;
			ss_help << "> === DigitalCurling Server 2018 ===========" << endl;
			ss_help << "> Commands:" << endl;
			ss_help << ">  'run' : run single match." << endl;
			ss_help << ">  'run MATCH_NAME' : run single match named MATCH_NAME (default 'match_default')" << endl;
			ss_help << ">  'q' or 'exit' : exit from server." << endl;
			ss_help << ">  'h' or 'help' : show all commands." << endl;
			ss_help << "> ==========================================" << endl;

			cout << ss_help.str();

			string s;
			// get inputs
			while (getline (std::cin, s)) {

				// Get tokens from input
				std::stringstream sstream(s);
				std::vector<std::string> tokens;
				std::string tok;
				while (getline(sstream, tok, ' ')) {
					if (!tok.empty()) {
						tokens.push_back(tok);
					}
				}

				if (tokens.size() == 0) {
					continue;
				}

				// process command
				if (tokens[0] == "q" || tokens[0] == "exit") {
					// Break if 'q' or 'exit'
					cout << "> exit." << endl;
					break;
				}
				else if (tokens[0] == "run") {
					std::string match_name;
					// Run single match
					if (tokens.size() == 1) {
						match_name = "match_default";
					}
					else {
						match_name = tokens[1];
					}
					cout << "> run single match '" << match_name << "'." << endl;
					SimpleServer(match_name);
				}
				else if (tokens[0] == "help" || tokens[0] == "h") {
 					// Show help
					cout << ss_help.str();
				}
				else {
					cerr << "> invalid command." << endl;
				}
			}

			return 1;
		}
	}
}

int main(void)
{
	// run cui server
	digital_curling::server::CuiServer();

	return 0;
}