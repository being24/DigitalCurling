//======================================================
// This is a sample of Curling AI program.
// This sample shows how to recieve or return commands from or to the server
// also how to use digital_curling::b2simulator.
//======================================================

#include <windows.h>
#include <iostream>

#include "../Simulator/dcurling_simulator.h"

#ifdef _DEBUG
#pragma comment( lib, "../x64/Debug/Simulator.lib" )
#endif
#ifndef _DEBUG
#pragma comment( lib, "../x64/Release/Simulator.lib" )
#endif


constexpr unsigned int kBufferSize = 1024;  // Buffer size
digital_curling::GameState gs;
digital_curling::b2simulator::Simulator *sim = nullptr;

unsigned int power_play_count = 0;
bool move_info = true;
int score_diff = 0;

void Send(const char* const Message);
void Recv(char* Message, size_t Size);
bool DoCommand(char *Message);

int main()
{
	char Message[kBufferSize];

	while (1) {
		memset(Message, 0x00, sizeof(Message));

		// Recieve command
		Recv(Message, sizeof(Message));

		// Process command
		DoCommand(Message);
	}

	return 0;
}

// Send Message
void Send(const char* const Message)
{
	DWORD NumberOfBytesWritten;
	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), Message, (DWORD)strlen(Message), &NumberOfBytesWritten, NULL);
}

// Recv Message
void Recv(char* Message, size_t Size)
{
	DWORD NumberOfBytesRead;
	ReadFile(GetStdHandle(STD_INPUT_HANDLE), Message, (DWORD)Size, &NumberOfBytesRead, NULL);
}

// Delete new line
void DeleteNL(char *Message)
{
	char *p;
	p = Message;

	while (*p != 0x00) {
		if (*p == '\n' || *p == '\r') {
			*p = 0x00;
			break;
		}
		p++;
	}
	return;
}

// Get arguments from message
bool GetArgument(char *lpResult, size_t numberOfElements, char *Message, int n)
{
	bool bRet = false;
	char *p, *q;

	if (Message != NULL) {
		p = Message;
		while (*p == ' ') {
			p++;
		}

		// ポインタを取得したい引数の先頭に合わせる
		for (int i = 0; i < n; i++) {
			while (*p != ' ') {
				if (*p == 0x00) {
					return false;
				}
				p++;
			}
			while (*p == ' ') {
				p++;
			}
		}

		// 取得したい引数をlpResultにコピーする
		q = strstr(p, " ");
		if (q == NULL) {
			strcpy_s(lpResult, kBufferSize, p);
			bRet = true;
		}
		else {
			strncpy_s(lpResult, numberOfElements, p, q - p);
			if ((unsigned int)(q - p) < numberOfElements) {
				lpResult[q - p] = 0x00;
			}
			bRet = true;
		}
	}

	return bRet;
}

namespace digital_curling{
	// Check a stone is in house or not
	bool IsInHouse(float x, float y)
	{
		return (pow(kCenterX - x, 2) + pow(kTeeY - y, 2)) < pow(kHouseR + kStoneR, 2);
	}

	// Get shot number of No.1 stone
	int GetNumberOneStone(GameState *gs, ShotPos *pShotPos)
	{
		int iRet = -1;
		float dist_max = 99999.000;
		float dist = 0.000;
		for (unsigned int i = 0; i < gs->ShotNum; i++) {
			dist = pow(kCenterX - gs->body[i][0], 2) + pow(kTeeY - gs->body[i][1], 2);
			if (dist < dist_max) {
				iRet = i;
				dist_max = dist;
			}
		}

		pShotPos->x = gs->body[iRet][0];
		pShotPos->y = gs->body[iRet][1];

		return iRet;
	}

	// Calclate simple shot
	ShotVec GetSimpleShot(GameState *gs)
	{
		ShotPos pos;
		ShotVec vec;

		// if No.1 stone is mine:
		//  Create guard shot
		int NumberOneStone = GetNumberOneStone(gs, &pos);
		if ((NumberOneStone % 2) == (gs->ShotNum % 2)) {
			if (IsInHouse(pos.x, pos.y) == TRUE) {
				pos.y += 2.000;
				if (pos.x > kCenterX) {
					pos.angle = 0;
				}
				else {
					pos.angle = 1;
				}
				sim->CreateShot(pos, &vec);
				return vec;
			}
		}
		// if No.1 stone is opponent's:
		//  Create hit shot
		else {
			if (IsInHouse(pos.x, pos.y) == TRUE) {
				if (IsInHouse(pos.x, pos.y) == TRUE) {
					if (pos.x > kCenterX) {
						pos.angle = 0;
					}
					else {
						pos.angle = 1;
					}

					//std::cerr << "CreateHitShot " << pos.x << " " << pos.y << " " << pos.angle << std::endl;
					sim->CreateHitShot(pos, 16, &vec);
					return vec;
				}
			}
		}

		// if there is no stone in house:
		//  Create draw shot to the center of house
		pos.x = kCenterX;
		pos.y = kTeeY;
		pos.angle = 0;
		sim->CreateShot(pos, &vec);

		return vec;
	}
}

