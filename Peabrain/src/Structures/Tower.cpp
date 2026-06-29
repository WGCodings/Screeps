

#include "Structures/Tower.hpp"

#include "nlohmann/json.hpp"
#include "Screeps/Constants.hpp"
#include "Screeps/Creep.hpp"
#include "Screeps/Game.hpp"
#include "Screeps/JS.hpp"
#include "Screeps/Room.hpp"
#include "Screeps/RoomPosition.hpp"


namespace Peabrain {

    void Tower::run()
    {
        // Fire first, then repair stuff
        if (!fire())
            repair();
    }

    /// Shoot hostile creeps
    bool Tower::fire()
    {
        // Check room memory for a focusFire target first
        JSON roomMemory = tower.room().memory();
        if (roomMemory.contains("focusTargetId"))
        {
            auto focusFireTarget = Screeps::Game.getObjectById(roomMemory["focusTargetId"].get<std::string>());
            if (focusFireTarget)
            {
                auto* enemy = dynamic_cast<Screeps::Creep*>(focusFireTarget.get());
                if (enemy)
                {
                    tower.attack(*enemy);
                    return true;
                }
            }
            // Target is gone, clear from memory
            roomMemory.erase("focusTargetId");
            tower.room().setMemory(roomMemory);
        }

        // No focus target , just find a target
        auto hostiles = tower.room().find(Screeps::FIND_HOSTILE_CREEPS);
        if (hostiles.empty()) return false;

        auto closest = tower.pos().findClosestByRange(hostiles);
        auto* enemy = dynamic_cast<Screeps::Creep*>(closest.get());

        tower.attack(*enemy);
        return true;
    }

    /// Look for a specific target and put enemy id in memory
    void Tower::focus() const {
        // Find the closest hostile and store its id in room memory
        // so all towers in the room shoot the same target
        auto hostiles = tower.room().find(Screeps::FIND_HOSTILE_CREEPS);
        if (hostiles.empty()) return;

        auto closest = tower.pos().findClosestByRange(hostiles);

        auto* enemy = dynamic_cast<Screeps::Creep*>(closest.get());

        JSON roomMemory = tower.room().memory();
        roomMemory["focusTargetId"] = enemy->id();
        tower.room().setMemory(roomMemory);
    }


    /// Try to repair a structure in need of repair
    /// Loop over memory where the status of the strucutre is 'repair'.
    void Tower::repair()
    {
        // Find a new repair target from blueprint entries marked "repair"
        JSON roomMemory = tower.room().memory();
        const JSON& blueprint = roomMemory.value("blueprint", JSON::object());

        for (auto& [key, entry] : blueprint.items())
        {
            if (entry.value("status", "") != "repair") continue;

            int x = entry["x"].get<int>();
            int y = entry["y"].get<int>();

            auto objects = tower.room().lookForAt(Screeps::LOOK_STRUCTURES, x, y);
            if (objects.empty()) continue;

            auto* structure = dynamic_cast<Screeps::Structure*>(objects.front().get());
            if (!structure || structure->hits() > structure->hitsMax()*0.9) continue;

            tower.repair(*structure);
            return;
        }
    }

}
