#ifndef TRILLEKGAME_H_INCLUDED
#define TRILLEKGAME_H_INCLUDED

#include "trillek-scheduler.hpp"
#include "systems/fake-system.hpp"
#include "os.hpp"
#include "systems/graphics.hpp"
#include "systems/sound-system.hpp"

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

    /** \brief Return the graphic system instance
     *
     * \return graphics::System& the instance
     *
     */
    static graphics::RenderSystem& GetGraphicSystem() { return gl_sys; };


    /** \brief Return the sound system instance
     *
     * \return sound::System&
     *
     */
    static sound::System& GetSoundSystem() { return *sound::System::GetInstance(); };

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
    static bool GetTerminateFlag() { return close_window; };

    /** \brief Tells that the user tries to close the window
     *
     * This function is called by a callback set in GLFW
     *
     * \return void
     *
     */
    static void NotifyCloseWindow() { close_window = true; };

private:

    static TrillekScheduler scheduler;
    static FakeSystem fake_system;
    static OS glfw_os;
    static graphics::RenderSystem gl_sys;
    static bool close_window;
};
}

#endif // TRILLEKGAME_H_INCLUDED
