#include "Creeps/Harvester.hpp"


#include <Screeps/Game.hpp>
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
		if (!deliver()){
			task = "upgrading";
			creep.say("Upgrade.");
			upgrade();
		}
	}
	else if (task == "upgrading"){
		upgrade();
	}
	memory = creep.memory();
	memory["task"] = task;
	creep.setMemory(memory);
}
}
