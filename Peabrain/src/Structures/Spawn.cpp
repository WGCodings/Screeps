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

        if (countCreepsWithRole("runner")   < 2) { spawnRunner(maxEnergyAvailable); return;}
        if (countCreepsWithRole("harvester") < 2) { spawnHarvester(maxEnergyAvailable); return;}
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

            auto work_parts = std::min(maxEnergyAvailable / 200, 6);
            auto extra_parts = std::max((maxEnergyAvailable-1200) / 150,0);

            for (int i = 0; i < work_parts; i++) {
                body.emplace_back("work");
                body.emplace_back("carry");
                body.emplace_back("move");
            }

            for (int i = 0; i < extra_parts; i++) {
                body.emplace_back("carry");
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

    void Spawn::spawnUpgrader(int maxEnergyAvailable)
    {
        if (spawn.room().energyAvailable() >= maxEnergyAvailable)
        {

            JS::console.log(std::string("Try spawn an upgrader."));

            std::vector<std::string> body;

            auto work_parts = std::min(maxEnergyAvailable / 200, 6);
            auto extra_parts = std::max((maxEnergyAvailable-1200) / 150,0);

            for (int i = 0; i < work_parts; i++) {
                body.emplace_back("work");
                body.emplace_back("carry");
                body.emplace_back("move");
            }

            for (int i = 0; i < extra_parts; i++) {
                body.emplace_back("carry");
                body.emplace_back("carry");
                body.emplace_back("move");
            }

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

            auto work_parts = std::min(maxEnergyAvailable / 200, 6);
            auto extra_parts = std::max((maxEnergyAvailable-1200) / 150,0);

            for (int i = 0; i < work_parts; i++) {
                body.emplace_back("work");
                body.emplace_back("carry");
                body.emplace_back("move");
            }

            for (int i = 0; i < extra_parts; i++) {
                body.emplace_back("carry");
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

    void Spawn::spawnRunner(int maxEnergyAvailable) {
        if (spawn.room().energyAvailable() >= maxEnergyAvailable)
        {

            JS::console.log(std::string("Try spawn a runner."));

            std::vector<std::string> body;

            auto extra_parts = maxEnergyAvailable / 150;

            for (int i = 0; i < extra_parts; i++) {
                body.emplace_back("carry");
                body.emplace_back("carry");
                body.emplace_back("move");
            }

            const std::string name = "Runner_" + std::to_string(Screeps::Game.time());

            JSON initialMemory;
            initialMemory["role"] = "runner";
            initialMemory["task"] = "gathering";

            JSON options;
            options["memory"] = initialMemory;

            spawn.spawnCreep(body, name, options);

        }
    }

    void Spawn::spawnMiner(int maxEnergyAvailable) {

        // A miner in stage 4 should be able to deposit to the links.
        const int roomStage = spawn.room().memory()["colony"]["stage"].get<int>();

        // 1400 is the maximum a miner needs for energy : 10 work, 4 carry, 4 move
        if (spawn.room().energyAvailable() >= std::min(maxEnergyAvailable,1400))
        {

            JS::console.log(std::string("Try spawn a miner."));

            std::vector<std::string> body;
            auto carryParts = (roomStage==4) ? 4 : 0;
            auto workParts = std::min((maxEnergyAvailable-200-50*carryParts) / 100, 10);
            auto moveParts = (carryParts+workParts)/3;

            for (int i = 0; i < moveParts; i++) {
                body.emplace_back("move");
            }
            for (int i = 0; i < carryParts; i++) {
                body.emplace_back("carry");
            }
            for (int i = 0; i < workParts; i++) {
                body.emplace_back("work");
            }

            const std::string name = "Miner_" + std::to_string(Screeps::Game.time());

            JSON initialMemory;
            initialMemory["role"] = "miner";
            initialMemory["task"] = "harvesting";

            JSON options;
            options["memory"] = initialMemory;

            spawn.spawnCreep(body, name, options);

        }
    }
}
