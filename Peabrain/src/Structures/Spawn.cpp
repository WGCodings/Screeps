#include "Structures/Spawn.hpp"
#include <Screeps/Game.hpp>
#include "Screeps/Constants.hpp"
#include "Screeps/Room.hpp"

namespace Peabrain {

    void Spawn::run()
    {
        spawnHarvester();
    }


    void Spawn::spawnHarvester()
    {
        if (spawn.room().energyAvailable() >= 200)
        {

            JS::console.log(std::string("Try spawn a harvester."));

            const std::vector<std::string> body = {Screeps::WORK, Screeps::CARRY, Screeps::MOVE};

            const std::string name = "Harvester_" + std::to_string(Screeps::Game.time());

            JSON initialMemory;
            initialMemory["role"] = "harvester";
            initialMemory["task"] = "harvesting";

            JSON options;
            options["memory"] = initialMemory;

            spawn.spawnCreep(body, name, options);

        }

    }

}
