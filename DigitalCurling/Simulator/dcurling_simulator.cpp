#include "dcurling_simulator.h"

#include <random>
#include <cmath>

// Include for debug TODO: Delete in Release build
#include <bitset>
#include <iostream>
#include <iomanip>

#include "Box2D/Box2D.h"

#ifdef _WIN32
#define DLLAPI __declspec(dllexport)
#else // _WIN32
#define DLLAPI
#endif // _WIN32

#ifdef _DEBUG
#pragma comment( lib, "Box2D/bin/Box2D_d.lib" )
#endif
#ifndef _DEBUG
#pragma comment( lib, "Box2D/bin/Box2D.lib" )
#endif

namespace digital_curling {

	namespace b2simulator {

		// Constant values for Stone
		constexpr float kStoneDensity    = 10.0f;
		constexpr float kStoneResitution = 1.0f;
		constexpr float kFriction        = 12.009216f;
		constexpr float kStoneFriction   = 0.50f;//12.009216f;  // TODO: set smaller value
		constexpr float kVerticalForceCoefficient = 0.066696f;
		constexpr float kStandardAngle   = 3.14f;
		//constexpr float kForceVerticalBase = kStandardAngle * kFriction;

		// Constant values for Rink
		constexpr float kPlayAreaXLeft   = 0.000f + kStoneR;
		constexpr float kPlayAreaXRight  = kSideX - kStoneR;
		constexpr float kPlayAreaYTop    = 3.050f + kStoneR;
		constexpr float kPlayAreaYBottom = kHogY - kStoneR;
		constexpr float kRinkYTop        = 0.000f + kStoneR;
		constexpr float kRinkYBottom     = 3.050f + kRinkHeight - kStoneR;

		// Constant values for simulation
		constexpr int kVelocityIterations = 10;        // Iteration?
		constexpr int kPositionIterations = 10;        // Iteration?
		constexpr float kTimeStep = (1.0f / 1000.0f);  // Flame rate

		// Create body (= stone)
		b2Body *CreateBody(float x, float y, b2World &world) {
			b2BodyDef body_def;

			body_def.type = b2_dynamicBody;  // set body type as dynamic
			body_def.position.Set(x, y);     // set position (x, y)
			body_def.angle = 0.0f;           // set angle  0 (not affected by angle?)

			// Create body
			b2Body *body = world.CreateBody(&body_def);

			// Set CircleShape as Stone
			b2CircleShape dynamic_ball;
			dynamic_ball.m_radius = kStoneR;

			// Set FixtureDef
			b2FixtureDef fixture_def;
			fixture_def.shape       = &dynamic_ball;
			fixture_def.density     = kStoneDensity;
			fixture_def.restitution = kStoneResitution;
			fixture_def.friction = kStoneFriction;

			// Create Fixture
			body->CreateFixture(&fixture_def);

			return body;
		}

		// State of Board for b2d simulator
		class Board {
		public:
			// Set stones into board
			Board(GameState const &gs, ShotVec const &vec) : world_(b2Vec2(0, 0)) {
				// Set shot_num_
				shot_num_ = gs.ShotNum;
				// Create bodies by positions of stone in GameState
				for (unsigned int i = 0; i < gs.ShotNum; i++) {
					body_[i] = CreateBody(gs.body[i][0], gs.body[i][1], world_);
				}

				// Set ShotVec
				assert(shot_num_ < 16);
				// Create body
				body_[shot_num_] = CreateBody(kCenterX, kHackY, world_);
				// Set verocity
				body_[shot_num_]->SetLinearVelocity(b2Vec2(vec.x, vec.y));
				if (vec.angle) {
					body_[shot_num_]->SetAngularVelocity(-1 * kStandardAngle);
				}
				else {
					body_[shot_num_]->SetAngularVelocity(kStandardAngle);
				}
			}
			~Board() {
				/*
				for (unsigned int i = 0; i < 16; i++) {
					if (body_[i] != nullptr) {
						world_.DestroyBody(body_[i]);
					}
				}
				*/
			}

