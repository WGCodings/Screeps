
#include "Creeps/Runner.hpp"
#include "Screeps/Room.hpp"
#include "Screeps/RoomPosition.hpp"
#include "Screeps/Store.hpp"
#include "Screeps/StructureStorage.hpp"

namespace Peabrain {

    void Runner::run()
    {
        JSON memory = creep.memory();

        std::string task = memory.value("task", "gathering");

        if ((task == "withdrawing" || task == "gathering") &&  creep.store().getFreeCapacity(Screeps::RESOURCE_ENERGY) == 0)
        {
            task = "delivering";
            creep.say("Deliver.");
        }
        else if (task == "delivering" && creep.store().getUsedCapacity(Screeps::RESOURCE_ENERGY) == 0)
        {
            task = "gathering";
            creep.say("Gather.");
        }


        if (task == "gathering") {
            if (!gather())
            {
                withdraw();
                task = "withdrawing";
                creep.say("Withdraw.");
            }
        }
        else if (task == "withdrawing") {
            withdraw();
        }
        else if (task == "delivering") {
            deliver();
        }

        memory = creep.memory();
        memory["task"] = task;
        creep.setMemory(memory);
    }

    /// Look for storage or containers to withdraw energy from
    bool Runner::withdraw() {


        JSON memory = creep.memory();

        if (!memory.contains("withdrawId"))
        {
            Runner::setWithdrawId();
            memory = creep.memory();

            if (!memory.contains("withdrawId")) return false;
        }

        auto withdrawId   = memory["withdrawId"].get<std::string>();

        auto roomObj = Screeps::Game.getObjectById(memory["withdrawId"]);

        if (!roomObj)
        {
            memory.erase("withdrawId");
            memory.erase("withdrawType");
            creep.setMemory(memory);
            return false;
        }

        if (creep.withdraw(*roomObj, Screeps::RESOURCE_ENERGY) != Screeps::OK) {
            creep.moveTo(*roomObj);
        }
        else {
            memory.erase("withdrawId");
            memory.erase("withdrawType");
            creep.setMemory(memory);
        }

        return true;
    }

    /// Set the id of the container or storage to memory from where to withdraw
    /// First look for storage then containers
    void Runner::setWithdrawId()
    {
        JSON memory = creep.memory();

        // Collect all container ids already claimed by other runners
        std::vector<std::string> claimedIds;

        for (auto& [name, other] : Screeps::Game.creeps())
        {
            if (other.name() == creep.name()) continue; // skip own creep
            JSON otherMemory = other.memory();
            if (otherMemory.value("role", "") == "runner" && otherMemory.contains("withdrawId"))
                claimedIds.push_back(otherMemory["withdrawId"].get<std::string>());
        }

        // 1. Find a container with enough energy that isn't claimed
        auto containers = creep.room().find(Screeps::FIND_STRUCTURES, [](const JS::Value& v) {
            return v["structureType"].as<std::string>() == Screeps::STRUCTURE_CONTAINER && v["store"]["energy"].as<int>() > 200;});

        if (!containers.empty())
        {

            // Filter out claimed containers by other runners
            std::vector<std::unique_ptr<Screeps::RoomObject>> availableContainers;
            for (auto& container : containers)
            {
                auto* structure = dynamic_cast<Screeps::Structure*>(container.get());

                if (structure && std::find(claimedIds.begin(), claimedIds.end(), structure->id()) == claimedIds.end()) {
                    availableContainers.push_back(std::move(container));
                    JS::console.log(std::string("found available container."));
                }

            }

            if (!availableContainers.empty())
            {
                auto closest = creep.pos().findClosestByRange(availableContainers);

                auto* structure = dynamic_cast<Screeps::Structure*>(closest.get());

                memory["withdrawId"]   = structure->id();
                memory["withdrawType"] = "container";
                creep.setMemory(memory);
                return;

            }
        }
        // 2. Storage
        auto storage = creep.room().storage();
        if (storage.has_value() && storage->store().getUsedCapacity(Screeps::RESOURCE_ENERGY) > 500)
        {
            memory["withdrawId"]   = storage->id();
            memory["withdrawType"] = "storage";
            creep.setMemory(memory);
        }
    }
}
