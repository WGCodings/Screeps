#include <Screeps/Context.hpp>
#include <Screeps/Creep.hpp>

#include <emscripten.h>
#include <emscripten/bind.h>

EMSCRIPTEN_KEEPALIVE
extern "C" void loop()
{
	Screeps::Context::update();

	JS::console.log(std::string("\n\n\n\n\n\n\n\n\n"));
	JS::console.log(std::string("Processing tick:\t") + std::to_string(Screeps::Game.time()));

	// Do things here


	JS::console.log("Used CPU:\t" + std::to_string(Screeps::Game.cpuGetUsed()));
}

EMSCRIPTEN_BINDINGS(loop)
{
	emscripten::function("loop", &loop);
}