			b2World world_;
			b2Body *body_[16];
			unsigned int shot_num_;
		};

		// Get which area stone is in
		int GetStoneArea(const b2Vec2 &pos) {
			int ret = 0;

			// Check stone is in Rink and Playarea
			if (kPlayAreaXLeft < pos.x && pos.x < kPlayAreaXRight) {
				if (kRinkYTop < pos.y && pos.y < kRinkYBottom) {
					ret |= IN_RINK;
				}
				if (kPlayAreaYTop < pos.y && pos.y < kPlayAreaYBottom) {
					ret |= IN_PLAYAREA;
					
					// Calculate distance from center of House
					float distance = b2Vec2(pos.x - kCenterX, pos.y - kTeeY).Length();
					// Check stone is in house
					if (distance < kHouseR + kStoneR) {
						ret |= IN_HOUSE;
					}
					else if (kTeeY + kStoneR < pos.y) {
						ret |= IN_FREEGUARD;
					}
				}
			}

			return ret;
		}

		// Add friction to single stone
		b2Vec2 FrictionStep(float friction, b2Vec2 vec, float angle)
		{
			b2Vec2	v_ret = vec;
			float v_length = vec.Length();

			if (v_length > friction) {
				b2Vec2 norm, force_vertical;

				// norm = normalized vector from vec
				norm = vec;
				norm.x = vec.x / v_length;
				norm.y = vec.y / v_length;

				// force_vertical = force which is applied vertically to vec
				force_vertical.x = norm.y;
				force_vertical.y = -1 * norm.y;

				norm *= friction;
				v_ret -= norm;

				// Add vertical force
				if (angle != 0.0f) {  // Only delivered shot has angle != 0
					force_vertical *= ((angle > 0) ? 
						-friction * kVerticalForceCoefficient:
						friction * kVerticalForceCoefficient);
					v_length = v_ret.Length();
					// Add vertical force
					v_ret += force_vertical;
					// Normalize again
					v_ret.Normalize();
					// Reform vec_ret
					v_ret *= v_length;
				}

				return v_ret;
			}
			else {
				return b2Vec2(0.0f, 0.0f);
			}
		}

		// Add friction to all stones
		void FrictionAll(float friction, Board &board) {
			b2Vec2 vec;

			// Add friction to each stones in board
			for (unsigned int i = 0; i < board.shot_num_ + 1; i++) {
				if (board.body_[i] != nullptr) {
					vec = FrictionStep(
						friction, 
						board.body_[i]->GetLinearVelocity(), 
						board.body_[i]->GetAngularVelocity());
					board.body_[i]->SetLinearVelocity(vec);
					if (vec.Length() == 0) {
						board.body_[i]->SetAngularVelocity(0.0f);
					}
				}
			}
		}

