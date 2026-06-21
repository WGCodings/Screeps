#include <Screeps/Context.hpp>
#include <Screeps/Creep.hpp>
#include <Screeps/StructureSpawn.hpp>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>


EMSCRIPTEN_KEEPALIVE
extern "C" void loop()
{
	Screeps::Context::update();

	JS::console.log(std::string("\n\n\n"));
	JS::console.log(std::string("Processing tick:\t") + std::to_string(Screeps::Game.time()));


	JS::console.log("Used CPU:\t" + std::to_string(Screeps::Game.cpuGetUsed()));
	JS::console.log("Bucket:\t" + std::to_string(static_cast<int>(Screeps::Game.cpu()["bucket"])));
}

EMSCRIPTEN_BINDINGS(loop)
{
	emscripten::function("loop", &loop);
}