#ifndef PEABRAIN_CREEPS_STEM_CREEP_HPP
#define PEABRAIN_CREEPS_STEM_CREEP_HPP

#include <Screeps/Creep.hpp>
#include <Screeps/Source.hpp>
#include <Screeps/Game.hpp>
#include <Screeps/JSON.hpp>

namespace Peabrain {
    // This is an abstract class which all creeps will inherit.
    // All creeps share some search function like findSource and getSourceById.
    // More in future probably.

    class StemCreep
    {
    public:
        explicit StemCreep(Screeps::Creep creep) : creep(std::move(creep)) {}
        virtual ~StemCreep() = default;

        virtual void run() = 0;

    protected:
        Screeps::Creep creep;

        virtual void upgrade();

        void findSource(JSON& memory);

        static Screeps::Source* getSourceById(const std::string& sourceId);
    };

}

#endif