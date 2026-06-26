#ifndef PEABRAIN_COLONY_HPP
#define PEABRAIN_COLONY_HPP
#include "Screeps/Room.hpp"


namespace Peabrain {

    class Colony
    {
    public:
        explicit Colony(Screeps::Room room) : room(std::move(room)) {}

        void colonise();

    private:

        void setScreepsMemory();

        Screeps::Room room;
    };

}

#endif