// Process command
bool DoCommand(char *Message)
{
	using namespace std;
	using namespace digital_curling;

	char CMD[kBufferSize];
	char Buffer[kBufferSize];

	// Delete new line
	DeleteNL(Message);

	// Get Command from message
	if (GetArgument(CMD, sizeof(CMD), Message, 0) == false) {
		return false;
	}

	// Process command
	if (_stricmp(CMD, "NEWGAME") == 0) {
		//======================================================
		// NOTE:
		//  You recieve 'NEWGAME' with name of two players.
		//  Use GetArgument() if you want to get them (please refer to 'ISREADY').
		//======================================================
		
	}
	else if (_stricmp(CMD, "ISREADY") == 0) {
		//======================================================
		// NOTE:
		//  You can initialize something which takes few seconds
		// after recieving 'ISREADY' from the server.
		//  Then you should return 'READYOK' to the server.
		//======================================================

		// Initialize Simulator
		if (sim == nullptr) {
			sim = new b2simulator::Simulator();
		}

		// Return 'READYOK' to the server
		Send("READYOK");
	}
	else if (_stricmp(CMD, "POSITION") == 0) {
		//======================================================
		// NOTE:
		//  You can get positions from 'POSITION'.
        //======================================================

		for (unsigned int i = 0; i < 16; i++) {
			// X coordinate
			if (!GetArgument(Buffer, sizeof(Buffer), Message, 2 * i + 1)) {
				return false;
			}
			gs.body[i][0] = (float)atof(Buffer);

			// Y coordinate
			if (!GetArgument(Buffer, sizeof(Buffer), Message, 2 * i + 2)) {
				return false;
			}
			gs.body[i][1] = (float)atof(Buffer);
		}
	}
	else if (_stricmp(CMD, "SETSTATE") == 0) {
		//======================================================
		// NOTE:
		//  You can get information about shot, end, move from 'SETSTATE'
		//======================================================

		// Get number of shots
		if (GetArgument(Buffer, sizeof(Buffer), Message, 1) == FALSE) {
			return false;
		}
		gs.ShotNum = atoi(Buffer);

		// Get number of ends
		if (GetArgument(Buffer, sizeof(Buffer), Message, 2) == FALSE) {
			return false;
		}
		gs.CurEnd = atoi(Buffer);

		// Gen number of last end
		if (GetArgument(Buffer, sizeof(Buffer), Message, 3) == FALSE) {
			return false;
		}
		gs.LastEnd = atoi(Buffer);

		// Get info about move (false: first player in 1st end, true: second player in 1st end)
		if (GetArgument(Buffer, sizeof(Buffer), Message, 4) == FALSE) {
			return false;
		}
		if (atoi(Buffer) == 1) {
			gs.WhiteToMove = true;
		}
		else {
			gs.WhiteToMove = false;
		}
	}
	else if (_stricmp(CMD, "PUTSTONE") == 0) {
		//======================================================
		// NOTE:
		//  You should return 'PUTSTONE num' (num = 0, 1, 2, 3)
		// to place stones for Mix Doubles rule.
		//======================================================
		if (power_play_count == 0 && score_diff < 0) {
			power_play_count++;
			Send("PUTSTONE 2");
			cerr << "PUTSTONE 2" << endl;
		}
		else {
			Send("PUTSTONE 0");
		}
	}
	else if (_stricmp(CMD, "GO") == 0) {
		//======================================================
		// NOTE:
		//  You should calclate the vector of shot and 
		// return 'BESTSHOT' to the server.
		//======================================================

		// You can get timelimit
		unsigned int timelimit;
		int num;
		move_info = gs.WhiteToMove;
		if (gs.WhiteToMove) {
			num = 2;
		}
		else {
			num = 1;
		}
		if (GetArgument(Buffer, sizeof(Buffer), Message, num)) {
			timelimit = atoi(Buffer);
		}
		else {
			timelimit = INT_MAX;
		}
		cerr << "timelimit = " << timelimit << endl;


		// Calclate shot vector to return
		ShotVec vec = GetSimpleShot(&gs);

		// 最善ショットの送信
		sprintf_s(Buffer, sizeof(Buffer), "BESTSHOT %f %f %d", vec.x, vec.y, vec.angle);
		cout << Buffer <<  endl;
		Send(Buffer);
	}
	else if (_stricmp(CMD, "SCORE") == 0) {
		if (GetArgument(Buffer, sizeof(Buffer), Message, 1) == FALSE) {
			return false;
		}
		if (move_info) {
			score_diff -= atoi(Buffer);
		}
		else {
			score_diff += atoi(Buffer);
		}
	}

	return true;
}