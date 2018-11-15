#include "game_process.h"

#include <ctime>
#include <iostream>
#include <iomanip>
#include <random>
#include <sstream>

#include <future>
#include <thread>

using std::cerr;
using std::endl;

namespace digital_curling
{
	GameProcess::GameProcess(Player *p1, Player *p2, int num_ends, int rule_type) : rule_type_(rule_type), log_file_(p1, p2) {

		// Initialize state of the game
		memset(&gs_, 0, sizeof(GameState));
		gs_.LastEnd = num_ends;  // set LastEnd as number of ends

		// Set random number
		//random_ = random;

		// Initialize bestshot and runshot
		memset(&best_shot_, 0, sizeof(ShotVec));
		memset(&run_shot_, 0, sizeof(ShotVec));

		// Set players
		player1_ = p1;
		player2_ = p2;

		// Initialize simulator
		sim = new b2simulator::Simulator();
	}

	GameProcess::GameProcess(Player *p1, Player *p2, int num_ends, int rule_type, SimulatorParams params) : rule_type_(rule_type), log_file_(p1, p2) {

		// Initialize state of the game
		memset(&gs_, 0, sizeof(GameState));
		gs_.LastEnd = num_ends;  // set LastEnd as number of ends

		// Set random number
		//random_ = random;

		// Initialize bestshot and runshot
		memset(&best_shot_, 0, sizeof(ShotVec));
		memset(&run_shot_, 0, sizeof(ShotVec));

		// Set players
		player1_ = p1;
		player2_ = p2;

		// Initialize simulator
		sim = new b2simulator::Simulator(params.friction);
		sim->random_type_ = params.random_generator;
	}

	GameProcess::~GameProcess() {}

	// Recieve message with std::promise
	void RecvThread(std::promise<std::string> p, Player* const player) {
		
		char msg[digital_curling::Player::kBufferSize];
		player->Recv(msg);
		p.set_value(std::string(msg));
		
	}
	void RecvThread2(std::string &p, Player* const player) {

		std::cout << "recieve" << endl;
		char msg[digital_curling::Player::kBufferSize];
		player->Recv(msg);
		p = std::string(msg);

	}

	// Send 'ISREADY' command and wait for recieving 'READYOK' from a player
	bool GameProcess::IsReady(Player *player, unsigned int time_out) {

		char msg[Player::kBufferSize];

		// Prepare recieve thread
		std::string str;
		std::future<void> f = std::async(std::launch::async, RecvThread2, std::ref(str), player);
		Sleep(100);

		player->Send("ISREADY");

		// Wait for message is ready
		std::future_status result = f.wait_for(std::chrono::milliseconds(time_out));

		if (result != std::future_status::timeout) {
			strcpy_s(msg, Player::kBufferSize, str.c_str());

			// split message as token
			std::vector<std::string> tokens = digital_curling::SpritAsTokens(msg, " ");
			if (tokens.size() == 0) {
				cerr << "Error: empty message" << endl;
				return false;
			}
			if (tokens[0] != "READYOK") {
				cerr << "Error: invalid command '" << tokens[0] << "'" << endl;
				return false;
			}
		}
		else {
			cerr << "Error: timeout for READYOK" << endl;
			return false;
		}

		return true;
	}

	// Send 'NEWGAMWE' command to both player
	bool GameProcess::NewGame() {
		if (player1_ == nullptr || player2_ == nullptr) {
			return false;
		}

		// Send 'NEWGAMWE p1->name_ p2->name_' command to both player
		std::stringstream sstream;
		sstream << "NEWGAME " << player1_->name_ << " " << player2_->name_;
		player1_->Send(sstream.str().c_str());
		player2_->Send(sstream.str().c_str());

		// Clear sstream
		sstream.str("");
		sstream.clear(std::stringstream::goodbit);

		// Send 'GAMEINFO'
		sstream << "GAMEINFO " << rule_type_ << " " << sim->random_type_;
		player1_->Send(sstream.str().c_str());
		player2_->Send(sstream.str().c_str());

		// Clear sstream
		sstream.str("");
		sstream.clear(std::stringstream::goodbit);

		std::stringstream sstream2;

		// Send 'RANDOMSIZE'
		sstream << "RANDOMSIZE " << player1_->random_x_ << " " << player1_->random_y_ << endl;
		sstream2 << "RANDOMSIZE " << player2_->random_x_ << " " << player2_->random_y_ << endl;

		player1_->Send(sstream.str().c_str());
		player2_->Send(sstream2.str().c_str());

		return true;
	}

