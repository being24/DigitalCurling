#include "util.hpp"

namespace dc = digital_curling;
using nlohmann::json;

TEST(simulator, AllStoneStop)
{
    dc::simulation::SimulatorSettingSimple1 setting;
    auto simulator = setting.CreateSimulator();
    EXPECT_TRUE(simulator->AreAllStonesStopped());
}

TEST(simulator_setting, Simple1)
{
    dc::simulation::SimulatorSettingSimple1 setting_simple1;
    setting_simple1.seconds_per_frame *= 10.f; // set non-default value
    json j = setting_simple1;
    auto setting = j.get<std::unique_ptr<dc::simulation::ISimulatorSetting>>();
    auto const& setting_simple1_copy = dynamic_cast<dc::simulation::SimulatorSettingSimple1&>(*setting);
    EXPECT_EQ(setting_simple1.seconds_per_frame, setting_simple1_copy.seconds_per_frame);
}

// TODO テスト書く
