#include "Creeps/Miner.hpp"

#include "Screeps/Room.hpp"
#include "Screeps/RoomPosition.hpp"
#include "Screeps/Store.hpp"
#include "Screeps/StructureContainer.hpp"
#include "Screeps/StructureLink.hpp"


namespace Peabrain {

    void Miner::run()
    {

        JSON memory = creep.memory();

        std::string task = memory.value("task", "moving");

        bool atLocation = memory.value("atLocation", false); // Will be put to yes in moveToContainer

        if (task == "moving" && atLocation)
        {
            task = "harvesting";
            creep.say("Mine.");
        }
        else if (task == "harvesting"
            && creep.store().getFreeCapacity(Screeps::RESOURCE_ENERGY) == 0
            && creep.store().getCapacity() > 0)
        {
            task = "delivering";
            creep.say("Deliver.");
        }
        else if (task == "delivering" && creep.store().getUsedCapacity(Screeps::RESOURCE_ENERGY) == 0)
        {
            task = "harvesting";
            creep.say("Mine.");
        }

        if (task == "moving")
        {
            moveToContainer();
        }
        else if (task == "harvesting")
        {
            harvest();
        }
        else if (task == "delivering"){
            // if you cant find a link to deposit, just drop it and start mining
            if (!deliver()) {
                creep.drop(Screeps::RESOURCE_ENERGY,creep.store().getUsedCapacity(Screeps::RESOURCE_ENERGY));
                creep.say("Mine.");
                task = "harvesting";
            }
        }

        memory["task"] = task;
        creep.setMemory(memory);
    }

    /// Find source and go harvest b*tch
    void Miner::harvest()
    {
        JSON memory = creep.memory();

        if (!memory.contains("sourceId"))
        {
            Miner::setSourceId();
            memory = creep.memory();

            if (!memory.contains("sourceId")) return;
        }

        Screeps::Source* source = getSourceById(memory["sourceId"]);

        if (!source) {
            memory.erase("sourceId");
            creep.setMemory(memory);
            return;
        }

        creep.harvest(*source);

    }

    // Overwrite of StemCreep deliver function but just simplified for the near link only.
    bool Miner::deliver()
    {
        JSON memory = creep.memory();

        if (!memory.contains("deliverId"))
        {
            Miner::setDeliverId();
            memory = creep.memory();

            if (!memory.contains("deliverId")) return false;
        }

        auto roomObj = Screeps::Game.getObjectById(memory["deliverId"]);

        if (!roomObj)
        {
            memory.erase("deliverId");
            memory.erase("deliverType");
            creep.setMemory(memory);
            return false;
        }

        auto structure = dynamic_cast<Screeps::Structure*>(roomObj.release());

        creep.transfer(*structure, Screeps::RESOURCE_ENERGY);

        return true;
    }

    /// This function finds a source and assign the id to the memory of the harvester
    void Miner::setSourceId()
    {
        auto roomObject = creep.pos().findClosestByRange(std::vector<std::unique_ptr<Screeps::RoomObject> >(Screeps::FIND_SOURCES));

        JSON memory = creep.memory();

        auto* source = dynamic_cast<Screeps::Source*>(roomObject.release());
        if (source)
        {
            memory["sourceId"] = source->id();
            creep.setMemory(memory);
        }

    }

    void Miner::setDeliverId()
    {
        JSON memory = creep.memory();

        JSON options;
        options["filter"]["structureType"] = Screeps::STRUCTURE_LINK;

        auto roomObject = creep.pos().findInRange(std::vector<std::unique_ptr<Screeps::RoomObject> >(Screeps::FIND_STRUCTURES),1,options);

        auto* link = dynamic_cast<Screeps::StructureLink*>(roomObject.front().release());

        if (link)
        {
            JS::console.log(std::string("Found link"));
            memory["deliverId"] = link->id();
            memory["deliverType"] = "link";
            creep.setMemory(memory);
        }
    }

    void Miner::moveToContainer() {

        JSON memory = creep.memory();

        // Sets the container id
        if (!memory.contains("containerId"))
        {
            // Do stuff to add unique unused container to memory

            // Collect all container ids already claimed by other miners
            std::vector<std::string> claimedIds;

            for (auto& [name, other] : Screeps::Game.creeps())
            {
                if (other.name() == creep.name()) continue; // skip own creep
                JSON otherMemory = other.memory();
                if (otherMemory.value("role", "") == "miner" && otherMemory.contains("containerId"))
                    claimedIds.push_back(otherMemory["containerId"].get<std::string>());
            }
            // Find a container
            auto containers = creep.room().find(Screeps::FIND_STRUCTURES, [](const JS::Value& v) {
                return v["structureType"].as<std::string>() == Screeps::STRUCTURE_CONTAINER ;});


            // Filter out claimed containers by other runners
            std::vector<std::unique_ptr<Screeps::RoomObject>> availableContainers;
            for (auto& container : containers)
            {
                auto* structure = dynamic_cast<Screeps::Structure*>(container.release());

                if (structure && std::find(claimedIds.begin(), claimedIds.end(), structure->id()) == claimedIds.end()) {
                    availableContainers.push_back(std::move(container));
                }
            }

            if (!availableContainers.empty())
            {
                auto first = std::move(availableContainers.front());

                auto* structure = dynamic_cast<Screeps::Structure*>(first.release());

                memory["containerId"]   = structure->id();
                creep.setMemory(memory);
            }
        }

        auto containerId   = memory["containerId"].get<std::string>();

        auto object = Screeps::Game.getObjectById(containerId);

        if (!object)
        {
            memory.erase("containerId");
            creep.setMemory(memory);
            return;
        }

        auto* container = dynamic_cast<Screeps::StructureContainer*>(object.release());

        auto containerPos = container->pos();

        if (creep.pos().inRangeTo(containerPos,0)){
            memory["atLocation"] = true;
        }
        else {
            memory["atLocation"] = false;
            creep.moveTo(container->pos());
        }
        creep.setMemory(memory);
    }
}
