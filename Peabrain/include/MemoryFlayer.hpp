#ifndef PEABRAIN_MEMORY_FLAYER_HPP
#define PEABRAIN_MEMORY_FLAYER_HPP

namespace Peabrain {

/// The memory flayer destroys all the memories of the dead creeps.
class MemoryFlayer
{
public:

	static void run();

private:

	static constexpr int flayInterval = 1000;

	static void flayDeadCreeps();
};

}

#endif