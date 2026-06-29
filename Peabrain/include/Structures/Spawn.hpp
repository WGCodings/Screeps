#ifndef PEABRAIN_STRUCTURES_SPAWN_HPP
#define PEABRAIN_STRUCTURES_SPAWN_HPP

#include <Screeps/StructureSpawn.hpp>

namespace Peabrain {

    class Spawn
    {
    public:
        explicit Spawn(Screeps::StructureSpawn spawn) : spawn(std::move(spawn)) {}

        void run();

        int countCreepsWithRole(const std::string &role) const;

    private:
        static constexpr int checkSpawnInterval = 10;

        void spawnHarvester(int maxEnergyAvailable);
        void spawnUpgrader();
        void spawnBuilder(int maxEnergyAvailable);

        Screeps::StructureSpawn spawn;
    };
}
#endif