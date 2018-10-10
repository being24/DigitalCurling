//#include "Box2D/Box2D.h"
#include "dcurling_simulator.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>

using digital_curling::GameState;
using digital_curling::ShotPos;
using digital_curling::ShotVec;

using digital_curling::b2simulator::Simulator;

using std::cout;
using std::endl;

void operator_test() {
	ShotPos pos1(5.0f, 1.0f, false);
	ShotPos pos2(2.0f, -2.0f, false);

	ShotPos pos_add = pos1 + pos2;

	cout << "pos_add.x = " << pos_add.x << endl;
	cout << "pos_add.y = " << pos_add.y << endl;
	cout << "pos_add.angle = " << pos_add.angle << endl;

	ShotPos pos_sub = pos1 - pos2;
	cout << "pos_sub.x = " << pos_sub.x << endl;
	cout << "pos_sub.y = " << pos_sub.y << endl;
	cout << "pos_sub.angle = " << pos_sub.angle << endl;

	pos_add += pos1;
	cout << "pos_add.x = " << pos_add.x << endl;
	cout << "pos_add.y = " << pos_add.y << endl;
	cout << "pos_add.angle = " << pos_add.angle << endl;
}

inline void PrintGameState(const GameState &gs) {
	cout << "ShotNum = " << gs.ShotNum << ", CurEnd = " << gs.CurEnd << "/" << gs.LastEnd << endl;
	for (int i = 0; i < 16; i++) {
		cout << "(" << gs.body[i][0] << ", " << gs.body[i][1] << ") ";
	}
	cout << endl;
}

void simulation_test() {
	GameState gs(8);
	ShotVec vec(-0.99074f, -29.559774f, false);

	const size_t traj_size = 4096;
	float trajectory[32 * traj_size];
	memset(trajectory, 0, sizeof(float) * 32 * traj_size);

	time_t time_start = clock();
	Simulator sim;
	time_t time_spent = clock() - time_start;
	cout << "time spent for initializing Simulator = " << time_spent << " [ms]" << endl;

	int steps = sim.Simulation(&gs, vec, 0, 0, nullptr, trajectory, traj_size);

	PrintGameState(gs);

	std::ofstream ofs("trajectory_log.txt");
	for (int i = 0; i < steps; i++) {
		for (unsigned int j = 0; j < 16; j++) {
			ofs <<
				std::fixed << std::setprecision(6) << trajectory[32 * i + 2 * j] << "," <<
				std::fixed << std::setprecision(6) << trajectory[32 * i + 2 * j + 1];
			if (j < 15) {
				ofs << ",";
			}
		}
		ofs << endl;
	}

	steps = sim.Simulation(&gs, vec, 0, 0, nullptr, trajectory, traj_size);

	PrintGameState(gs);
}

void score_test() {
	using namespace digital_curling;
	GameState gs(8);
	gs.body[0][0] = kCenterX + kStoneR;
	gs.body[0][1] = kTeeY;
	gs.body[1][0] = kCenterX;
	gs.body[1][1] = kTeeY + 3 * kStoneR;
	gs.body[2][0] = kCenterX + kStoneR;
	gs.body[2][1] = kTeeY + 5 * kStoneR;
	gs.ShotNum = 3;

	cout << "Score = " << b2simulator::GetScore(&gs) << endl;
}

void create_shot_test() {
	using namespace digital_curling;
	ShotVec vec;

	Simulator sim;
	sim.CreateShot(ShotPos(kCenterX, kTeeY, false), &vec);

	GameState gs;
	sim.Simulation(&gs, vec, 0, 0, nullptr, nullptr, -1);

	ShotPos pos(gs.body[0][0], gs.body[0][1], false);
	cout << "dist(after sim) = " << sqrt(pow(pos.x, 2) + pow(pos.y, 2)) << endl;

	// CreateHitShot test
	const size_t traj_size = 4096;
	float trajectory[32 * traj_size];
	memset(trajectory, 0, sizeof(float) * 32 * traj_size);

	sim.CreateHitShot(ShotPos(kCenterX, kTeeY, false), 16, &vec);
	sim.Simulation(&gs, vec, 0, 0, nullptr, trajectory, traj_size);

	std::ofstream ofs("trajectory_log.txt");
	for (int i = 0; i < traj_size; i++) {
		for (unsigned int j = 0; j < 16; j++) {
			ofs <<
				std::fixed << std::setprecision(6) << trajectory[32 * i + 2 * j] << "," <<
				std::fixed << std::setprecision(6) << trajectory[32 * i + 2 * j + 1];
			if (j < 15) {
				ofs << ",";
			}
		}
		ofs << endl;
	}

	cout << "vec = (" << vec.x << "," << vec.y << ")" << endl;
	cout << "pos = (" << pos.x << "," << pos.y << ")" << endl;
}

void random_test() {
	using namespace digital_curling;

	// Create game_state
	GameState gs(8);

	// Create shot
	ShotVec vec, vec_tmp;

	Simulator sim;
	sim.CreateShot(ShotPos(kCenterX, kTeeY, false), &vec);


	int sigma1_x = 0;
	int sigma2_x = 0;
	const int loop = 5000;
	const float random = 0.145f;
	time_t start = clock();

	for (int i = 0; i < loop; i++) {
		gs.Clear();
		vec_tmp = vec;
		sim.Simulation(&gs, vec_tmp, random, random, nullptr, nullptr, 0);
		if (pow(kCenterX - gs.body[0][0], 2) < pow(random, 2)) {
			sigma1_x++;
		}
		if (pow(kCenterX - gs.body[0][0], 2) < pow(2 * random, 2)) {
			sigma2_x++;
		}
	}
	time_t time_spent = clock() - start;

	float sigma1 = (float)sigma1_x / (float)loop;
	float sigma2 = (float)sigma2_x / (float)loop;
	cout << "Rate in sigma1, sigma2: " << sigma1 << ", " << sigma2 << endl;
	cout << "Time spent = " << time_spent << endl;
}

void convert_test() {
	using namespace digital_curling;
	ShotVec vecr, vecrp;
	ShotVecP vecp;

	Simulator sim;
	sim.CreateShot(ShotPos(kCenterX, kTeeY, false), &vecr);

	vecp = b2simulator::ConvertVec(vecr);

	vecrp = b2simulator::ConvertVec(vecp);

	cout << "hoge" << endl;
}

int  main(void) {

	//operator_test();
	//simulation_test();
	//score_test();
	create_shot_test();
	//random_test();
	//convert_test();

	return 0;
}