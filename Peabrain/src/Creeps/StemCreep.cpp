#include "Creeps/StemCreep.hpp"

#include <Screeps/Constants.hpp>

#include <algorithm>
#include <numeric>
#include <random>

#include "Screeps/Room.hpp"
#include "Screeps/StructureController.hpp"

namespace Peabrain {

void StemCreep::upgrade()
{
    Screeps::StructureController controller = creep.room().controller().value();

    if (creep.upgradeController(controller) == Screeps::ERR_NOT_IN_RANGE)
        creep.moveTo(controller);
}
/// This function finds a source and assign the id to the memory of the harvester
void StemCreep::findSource(JSON& memory)
    {
        auto sources = creep.room().find(Screeps::FIND_SOURCES_ACTIVE);
        if (sources.empty()) return;

        std::vector<int> indices(sources.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), std::default_random_engine(Screeps::Game.time()));

        for (int i : indices)
        {
            auto* source = dynamic_cast<Screeps::Source*>(sources[i].get());
            if (source && source->energy() > 0)
            {
                memory["sourceId"] = source->id();
                creep.setMemory(memory);
                return;
            }
        }
    }

/// This function return the source object given an id
Screeps::Source* StemCreep::getSourceById(const std::string& sourceId)
    {
        auto object = Screeps::Game.getObjectById(sourceId);
        if (!object) return nullptr;
        return dynamic_cast<Screeps::Source*>(object.release());
    }

}
