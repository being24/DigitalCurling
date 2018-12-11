#define _CRT_SECURE_NO_WARNINGS 1

#include "game_log.h"

#include <ctime>
#include <fstream>
#include <string>

using std::endl;

namespace digital_curling {

	// Create file and write 'GameInfo' statement
	GameLog::GameLog(const Player* const p1, const Player* const p2) {
		if (p1 == nullptr || p2 == nullptr) {
			return;
		}

		// Get current time
		char date[128];
		time_t t = time(NULL);
		strftime(date, sizeof(date), "(%Y%m%d_%H%M%S)", localtime(&t));

		// Set filename of log e.g. 'Player1_Player2(20180131_123456).dcl'
		std::string file_name;
		file_name = p1->name_ + "_" + p2->name_ + date + ".dcl";

		file_path_ = "Log\\" + file_name;

		// Create file
		std::ofstream ofs(file_path_);
		if (!ofs.is_open()) {
			return;
		}

		// Write 'GameInfo' statement
		ofs << "[GameInfo]" << endl;
		ofs << "First=" << p1->name_ << endl;
		ofs << "FirstRemTime=" << p1->time_limit_ << endl;
		ofs << "FirstRandom_1=" << p1->pinfo_.params[0].random_1 << endl;
		ofs << "FirstRandom_2=" << p1->pinfo_.params[0].random_2 << endl;
		ofs << "Second=" << p2->name_ << endl;
		ofs << "SecondRemTime=" << p2->time_limit_ << endl;
		ofs << "SecondRandom_1=" << p2->pinfo_.params[0].random_1 << endl;
		ofs << "SecondRandom_2=" << p2->pinfo_.params[0].random_2 << endl;
	}

	GameLog::~GameLog() {}

	// Write to logfile
	void GameLog::Write(std::string message)
	{
		// Create file
		std::ofstream ofs(file_path_, std::ios::in | std::ios::ate);
		if (!ofs.is_open()) {
			return;
		}

		ofs << message << endl;
	}
}