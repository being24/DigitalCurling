#pragma once

#include <Windows.h>

#include <string>
#include <vector>

#include "dc_util.h"
#include "game_player.h"
#include "game_log.h"

namespace digital_curling
{
	// Options for simulator
	struct SimulatorParams {
		float friction;
		int random_generator;
		unsigned int freeguard_num;
	};

	// DigitalCrling game
	class GameProcess {
	public:
		GameProcess(Player *p1, Player *p2, int num_ends, int rule_type);
		GameProcess(Player *p1, Player *p2, int num_ends, int rule_type, bool extend_end, SimulatorParams params);
		~GameProcess();

		enum {
			ERR,
			BESTSHOT,
			CONCEDE,
			TIMEOUT
		};

		const int rule_type_;  // Type of rule (0: standard, 1:mix_doubles) 

		// Send 'ISREADY' command and wait for recieving 'READYOK' from a player
		bool IsReady(Player *p, unsigned int time_out);

		// Send 'NEWGAMWE' command to both player
		bool NewGame();

		// Prepare for End
		bool PrepareEnd(unsigned int time_out);

		// Send 'SETSTATE' command and 'POSITION' command to both player
		bool SendState();

		// Send 'GO' command and wait for recieving 'BESTSHOT' from a player
		int Go();

		// Simulate a shot
		bool RunSimulation();

		// Send 'SCORE' command
		bool SendScore();

		// Exit game process
		bool Exit();

		b2simulator::Simulator *sim;  // Simulator

		GameState gs_;         // State of the game
		//float     random_;     // Size of random number

		ShotVec best_shot_;    // Shot vector recieved
		ShotVec run_shot_;     // Shot vector (add random)

		Player    *player1_;   // Player 1
		Player    *player2_;   // Player 2

		GameLog log_file_;     // log file

		bool extended_end_;     // do extended end if draw
	};

	// Split message as tokens
	std::vector<std::string> SpritAsTokens(char *message, const char* const delim);
}