#ifndef PEABRAIN_CREEPS_MINER_HPP
#define PEABRAIN_CREEPS_MINER_HPP
#include "StemCreep.hpp"


namespace Peabrain {

    class Miner : public StemCreep
    {
    public:
        using StemCreep::StemCreep;

        void run() override;

    protected:
        void harvest() override;

        void setSourceId() override;

        bool deliver() override;

        void setDeliverId() override;

        void moveToContainer();

    };

}

#endif