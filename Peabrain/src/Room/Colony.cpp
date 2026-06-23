
#include "Room/Colony.hpp"

#include "Screeps/Game.hpp"
#include "Screeps/Room.hpp"
#include "Screeps/RoomPosition.hpp"
#include "Screeps/RoomTerrain.hpp"
#include "Screeps/StructureController.hpp"


namespace Peabrain {
    /// The idea of the colony planner is to put all structures to be built ever in memory.
    /// It will be build up like
    /// {
    /// [
    /// {x1,y1,controllerLevel1,structType1},
    /// {x2,y2,controllerLevel2,structType2},
    /// ...
    /// ]
    /// }
    /// Then later we can build on every spot based on the controller level of our room.
    void Colony::plan() {

        JSON memory = room.memory();

        // Only plan once
        if (memory.value("planned", false))
            return;

        JS::console.log(std::string("Planning the room."));

        planContainers();
        planLinks();

        // Set planned to true in memory such that this only runs once per room.
        memory = room.memory();
        memory["planned"] = true;
        room.setMemory(memory);

    }
    /// This plans the containers and puts them in memory
    /// Places containers around energy sources
    void Colony::planContainers() {

        JS::console.log(std::string("Planning the containers."));

        JSON memory = room.memory();

        // Initialize blueprint
        if (!memory.contains("blueprint"))
            memory["blueprint"] = JSON::object();

        auto sources = room.find(Screeps::FIND_SOURCES_ACTIVE);

        if (sources.empty()) return;

        int controller_x = room.controller().value().pos().x();
        int controller_y = room.controller().value().pos().y();


        for (auto &source : sources) {

            int lowest_cost = 1000;
            int best_x = 0;
            int best_y = 0;
            int source_x = source->pos().x();
            int source_y = source->pos().y();

            for (int dx = -1; dx < 2; dx++) {
                for (int dy = -1; dy < 2; dy++) {
                    if (dx == 0 && dy == 0)
                        continue;
                    int x = source_x + dx;
                    int y = source_y + dy;
                    if (room.getTerrain().get(x, y) != Screeps::TERRAIN_MASK_WALL) {
                        int cost = std::abs(x - controller_x) + std::abs(y - controller_y);
                        if (cost < lowest_cost) {
                            lowest_cost = cost;
                            best_x = x;
                            best_y = y;
                        }
                    }
                }
            }
            std::string key = std::string(Screeps::STRUCTURE_CONTAINER)+ "_" + std::to_string(best_x)+ "_" + std::to_string(best_y);

            JSON entry;
            entry["x"] = best_x;
            entry["y"] = best_y;
            entry["sType"]  = Screeps::STRUCTURE_CONTAINER;
            entry["cLevel"] = room.controller()->level();
            entry["status"] = "planned";

            memory["blueprint"][key] = entry;
        }
        room.setMemory(memory);
    }

    /// This planner plans the links
    /// Look for spot near spawn for easy acces
    /// Looks for another two spots adjacent to already planned containers
    /// TODO Functions should be simplified by helper functions, lots of double code
    void Colony::planLinks()
    {
        JSON memory = room.memory();

        JS::console.log(std::string("Planning the links."));

        // Central link, 2 above/left/right/under spawn, for easy acces later
        auto spawns = room.find(Screeps::FIND_MY_SPAWNS);
        if (spawns.empty()) return;

        auto* spawn = dynamic_cast<Screeps::Structure*>(spawns.front().get());

        int spawn_x = spawn->pos().x();
        int spawn_y = spawn->pos().y();

        const std::vector<std::pair<int,int>> directions = {{0,-2},{2,0},{0,2},{-2,0}};

        for (auto& [dx, dy] : directions)
        {
            int x = spawn_x + dx;
            int y = spawn_y + dy;

            if (room.getTerrain().get(x, y) != Screeps::TERRAIN_MASK_WALL)
            {
                std::string key = std::string(Screeps::STRUCTURE_LINK)+ "_" + std::to_string(x)+ "_" + std::to_string(y);

                JSON entry;
                entry["x"]      = x;
                entry["y"]      = y;
                entry["sType"]  = Screeps::STRUCTURE_LINK;
                entry["cLevel"] = 6;
                entry["status"] = "planned";

                memory["blueprint"][key] = entry;
                break;
            }
        }

        // Links somewhere around the containers already in memory
        for (auto& [name, entry] : memory["blueprint"].items())
        {
            // Only process containers
            if (entry.value("sType", "") != Screeps::STRUCTURE_CONTAINER)
                continue;

            int container_x = entry["x"].get<int>();
            int container_y = entry["y"].get<int>();

            int lowest_cost = 1000;
            int best_x = 0;
            int best_y = 0;

            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    if (dx == 0 && dy == 0) continue;

                    int x = container_x + dx;
                    int y = container_y + dy;

                    if (room.getTerrain().get(x, y) != Screeps::TERRAIN_MASK_WALL)
                    {
                        int cost = std::abs(x - spawn_x) + std::abs(y - spawn_y);
                        if (cost < lowest_cost)
                        {
                            lowest_cost = cost;
                            best_x = x;
                            best_y = y;
                        }
                    }
                }
            }

            if (best_x == 0 && best_y == 0) continue; // no valid spot

            std::string key = std::string(Screeps::STRUCTURE_LINK)+ "_" + std::to_string(best_x)+ "_" + std::to_string(best_y);

            JSON entry2;
            entry2["x"]      = best_x;
            entry2["y"]      = best_y;
            entry2["sType"]  = Screeps::STRUCTURE_LINK;
            entry2["cLevel"] = 6;
            entry2["status"] = "planned";

            memory["blueprint"][key] = entry2;
        }

        room.setMemory(memory);
    }
}
