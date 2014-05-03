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

        /** \brief Get the terminate flag
         *
         * The flag tells the world that the program will terminate
         *
         * \return bool true if we are about to terminate the program
         *
         */
        static bool GetTerminateFlag() { return GetOS().Closing(); };

        /** \brief Return the condition_variable associated with the window close operation
         *
         * \return std::condition_variable& the condition variable
         *
         */
        static std::condition_variable& GetCloseWindowCV() { return close_window; };

        /** \brief Notify the thread waiting the window to be closed
         *
         * This function is called by a callback set in GLFW
         *
         * \return void
         *
         */
        static void NotifyCloseWindow() { close_window.notify_all(); };

    private:

        static TrillekScheduler scheduler;
        static FakeSystem fake_system;
        static OS glfw_os;
        static std::condition_variable close_window;
   };
}

#endif // TRILLEKGAME_H_INCLUDED