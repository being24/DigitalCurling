#ifndef DIGITAL_CURLING_SIMULATION_SIMPLE1_HPP
#define DIGITAL_CURLING_SIMULATION_SIMPLE1_HPP

#include "simulator.hpp"



namespace digital_curling::simulation {



/// <summary>
/// simple1シミュレータの設定
/// </summary>
class SimulatorSettingSimple1 : public ISimulatorSetting {
public:
    static constexpr char kType[] = "simple1";

    /// <summary>
    /// フレームレート(フレーム毎秒)
    /// </summary>
    float seconds_per_frame = 0.001f;

    SimulatorSettingSimple1() = default;
    virtual ~SimulatorSettingSimple1() = default;

    virtual std::unique_ptr<ISimulator> CreateSimulator() const override;
    virtual void ToJson(nlohmann::json & json) const override;
};

void to_json(nlohmann::json &, SimulatorSettingSimple1 const&);
void from_json(nlohmann::json const&, SimulatorSettingSimple1 &);



} // namespace digital_curling::simulation

#endif // DIGITAL_CURLING_SIMULATION_SIMPLE1_HPP
