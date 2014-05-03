#ifndef TRILLEKGAME_H_INCLUDED
#define TRILLEKGAME_H_INCLUDED

#include "TrillekScheduler.h"
#include "FakeSystem.h"
#include "OS.h"

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

        /** \brief Return the GLFW instance
         *
         * \return OS& the instance
         *
         */
        static OS& GetOS() { return glfw_os; };

        /** \brief Get the FakeSystem
         *
         * \return FakeSystem& the fake system
         *
         */
        static FakeSystem& GetFakeSystem() { return fake_system; };

        /** \brief Set the terminate flag
         *
         * Used to tell the world we will terminate soon
         *
         */
        static void SetTerminateFlag() { terminateFlag = true; };

        /** \brief Get the terminate flag
         *
         * The flag tells the world that the program will terminate
         *
         * \return bool true if we are about to terminate the program
         *
         */
        static bool GetTerminateFlag() { return terminateFlag; };

    private:

        static TrillekScheduler scheduler;
        static FakeSystem fake_system;
        static OS glfw_os;
        static bool terminateFlag;
   };
}

#endif // TRILLEKGAME_H_INCLUDED