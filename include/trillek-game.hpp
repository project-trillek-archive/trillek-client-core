#ifndef TRILLEKGAME_H_INCLUDED
#define TRILLEKGAME_H_INCLUDED

#include "trillek-scheduler.hpp"
#include "systems/fake-system.hpp"
#include "os.hpp"
#include "systems/physics.hpp"
#include "systems/graphics.hpp"
#include "systems/meta-engine-system.hpp"
#include "systems/sound-system.hpp"

namespace trillek {

namespace graphics {
class RenderSystem;
}

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
    static graphics::RenderSystem& GetGraphicSystem();

    /** \brief Return the graphic system instance pointer
     *
     * \return std::shared_ptr<graphics::RenderSystem> the instance
     *
     */
    static std::shared_ptr<graphics::RenderSystem> GetGraphicsInstance();

    /** \brief Return the physics system instance
     *
     * \return physics::PhysicsSystem& the instance
     */
    static physics::PhysicsSystem& GetPhysicsSystem() { return phys_sys; };

    /** \brief Return the meta engine system instance
    *
    * This sytem wraps together some function calls of graphic and physics systems
    *
    * \return MetaEngineSystem& the instance
    */
    static MetaEngineSystem& GetEngineSystem() { return engine_sys; };

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
    static std::once_flag once_graphics;
    static std::shared_ptr<graphics::RenderSystem> gl_sys_ptr;
    static physics::PhysicsSystem phys_sys;
    static MetaEngineSystem engine_sys;
    static bool close_window;
};
}

#endif // TRILLEKGAME_H_INCLUDED
