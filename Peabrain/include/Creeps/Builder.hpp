#ifndef PEABRAIN_CREEPS_BUILDER_HPP
#define PEABRAIN_CREEPS_BUILDER_HPP

#include "Creeps/StemCreep.hpp"

namespace Peabrain {

    class Builder : public StemCreep
    {
    public:
        using StemCreep::StemCreep;

        void run() override;

    private:
        void harvest();
        void build();
    };

}

#endif