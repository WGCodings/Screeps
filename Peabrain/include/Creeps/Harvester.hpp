#ifndef PEABRAIN_CREEPS_HARVESTER_HPP
#define PEABRAIN_CREEPS_HARVESTER_HPP
#include "StemCreep.hpp"


namespace Peabrain {

    class Harvester : public StemCreep
    {
    public:
        using StemCreep::StemCreep;

        void run() override;

    };

}

#endif