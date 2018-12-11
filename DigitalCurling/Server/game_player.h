#pragma once

#include <Windows.h>
#include <string>

namespace digital_curling {

	// Player information
	struct PlayerInfo {
		unsigned int nplayers;       // number of players
		unsigned int order[4];        // shot order
		struct Params {
			float random_1;  // size of random 1 (x or v)
			float random_2;  // size of random 2 (y or theta)
			float shot_max;  // max size of shot
		} params[4];        // parameters of each player

		PlayerInfo();
		PlayerInfo(unsigned int num_players);
	};

	// Information of player (abstruct class)
	class Player {
	public:
		static const size_t kBufferSize = 1024;
		static const int kTimeLimitInfinite = INT_MAX;

		// Send message from player
		virtual int Send(const char *message) = 0;
		// Recieve message from player
		virtual int Recv(char *message) = 0;

		// Create process 
		// This function returns 0 when CreateProcess was failed
		virtual int InitProcess() = 0;
		// Exit process
		virtual int ExitProcess() = 0;

		std::string name_;  // Player's name
		int time_limit_;    // Timelimit
		int time_remain_;   // Timelimit remaining

		//float random_x_;    // random number x
		//float random_y_;    // random number y

		PlayerInfo pinfo_;
		
		bool mix_doubles;
	};

	// Player running on local
	class LocalPlayer : public Player {
	public:
		LocalPlayer(std::string path, int time_limit, float random_x, float random_y);
		LocalPlayer(std::string path, int time_limit, PlayerInfo pinfo);
		~LocalPlayer();

		// Send message to player
		int Send(const char *message);
		// Recieve message from player
		int Recv(char *message);

		// Create process 
		// This function returns 0 when CreateProcess was failed
		int InitProcess();
		// Exit process
		int ExitProcess();

		std::string path_;  // Full path of .exe file

		PROCESS_INFORMATION pi_;
		HANDLE write_pipe_;
		HANDLE read_pipe_;

	};

	// Player running on nerwork (using socket to communicate)
	/*
	// TODO: ŽÀ‘•‚·‚é
	class NetworkPlayer : public Player {
	public:

		NetworkPlayer();
		~NetworkPlayer();

		// Send message to player
		int Send(const char *message);
		// Recieve message from player
		int Recv(char *message);
	};
	*/
}