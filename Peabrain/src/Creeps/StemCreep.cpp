#include "Creeps/StemCreep.hpp"

#include <Screeps/Constants.hpp>
#include <random>

#include "Screeps/Resource.hpp"
#include "Screeps/Room.hpp"
#include "Screeps/RoomPosition.hpp"
#include "Screeps/Store.hpp"
#include "Screeps/StructureController.hpp"
#include "Screeps/StructureStorage.hpp"

namespace Peabrain {

    /// Upgrade the controller
    void StemCreep::upgrade()
    {
        Screeps::StructureController controller = creep.room().controller().value();

        if (creep.upgradeController(controller) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(controller);
    }

    /// Find source and go harvest b*tch
    void StemCreep::harvest()
    {
        JSON memory = creep.memory();

        if (!memory.contains("sourceId"))
        {
            setSourceId();
            if (!memory.contains("sourceId")) return;
        }

        Screeps::Source* source = getSourceById(memory["sourceId"]);

        if (!source) { memory.erase("sourceId"); creep.setMemory(memory); return; }

        if (creep.harvest(*source) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(*source);
    }

    /// Function to find the closest dropped energy resource
    void StemCreep::gather()
    {
        JSON memory = creep.memory();

        if (!memory.contains("gatherId"))
        {
            setGatherId();
            memory = creep.memory();
            if (!memory.contains("gatherId")) return;
        }

        auto gatherId   = memory["gatherId"].get<std::string>();
        auto gatherType = memory.value("gatherType", "dropped");

        auto roomObj = Screeps::Game.getObjectById(memory["gatherId"]);

        // If we find no gather source, or it has disappeared -> Stop
        if (!roomObj)
        {
            memory.erase("gatherId");
            memory.erase("gatherType");
            creep.setMemory(memory);
            return;
        }

        auto* resource = dynamic_cast<Screeps::Resource*>(roomObj.get());

        if (creep.pickup(*resource) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(*roomObj);
    }

    /// Look for storage or containers to withdraw energy from
    void StemCreep::withdraw() {
        JSON memory = creep.memory();

        if (!memory.contains("withdrawId"))
        {
            setGatherId();
            memory = creep.memory();
            if (!memory.contains("withdrawId")) return;
        }

        auto gatherId   = memory["withdrawId"].get<std::string>();
        auto gatherType = memory.value("withdrawType", "storage");

        auto roomObj = Screeps::Game.getObjectById(memory["withdrawId"]);

        if (!roomObj)
        {
            memory.erase("withdrawId");
            memory.erase("withdrawType");
            creep.setMemory(memory);
            return;
        }

        if (creep.withdraw(*roomObj, Screeps::RESOURCE_ENERGY) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(*roomObj);
    }




    /// What it does it scans the current room and looks for containers with energy or tombstones with energy
    /// 1. Look for dropped resources
    void StemCreep::setGatherId() {
        JSON memory = creep.memory();

        // 1. Dropped resources
        auto dropped = creep.room().find(Screeps::FIND_DROPPED_RESOURCES);
        if (!dropped.empty())
        {
            auto closest = creep.pos().findClosestByPath(dropped);
            if (closest)
            {
                auto* resource = dynamic_cast<Screeps::Resource*>(closest.get());
                if (resource)
                {
                    memory["gatherId"]   = resource->id();
                    memory["gatherType"] = "dropped";
                    creep.setMemory(memory);
                }
            }
        }
    }
    /// Set the id of the container or storage to memory from where to withdraw
    /// First look for storage then containers
    void StemCreep::setWithdrawId()
    {
        JSON memory = creep.memory();
        // 1. Storage
        auto storage = creep.room().storage();
        if (storage.has_value() && storage->store().getUsedCapacity(Screeps::RESOURCE_ENERGY) > 500)
        {
            memory["withdrawId"]   = storage->id();
            memory["withdrawType"] = "storage";
            creep.setMemory(memory);
            return;
        }

        // 1. Containers
        auto containers = creep.room().find(Screeps::FIND_STRUCTURES, [](const JS::Value& v) {
            return v["structureType"].as<std::string>() == Screeps::STRUCTURE_CONTAINER;});
        if (!containers.empty())
        {
            auto closest = creep.pos().findClosestByPath(containers);
            if (closest)
            {
                auto* structure = dynamic_cast<Screeps::Structure*>(closest.get());
                if (structure)
                {
                    memory["withdrawId"]   = structure->id();
                    memory["withdrawType"] = "container";
                    creep.setMemory(memory);
                }
            }
        }
    }

    /// This function finds a source and assign the id to the memory of the harvester
    void StemCreep::setSourceId()
        {
            auto sources = creep.room().find(Screeps::FIND_SOURCES_ACTIVE);

            if (sources.empty()) return;

            JSON memory = creep.memory();

            srand (Screeps::Game.time());
            int index = rand()%sources.size();

            auto* source = dynamic_cast<Screeps::Source*>(sources[index].get());
            if (source)
            {
                memory["sourceId"] = source->id();
                creep.setMemory(memory);
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
