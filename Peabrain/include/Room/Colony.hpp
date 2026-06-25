#ifndef PEABRAIN_COLONY_HPP
#define PEABRAIN_COLONY_HPP
#include "Screeps/Room.hpp"

namespace Peabrain {

    class Colony
    {
    public:
        explicit Colony(Screeps::Room room) : room(std::move(room)) {}

        void plan();

    private:


        void planContainers();
        void planLinks();
        void planStorage();
        void planTowers();
        void planExtensions();
        void planRoads();


        static bool hasEntryOnCoords(const JSON &memory, int x, int y);

        static void addEntryOnCoords(JSON &memory,  int x, int y, const std::string &sType, int cLevel);

        Screeps::Room room;
    };

}

#endif