	// Prepare for End
	bool GameProcess::PrepareEnd(unsigned int time_out) {
		if (player1_ == nullptr || player2_ == nullptr) {
			return false;
		}

		// Clear gs_.body
		memset(gs_.body, 0, 2 * 16 * sizeof(int));

		// For mix doubles rule
		if (rule_type_ == 1) {
			//using namespace constant_num;
			const float   GUARD_OFFSET_X = 1.070f;
			const float   GUARD_OFFSET_Y = 2.286f;
			const ShotPos CENTER_HOUSE(kCenterX, kTeeY - 3 * kStoneR, 0);
			const ShotPos CENTER_GUARD(kCenterX, kTeeY + kHouseR + GUARD_OFFSET_Y, 0);
			const ShotPos SIDE_HOUSE(kCenterX - kHouse8FootR, kTeeY + kStoneR, 0);
			const ShotPos SIDE_GUARD(kCenterX - GUARD_OFFSET_X, kTeeY + kHouseR + GUARD_OFFSET_Y, 0);

			// Set player which "PUTSTONE" command send to
			Player *player = (gs_.WhiteToMove) ? player1_ : player2_;

			char msg[Player::kBufferSize];

			// Prepare recieve thread
			std::string str;
			//std::future<void> f = std::async(std::launch::async, RecvThread2, std::ref(str), player);

			// Send "PUTSTONE" command
			player->Send("PUTSTONE");

			// Wait for message is ready
			//std::future_status result = f.wait_for(std::chrono::milliseconds(time_out));

			int putstone_type = 0;
			std::vector<std::string> tokens;
			/*
			if (result != std::future_status::timeout) {
				strcpy_s(msg, Player::kBufferSize, str.c_str());
				// set putstone_type
				tokens = digital_curling::SpritAsTokens(msg, " ");
				if (tokens.size() == 0) {
					cerr << "Error: too few aguments in message: '" << msg << "'" << endl;
				}
				if (tokens[0] == "PUTSTONE") {
					if (tokens.size() >= 2) {
						putstone_type = atoi(tokens[1].c_str());
					}
				}
			}
			else {
				cerr << "time out for 'PUTSTONE'" << endl;
			}*/

			switch (putstone_type)
			{
			case 0:
				gs_.body[0][0] = CENTER_GUARD.x;
				gs_.body[0][1] = CENTER_GUARD.y;
				gs_.body[1][0] = CENTER_HOUSE.x;
				gs_.body[1][1] = CENTER_HOUSE.y;
				break;
			case 1:
				gs_.body[1][0] = CENTER_GUARD.x;
				gs_.body[1][1] = CENTER_GUARD.y;
				gs_.body[0][0] = CENTER_HOUSE.x;
				gs_.body[0][1] = CENTER_HOUSE.y;
				gs_.WhiteToMove ^= 1;
				break;
			case 2:
				gs_.body[0][0] = SIDE_GUARD.x;
				gs_.body[0][1] = SIDE_GUARD.y;
				gs_.body[1][0] = SIDE_HOUSE.x;
				gs_.body[1][1] = SIDE_HOUSE.y;
				break;
			case 3:
				gs_.body[1][0] = SIDE_GUARD.x;
				gs_.body[1][1] = SIDE_GUARD.y;
				gs_.body[0][0] = SIDE_HOUSE.x;
				gs_.body[0][1] = SIDE_HOUSE.y;
				gs_.WhiteToMove ^= 1;
				break;
			default:
				gs_.body[0][0] = CENTER_HOUSE.x;
				gs_.body[0][1] = CENTER_HOUSE.y;
				gs_.body[1][0] = CENTER_GUARD.x;
				gs_.body[1][1] = CENTER_GUARD.y;
				break;
			}

			// Write dummy statements to logfile
			std::stringstream sstream;
			for (int i = 0; i < 6; i++) {
				// Clear sstream
				sstream.str("");
				sstream.clear(std::stringstream::goodbit);

				// Set dummy statements
				sstream << "[" << 
					std::setfill('0') << std::setw(2) << gs_.CurEnd <<
					std::setfill('0') << std::setw(2) << i << "]" << endl;
				sstream << "POSITION=POSITION";
				for (int i = 0; i < 16; i++) {
					sstream << " " << gs_.body[i][0] << " " << gs_.body[i][1];
				}
				sstream << endl;
				sstream << "SETSTATE=SETSTATE " << i << " " << gs_.CurEnd << " " << gs_.LastEnd << " " << gs_.WhiteToMove << endl;
				sstream << "BESTSHOT=BESTSHOT 0 0 0" << endl;
				sstream << "RUNSHOT=RUNSHOT 0 0 0";

				// Write to logfile
				log_file_.Write(sstream.str());
			}

			gs_.ShotNum = 6;
		}

		return true;
	}

