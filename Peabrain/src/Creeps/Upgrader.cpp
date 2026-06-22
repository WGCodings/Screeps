#include "Creeps/Upgrader.hpp"

#include <Screeps/Constants.hpp>
#include <Screeps/StructureController.hpp>

#include "Screeps/Room.hpp"
#include "Screeps/Store.hpp"

namespace Peabrain {

    void Upgrader::run()
    {
        JSON memory = creep.memory();

        std::string task = memory.value("task", "harvesting");

        if (task == "harvesting" &&  creep.store().getFreeCapacity(Screeps::RESOURCE_ENERGY) == 0)
        {
            task = "upgrading";
            creep.say("Upgrade.");
        }
        else if (task == "upgrading" && creep.store().getUsedCapacity(Screeps::RESOURCE_ENERGY) == 0)
        {
            task = "harvesting";
            creep.say("Harvest.");
        }

        memory["task"] = task;
        creep.setMemory(memory);

        if (task == "harvesting")
            harvest();
        else
            upgrade();
    }

    void Upgrader::harvest()
    {
        JSON memory = creep.memory();

        if (!memory.contains("sourceId"))
        {
            findSource(memory);
            if (!memory.contains("sourceId")) return;
        }

        Screeps::Source* source = getSourceById(memory["sourceId"]);

        if (!source) { memory.erase("sourceId"); creep.setMemory(memory); return; }

        if (creep.harvest(*source) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(*source);
    }
}