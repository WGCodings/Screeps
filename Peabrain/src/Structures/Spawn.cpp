#include "Structures/Spawn.hpp"
#include <Screeps/Game.hpp>
#include "Screeps/Constants.hpp"
#include "Screeps/Creep.hpp"
#include "Screeps/Room.hpp"

namespace Peabrain {

    void Spawn::run()
    {
        // Only run every 10 ticks
        if (Screeps::Game.time() % checkSpawnInterval != 0)
            return;

        auto nrOfHarvesters = spawn.room().memory()["colony"]["harvesters"];

        auto maxEnergyAvailable = spawn.room().energyCapacityAvailable();

        if (countCreepsWithRole("harvester") < 4) { spawnHarvester(maxEnergyAvailable); return;}
        if (countCreepsWithRole("builder")   < 2) { spawnBuilder(maxEnergyAvailable);}


    }

    int Spawn::countCreepsWithRole(const std::string& role) const
    {
        auto result = spawn.room().find(Screeps::FIND_MY_CREEPS, [&role](const JS::Value& v) {
        Screeps::Creep c(v);return c.memory().value("role", "") == role;}).size();
        if (result) {
            return static_cast<int>(result);
        }
        return 0;

    }

    void Spawn::spawnHarvester(int maxEnergyAvailable)
    {
        if (spawn.room().energyAvailable() >= maxEnergyAvailable)
        {

            JS::console.log(std::string("Try spawn a harvester."));

            std::vector<std::string> body;

            for (int i = 0; i < maxEnergyAvailable / 200; i++) {
                body.emplace_back("work");
                body.emplace_back("carry");
                body.emplace_back("move");
            }

            const std::string name = "Harvester_" + std::to_string(Screeps::Game.time());

            JSON initialMemory;
            initialMemory["role"] = "harvester";
            initialMemory["task"] = "harvesting";

            JSON options;
            options["memory"] = initialMemory;

            JS::console.log(std::string("Calling spawnCreep."));

            spawn.spawnCreep(body, name, options);

        }

    }

    void Spawn::spawnUpgrader()
    {
        if (spawn.room().energyAvailable() >= 200)
        {

            JS::console.log(std::string("Try spawn an upgrader."));

            const std::vector<std::string> body = {Screeps::WORK, Screeps::CARRY, Screeps::MOVE};

            const std::string name = "Upgrader_" + std::to_string(Screeps::Game.time());

            JSON initialMemory;
            initialMemory["role"] = "upgrader";
            initialMemory["task"] = "harvesting";

            JSON options;
            options["memory"] = initialMemory;

            spawn.spawnCreep(body, name, options);

        }

    }

    void Spawn::spawnBuilder(int maxEnergyAvailable) {
        if (spawn.room().energyAvailable() >= maxEnergyAvailable)
        {

            JS::console.log(std::string("Try spawn a builder."));

            std::vector<std::string> body;

            for (int i = 0; i < maxEnergyAvailable / 200; i++) {
                body.emplace_back("work");
                body.emplace_back("carry");
                body.emplace_back("move");
            }

            const std::string name = "Builder_" + std::to_string(Screeps::Game.time());

            JSON initialMemory;
            initialMemory["role"] = "builder";
            initialMemory["task"] = "harvesting";

            JSON options;
            options["memory"] = initialMemory;

            spawn.spawnCreep(body, name, options);

        }
    }
}