	// Send 'SETSTATE' command and 'POSITION' command
	bool GameProcess::SendState() {
		if (player1_ == nullptr || player2_ == nullptr) {
			return false;
		}
		// Write "[0000]" statement to logfile
		std::stringstream sstream;
		sstream << "[" <<
			std::setfill('0') << std::setw(2) << gs_.CurEnd <<
			std::setfill('0') << std::setw(2) << gs_.ShotNum << "]";
		log_file_.Write(sstream.str());

		// Clear sstream
		sstream.str("");
		sstream.clear(std::stringstream::goodbit);

		// Send POSITION command ('POSITION body[0][0] body[0][1] body[1][0] body[1][1] ... body[15][0] body[15][0]')
		sstream << "POSITION";
		for (int i = 0; i < 16; i++) {
			sstream << " " <<  gs_.body[i][0] << " " << gs_.body[i][1];
		}
		player1_->Send(sstream.str().c_str());
		if (player1_ != player2_) {
			player2_->Send(sstream.str().c_str());
		}
		// Write to logfile
		log_file_.Write("POSITION=" + sstream.str());

		// Clear sstream
		sstream.str("");
		sstream.clear(std::stringstream::goodbit);

		// Send SETSTATE command ('SETSTATE ShotNum CurEnd LastEnd WhiteToMove')
		sstream << "SETSTATE " << gs_.ShotNum << " " << gs_.CurEnd << " " << gs_.LastEnd << " " << gs_.WhiteToMove;
		player1_->Send(sstream.str().c_str());
		if (player1_ != player2_) {
			player2_->Send(sstream.str().c_str());
		}
		// Write to logfile
		log_file_.Write("SETSTATE=" + sstream.str());

		return true;
	}


	// Send 'GO' command and wait for recieving 'BESTSHOT' from a player
	int GameProcess::Go() {
		if (player1_ == nullptr || player2_ == nullptr) {
			return ERR;
		}
		// Prepare "GO" command
		std::stringstream sstream;
		sstream << "GO " << player1_->time_remain_ << " " << player2_->time_remain_;
		Player *next_player = (gs_.WhiteToMove == 0) ? player1_ : player2_;

		// Prepare recieve thread
		//char msg[digital_curling::Player::kBufferSize];
		//next_player->Recv(msg);
		char msg[Player::kBufferSize];

		std::promise<std::string> p;
		std::future<std::string> f = p.get_future();

		std::thread th(RecvThread, std::move(p), next_player);

		Sleep(100);

		// Send "GO" command
		time_t time_start = clock();
		next_player->Send(sstream.str().c_str());

		// Wait for message is ready
		std::future_status result = f.wait_for(std::chrono::milliseconds(next_player->time_remain_));
		th.join();

		if (result != std::future_status::timeout) {
			strcpy_s(msg, Player::kBufferSize, f.get().c_str());

			// Check timelimit
			if (next_player->time_remain_ < Player::kTimeLimitInfinite) {
				time_t time_used = clock() - time_start;
				next_player->time_remain_ -= (int)time_used;
			}

			// Write to logfile
			log_file_.Write("BESTSHOT=" + std::string(msg));  // TODO: move to appropriate place

			// Split message as token
			//char msg_tmp[digital_curling::Player::kBufferSize];
			//std::char_traits<char>::copy(msg_tmp, msg.c_str(), msg.size() + 1);
			std::vector<std::string> tokens;
			tokens = digital_curling::SpritAsTokens(msg, " ");
			if (tokens.size() == 0) {
				cerr << "Error: too few aguments in message: '" << msg << "'" << endl;
				return ERR;
			}
			if (tokens[0] == "BESTSHOT") {
				// Set best_shot_ if command is 'BESTSHOT'
				if (tokens.size() < 4) {
					cerr << "Error: too few aguments in message: '" << msg << "'" << endl;
					return ERR;
				}
				best_shot_.x = (float)atof(tokens[1].c_str());
				best_shot_.y = (float)atof(tokens[2].c_str());
				best_shot_.angle = (bool)atoi(tokens[3].c_str());

				return BESTSHOT;
			}
			else if (tokens[0] == "CONSEED") {
				// Jump to conseed and exit process if command is 'CONSEED'
				// TODO: jump to conseed and exit process
				return CONCEED;
			}
			else {
				// Print error message and exit
				cerr << "Error: invalid command '" << tokens[0] << "'" << endl;
				return ERR;
			}
		}
		else {
				// TODO: jump to timeover and exit process
				cerr << "TimeOut" << endl;
				return TIMEOUT;
		}

	}

