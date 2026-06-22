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
        void spawnHarvester();

        Screeps::StructureSpawn spawn;
    };

}

#endif