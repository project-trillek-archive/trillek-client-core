#ifndef TRILLEKGAME_H_INCLUDED
#define TRILLEKGAME_H_INCLUDED

#include "TrillekScheduler.h"
#include "FakeSystem.h"

namespace trillek {
    class TrillekGame {
    public:

        TrillekGame() {};
        virtual ~TrillekGame() {};

        /** \brief Return the scheduler
         *
         * \return TrillekScheduler& the scheduler
         *
         */
        static TrillekScheduler& GetScheduler() { return scheduler; };

        /** \brief Get the FakeSystem
         *
         * \return FakeSystem& the fake system
         *
         */
        static FakeSystem& GetFakeSystem() { return fake_system; };

    private:

        static TrillekScheduler scheduler;
        static FakeSystem fake_system;
   };
}

#endif // TRILLEKGAME_H_INCLUDED