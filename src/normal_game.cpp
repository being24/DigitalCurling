#include "digital_curling/detail/normal_game/normal_game.hpp"

#include <cassert>
#include <stdexcept>
#include <random>
#include <limits>

namespace digital_curling::normal_game {

namespace {

// overloadedトリック用ヘルパークラス
// 参考: https://dev.to/tmr232/that-overloaded-trick-overloading-lambdas-in-c17
template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> Overloaded(Ts...)->Overloaded<Ts...>;

} // unnamed namespace



State::State()
    : stone_positions()
    , scores({})
    , current_shot(0)
    , current_end_first(TeamId::k0)
    , current_end(0)
    , extra_end_score(0)
    , game_result()
{}

std::uint32_t State::GetScore(TeamId team) const
{
    std::uint32_t score_sum = 0;

    for (auto const score : scores) {
        if (team == TeamId::k0 && score > 0) {
            score_sum += score;
        } else if (team == TeamId::k1 && score < 0) {
            score_sum -= score;
        }
    }

    if (team == TeamId::k0 && extra_end_score > 0) {
        score_sum += extra_end_score;
    } else if (team == TeamId::k1 && extra_end_score < 0) {
        score_sum -= extra_end_score;
    }

    return score_sum;
}

TeamId State::GetCurrentTeam() const
{
    switch (current_end_first) {
        case TeamId::k0:
            if (current_shot % 2 == 0) {
                return TeamId::k0;
            } else {
                return TeamId::k1;
            }
            break;  // 到達しない

        case TeamId::k1:
            if (current_shot % 2 == 0) {
                return TeamId::k1;
            } else {
                return TeamId::k0;
            }
            break;  // 到達しない
    }

    return TeamId::k0;  // 到達しない
}



namespace {

constexpr float kHogLineYOnSheet = 10.9725f;
constexpr float kTeeLineYOnSheet = 17.3735f;
constexpr float kBackLineYOnSheet = 19.2025f;
constexpr float kHackLineYOnSheet = 21.0315f;
constexpr float kBackBoardYOnSheet = 22.8605f;
constexpr float kHouseRadius = 1.829f;
constexpr Vector2 kTee(0.f, kTeeLineYOnSheet);


inline float GetShotAngularVelocity(move::Shot::Rotation shot_rotation)
{
    float angular_velocity_sign = 1.f;
    switch (shot_rotation) {
        case move::Shot::Rotation::kCCW:
            angular_velocity_sign = 1.f;
            break;
        case move::Shot::Rotation::kCW:
            angular_velocity_sign = -1.f;
            break;
        default:
            assert(false);
            break;
    }

    return kPi / 2.f * angular_velocity_sign;
}


inline bool GetSheetSide(std::uint8_t current_end)
{
    return current_end % 2 == 0;
}


/// <summary>
/// 座標をシート座標系に変換する．
/// </summary>
/// <param name="pos_on_shot"><paramref name="sheet_side"/>で指定したサイドのショット座標系</param>
/// <param name="sheet_side">サイド</param>
/// <returns>シート座標系</returns>
inline Vector2 TransformPositionToSheet(Vector2 pos_on_shot, bool sheet_side)
{
    if (!sheet_side) {
        // side 0
        return { pos_on_shot.x, pos_on_shot.y - kHackLineYOnSheet };
    } else {
        // side 1
        return { -pos_on_shot.x, -pos_on_shot.y + kHackLineYOnSheet };
    }
}


/// <summary>
/// ショット座標系に変換する．
/// </summary>
/// <param name="pos_on_sheet">シート座標系</param>
/// <param name="sheet_side">サイド</param>
/// <returns><paramref name="sheet_side"/>で指定したサイドのショット座標系</returns>
inline Vector2 TransformPositionToShot(Vector2 pos_on_sheet, bool sheet_side)
{
    if (!sheet_side) {
        // side 0
        return { pos_on_sheet.x, pos_on_sheet.y + kHackLineYOnSheet };
    } else {
        // side 1
        return { -pos_on_sheet.x, -pos_on_sheet.y + kHackLineYOnSheet };
    }
}


/// <summary>
/// 角度をシート座標系に変換する．
/// </summary>
/// <param name="angle_on_shot"><paramref name="sheet_side"/>で指定したサイドでの角度</param>
/// <param name="sheet_side">サイド</param>
/// <returns>シート座標系での角度</returns>
inline float TransformAngleToSheet(float angle_on_shot, bool sheet_side)
{
    if (!sheet_side) {
        // side 0
        return angle_on_shot;
    } else {
        // side 1
        return angle_on_shot + kPi;
    }
}

inline Vector2 TransformVelocityToSheet(Vector2 velocity_on_shot, bool sheet_side)
{
    if (!sheet_side) {
        // side 0
        return velocity_on_shot;
    } else {
        // side 1
        return -velocity_on_shot;
    }
}

/// <summary>
/// 両サイドで異なるシート座標をサイド0側にひとまとめにする．
/// </summary>
/// <param name="pos_on_sheet">シート座標系</param>
/// <param name="sheet_side">サイド</param>
/// <returns>サイド0側のシート座標</returns>
inline Vector2 CanonicalizePositionOnSheet(Vector2 pos_on_sheet, bool sheet_side)
{
    if (!sheet_side) {
        // side 0
        return pos_on_sheet;
    } else {
        // side 1
        return -pos_on_sheet;
    }
}


/// <summary>
/// シミュレーション中(ストーンが動いている間)にストーンが有効範囲内にいるかを判定する
/// </summary>
/// <param name="stone_position"></param>
/// <param name="sheet_width"></param>
/// <param name="stone_radius"></param>
/// <param name="sheet_side"></param>
/// <returns></returns>
inline bool IsStoneValidWhileSimulation(Vector2 stone_position, float sheet_width, float stone_radius, bool sheet_side)
{
    Vector2 const canonical_stone_position = CanonicalizePositionOnSheet(stone_position, sheet_side);
    return canonical_stone_position.x + stone_radius < sheet_width / 2.f
        && canonical_stone_position.x - stone_radius > -sheet_width / 2.f
        && canonical_stone_position.y - stone_radius < kBackLineYOnSheet  // バックラインの内側判定(例外的なので注意)
        && canonical_stone_position.y - stone_radius > -kBackBoardYOnSheet;  // バックボードの内側

    // バックラインの内側判定 について
    // ストーンがバックラインに少しでも掛かっていれば内側と見なされるため
    //     stone_position.y - stone_radius < kBackLineYOnSheet
    // という判定にしている．
}


/// <summary>
/// ストーン停止時のストーン除外判定に使う．
/// なお，ここではホグラインを超えているかの判定しかしない．
/// それ以外の判定は<see cref="IsStoneValidWhileSimulation"/>ですでに行われているため．
/// </summary>
/// <remarks>
/// </remarks>
/// <param name="stone_position"></param>
/// <param name="stone_radius"></param>
/// <param name="sheet_side"></param>
/// <returns></returns>
inline bool IsStoneInPlayArea(Vector2 stone_position, float stone_radius, bool sheet_side)
{
    Vector2 const canonical_stone_position = CanonicalizePositionOnSheet(stone_position, sheet_side);
    return canonical_stone_position.y - stone_radius > kHogLineYOnSheet;
}


inline bool IsStoneInHouse(Vector2 stone_position, float stone_radius, bool sheet_side)
{
    Vector2 const canonical_stone_position = CanonicalizePositionOnSheet(stone_position, sheet_side);
    return (canonical_stone_position - kTee).Length() < kHouseRadius + stone_radius;
}


/// <summary>
/// ストーンがフリーガードゾーンにあるか否かを判定する．
/// 判定するのはハウス内かとティーラインの判定のみ．
/// それ以外の判定は<see cref="IsStoneValidWhileSimulation"/>と<see cref="IsStoneInPlayArea"/>で行う．
/// </summary>
/// <param name="stone_position"></param>
/// <param name="stone_radius"></param>
/// <param name="sheet_side"></param>
/// <returns></returns>
inline bool IsStoneInFreeGuardZone(Vector2 stone_position, float stone_radius, bool sheet_side)
{
    if (IsStoneInHouse(stone_position, stone_radius, sheet_side)) {
        return false;
    }

    Vector2 const canonical_stone_position = CanonicalizePositionOnSheet(stone_position, sheet_side);
    return canonical_stone_position.y + stone_radius < kTeeLineYOnSheet;
}


inline std::int8_t CheckScore(simulation::AllStoneData const& stones, float stone_radius, bool sheet_side, TeamId current_end_first)
{
    // 全ストーンのティーからの位置を計算し，格納．
    std::array<float, kStoneMax> distances{};
    for (StoneId i = 0; i < kStoneMax; ++i) {
        if (stones[i]) {
            auto const canonical_stone_position = CanonicalizePositionOnSheet(stones[i]->position, sheet_side);
            distances[i] = (canonical_stone_position - kTee).Length();
        } else {
            distances[i] = std::numeric_limits<float>::max();
        }
    }

    // プレイヤー0のNo.1ストーンまでの距離をminDistance0に
    // プレイヤー1のNo.1ストーンまでの距離をminDistance1に格納．
    std::array<float, 2> minDistance{ {
        kHouseRadius + stone_radius,
        kHouseRadius + stone_radius} };

    for (StoneId i = 0; i < kStoneMax; ++i) {
        size_t const team_id = (i + static_cast<size_t>(current_end_first)) % 2;
        if (distances[i] < minDistance[team_id]) {
            minDistance[team_id] = distances[i];
        }
    }

    std::int8_t score = 0;

    if (minDistance[0] < minDistance[1]) {
        // プレイヤー0の得点
        for (StoneId i = static_cast<size_t>(current_end_first); i < kStoneMax; i += 2) {  // プレイヤー0のストーンを列挙
            if (distances[i] < minDistance[0]) {  // minDistance*は最大でもハウス半径+石半径になっているので，ハウス内の判定も同時に行える．
                score++;
            }
        }
    } else {
        // プレイヤー1の得点
        for (StoneId i = (static_cast<size_t>(current_end_first) + 1) % 2; i < kStoneMax; i += 2) {  // プレイヤー1のストーンを列挙
            if (distances[i] < minDistance[1]) {
                score--;
            }
        }
    }

    return score;
}


void StoreStonePositions(std::array<std::optional<Vector2>, kStoneMax> & stone_positions, simulation::AllStoneData const& all_stone_data, bool sheet_side)
{
    for (StoneId i = 0; i < kStoneMax; ++i) {
        if (all_stone_data[i]) {
            stone_positions[i] = TransformPositionToShot(all_stone_data[i]->position, sheet_side);
        } else {
            stone_positions[i] = std::nullopt;
        }
    }
}


Vector2 RandomizeShotVelocity(Vector2 shot_velocity, float stddev_shot_speed, float stddev_shot_angle)
{
    thread_local auto random_engine = std::default_random_engine(std::random_device()());
    auto speed_random_dist = std::normal_distribution<float>(0.f, stddev_shot_speed);
    auto angle_random_dist = std::normal_distribution<float>(0.f, stddev_shot_angle);
    float const speed = shot_velocity.Length() + speed_random_dist(random_engine);
    float const angle = std::atan2(shot_velocity.y, shot_velocity.x) + angle_random_dist(random_engine);
    return speed * Vector2(std::cos(angle), std::sin(angle));
}



} // unnamed namespace


void ApplyMove(
    Setting const& setting,
    State & state,
    simulation::ISimulator & simulator,
    Move & move,
    MoveResult & move_result)
{
    // TODO 例外時の保証

    // ゲームが既に終了している
    if (state.game_result) return;

    bool const sheet_side = GetSheetSide(state.current_end);
    float const stone_radius = simulator.GetStoneRadius();
    bool is_shot = std::holds_alternative<move::Shot>(move);

    // シート上のストーンの初期状態を設定

    simulation::AllStoneData initial_stones;
    for (StoneId i = 0; i < kStoneMax; ++i) {
        if (i < state.current_shot && state.stone_positions[i]) {
            // ストーンの角度はsimulatorから取得する．
            float const stone_angle = simulator.GetStones()[i] ? simulator.GetStones()[i]->angle : 0.f;
            initial_stones[i] = simulation::StoneData(
                TransformPositionToSheet(*state.stone_positions[i], sheet_side),
                stone_angle,
                Vector2(0.f, 0.f),
                0.f);
        } else {
            initial_stones[i] = std::nullopt;
        }
    }

    if (is_shot) {
        move::Shot & shot = std::get<move::Shot>(move);
        // 最大速度制限を適用．
        if (auto speed = shot.velocity.Length(); speed > setting.max_shot_speed) {
            shot.velocity *= setting.max_shot_speed / speed;
        }
        // 乱数を加える
        if (setting.randomize_initial_shot_velocity) {
            shot.velocity = RandomizeShotVelocity(shot.velocity, setting.stddev_shot_speed, setting.stddev_shot_angle);
        }
        initial_stones[state.current_shot] = simulation::StoneData(
            TransformPositionToSheet(Vector2(0.f, 0.f), sheet_side),
            TransformAngleToSheet(0.f, sheet_side),
            TransformVelocityToSheet(shot.velocity, sheet_side),
            GetShotAngularVelocity(shot.rotation));
    }

    simulator.SetStones(initial_stones);

    // シミュレーション
    if (is_shot) {
        while (!simulator.AreAllStonesStopped()) {
            simulator.Step();
            simulation::AllStoneData stones = simulator.GetStones();
            bool stone_removed = false;
            for (StoneId i = 0; i <= state.current_shot; ++i) {
                if (stones[i] && !IsStoneValidWhileSimulation(stones[i]->position, setting.sheet_width, stone_radius, sheet_side)) {
                    stones[i] = std::nullopt;
                    stone_removed = true;
                }
            }
            if (stone_removed) {
                simulator.SetStones(stones);
            }
            if (move_result.on_step) {
                move_result.on_step(simulator);
            }
        }

        // すべてのストーンが停止した後，プレイエリア外のストーンを削除
        {
            simulation::AllStoneData stones = simulator.GetStones();
            bool stone_removed = false;
            for (StoneId i = 0; i <= state.current_shot; ++i) {
                if (stones[i] && !IsStoneInPlayArea(stones[i]->position, stone_radius, sheet_side)) {
                    stones[i] = std::nullopt;
                    stone_removed = true;
                }
            }
            if (stone_removed) {
                simulator.SetStones(stones);
            }
        }

        // フリーガードゾーンルールの適用
        bool free_guard_faul = false;
        std::uint8_t const free_guard_zone_count = setting.five_rock_rule ? 5 : 4;
        if (state.current_shot < free_guard_zone_count) {
            simulation::AllStoneData const& stones = simulator.GetStones();
            for (StoneId i = (state.current_shot + 1) % 2; i < state.current_shot; i += 2) {  // 相手のショットを列挙
                if (initial_stones[i] && IsStoneInFreeGuardZone(initial_stones[i]->position, stone_radius, sheet_side)
                    && !stones[i]) {
                    free_guard_faul = true;
                    break;
                }
            }
        }
        if (free_guard_faul) {
            simulator.SetStones(initial_stones);
        }
    }

    // move_resultを構築
    move_result.team = state.GetCurrentTeam();
    move_result.shot = state.current_shot;
    move_result.end = state.current_end;
    StoreStonePositions(move_result.stone_positions, simulator.GetStones(), sheet_side);

    // エンド終了時の処理
    if (state.current_shot == 15) {
        // スコア算出
        auto const score = CheckScore(simulator.GetStones(), stone_radius, sheet_side, state.current_end_first);

        // スコアを反映
        if (state.current_end < setting.end) {  // 通常エンド
            state.scores[state.current_end] = score;
        } else {  // エクストラエンド
            state.extra_end_score = score;
        }

        // 手番の変更．スコアが0(ブランクエンド)の時は変更されない．
        if (score > 0) {
            state.current_end_first = TeamId::k1;
        } else if (score < 0) {
            state.current_end_first = TeamId::k0;
        }

        // ストーン位置のリセット
        for (auto & stone_position : state.stone_positions) {
            stone_position = std::nullopt;
        }

        // エンドとショットカウントを更新
        state.current_shot = 0;
        ++state.current_end;

        // 勝敗の決定
        if (state.current_end >= setting.end) {
            std::int32_t score_sum = 0;
            for (auto score : state.scores) {
                score_sum += score;
            }
            score_sum += state.extra_end_score;

            if (score_sum > 0) {
                state.game_result.emplace();
                state.game_result->win = TeamId::k0;
                state.game_result->reason = GameResult::Reason::kScore;
            } else if (score_sum < 0) {
                state.game_result.emplace();
                state.game_result->win = TeamId::k1;
                state.game_result->reason = GameResult::Reason::kScore;
            } else if (state.current_end >= kExtraEndMax) {  // スコア差無し かつ 延長エンド数限界
                state.game_result.emplace();
                state.game_result->win = std::nullopt;
                state.game_result->reason = GameResult::Reason::kInvalid;
            }
        }

    } else {  // エンド中の処理
        StoreStonePositions(state.stone_positions, simulator.GetStones(), sheet_side);
        ++state.current_shot;
    }

    // コンシードや時間切れの場合はstateを上書きする．
    if (!is_shot) {
        state.game_result.emplace();

        switch (move_result.team) {
            case TeamId::k0:
                state.game_result->win = TeamId::k1;
                break;
            case TeamId::k1:
                state.game_result->win = TeamId::k0;
                break;
            default:
                assert(false);
                break;
        }

        std::visit(
            Overloaded{
                [&reason = state.game_result->reason](move::Concede) {
                    reason = GameResult::Reason::kConcede;
                },
                [&reason = state.game_result->reason](move::TimeLimit) {
                    reason = GameResult::Reason::kTimeLimit;
                },
                [](auto const&) {
                    assert(false);
                }
            },
            move);
    }
}

} // namespace digital_curling::normal_game

