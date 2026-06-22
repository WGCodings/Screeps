#include "Creeps/Harvester.hpp"

#include <random>
#include <Screeps/Game.hpp>
#include <Screeps/Source.hpp>
#include <Screeps/StructureSpawn.hpp>

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

	memory["task"] = task;
	creep.setMemory(memory);

	if (task == "harvesting")
	{
		harvest();
	}
	else
	{
		deliver();
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
void Harvester::deliver()
{
	std::map<std::string, Screeps::StructureSpawn> spawns = Screeps::Game.spawns();

	if (spawns.empty())
	{
		return;
	}

	Screeps::StructureSpawn& target = spawns.begin()->second;

	if (creep.transfer(target, Screeps::RESOURCE_ENERGY) == Screeps::ERR_NOT_IN_RANGE)
	{
		creep.moveTo(target);
	}
}

/// This function finds a source and assign the id to the memory of the harvester
void Harvester::findSource(JSON& memory)
{
	auto sources = creep.room().find(Screeps::FIND_SOURCES_ACTIVE);
	if (sources.empty()) return;

	std::vector<int> indices(sources.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::shuffle(indices.begin(), indices.end(), std::default_random_engine(Screeps::Game.time()));

	for (int i : indices)
	{
		auto* source = dynamic_cast<Screeps::Source*>(sources[i].get());
		if (source && source->energy() > 0)
		{
			memory["sourceId"] = source->id();
			creep.setMemory(memory);
			return;
		}
	}
}

/// This function return the source object given an id
Screeps::Source* Harvester::getSourceById(const std::string& sourceId)
{
	auto object = Screeps::Game.getObjectById(sourceId);
	if (!object) return nullptr;
	return dynamic_cast<Screeps::Source*>(object.release());
}

}
