#include "Creeps/Harvester.hpp"


#include <Screeps/Game.hpp>
#include <Screeps/Source.hpp>
#include <Screeps/StructureSpawn.hpp>

#include "Creeps/Upgrader.hpp"
#include "nlohmann/json.hpp"
#include "Screeps/Store.hpp"
#include "Screeps/Room.hpp"


namespace Peabrain {

void Harvester::run()
{

	JSON memory = creep.memory();

	std::string task = memory.value("task", "harvesting");

	if (task == "harvesting" &&  creep.store().getFreeCapacity(Screeps::RESOURCE_ENERGY) == 0)
	{
		task = "delivering";
		creep.say("Deliver.");
	}
	else if (task == "delivering" && creep.store().getUsedCapacity(Screeps::RESOURCE_ENERGY) == 0)
	{
		task = "harvesting";
		creep.say("Harvest.");
	}
	else if (task == "upgrading" && creep.store().getUsedCapacity(Screeps::RESOURCE_ENERGY) == 0)
	{
		task = "harvesting";
		creep.say("Harvest.");
	}

	memory["task"] = task;
	creep.setMemory(memory);

	if (task == "harvesting")
	{
		harvest();
	}
	else if (task == "delivering")
	{
		deliver();
	}
	else {
		upgrade();
	}
}


/// Find source and go harvest b*tch
void Harvester::harvest()
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

/// Deliver your energy to the source b*tch
/// If all sources are full, go upgrading the controller
void Harvester::deliver()
{

	JSON memory = creep.memory();

	std::map<std::string, Screeps::StructureSpawn> spawns = Screeps::Game.spawns();

	if (spawns.empty())
	{
		return;
	}

	Screeps::StructureSpawn& target = spawns.begin()->second;


	// If there is no place to store the energy, go upgrade the controller
	int freeCapacity = target.store().getFreeCapacity(Screeps::RESOURCE_ENERGY).value();

	if (freeCapacity == 0)
	{
		memory["task"] = "upgrading";
		creep.setMemory(memory);
		upgrade();
		return;
	}

	if (creep.transfer(target, Screeps::RESOURCE_ENERGY) == Screeps::ERR_NOT_IN_RANGE)
	{
		creep.moveTo(target);
	}
}
}
