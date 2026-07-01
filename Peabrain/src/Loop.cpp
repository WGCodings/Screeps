#include <Screeps/Context.hpp>
#include <Screeps/Creep.hpp>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "Brain.hpp"


EMSCRIPTEN_KEEPALIVE
extern "C" void loop()
{
	Screeps::Context::update();


	JS::console.log(std::string("Processing tick:\t") + std::to_string(Screeps::Game.time()));

	{
		Peabrain::Brain::run();
	}

	JS::console.log("Used CPU:\t" + std::to_string(Screeps::Game.cpuGetUsed()));
	JS::console.log("Bucket:\t" + std::to_string(static_cast<int>(Screeps::Game.cpu()["bucket"])));
	JS::console.log(std::string("\n"));
}

EMSCRIPTEN_BINDINGS(loop)
{
	emscripten::function("loop", &loop);
}