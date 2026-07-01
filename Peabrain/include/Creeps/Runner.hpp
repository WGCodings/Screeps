#ifndef PEABRAIN_CREEPS_RUNNER_HPP
#define PEABRAIN_CREEPS_RUNNER_HPP

#include "StemCreep.hpp"


namespace Peabrain {

    class Runner : public StemCreep
    {
    public:
        using StemCreep::StemCreep;

        void run() override;

    protected:
        void setWithdrawId() override;

        bool withdraw() override;
    };

}

#endif