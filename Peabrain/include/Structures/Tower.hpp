#ifndef PEABRAIN_STRUCTURES_TOWER_HPP
#define PEABRAIN_STRUCTURES_TOWER_HPP
#include "Screeps/StructureTower.hpp"


namespace Peabrain {

    class Tower
    {
    public:
        explicit Tower(Screeps::StructureTower tower) : tower(std::move(tower)) {}

        void run();

    private:

        void repair();
        bool fire();
        void focus() const;

        Screeps::StructureTower tower;
    };
}
#endif