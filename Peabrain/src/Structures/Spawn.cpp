#include "Structures/Spawn.hpp"
#include <Screeps/Game.hpp>
#include "Screeps/Constants.hpp"
#include "Screeps/Creep.hpp"
#include "Screeps/Room.hpp"

namespace Peabrain {

    void Spawn::run()
    {
        if (spawn.room().find(Screeps::FIND_CREEPS).size() < 20)
            {
            spawnHarvester();
            }
    }


    void Spawn::spawnHarvester()
    {
        if (spawn.room().energyAvailable() >= 300)
        {

            JS::console.log(std::string("Try spawn a harvester."));

            const std::vector<std::string> body = {Screeps::WORK, Screeps::CARRY, Screeps::MOVE, Screeps::CARRY, Screeps::MOVE};

            const std::string name = "Harvester_" + std::to_string(Screeps::Game.time());

            JSON initialMemory;
            initialMemory["role"] = "harvester";
            initialMemory["task"] = "harvesting";

            JSON options;
            options["memory"] = initialMemory;

            spawn.spawnCreep(body, name, options);

        }

    }

    void Spawn::spawnUpgrader()
    {
        if (spawn.room().energyAvailable() >= 300)
        {

            JS::console.log(std::string("Try spawn an upgrader."));

            const std::vector<std::string> body = {Screeps::WORK, Screeps::CARRY, Screeps::MOVE, Screeps::CARRY, Screeps::MOVE};

            const std::string name = "Upgrader_" + std::to_string(Screeps::Game.time());

            JSON initialMemory;
            initialMemory["role"] = "upgrader";
            initialMemory["task"] = "harvesting";

            JSON options;
            options["memory"] = initialMemory;

            spawn.spawnCreep(body, name, options);

        }

    }

}
