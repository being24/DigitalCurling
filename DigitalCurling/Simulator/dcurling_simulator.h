#pragma once

#ifndef DLLAPI
#ifdef _WIN32
#define DLLAPI __declspec(dllexport)
#else // _WIN32
#define DLLAPI
#endif // _WIN32
#endif // _DLLAPI

namespace digital_curling {

		// Constant values
		constexpr unsigned int kLastEndMax = 10;  // Maximum number of LastEnd

		constexpr float kCenterX     =  2.375f;  // X coord of Center Line
		constexpr float kTeeY        =  4.880f;  // Y coord of Tee Line
		constexpr float kSideX       =  4.750f;  // X coord of Side Line
		constexpr float kHogY        = 11.280f;  // Y coord of Hog Line
		constexpr float kHackY       = 41.280f;  // Y coord of Hack?
		constexpr float kRinkHeight  = 42.500f;  // Height of Rink
		constexpr float kStoneR      =  0.145f;  // Radius of Stone
		constexpr float kHouseR      =  1.830f;  // Radius of House (12 foot circle)
		constexpr float kHouse8FootR =  1.220f;  // Radius of House ( 8 foot circle)
		constexpr float kHouse4FootR =  0.610f;  // Radius of House ( 4 foot circle)

		// Play Area coordinates
		constexpr float kPlayAreaXLeft = 0.000f + kStoneR; 
		constexpr float kPlayAreaXRight = kSideX - kStoneR;
		constexpr float kPlayAreaYTop = 3.050f + kStoneR;
		constexpr float kPlayAreaYBottom = kHogY - kStoneR;

		// RInk coordinates
		constexpr float kRinkYTop = 0.000f + kStoneR;
		constexpr float kRinkYBottom = 3.050f + kRinkHeight - kStoneR;

		// State of DigitalCurling game
		class DLLAPI GameState {
		public:
			GameState();
			GameState(unsigned int last_end);
			~GameState();

			// Clear body and Set ShotNum = 0
			void Clear();

			// Clear all variables
			void ClearAll();

			// Set stone 
			void Set(unsigned int num, float x, float y);

			//  Note: Same member variables as GAMESTATE in CurlingSimulator older ver2.x
			unsigned int ShotNum;    // Number of current Shot

			unsigned int CurEnd;     // Number of current End (0 to LastEnd - 1)
			unsigned int LastEnd;    // Number of last End    (up to kLastEndMax)

			int Score[kLastEndMax];  // Score of each End 
									 //       > 0  : first player scored (second scored 0)
									 //       < 0  : second player scored (first scored 0)
									 //       = 0  : blank end (both scored 0)

			bool WhiteToMove;        // which have next shot
									 //       true  : first
									 //       false : second

			float body[16][2];       // position of stones
			                         //       [n][0] : x coord of n th deliverd stone
			                         //       [n][1] : y coord
		};

		// Position of Stone
		class DLLAPI ShotPos {
		public:
			ShotPos();
			ShotPos(float x, float y, bool angle);
			~ShotPos();

			float x;     // X-coordinate
			float y;     // Y-coordinate
			bool angle;  // Curl angle TODO: delete
		};

		class DLLAPI ShotVecP;

		// Vector (velocity vector) of Stone
		class DLLAPI ShotVec {
		public:
			ShotVec();
			ShotVec(float x, float y, bool angle);
			~ShotVec();

			// Convert to ShotVecP
			ShotVecP Convert();

			float x;     // X-conponent
			float y;     // Y-conponent
			bool angle;  // Curl angle
		};

		// Vector (velocity vector) of Stone (Polar Coordinate System)
		class DLLAPI ShotVecP {
		public:
			ShotVecP();
			ShotVecP(float v, float theta, bool angle);
			~ShotVecP();

			// Convert to ShotVec
			ShotVec Convert();

			float v;      // V-conponent
			float theta;  // Theta-conponent
			bool angle;   // Curl angle
		};

		// Simulator with Box2D 2.3.0 (http://box2d.org/)
		namespace b2simulator {

			// Area of stone
			typedef enum {
				OUT_OF_RINK  = 0b0000,  // out of rink
				IN_RINK      = 0b0001,  // in rink (include below hog line)
				IN_PLAYAREA  = 0b0010,  // in play area
				IN_FREEGUARD = 0b0100,  // in freeguard zone (below tee line && out of house)
				IN_HOUSE     = 0b1000   // in house
			} StoneArea;
			// Get area of stone
			int GetStoneArea(ShotPos pos);

			enum {
				RECTANGULAR,  // rectangular coordinate system
				POLAR         // polar coordinate system
			};

			class DLLAPI Simulator {
			public:
				Simulator();
				Simulator(float friction);
				Simulator(float friction, float friction_stone);

				// Simulation with Box2D, returns number of steps taken
				// - GameState* game_state    : Current state and updated state after simulation
				// - ShotVec* shot_vec        : Shot Vector
				// - float random_x, random_y : Size of random number (v, theta when random_type_ = POLAR)
				// - ShotVec* run_shot        : Shot Vector which with random numbers (pass nullptr if you don't need)
				// - float trajectory         : trajectory log (pass float[traj_size][2], or nullptr if you don't need)
				// - float traj_size          : size of trajectory log (number of steps)
				int Simulation(
					GameState* const game_state, ShotVec shot_vec,
					float random_x, float random_y,
					ShotVec* const run_shot, float *trajectory, size_t traj_size);

				// Create ShotVec from ShotPos which stone will stop at
				void CreateShot(ShotPos pos, ShotVec* const vec);

				// Create ShotVec from ShotPos which stone will pass through
				void CreateHitShot(ShotPos pos, float weight, ShotVec* const vec);

				// Add random number to ShotVec
				//  random_1 : x (rectangular), v (polar)
				//  random_2 : y (rectangular), theta (polar)
				void AddRandom2Vec(float random_1, float random_2, ShotVec* const vec);

				// Get score for second player (which has last shot in this end)
				static int GetScore(const GameState* const game_state);

				unsigned int num_freeguard_;   // Number of shots which freeguard rule is applied
				StoneArea area_freeguard_;     // Area of freeguard
				unsigned int random_type_;      // Type of random number generator (0: )
			
			private:
				int init_shot_table();

				float friction_;       // friction between stone and ice
				float friction_stone;  // friction between 2 stones

				static const unsigned int kTableSize = 10000;//8192;
				struct ShotTable {
					unsigned int index_end;
					ShotPos pos[kTableSize];
					ShotVec vec[kTableSize];
				} shot_table_;
			};
		}

		// Operators
		DLLAPI ShotPos operator+(ShotPos pos_l, ShotPos pos_r);
		DLLAPI ShotPos operator-(ShotPos pos_l, ShotPos pos_rs);
		DLLAPI ShotPos operator+=(ShotPos &pos_l, ShotPos pos_r);
		DLLAPI ShotPos operator-=(ShotPos &pos_l, ShotPos pos_r);
		DLLAPI ShotVec operator+(ShotVec pos_l, ShotVec pos_r);
		DLLAPI ShotVec operator-(ShotVec pos_l, ShotVec pos_r);
		DLLAPI ShotVec operator+=(ShotVec &pos_l, ShotVec pos_r);
		DLLAPI ShotVec operator-=(ShotVec &pos_l, ShotVec pos_r);
		DLLAPI ShotVec operator*(ShotVec &vec_l, float ratio);
}