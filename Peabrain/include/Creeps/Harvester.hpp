#ifndef PEABRAIN_ROLES_HARVESTER_HPP
#define PEABRAIN_ROLES_HARVESTER_HPP
#include "Screeps/Creep.hpp"


namespace Peabrain {

    class Harvester
    {
    public:
        explicit Harvester(Screeps::Creep creep) : creep(std::move(creep)) {}
        void run();

    private:
        void harvest();
        void deliver();

        Screeps::Creep creep;
    };

}

#endif