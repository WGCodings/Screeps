
#include "Creeps/Builder.hpp"

#include "Creeps/Upgrader.hpp"

#include <Screeps/Constants.hpp>
#include <Screeps/StructureController.hpp>

#include "Screeps/ConstructionSite.hpp"
#include "Screeps/Room.hpp"
#include "Screeps/RoomPosition.hpp"
#include "Screeps/Store.hpp"

namespace Peabrain {

    void Builder::run()
    {
        JSON memory = creep.memory();

        std::string task = memory.value("task", "harvesting");

        if ((task == "withdrawing" || task == "harvesting") &&  creep.store().getFreeCapacity(Screeps::RESOURCE_ENERGY) == 0)
        {
            task = "building";
            creep.say("Build.");
        }
        else if ((task == "building" || task == "upgrading") && creep.store().getUsedCapacity(Screeps::RESOURCE_ENERGY) == 0)
        {
            task = "withdrawing";
            creep.say("Withdraw.");
        }

        if (task == "withdrawing") {
            if (!withdraw())
            {
                harvest();
                task = "harvesting";
                creep.say("Harvest.");
            }
        }
        else if (task == "harvesting") {
            harvest();
        }
        else if (task == "upgrading") {
            upgrade();
        }
        else if (task == "building") {
            // if no building site, upgrade
            if (!build())
            {
                upgrade();
                task = "upgrading";
                creep.say("Upgrade.");
            }
        }
        memory = creep.memory();
        memory["task"] = task;
        creep.setMemory(memory);
    }

    bool Builder::build() {
        JSON memory = creep.memory();

        if (!memory.contains("constructionSiteId"))
        {
            setConstructionSiteId();
            memory = creep.memory();
            // what to do if no construction site
            if (!memory.contains("constructionSiteId")) return false;
        }

        auto* site = getConstructionSiteById(memory["constructionSiteId"]);
        if (!site)
        {
            memory.erase("constructionSiteId");
            creep.setMemory(memory);
            return false;
        }

        if (creep.build(*site) == Screeps::ERR_NOT_IN_RANGE)
            creep.moveTo(*site);

        return true;
    }

    void Builder::setConstructionSiteId()
    {
        auto sites = creep.room().find(Screeps::FIND_CONSTRUCTION_SITES);
        if (sites.empty()) return;

        // Find closest construction site by path
        auto closest = creep.pos().findClosestByPath(sites);
        if (!closest) return;

        auto* site = dynamic_cast<Screeps::ConstructionSite*>(closest.get());
        if (!site) return;

        JSON memory = creep.memory();
        memory["constructionSiteId"] = site->id();
        creep.setMemory(memory);
    }

    Screeps::ConstructionSite* Builder::getConstructionSiteById(const std::string& cSiteId)
    {
        auto obj = Screeps::Game.getObjectById(cSiteId);
        if (!obj) return nullptr;
        return dynamic_cast<Screeps::ConstructionSite*>(obj.release());
    }
}