namespace nlohmann {

void adl_serializer<digital_curling::normal_game::Move>::to_json(json & j, digital_curling::normal_game::Move const& m)
{
    std::visit(
        [&j](auto const& m) {
            j["type"] = std::decay_t<decltype(m)>::kType;
        },
        m);

    if (std::holds_alternative<digital_curling::normal_game::move::Shot>(m)) {
        auto const& shot = std::get<digital_curling::normal_game::move::Shot>(m);
        j["velocity"] = shot.velocity;
        j["rotation"] = shot.rotation;
    }
}

void adl_serializer<digital_curling::normal_game::Move>::from_json(json const& j, digital_curling::normal_game::Move & m)
{
    auto type = j.at("type").get<std::string>();
    if (type == digital_curling::normal_game::move::Shot::kType) {
        digital_curling::normal_game::move::Shot shot;
        j.at("velocity").get_to(shot.velocity);
        j.at("rotation").get_to(shot.rotation);
        m = std::move(shot);
    } else if (type == digital_curling::normal_game::move::Concede::kType) {
        m = digital_curling::normal_game::move::Concede();
    } else if (type == digital_curling::normal_game::move::TimeLimit::kType) {
        m = digital_curling::normal_game::move::TimeLimit();
    } else {
        throw std::runtime_error("Move type was not found.");
    }
}

} // namespace nlohmann
