#ifndef PEABRAIN_CREEPS_UPGRADER_HPP
#define PEABRAIN_CREEPS_UPGRADER_HPP

#include "Creeps/StemCreep.hpp"

namespace Peabrain {

    class Upgrader : public StemCreep
    {
    public:
        using StemCreep::StemCreep;

        void run() override;

    private:
        void harvest();
    };

}

#endif