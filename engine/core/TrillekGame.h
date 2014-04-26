#ifndef TRILLEKGAME_H_INCLUDED
#define TRILLEKGAME_H_INCLUDED

#include "engine/core/TrillekScheduler.h"

namespace trillek {
    class TrillekGame {
    public:

        TrillekGame() {};
        virtual ~TrillekGame() {};

        static TrillekScheduler& GetScheduler() { return scheduler; };

    private:

        static TrillekScheduler scheduler;
   };
}

#endif // TRILLEKGAME_H_INCLUDED