		// Main loop for simulation (with recording trajectory)
		int MainLoop(const float time_step, const int loop_count, Board &board, const float friction, float *trajectory, size_t traj_size) {
			int num_steps;

			// Add friction 0.5 step at first
			FrictionAll(friction * time_step * 0.5f, board);

			for (num_steps = 0; num_steps < loop_count || loop_count == -1; num_steps++) {
				// Calclate friction
				board.world_.Step(time_step, kVelocityIterations, kPositionIterations);
				FrictionAll(friction * time_step, board);

				// Record to trajectory array
				if (trajectory != nullptr) {
					b2Vec2 vec;
					for (unsigned int i = 0; i < board.shot_num_ + 1 && i < traj_size; i++) {
						if (board.body_[i] != nullptr) {
							vec = board.body_[i]->GetPosition();
							trajectory[num_steps * 32 + i * 2] = vec.x;
							trajectory[num_steps * 32 + i * 2 + 1] = vec.y;
						}
						else {
							trajectory[num_steps * 32 + i * 2] = vec.x;
							trajectory[num_steps * 32 + i * 2 + 1] = vec.y;
						}
					}
				}

				// Check state of each stone
				for (unsigned int i = 0; i < board.shot_num_ + 1; i++) {
					if (board.body_[i] != nullptr) {
						b2Vec2 vec = board.body_[i]->GetLinearVelocity();
						// Get area of stone
						int area = GetStoneArea(board.body_[i]->GetPosition());
						if (area == OUT_OF_RINK) {
							//  Destroy body if a stone is out from Rink
							board.world_.DestroyBody(board.body_[i]);
							board.body_[i] = nullptr;
						}
						else if (vec.x != 0.0f || vec.y != 0.0f) {
							// Continue first loop if a stone is awake
							break;
						}
					}
					if (i == board.shot_num_) {
						// Break first loop if all stone is stopped
						goto LOOP_END;
					}
				}
			}

		LOOP_END:

			// Remove all stones if not in playarea
			for (unsigned int i = 0; i < board.shot_num_ + 1; i++) {
				if (board.body_[i] != nullptr) {
					// Get area of stone
					int area = GetStoneArea(board.body_[i]->GetPosition());
					if (!(area & IN_PLAYAREA)) {
						//  Destroy body if a stone is out from playarea
						board.world_.DestroyBody(board.body_[i]);
						board.body_[i] = nullptr;
					}
				}
			}

			return num_steps;
		}

		// Check Freeguard rule
		//  returns true if stone is removed in freeguard
		bool IsFreeguardFoul(
			const Board &board,               // Board after simulation
			const GameState* const gs,        // GameState before simulation
			const unsigned int num_freeguard, // Number of shot
			const StoneArea area_freeguard    // Area of freeguard
		) {
			if (gs->ShotNum < num_freeguard) {
				int area_before;
				int area_after;
				for (unsigned int i = 0; i < gs->ShotNum; i++) {
					// Get stone area before and after Simulation
					area_before =
						GetStoneArea(b2Vec2(gs->body[i][0], gs->body[i][1]));
					area_after = (board.body_[i] != nullptr) ?
						GetStoneArea(board.body_[i]->GetPosition()) :
						OUT_OF_RINK;
					if ((area_before & area_freeguard) && !(area_after & IN_PLAYAREA)) {
						return true;
					}
				}
			}
			return false;
		}

		// Get distance from (0,0)
		inline float GetDistance(float x, float y) {
			return sqrt(pow(x, 2) + pow(y, 2));
		}

		inline ShotVec RotateVec(ShotVec vec, float deg) {
			ShotVec vec_ret;

			vec_ret.x = vec.x * cos(deg) - vec.y * sin(deg);
			vec_ret.y = vec.x * sin(deg) + vec.y * cos(deg);
			vec_ret.angle = vec.angle;

			return vec_ret;
		}

		// Return score of second (which has last shot in this end)
		int GetScore(const GameState* const game_state) {

			struct Stone {
				unsigned int shot_num = 0;   // Number of shot
				float distance = 9999.0f;    // Distance from center of House
			} stone[16];

			// Get Number and Distance
			for (unsigned int i = 0; i < game_state->ShotNum; i++) {
				stone[i].shot_num = i;
				stone[i].distance = GetDistance(
					game_state->body[i][0] - kCenterX, 
					game_state->body[i][1] - kTeeY);
			}

			// Sort by distance
			std::sort(stone, stone + game_state->ShotNum, 
				[](const Stone& s1, const Stone s2) {return s1.distance < s2.distance; });

			// Calculate score
			int score = 0;                    // Score
			int mod = stone[0].shot_num % 2;  // Player which has no.1 stone
			int base = (mod == 0) ? -1 : 1;   // Score per 1 stone
			for (unsigned int i = 0; i < game_state->ShotNum; i++) {
				if ((stone[i].distance < kHouseR + kStoneR) &&
					(stone[i].shot_num % 2 == mod)) {
					score += base;
				}
				else {
					break;
				}
			}
			
			return score;
		}

