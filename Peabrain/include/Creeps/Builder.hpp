#ifndef PEABRAIN_CREEPS_BUILDER_HPP
#define PEABRAIN_CREEPS_BUILDER_HPP

#include "StemCreep.hpp"


namespace Peabrain {

    class Builder : public StemCreep
    {
    public:
        using StemCreep::StemCreep;

        void run() override;

    private:
        bool build();
        void setConstructionSiteId();

        static Screeps::ConstructionSite* getConstructionSiteById(const std::string& cSiteId);
    };

}

#endif