	// Simulate a shot
	bool GameProcess::RunSimulation() {
		Player *p = (gs_.WhiteToMove) ? player2_ : player1_;
		//Simulation(&gs_, best_shot_, p->random_x_, &run_shot_, -1);
		sim->Simulation(&gs_, best_shot_, p->random_x_, p->random_y_, &run_shot_, nullptr, 0);

		// Write to log file
		std::stringstream sstream;
		sstream << "RUNSHOT=RUNSHOT " << run_shot_.x << ' ' << run_shot_.y << ' ' << run_shot_.angle;
		log_file_.Write(sstream.str());

		return true;
	}

	// Send 'SCORE' command
	bool GameProcess::SendScore() {
		if (player1_ == nullptr || player2_ == nullptr) {
			return false;
		}

		std::stringstream sstream;

		// Write "[0016]" statement to logfile
		sstream << "[" <<
			std::setfill('0') << std::setw(2) << gs_.CurEnd <<
			std::setfill('0') << std::setw(2) << gs_.ShotNum << "]" << endl;
		sstream << "POSITION=POSITION";
		for (int i = 0; i < 16; i++) {
			sstream << " " << gs_.body[i][0] << " " << gs_.body[i][1];
		}
		log_file_.Write(sstream.str());

		// Clear sstream
		sstream.str("");
		sstream.clear(std::stringstream::goodbit);

		// Sned "SCORE" command
		sstream << "SCORE " << gs_.Score[gs_.CurEnd];
		player1_->Send(sstream.str().c_str());
		if (player1_ != player2_) {
			player2_->Send(sstream.str().c_str());
		}
		// Write to logfile
		log_file_.Write("SCORE=" + sstream.str());

		// Crear board of gs_
		gs_.Clear();
		gs_.CurEnd++;

		return true;
	}

	bool GameProcess::Exit() {
		if (player1_ == nullptr || player2_ == nullptr) {
			return false;
		}
		// Calclate total score
		int score_p1 = 0;
		int score_p2 = 0;
		for (unsigned int i = 0; i < gs_.LastEnd; i++) {
			if (gs_.Score[i] > 0) {
				score_p1 += gs_.Score[i];
			}
			else {
				score_p2 -= gs_.Score[i];
			}
		}

		// Send "GAMEOVER" to each player
		Player *p_won = nullptr;
		Player *p_lost = nullptr;
		if (score_p1 == score_p2) {
			player1_->Send("GAMEOVER DRAW");
			player2_->Send("GAMEOVER DRAW");
		}
		else {
			if (score_p1 > score_p2) {
				p_won = player1_;
				p_lost = player2_;
			}
			if (score_p1 < score_p2) {
				p_won = player2_;
				p_lost = player1_;
			}
			p_won->Send("GAMEOVER WIN");
			p_lost->Send("GAMEOVER LOSE");
		}

		// Write to log file
		std::stringstream sstream;
		sstream << "[GameOverInfo]" << endl;
		sstream << "ENDSTATE=" << "NORMAL" << endl;

		if (p_won != nullptr && p_lost != nullptr) {
			sstream << "WIN=" << p_won->name_ << endl;
			sstream << "LOSE=" << p_lost->name_ << endl;
		}
		else {
			sstream << "WIN=" << endl;
			sstream << "LOSE=" << endl;
		}

		sstream << "TOTALSCORE=TOTALSCORE " << score_p1 << " " << score_p2 << endl;

		log_file_.Write(sstream.str());

		// Exit Player Process
		if (player1_->ExitProcess() == 0) {
			cerr << "failed to player1_->ExitProcess()" << endl;
		}
		if (player1_ != player2_) {
			if (player2_->ExitProcess() == 0) {
				cerr << "failed to player2_->ExitProcess()" << endl;
			}
		}

		return true;
	}

	// Split message as tokens (message will be destroyed)
	std::vector<std::string> SpritAsTokens(char *message, const char* const delim) {
		std::vector<std::string> tokens;
		char *ctx;
		char *token = strtok_s(message, delim, &ctx);
		while (token != nullptr) {
			tokens.push_back(token);
			token = strtok_s(nullptr, delim, &ctx);
		}
		return tokens;
	}
}