		// Update game_state from board
		void UpdateState(const Board &board, GameState* const game_state) {
			// Update ShotNum (ShotNum can be 16, not reset to 0)
			game_state->ShotNum++;

			// Update positions
			for (unsigned int i = 0; i < game_state->ShotNum; i++) {
				if (board.body_[i] != nullptr) {
					b2Vec2 pos = board.body_[i]->GetPosition();
					game_state->body[i][0] = pos.x;
					game_state->body[i][1] = pos.y;
				}
				else {
					game_state->body[i][0] = 0.0f;
					game_state->body[i][1] = 0.0f;
				}
			}

			// Update Score if ShotNum == 16
			if (game_state->ShotNum == 16) {
				// Calculate Socre
				int score = GetScore(game_state);
				game_state->Score[game_state->CurEnd] = (game_state->WhiteToMove) ? -score : score;
				// Update WhiteToMove
				game_state->WhiteToMove ^= (score <= 0);
			}
			else {
				// Update WhiteToMove
				game_state->WhiteToMove ^= true;
			}
		}

		ShotVecP ConvertVec(ShotVec vec_rect) {
			ShotVecP vec_polar;
			
			vec_polar.v = GetDistance(vec_rect.x, vec_rect.y);  // v
			vec_polar.theta = atan2(vec_rect.x, vec_rect.y);  // theta

			return vec_polar;
		}

		ShotVec ConvertVec(ShotVecP vec_polar) {
			ShotVec vec_rect;

			vec_rect.x = vec_polar.v * sin(vec_polar.theta);
			vec_rect.y = vec_polar.v * cos(vec_polar.theta);

			return vec_rect;
		}

		/*** Member functions of class 'Simulator' ***/

		Simulator::Simulator() :
			num_freeguard_(3),
			area_freeguard_(IN_FREEGUARD),
			random_type_(RECTANGULAR),
			friction_(kFriction) {

			// initialize shot_table
			init_shot_table();
		}

		Simulator::Simulator(float friction) :
			num_freeguard_(3),
			area_freeguard_(IN_FREEGUARD),
			random_type_(RECTANGULAR),
			friction_(friction) {

			// initialize shot_table
			init_shot_table();
		}

		// Simulation with Box2D (compatible with Simulation() in CurlingSimulator.h)
		int Simulator::Simulation(
			GameState* const game_state, 
			ShotVec shot_vec, 
			float random_x, float random_y, 
			ShotVec* const run_shot, 
			float *trajectory, size_t traj_size) {

			if (game_state->ShotNum > 15) {
				return -1;
			}

			// Add random number to shot
			AddRandom2Vec(random_x, random_y, &shot_vec);
			if (run_shot != nullptr) {
				// Copy random-added shot_vec to run_shot
				memcpy_s(run_shot, sizeof(ShotVec), &shot_vec, sizeof(ShotVec));
			}

			// Create board
			Board board(*game_state, shot_vec);

			// Run mainloop of simulation
			int steps;
			steps = MainLoop(kTimeStep, -1, board, friction_, trajectory, traj_size);

			// Check freeguard zone rule
			if (IsFreeguardFoul(board, game_state, num_freeguard_, area_freeguard_)) {
				game_state->ShotNum++;
				game_state->WhiteToMove ^= 1;
				return 0;
			}

			// Update game_state
			UpdateState(board, game_state);

			return steps;
		}

		// Create shot from coordinate (x, y)
		b2Vec2 CreateShotXY(float x, float y, float friction) {
			b2Vec2 Shot;
			float len;

			Shot.Set(x - kCenterX, y - kHackY);
			len = Shot.Length();
			Shot.Normalize();
			Shot.operator *=(sqrt(len * 2.0f * friction));

			return Shot;
		}

