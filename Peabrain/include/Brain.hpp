#ifndef PEABRAIN_BRAIN_HPP
#define PEABRAIN_BRAIN_HPP

namespace Peabrain {

    class Brain
    {
    public:
        static void run();

    private:
        static void runStructures();
        static void runCreeps();
    };

}

#endif