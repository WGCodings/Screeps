
#include "Room/Architect.hpp"

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
    /// {x1,y1,controllerLevel1,structType1,status},
    /// {x2,y2,controllerLevel2,structType2,status},
    /// ...
    /// ]
    /// }
    /// Then later we can build on every spot based on the controller level of our room.
    void Architect::plan() {
        planSpawns();
        planContainers();
        planLinks();
        planStorage();
        planTowers();
        planExtensions();
        planRoads();
        // TODO planWallsAndRamparts();
        // TODO planExtractor();
        // TODO planLabs();
        // TODO planTerminal();
        // TODO planFactory();
        // TODO planObserver() planPowerSpawn() planNuker()
        }

    /// This function iterates over all the structures in memory.
    /// It first checks if there is a structure present on the x,y.
    /// If present, then it checks hits/hitsmax, if lower than 0.5 then set status to 'repair', else 'built'.
    /// If there is not structure, check if there is an construction site. If not, it was destroyed, so set status to 'planned'.
    void Architect::reviewStructures() {

        if (Screeps::Game.time() % 90 != 0)
            return;

        JS::console.log(std::string("Architect is reviewing the room structure plan."));

        JSON memory = room.memory();

        for (auto& [key, entry] : memory["blueprint"].items()) {
            int x = entry["x"].get<int>();
            int y = entry["y"].get<int>();
            auto structures = room.lookForAt(Screeps::LOOK_STRUCTURES,x,y);
            // If there is a structure, check hits.
            // If damages set status to repair, else set status to built.
            if (!structures.empty()) {
                auto* structure = dynamic_cast<Screeps::Structure*>(structures.front().get());
                if (structure->hits() < 0.5*structure->hitsMax()) {
                    entry["status"] = "repair";
                    JS::console.log(std::string(key) + "needs repair.");
                }
                else {
                    entry["status"] = "built";
                }
            }
            // If there is not a structure on this spot, check if there is a construction site
            // If there is no construction site, this means it got destroyed so set the status to planned
            else {
                auto constructionSites = room.lookForAt(Screeps::LOOK_CONSTRUCTION_SITES,x,y);
                if (constructionSites.empty() && entry["status"].get<std::string>() != "planned") {
                    entry["status"] = "planned";
                    JS::console.log(std::string(key) + " was destroyed and should be replanned.");
                }
                else {
                    entry["status"] = "construction";
                }
            }
            memory["blueprint"][key] = entry;
        }
        room.setMemory(memory);
        JS::console.log(std::string("Architect is done reviewing the room structure plan."));
    }

    /// This function iterates over all the structures in memory.
    /// Checks if the structure has status planned and can be built based on controller level
    /// Make the construction site on the roomPosition and set status to 'construction'.
    void Architect::buildStructures() {
        if (Screeps::Game.time() % 200 != 0)
            return;

        JS::console.log(std::string("Architect is placing construction tasks."));

        JSON memory = room.memory();

        auto controllerLevel = room.controller().value().level();

        for (auto& [key, entry] : memory["blueprint"].items()) {
            // If entry has status = planned and cLevel of this entry <= actual controller level -> it can and should be constructed.
            if (entry["status"] == "planned" && entry["cLevel"] <= controllerLevel) {
                int x = entry["x"].get<int>();
                int y = entry["y"].get<int>();

                auto constructionPosition = Screeps::RoomPosition(room.name(),x,y);

                constructionPosition.createConstructionSite(entry["sType"]);

                entry["status"] = "construction";
                memory["blueprint"][key] = entry;
                room.setMemory(memory);
                JS::console.log(std::string("Architect is placing a construction task for ") + std::string(key));

            }
        }
        JS::console.log(std::string("Architect is done placing construction tasks."));
    }

    /// Planner for placing the spawns
    void Architect::planSpawns() {
        JSON memory = room.memory();

        if (memory["spawnsPlanned"] == true) {
            return;
        }
        JS::console.log(std::string("Planning the spawns."));

        // Initialize blueprint
        if (!memory.contains("blueprint"))
            memory["blueprint"] = JSON::object();

        auto spawns = room.find(Screeps::FIND_MY_SPAWNS);
        auto* spawn = dynamic_cast<Screeps::Structure*>(spawns.front().get());

        int x = spawn->pos().x();
        int y = spawn->pos().y();

        addEntryOnCoords(memory, x,y,Screeps::STRUCTURE_SPAWN,1);

        memory["spawnsPlanned"] = true;
        room.setMemory(memory);

    }

    /// This plans the containers and puts them in memory
    /// Places containers around energy sources
    void Architect::planContainers() {

        JSON memory = room.memory();

        if (memory["containersPlanned"] == true) {
            return;
        }

        JS::console.log(std::string("Planning the containers."));

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
            addEntryOnCoords(memory, best_x, best_y, Screeps::STRUCTURE_CONTAINER,  1);
        }
        memory["containersPlanned"] = true;
        room.setMemory(memory);
    }

    /// This planner plans the links
    /// Look for spot near spawn for easy acces
    /// Looks for another two spots adjacent to already planned containers
    /// TODO Functions should be simplified by helper functions, lots of double code
    void Architect::planLinks()
    {
        JSON memory = room.memory();

        if (memory["linksPlanned"] == true) {
            return;
        }

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
                entry["role"]   = "central";

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
            entry2["role"]    = "source";

            memory["blueprint"][key] = entry2;
        }
        memory["linksPlanned"] = true;
        room.setMemory(memory);
    }

    /// Plans the storage
    /// Must be 2 left,right,under or above the central link
    void Architect::planStorage(){
        JSON memory = room.memory();

        if (memory["storagePlanned"] == true) {
            return;
        }

        JS::console.log(std::string("Planning the storage."));

        for (auto& [name, entry] : memory["blueprint"].items())
        {
            // Look for link with role 'central'
            if (entry.value("sType", "") == Screeps::STRUCTURE_LINK && entry.value("role", "") == "central") {
                int link_x = entry["x"].get<int>();
                int link_y = entry["y"].get<int>();

                const std::vector<std::pair<int,int>> directions = {{0,-2},{2,0},{0,2},{-2,0}};

                for (auto& [dx, dy] : directions)
                {
                    int x = link_x + dx;
                    int y = link_y + dy;

                    if ((room.getTerrain().get(x, y) != Screeps::TERRAIN_MASK_WALL) && !hasEntryOnCoords(memory,x,y))
                    {
                        addEntryOnCoords(memory, x, y, Screeps::STRUCTURE_STORAGE,  4);
                        break;
                    }
                }
                room.setMemory(memory);
            }

        }
        memory["storagePlanned"] = true;
        room.setMemory(memory);
    }

    /// Plans the towers in checkerboard formation around spawn.
    void Architect::planTowers() {


        JSON memory = room.memory();

        if (memory["towersPlanned"] == true) {
            return;
        }

        JS::console.log(std::string("Planning the Towers."));

        auto spawns = room.find(Screeps::FIND_MY_SPAWNS);
        if (spawns.empty()) return;

        auto* spawn = dynamic_cast<Screeps::Structure*>(spawns.front().get());

        int spawn_x = spawn->pos().x();
        int spawn_y = spawn->pos().y();

        // To iterate over checkerboard from bottomright clockwise in increasing radius
        const std::vector<std::pair<int,int>> directions = {{-2,0},{0,-2},{2,0},{0,2}};

        constexpr int max_towers = 6;
        int radius = 0;
        int towers_placed = 0;
        int cLevel;

        while (towers_placed < max_towers) {
            radius ++;
            int x = spawn_x + radius;
            int y = spawn_y + radius;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < radius; j++) {
                    x += directions.at(i).first;
                    y += directions.at(i).second;

                    if (!hasEntryOnCoords(memory, x, y) && towers_placed < max_towers && (room.getTerrain().get(x, y) != Screeps::TERRAIN_MASK_WALL)) {

                        // More towers are placed at higher cLevel
                        if (towers_placed == 0) {cLevel = 3;}else if (towers_placed == 1) {cLevel = 5;}else if (towers_placed ==2) { cLevel = 7;} else { cLevel = 8;}

                        towers_placed ++;

                        addEntryOnCoords(memory, x, y, Screeps::STRUCTURE_TOWER,  cLevel);

                        room.setMemory(memory);
                    }
                }
            }
        }
        memory["towersPlanned"] = true;
        room.setMemory(memory);
    }

    /// Plans the extensions in checkerboard formation around spawn.
    /// Copy paste from planTowers, should be simplified in common helper function
    void Architect::planExtensions() {

        JSON memory = room.memory();

        if (memory["extensionsPlanned"] == true) {
            return;
        }

        JS::console.log(std::string("Planning the Extensions."));

        auto spawns = room.find(Screeps::FIND_MY_SPAWNS);
        if (spawns.empty()) return;

        auto* spawn = dynamic_cast<Screeps::Structure*>(spawns.front().get());

        int spawn_x = spawn->pos().x();
        int spawn_y = spawn->pos().y();

        // To iterate over checkerboard from bottomright clockwise in increasing radius
        const std::vector<std::pair<int,int>> directions = {{-2,0},{0,-2},{2,0},{0,2}};

        constexpr int max_extensions = 60;
        int radius = 0;
        int extensions_placed = 0;
        int cLevel = 2;

        while (extensions_placed < max_extensions) {
            radius ++;
            int x = spawn_x + radius;
            int y = spawn_y + radius;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < radius; j++) {
                    x += directions.at(i).first;
                    y += directions.at(i).second;

                    if (!hasEntryOnCoords(memory, x, y) && extensions_placed < max_extensions && (room.getTerrain().get(x, y) != Screeps::TERRAIN_MASK_WALL)) {

                        extensions_placed ++;
                        // More towers are placed at higher cLevel, should find a function for this.
                        if (extensions_placed <=5) {cLevel = 2;} else if (extensions_placed <=10) {cLevel = 3;} else if (extensions_placed <=20) {cLevel = 4;}
                        else if (extensions_placed <=30) {cLevel = 5;} else if (extensions_placed <=40) {cLevel = 6;}
                        else if (extensions_placed <=50) {cLevel = 7;} else if (extensions_placed <=60) {cLevel = 8;}

                        addEntryOnCoords(memory, x, y, Screeps::STRUCTURE_EXTENSION,  cLevel);

                        room.setMemory(memory);
                    }
                }
            }
        }
        memory["extensionsPlanned"] = true;
        room.setMemory(memory);
    }

    /// Plan the roadnetwork.
    /// In this simple version I will just place roads around every structure that is in memory
    /// Then I will add the path from sources to the storage as well
    /// Also the path from the storage to the controller I will add to the network.
    void Architect::planRoads() {

        JSON memory = room.memory();

        if (memory["roadsPlanned"] == true) {
            return;
        }

        JS::console.log(std::string("Planning the Roads."));

        // This part makes road between sources and the storage and controller and storage
        for (auto& [name, entry] : memory["blueprint"].items())
        {
            // Look for storage
            if (entry.value("sType", "") == Screeps::STRUCTURE_STORAGE) {
                int storage_x = entry["x"].get<int>();
                int storage_y = entry["y"].get<int>();

                auto storage_pos = Screeps::RoomPosition(room.name(), storage_x, storage_y);

                auto sources = room.find(Screeps::FIND_SOURCES);

                JSON pathOptions;
                pathOptions["ignoreCreeps"] = true;

                for (auto &source : sources) {

                    auto path = room.findPath(source->pos(),storage_pos, pathOptions);

                    // Loop over steps in path and place road everywhere where possible
                    for (auto& step : path) {
                        bool result = addEntryOnCoords(memory, step.x, step.y, Screeps::STRUCTURE_ROAD,  2);
                        if (result) {
                            JS::console.log(std::string("Placing road along source path on x = " + std::to_string(step.x) + " y = " + std::to_string(step.y)));
                            room.setMemory(memory);
                        }
                    }
                }

                // Finally make a path from the controller to the storage
                auto path = room.findPath(room.controller()->pos(), storage_pos, pathOptions);
                for (auto& step : path) {
                    bool result = addEntryOnCoords(memory, step.x, step.y, Screeps::STRUCTURE_ROAD, 2);
                    if (result) {
                        JS::console.log(std::string("Placing road along controller path on x = " + std::to_string(step.x) + " y = " + std::to_string(step.y)));
                        room.setMemory(memory);
                    }
                }
            }
        }

        // This part loops over all memory entries and puts raods there. Im a bit sloppy with setting cLevel here.
        // It is just the same as the one from entry. Not ideal but it works I hope?
        for (auto& [name, entry] : memory["blueprint"].items())
        {
            int x_entry = entry["x"].get<int>();
            int y_entry = entry["y"].get<int>();
            int cLevel = entry["cLevel"].get<int>();

            if (entry.value("sType", "") == Screeps::STRUCTURE_ROAD) {
                continue;
            }

            const std::vector<std::pair<int,int>> directions = {{0,1},{0,-1},{1,1},{1,-1},{1,0},{-1,1},{-1,-1},{-1,0}};

            for (auto& [dx, dy] : directions)
            {
                int x = x_entry + dx;
                int y = y_entry + dy;

                if (!hasEntryOnCoords(memory, x, y)) {
                    if (room.getTerrain().get(x, y) != Screeps::TERRAIN_MASK_WALL)
                    {
                        bool result = addEntryOnCoords(memory,x,y,Screeps::STRUCTURE_ROAD, cLevel);

                        if (result) {
                            JS::console.log(std::string("Placing road around on x = " + std::to_string(x) + " y = " + std::to_string(y)));
                            // TODO check if you can make this setMemory on a mutable memory?
                            room.setMemory(memory);
                            memory = room.memory();
                        }
                    }
                }
            }
        }

        memory["roadsPlanned"] = true;
        room.setMemory(memory);

    }

    /// Just a helper function to find if tehre already exists an entry on given coordinates
    bool Architect::hasEntryOnCoords(const JSON& memory, int x, int y) {

        for (auto& [key, entry] : memory["blueprint"].items())
        {
            if (entry.value("x", -1) == x && entry.value("y", -1) == y)
                return true;
        }

        return false;
    }

    /// Helper functon to add an entry in memory on the given coordinates, structure type and clevel
    /// It used hasEntryOnCoords() to filter out entries that are already in memory.
    bool Architect::addEntryOnCoords(JSON& memory, int x, int y, const std::string& sType, int cLevel)
    {
        if (hasEntryOnCoords(memory, x, y)) {
            return false;
        } // skip if spot already taken

        std::string key = sType + "_" + std::to_string(x) + "_" + std::to_string(y);
        memory["blueprint"][key] = {
            {"x",      x},
            {"y",      y},
            {"sType",  sType},
            {"cLevel", cLevel},
            {"status", "planned"},
        };
        return true;
    }


}