		// Create ShotVec from ShotPos which a stone will stop at
		void Simulator::CreateShot(ShotPos pos, ShotVec* const vec) {
			float tt = 0.0335f;
			float x_base = 1.22f;
			b2Vec2 vec_tmp;

			if (pos.angle == true) {
				vec_tmp = CreateShotXY(
					x_base + pos.x - tt*(pos.y - kTeeY), 
					tt*(pos.x - kCenterX) + pos.y,
					friction_);
			}
			else {
				vec_tmp = CreateShotXY(
					-x_base + pos.x + tt*(pos.y - kTeeY), 
					-tt*(pos.x - kCenterX) + pos.y,
					friction_);
			}

			vec->x = vec_tmp.x;
			vec->y = vec_tmp.y;
			vec->angle = pos.angle;
		}

		// Only for CreateHitShot from CurlingSimulator v2.x
		// ==================================================
		int CreateShot(ShotPos Shot, ShotVec *lpResShot)
		{
			float tt = 0.0335f;
			b2Vec2 vec, TeePos;
			TeePos.Set(kCenterX, kTeeY);

			if (Shot.angle == false) {
				vec = CreateShotXY(1.22f + Shot.x - tt * (Shot.y - TeePos.y), tt*(Shot.x - TeePos.x) + Shot.y, kFriction);
			}
			else {
				vec = CreateShotXY(-1.22f + Shot.x + tt * (Shot.y - TeePos.y), -tt * (Shot.x - TeePos.x) + Shot.y, kFriction);
			}

			lpResShot->x = vec.x;
			lpResShot->y = vec.y;
			lpResShot->angle = Shot.angle;

			return true;
		}
		// ショットの方向・強さ・回転方向からショットを生成
		bool CreateShotPower(ShotPos Shot, float Power, ShotVec *lpResShot)
		{
			bool bRet = false;
			ShotPos tmpShot;
			float StY;

			if (Power >= 0.0f) {
				if (Power >= 0.0f && Power <= 1.0f) {
					StY = 11.28f + ((0.0f - Power) * 0.76f);
				}
				else if (Power > 1.0f && Power <= 3.0f) {
					StY = 10.52f + ((1.0f - Power) * 1.52f);
				}
				else if (Power > 3.0f && Power <= 4.0f) {
					StY = 7.47f + ((3.0f - Power) * 0.76f);
				}
				else if (Power > 4.0f && Power <= 10.0f) {
					StY = 6.71f + ((4.0f - Power) * 0.61f);
				}
				else if (Power > 10.0f) {
					if (Power > 16) {
						Power = 16;
					}
					StY = 3.05f + ((10.0f - Power) * 1.52f);
				}

				tmpShot.x = ((Shot.x - kCenterX) * (StY - 41.28f) / (Shot.y - 41.28f)) + kCenterX;
				tmpShot.y = StY;
				tmpShot.angle = Shot.angle;

				CreateShot(tmpShot, lpResShot);
				bRet = true;
			}
			else {
				CreateShot(Shot, lpResShot);
				bRet = true;
			}

			return bRet;
		}
		// ==================================================
		// Only for CreateHitShot from CurlingSimulator v2.x

