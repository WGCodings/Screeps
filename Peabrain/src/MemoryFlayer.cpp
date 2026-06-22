#include "MemoryFlayer.hpp"

#include <Screeps/Game.hpp>
#include <Screeps/Memory.hpp>

#include "nlohmann/json.hpp"
#include "Screeps/Creep.hpp"

namespace Peabrain {

    /// Triggers flayDeadCreeps which removes memory of dead creeps every flayInterval ticks.
    void MemoryFlayer::run()
    {
        if (Screeps::Game.time() % flayInterval != 0)
        {
            return;
        }

        flayDeadCreeps();
    }

    /// Loops over names of creeps in memery and compares names with the names of the alive creeps.
    /// If not in alive creeps, push to delete
    void MemoryFlayer::flayDeadCreeps()
    {
        JS::console.log(std::string("Clearing dead creep memory."));

        JSON creepsMemory = Screeps::Memory["creeps"];

        auto liveCreeps = Screeps::Game.creeps();

        std::vector<std::string> staleNames;

        for (auto& [name, value] : creepsMemory.items())
        {
            if (!liveCreeps.contains(name))
            {
                staleNames.push_back(name);
            }
        }

        if (staleNames.empty())
        {
            return;
        }

        for (auto& name : staleNames)
        {
            creepsMemory.erase(name);
        }

        Screeps::Memory.set("creeps", creepsMemory);
    }

}