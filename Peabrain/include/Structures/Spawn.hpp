#ifndef PEABRAIN_STRUCTURES_SPAWNER_HPP
#define PEABRAIN_STRUCTURES_SPAWNER_HPP

#include <Screeps/StructureSpawn.hpp>

namespace Peabrain {

    class Spawn
    {
    public:
        explicit Spawn(Screeps::StructureSpawn spawn) : spawn(std::move(spawn)) {}

        void run();

    private:
        static constexpr int checkSpawnInterval = 10;

        void spawnHarvester();
        void spawnUpgrader();
        void spawnBuilder();

        Screeps::StructureSpawn spawn;
    };
}
#endif