		// Create ShotVec from ShotPos which stone will pass through
		void Simulator::CreateHitShot(ShotPos pos, float weight, ShotVec* const vec) {
			// Calclate distance between target pos and initial pos
			b2Vec2 pos_target(pos.x - kCenterX, kHackY - pos.y);
			float dist_target = GetDistance(pos_target.x, pos_target.y);

			// Prepare board
			ShotVec vec_tmp;
			CreateShotPower(pos, weight, &vec_tmp);
			GameState game_state;

			// Create board
			Board board(game_state, vec_tmp);

			float time_step = kTimeStep;
			// Add friction 0.5 step at first
			FrictionAll(friction_ * time_step * 0.5f, board);

			b2Vec2 pos_now, pos_prev;
			unsigned int num_steps;
			for (num_steps = 0; num_steps < kTableSize; num_steps++) {
				// Calclate friction
				board.world_.Step(time_step, kVelocityIterations, kPositionIterations);
				FrictionAll(friction_ * time_step, board);

				pos_prev = pos_now;
				pos_now = board.body_[0]->GetPosition();
				pos_now = b2Vec2(pos_now.x - kCenterX, kHackY - pos_now.y);

				// Get current position
				float dist = pos_now.Length();
				if (dist > dist_target) {
					// Calclate position of pseudo target
					b2Vec2 u = pos_now - pos_prev;
					float a = dist - pos_prev.Length();
					float b = dist_target - pos_prev.Length();
					u *= b / a;
					b2Vec2 pseudo_target = pos_prev + u;

					// Calclate rotation degree
					float deg_tar = atan2(pos_target.y, pos_target.x);
					float deg_pseudo_tar = atan2(pseudo_target.y, pseudo_target.x);
					float deg = deg_pseudo_tar - deg_tar;

					// Rotate vector
					*vec = RotateVec(vec_tmp, deg);
					break;
				}
			}
		}

		// Add random number to ShotVec (normal distribution)
		void Simulator::AddRandom2Vec(float random_1, float random_2, ShotVec* const vec) {
			if (random_1 == 0.0f && random_2 == 0.0f) {
				return;
			}

			// Prepare random
			std::random_device seed_gen;
			std::default_random_engine engine(seed_gen());

			ShotPos tee_pos(kCenterX, kTeeY, vec->angle);
			ShotVec tee_shot, add_rand_tee_shot;
			float r1 = 0.0f;
			float r2 = 0.0f;

			// Add random to tee_pos (normal distribution)
			if (random_1 != 0.0f) {
				std::normal_distribution<float> dist_1(0, random_1);
				r1 = dist_1(engine);
			}
			if (random_2 != 0.0f) {
				std::normal_distribution<float> dist_2(0, random_2);
				r2 = dist_2(engine);
			}

			// for rectangular coordinate system
			if (random_type_ == RECTANGULAR) {
				// Create shot to center of house
				CreateShot(tee_pos, &tee_shot);
				CreateShot(tee_pos + ShotPos(r1, r2, vec->angle), &add_rand_tee_shot);

				// Add random to vec
				*vec += tee_shot - add_rand_tee_shot;
			} 
			// for polar coordinate system
			else if (random_type_ == POLAR) {
				ShotVecP vec_polar;
				vec_polar = ConvertVec(*vec);
				vec_polar.v += r1;
				vec_polar.theta += r2;
				*vec = ConvertVec(vec_polar);
			}
		}

		// Initialize shot_table
		int Simulator::init_shot_table() {

			ShotVec shot_vec(0.0f, -55.0f, false);
			GameState game_state;

			float time_step = kTimeStep / 2.0f;

			// Create board
			Board board(game_state, shot_vec);

			// Add friction 0.5 step at first
			FrictionAll(friction_ * time_step * 0.5f, board);

			unsigned int num_steps;
			for (num_steps = 0; num_steps < kTableSize; num_steps++) {
				// Calclate friction
				board.world_.Step(time_step, kVelocityIterations, kPositionIterations);
				FrictionAll(friction_ * time_step, board);

				// Record to shot_table_
				b2Vec2 pos = board.body_[0]->GetPosition();
				shot_table_.pos[num_steps].x = pos.x;
				shot_table_.pos[num_steps].y = pos.y;
				b2Vec2 vec = board.body_[0]->GetLinearVelocity();
				shot_table_.vec[num_steps].x = vec.x;
				shot_table_.vec[num_steps].y = vec.y;
				shot_table_.vec[num_steps].angle = false;

				if ( vec.x == 0.0f && vec.y == 0.0f) {
					break;
				}
			}
			shot_table_.index_end = num_steps;

			return 0;
		}
	}
}