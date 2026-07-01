#include "Room/Colony.hpp"

#include "nlohmann/json.hpp"
#include "Screeps/Game.hpp"
#include "Screeps/JS.hpp"
#include "Screeps/StructureController.hpp"


namespace Peabrain {

    void Colony::colonise() {

        if ((Screeps::Game.time()-5) % 100 != 0)
            return;

        setScreepsMemory();
    }

    /// Sets the maximum number of creeps for some conditions.
    /// Splits in four stages of civilization, based on controller level and structures built.
    void Colony::setScreepsMemory() {

        JS::console.log(std::string("Setting the creeps memory for the room."));

        JSON memory = room.memory();

        auto cLevel = room.controller().value().level();

        auto nrOfSources = room.find(Screeps::FIND_SOURCES_ACTIVE).size();

        int containersBuilt = 0;

        int extensionsBuilt = 0;

        bool storageBuilt = false;

        int linksBuilt = 0;

        for (auto& [key, entry] : memory["blueprint"].items()) {

            auto status = entry["status"].get<std::string>();

            if (status == "built" || status == "repair") {

                auto sType = entry["sType"].get<std::string>();

                if (sType == "storage") {
                    storageBuilt = true;
                }
                if (sType == "link") {
                    linksBuilt++;
                }
                if (sType == "container") {
                    containersBuilt++;
                }
                if (sType == "extension") {
                    extensionsBuilt++;
                }
            }
        }

        auto stageTwo = cLevel >=2 && containersBuilt >= nrOfSources && extensionsBuilt >= 5;
        auto stageThree = stageTwo && cLevel >= 4 && storageBuilt;
        auto stageFour = stageThree && cLevel >= 6 && linksBuilt == nrOfSources+1;

        if (stageFour) {
            memory["colony"] = {
                {"stage",       4},
                {"harvesters",  0},
                {"upgraders",   0},
                {"builders",    2},
                {"haulers",     0},
                {"miners",      2},
                {"linker",      1}
            };
        }
        else if (stageThree) {
            memory["colony"] = {
                {"stage",       3},
                {"harvesters",  0},
                {"upgraders",   0},
                {"builders",    2},
                {"haulers",     2},
                {"miners",      2},
                {"linker",      0}
            };
        }
        else if (stageTwo) {
            memory["colony"] = {
                {"stage",       2},
                {"harvesters",  0},
                {"upgraders",   1},
                {"builders",    3},
                {"haulers",     2},
                {"miners",      2},
                {"linker",      0}
            };
        }
        // Stage one
        else {
            memory["colony"] = {
                {"stage",       1},
                {"harvesters",  4},
                {"upgraders",   0},
                {"builders",    4},
                {"haulers",     0},
                {"miners",      0},
                {"linker",      0}
            };
        }
        room.setMemory(memory);
    }
}
