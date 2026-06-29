#include "Creeps/StemCreep.hpp"

#include <Screeps/Constants.hpp>
#include <random>

#include "Screeps/Resource.hpp"
#include "Screeps/Room.hpp"
#include "Screeps/RoomPosition.hpp"
#include "Screeps/Store.hpp"
#include "Screeps/StructureController.hpp"
#include "Screeps/StructureStorage.hpp"

#include <Screeps/Game.hpp>
#include <Screeps/StructureSpawn.hpp>

#include "Screeps/StructureExtension.hpp"

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
            memory = creep.memory();

            if (!memory.contains("sourceId")) return;
        }

        Screeps::Source* source = getSourceById(memory["sourceId"]);

        if (!source) {
            memory.erase("sourceId");
            creep.setMemory(memory);
            return;
        }

        if (creep.harvest(*source) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(*source);
    }




    /// Deliver your energy to the source b*tch
    /// First deliver to towers (More than 500 energy), then extensions, then spawn and finally to storage
    /// TODO setDeliveryId is very inefficient, it constantly calls findClosestInRange which is expensive.
    /// better to use a combination of the memory and lookForAt for example
    bool StemCreep::deliver()
    {
        setDeliverId();

        JSON memory = creep.memory();
        if (!memory.contains("deliverId")) return false;

        auto roomObj = Screeps::Game.getObjectById(memory["deliverId"]);

        if (!roomObj)
        {
            memory.erase("deliverId");
            memory.erase("deliverType");
            creep.setMemory(memory);
            return false;
        }

        auto structure = dynamic_cast<Screeps::Structure*>(roomObj.release());

        if (creep.transfer(*structure, Screeps::RESOURCE_ENERGY) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(*structure);

        return true;
    }

    /// Function to find the closest dropped energy resource
    bool StemCreep::gather()
    {
        JSON memory = creep.memory();

        if (!memory.contains("gatherId"))
        {
            setGatherId();
            memory = creep.memory();
            if (!memory.contains("gatherId")) return false;
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
            return false;
        }

        auto* resource = dynamic_cast<Screeps::Resource*>(roomObj.get());

        if (creep.pickup(*resource) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(*roomObj);

        return true;
    }

    /// Look for storage or containers to withdraw energy from
    bool StemCreep::withdraw() {



        setWithdrawId();

        JSON memory = creep.memory();

        if (!memory.contains("withdrawId")) return false;

        auto withdrawId   = memory["withdrawId"].get<std::string>();
        auto withdrawType = memory.value("withdrawType", "storage");

        auto roomObj = Screeps::Game.getObjectById(memory["withdrawId"]);

        if (!roomObj)
        {
            memory.erase("withdrawId");
            memory.erase("withdrawType");
            creep.setMemory(memory);
            return false;
        }

        if (creep.withdraw(*roomObj, Screeps::RESOURCE_ENERGY) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(*roomObj);

        return true;
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
        auto containers = creep.room().find(Screeps::FIND_STRUCTURES,[this](const JS::Value& v) {
            return v["structureType"].as<std::string>() == Screeps::STRUCTURE_CONTAINER &&
               v["store"]["energy"].as<int>() >= creep.store().getFreeCapacity();});

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
        else {
            memory.erase("withdrawId");
            memory.erase("withdrawType");
            creep.setMemory(memory);
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

            //JS::console.log(std::string("chosing source with index .") + std::to_string(index) + std::string(" out of number sources : ") + std::to_string(sources.size()));

            auto* source = dynamic_cast<Screeps::Source*>(sources[index].get());
            if (source)
            {
                memory["sourceId"] = source->id();
                creep.setMemory(memory);
            }

        }

    void StemCreep::setDeliverId()
    {
        JSON memory = creep.memory();

        // Repeated function
        auto trySet = [&](const std::string& id, const std::string& type) -> bool
        {
            memory["deliverId"]   = id;
            memory["deliverType"] = type;
            creep.setMemory(memory);
            return true;
        };

        // First fill towers that have less than 500 energy
        auto towers = creep.room().find(Screeps::FIND_MY_STRUCTURES, [](const JS::Value& v) {
            return v["structureType"].as<std::string>() == Screeps::STRUCTURE_TOWER
                && v["store"]["energy"].as<int>() < 900;
        });
        if (!towers.empty()) {
            auto closest = creep.pos().findClosestByRange(towers);
            if (closest) {
                auto* t = dynamic_cast<Screeps::Structure*>(closest.get());
                trySet(t->id(), "tower"); return;
            }
        }

        // Secondly fill extensions
        auto extensions = creep.room().find(Screeps::FIND_MY_STRUCTURES, [](const JS::Value& v) {
            if (v["structureType"].as<std::string>() == Screeps::STRUCTURE_EXTENSION && v["energy"].as<int>() < v["energyCapacity"].as<int>())
            {
                return true;
            }
            return false;
        });


        if (!extensions.empty()) {
            auto closest = creep.pos().findClosestByRange(extensions);
            if (closest) {
                auto* s = dynamic_cast<Screeps::Structure*>(closest.get());
                if (s) { trySet(s->id(), "extension"); return; }
            }
        }

        // Then fill spawns
        auto spawns = creep.room().find(Screeps::FIND_MY_SPAWNS);
        if (!spawns.empty()) {
            auto closest = creep.pos().findClosestByRange(spawns);
            if (closest ) {
                auto* s = dynamic_cast<Screeps::StructureSpawn*>(closest.get());
                if (s && s->store().getFreeCapacity(Screeps::RESOURCE_ENERGY) > 0) { trySet(s->id(), "spawn"); return; }
            }
        }

        // Finally drop off in storage
        auto storage = creep.room().storage();
        if (storage.has_value() && storage->store().getFreeCapacity(Screeps::RESOURCE_ENERGY) > 0)
        { trySet(storage->id(), "storage"); return; }

        // Nothing found so erase ids
        memory.erase("deliverId");
        memory.erase("deliverType");
        creep.setMemory(memory);
    }

    /// This function return the source object given an id
    Screeps::Source* StemCreep::getSourceById(const std::string& sourceId)
        {
            auto object = Screeps::Game.getObjectById(sourceId);
            if (!object) return nullptr;
            return dynamic_cast<Screeps::Source*>(object.release());
        }

}
