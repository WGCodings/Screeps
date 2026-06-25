#include "Brain.hpp"


#include "structures/Spawn.hpp"

#include <Screeps/Context.hpp>
#include <Screeps/Game.hpp>
#include "Screeps/Room.hpp"

#include "MemoryFlayer.hpp"
#include "Creeps/Harvester.hpp"
#include "Creeps/Upgrader.hpp"
#include "nlohmann/json.hpp"
#include "Room/Architect.hpp"
#include "Screeps/Creep.hpp"
#include "Screeps/StructureController.hpp"


namespace Peabrain {

    void Brain::run()
    {
        runStructures();
        runCreeps();
        runRooms();
        MemoryFlayer::run();
    }

    void Brain::runStructures() {

        for (auto& [name, spawn] : Screeps::Game.spawns())
            {
            Spawn spawner(spawn);
            spawner.run();
            }
    }

    void Brain::runRooms() {

        for (auto& [name, room] : Screeps::Game.rooms())
        {
            if (room.controller().value().my()) {
                Architect architect(room);
                architect.plan();
                architect.reviewStructures();
            }
        }
    }

    void Brain::runCreeps()
    {
        for (auto& [name, creep] : Screeps::Game.creeps())
        {
            JSON memory = creep.memory();

            // If memory does not contain role, skip. Better to kill or fix role.
            if (!memory.contains("role"))
            {
                memory["role"] = "harvester";
                creep.setMemory(memory);
                continue;
            }

            auto role = memory["role"].get<std::string>();

            // Start looping over all the roles

            if (role == "harvester")
             {
                Harvester harvester(creep);
                harvester.run();
             }
            if (role == "upgrader")
            {
                Upgrader upgrader(creep);
                upgrader.run();
            }
        }
    }

}
