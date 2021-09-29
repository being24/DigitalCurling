#include "digital_curling/detail/simulation/simulator.hpp"
#include <unordered_map>
#include <functional>
#include "simulator_simple1.hpp"

namespace digital_curling::simulation {

void to_json(nlohmann::json & j, ISimulatorSetting const& setting)
{
    setting.ToJson(j);
}

} // namespace digital_curling::simulation



namespace nlohmann {

namespace {

template <class T>
std::unique_ptr<digital_curling::simulation::ISimulatorSetting> CreateSimulatorSettingImpl(json const& j)
{
    auto setting = std::make_unique<T>();
    *setting = j.get<T>();
    return setting;
}

} // unnamed namespace

std::unique_ptr<digital_curling::simulation::ISimulatorSetting> adl_serializer<std::unique_ptr<digital_curling::simulation::ISimulatorSetting>>::from_json(json const & j)
{
    auto type = j.at("type").get<std::string>();

    if (type == digital_curling::simulation::SimulatorSettingSimple1::kType) {
        return CreateSimulatorSettingImpl<digital_curling::simulation::SimulatorSettingSimple1>(j);
    } else {
        throw std::runtime_error("no such type simulator.");
    }
}

void adl_serializer<std::unique_ptr<digital_curling::simulation::ISimulatorSetting>>::to_json(json & j, std::unique_ptr<digital_curling::simulation::ISimulatorSetting> const& setting)
{
    setting->ToJson(j);
}

} // namespace nlohmann
