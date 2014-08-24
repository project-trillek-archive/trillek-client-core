#ifndef TRILLEKGAME_HPP_INCLUDED
#define TRILLEKGAME_HPP_INCLUDED

#include <memory>
#include <mutex>

namespace trillek {

class TrillekScheduler;
class MetaEngineSystem;
class FakeSystem;
class OS;

namespace network {
class NetworkController;
}
namespace graphics {
class RenderSystem;
}
namespace physics {
class PhysicsSystem;
}
namespace sound {
class System;
}
namespace component {
class Shared;
class System;
class SystemValue;
}

class TrillekGame {
public:

    TrillekGame() {};
    virtual ~TrillekGame() {};
    static void Initialize();

    /** \brief Return the scheduler
     *
     * \return TrillekScheduler& the scheduler
     */
    static TrillekScheduler& GetScheduler() { return *scheduler.get(); }

    /** \brief Return the GLFW instance
     *
     * \return OS& the instance
     */
    static OS& GetOS() { return *glfw_os.get(); };

    /** \brief Return the physics system instance
     *
     * \return physics::PhysicsSystem& the instance
     */
    static physics::PhysicsSystem& GetPhysicsSystem() { return *phys_sys.get(); }

    /** \brief Get the FakeSystem
     *
     * \return FakeSystem& the fake system
     */
    static FakeSystem& GetFakeSystem() { return *fake_system.get(); }

    /** \brief Get the client network controller
     *
     * \return NetworkController& the network controller
     */
    static network::NetworkController& GetNetworkClient() { return *network_system_client.get(); }

    /** \brief Get the server network controller
     *
     * \return NetworkController& the network controller
     */
    static network::NetworkController& GetNetworkServer() { return *network_system.get(); }


    /** \brief Get the storage of shared components
     *
     * \return component::Shared& the storage
     *
     */
    static component::Shared& GetSharedComponent() { return *shared_component.get(); };

    /** \brief Get the storage of system components stored by pointers
     *
     * \return component::System& the storage
     *
     */
    static component::System& GetSystemComponent() { return *system_component.get(); };

    /** \brief Get the storage of system components stored by values
     *
     * \return component::SystemValue& the storage
     *
     */
    static component::SystemValue& GetSystemValueComponent() { return *system_value_component.get(); };

    /** \brief Get the terminate flag
     *
     * The flag tells the world that the program will terminate
     *
     * \return bool true if we are about to terminate the program
     */
    static bool GetTerminateFlag() { return close_window; };

    /** \brief Tells that the user tries to close the window
     *
     * This function is called by a callback set in GLFW
     */
    static void NotifyCloseWindow() { close_window = true; };

#if defined(_CLIENT_) || defined(_STANDALONE_)
    /** \brief Return the sound system instance
     *
     * \return sound::System&
     */
    static sound::System& GetSoundSystem();

    /** \brief Return the graphic system instance
     *
     * \return graphics::System& the instance
     */
    static graphics::RenderSystem& GetGraphicSystem();

    /** \brief Return the graphic system instance pointer
     *
     * \return std::shared_ptr<graphics::RenderSystem> the instance
     */
    static std::shared_ptr<graphics::RenderSystem> GetGraphicsInstance();

    /** \brief Return the meta engine system instance
     *
     * This sytem wraps together some function calls of graphic and physics systems
     *
     * \return MetaEngineSystem& the instance
     */
    static MetaEngineSystem& GetEngineSystem() { return *engine_sys.get(); };
#endif // defined(_CLIENT_) || defined(_STANDALONE_)

private:

    static std::unique_ptr<TrillekScheduler> scheduler;
    static std::unique_ptr<network::NetworkController> network_system;
    static std::unique_ptr<network::NetworkController> network_system_client;
    static std::unique_ptr<FakeSystem> fake_system;
    static std::unique_ptr<physics::PhysicsSystem> phys_sys;
    static std::unique_ptr<OS> glfw_os;
    static std::unique_ptr<component::Shared> shared_component;
    static std::unique_ptr<component::System> system_component;
    static std::unique_ptr<component::SystemValue> system_value_component;
    static bool close_window;

#if defined(_CLIENT_) || defined(_STANDALONE_)
    static std::once_flag once_graphics;
    static std::shared_ptr<graphics::RenderSystem> gl_sys_ptr;
    static std::unique_ptr<MetaEngineSystem> engine_sys;
#endif
};
}

#endif // TRILLEKGAME_HPP_INCLUDED
